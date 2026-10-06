#include "backup.h"
#include "platform.h"
#include "sha256.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRY(call) do { SBStatus r_=(call); if (r_.code!=SB_OK) return r_; } while (0)
typedef struct { char *path; unsigned kind; uint32_t size; unsigned char digest[32]; } Entry;
typedef struct { Entry *items; size_t count,capacity; uint64_t bytes; char id[65],name[SB_NAME_CAP]; } Manifest;
typedef struct { SBBackupCallback callback; void *userdata; SBBackupPhase phase; size_t entries,total; uint64_t bytes,bytes_total; } Progress;
typedef struct { SBFile *file; SBSha256 hash; } Stream;
static const unsigned char magic[8]={'S','B','B','R','A','I','N',1};
static SBStatus update(Progress *p,const char *path) {
    SBBackupProgress state={p->phase,p->entries,p->total,p->bytes,p->bytes_total,path};
    return !p->callback || p->callback(&state,p->userdata) ? sb_ok() : sb_error(SB_CANCELLED,"Abgebrochen.");
}
static unsigned char lower(unsigned char c) { return c>='A' && c<='Z' ? (unsigned char)(c+'a'-'A') : c; }
static int folded(const char *a,const char *b) {
    while (*a && lower((unsigned char)*a)==lower((unsigned char)*b)) { ++a; ++b; }
    return (int)lower((unsigned char)*a)-(int)lower((unsigned char)*b);
}
static bool component(const char *s,size_t n) {
    if (!n || n>255 || (n==1 && *s=='.') || (n==2 && s[0]=='.' && s[1]=='.') || s[n-1]=='.' || s[n-1]==' ') return false;
    char stem[5]={0}; size_t length=0;
    for (size_t i=0;i<n;++i) {
        unsigned char c=(unsigned char)s[i];
        if (c<32 || c==127 || strchr("\\:<>\"|?*",(int)c)) return false;
    }
    while (length<n && s[length]!='.') ++length;
    if (length<=4) {
        for (size_t i=0;i<length;++i) stem[i]=(char)lower((unsigned char)s[i]);
        if (!strcmp(stem,"con") || !strcmp(stem,"prn") || !strcmp(stem,"aux") || !strcmp(stem,"nul")) return false;
        if (length==4 && ((!strncmp(stem,"com",3)) || !strncmp(stem,"lpt",3)) && stem[3]>='1' && stem[3]<='9') return false;
    }
    return true;
}
static bool safe_path(const char *path) {
    size_t length=strlen(path); unsigned depth=0;
    if (!length || length>=SB_PATH_CAP || !sb_utf8_valid(path,length)) return false;
    const char *begin=path;
    for (const char *p=path;;++p) if (*p=='/' || !*p) {
        if (++depth>32 || !component(begin,(size_t)(p-begin))) return false;
        if (!*p) return true;
        begin=p+1;
    }
}
static void dispose(Manifest *m) {
    for (size_t i=0;i<m->count;++i) free(m->items[i].path);
    free(m->items); memset(m,0,sizeof(*m));
}
static SBStatus entry_add(Manifest *m,const char *path,unsigned kind,Entry **out) {
    if (!safe_path(path)) return sb_error(SB_INVALID,"Dieser Dateipfad ist nicht plattformübergreifend verwendbar: %s",path);
    if (m->count>=SB_BACKUP_ENTRIES) return sb_error(SB_LIMIT,"Sicherung enthält mehr als 4096 Dateien und Ordner.");
    if (m->count==m->capacity) {
        size_t capacity=m->capacity ? m->capacity*2 : 32;
        Entry *items=realloc(m->items,capacity*sizeof(*items));
        if (!items) return sb_error(SB_MEMORY,"Nicht genug Arbeitsspeicher.");
        m->items=items; m->capacity=capacity;
    }
    Entry *e=&m->items[m->count]; memset(e,0,sizeof(*e)); e->path=malloc(strlen(path)+1);
    if (!e->path) return sb_error(SB_MEMORY,"Nicht genug Arbeitsspeicher.");
    strcpy(e->path,path); e->kind=kind; ++m->count; *out=e; return sb_ok();
}
static int order(const void *a,const void *b) { return strcmp(((const Entry *)a)->path,((const Entry *)b)->path); }
static int fold_order(const void *a,const void *b) { return folded((*(const Entry *const *)a)->path,(*(const Entry *const *)b)->path); }
static SBStatus validate(Manifest *m) {
    if (!m->count) return sb_error(SB_INVALID,"Sicherung enthält keine Projektdateien.");
    Entry **indices=malloc(m->count*sizeof(*indices));
    if (m->count && !indices) return sb_error(SB_MEMORY,"Nicht genug Arbeitsspeicher.");
    for (size_t i=0;i<m->count;++i) indices[i]=&m->items[i];
    qsort(indices,m->count,sizeof(*indices),fold_order);
    for (size_t i=1;i<m->count;++i) if (!folded(indices[i-1]->path,indices[i]->path)) {
        free(indices); return sb_error(SB_INVALID,"Sicherung enthält doppelte oder nur durch Großschreibung verschiedene Pfade.");
    }
    free(indices);
    for (size_t i=0;i<m->count;++i) {
        Entry *e=&m->items[i];
        if (i && strcmp(m->items[i-1].path,e->path)>=0) return sb_error(SB_INVALID,"Sicherungspfade sind nicht eindeutig geordnet.");
        const char *slash=strrchr(e->path,'/');
        if (slash) {
            char parent[SB_PATH_CAP]; size_t n=(size_t)(slash-e->path); memcpy(parent,e->path,n); parent[n]=0;
            Entry key={0}; key.path=parent;
            Entry *found=bsearch(&key,m->items,i,sizeof(*m->items),order);
            if (!found || found->kind!=2) return sb_error(SB_INVALID,"Übergeordneter Sicherungsordner fehlt: %s",e->path);
        }
    }
    return sb_ok();
}
static SBStatus read_exact(Stream *s,void *data,size_t length,bool hash) {
    unsigned char *p=data; size_t total=0;
    while (total<length) {
        size_t got=0; TRY(sb_file_read(s->file,p+total,length-total,&got));
        if (!got) return sb_error(SB_INVALID,"Sicherung ist unvollständig.");
        total+=got;
    }
    if (hash) sb_sha256_update(&s->hash,data,length);
    return sb_ok();
}
static SBStatus write_data(Stream *s,const void *data,size_t length) {
    TRY(sb_file_write(s->file,data,length)); sb_sha256_update(&s->hash,data,length); return sb_ok();
}
static void little(unsigned char *p,uint64_t value,unsigned n) { for (unsigned i=0;i<n;++i) p[i]=(unsigned char)(value>>(8*i)); }
static uint64_t integer(const unsigned char *p,unsigned n) { uint64_t value=0; for (unsigned i=0;i<n;++i) value|=(uint64_t)p[i]<<(8*i); return value; }
static SBStatus hash_file(const char *path,Entry *e,Progress *p,Stream *archive) {
    SBFile *file=NULL; TRY(sb_file_open(path,false,&file));
    SBSha256 hash; sb_sha256_init(&hash); unsigned char buffer[65536],digest[32]; uint64_t total=0;
    SBStatus status=sb_ok();
    for (;;) {
        size_t got=0; status=sb_file_read(file,buffer,sizeof(buffer),&got);
        if (status.code!=SB_OK || !got) break;
        total+=got;
        if (total>SB_TEXT_LIMIT || (archive && total>e->size)) { status=sb_error(SB_LIMIT,"Datei ist zu groß oder wurde während der Sicherung geändert: %s",e->path); break; }
        sb_sha256_update(&hash,buffer,got);
        if (archive) status=write_data(archive,buffer,got);
        p->bytes+=got;
        if (status.code==SB_OK) status=update(p,e->path);
        if (status.code!=SB_OK) break;
    }
    SBStatus closed=sb_file_close(file,false); if (status.code==SB_OK) status=closed;
    if (status.code!=SB_OK) return status;
    sb_sha256_finish(&hash,digest);
    if (!archive) { e->size=(uint32_t)total; memcpy(e->digest,digest,32); }
    else if (total!=e->size || memcmp(e->digest,digest,32)) return sb_error(SB_CONFLICT,"Datei wurde während der Sicherung geändert: %s",e->path);
    return sb_ok();
}
typedef struct { Manifest *manifest; const char *root; char relative[SB_PATH_CAP]; Progress *progress; } Walk;
static SBStatus collect(const char *name,int kind,void *userdata) {
    Walk *w=userdata; char relative[SB_PATH_CAP],full[SB_PATH_CAP]; Entry *entry;
    if (kind!=1 && kind!=2) return sb_error(SB_INVALID,"Verknüpfung oder unlesbare Datei wird nicht gesichert: %s",name);
    TRY(sb_path_join(relative,sizeof(relative),w->relative,name));
    TRY(sb_path_join(full,sizeof(full),w->root,relative));
    TRY(entry_add(w->manifest,relative,(unsigned)kind,&entry));
    TRY(update(w->progress,relative));
    if (kind==2) {
        Walk child=*w; strcpy(child.relative,relative); TRY(sb_fs_list(full,collect,&child));
    } else {
        TRY(hash_file(full,entry,w->progress,NULL));
        w->manifest->bytes+=entry->size;
        if (w->manifest->bytes>SB_BACKUP_BYTES) return sb_error(SB_LIMIT,"Sicherung überschreitet 256 MiB Dateiinhalte.");
    }
    ++w->progress->entries; return sb_ok();
}
static SBStatus scan(const char *root,Manifest *m,Progress *p) {
    if (sb_fs_kind(root)!=2) return sb_error(SB_INVALID,"Projekt ist kein regulärer Ordner.");
    Walk w={0}; w.manifest=m; w.root=root; w.progress=p;
    TRY(sb_fs_list(root,collect,&w)); if (m->count>1) qsort(m->items,m->count,sizeof(*m->items),order); return validate(m);
}
static SBStatus remove_child(const char *name,int kind,void *userdata);
static SBStatus remove_tree(const char *path) {
    if (sb_fs_kind(path)!=2) return sb_error(SB_IO,"Temporärer Ordner ist nicht mehr verfügbar.");
    TRY(sb_fs_list(path,remove_child,(void *)path)); return sb_fs_rmdir(path);
}
static SBStatus remove_child(const char *name,int kind,void *userdata) {
    char full[SB_PATH_CAP]; TRY(sb_path_join(full,sizeof(full),userdata,name));
    return kind==2 ? remove_tree(full) : sb_fs_remove(full);
}
static SBStatus temporary(const char *parent,char out[SB_PATH_CAP],bool directory,SBFile **file) {
    for (unsigned i=0;i<256;++i) {
        char name[80]; snprintf(name,sizeof(name),".sb-%s-%lu-%u",directory ? "restore" : "backup",sb_process_id(),i);
        TRY(sb_path_join(out,SB_PATH_CAP,parent,name));
        SBStatus status=directory ? sb_fs_mkdir(out) : sb_file_open(out,true,file);
        if (status.code!=SB_EXISTS) return status;
    }
    return sb_error(SB_IO,"Kein freier temporärer Pfad verfügbar.");
}
static SBStatus cleanup(SBStatus status,const char *path,bool directory) {
    SBStatus removed=directory ? remove_tree(path) : sb_fs_remove(path);
    if (removed.code!=SB_OK && sb_fs_kind(path)!=0)
        return sb_error(SB_IO,"%s Temporäre Daten konnten nicht entfernt werden: %s",status.message,path);
    return status;
}
static SBStatus metadata_file(const char *root,char name[SB_NAME_CAP]) {
    char path[SB_PATH_CAP],*json=NULL; size_t length=0;
    TRY(sb_path_join(path,sizeof(path),root,"brain.json"));
    if (sb_fs_kind(path)!=1) return sb_error(SB_INVALID,"Projektmetadaten fehlen.");
    TRY(sb_fs_read(path,&json,&length)); SBStatus status=sb_metadata_validate(json,length,name); free(json); return status;
}
static SBStatus create_file(const char *root,const Manifest *m,Stream *s,Progress *p) {
    unsigned char header[21]; memcpy(header,magic,8); little(header+8,m->count,4); little(header+12,m->bytes,8); header[20]=(unsigned char)strlen(m->id);
    TRY(write_data(s,header,sizeof(header))); TRY(write_data(s,m->id,strlen(m->id)));
    for (size_t i=0;i<m->count;++i) {
        Entry *e=&m->items[i]; unsigned char record[39]; size_t length=strlen(e->path);
        record[0]=(unsigned char)e->kind; little(record+1,length,2); little(record+3,e->size,4); memcpy(record+7,e->digest,32);
        TRY(write_data(s,record,sizeof(record))); TRY(write_data(s,e->path,length));
        if (e->kind==1) { char full[SB_PATH_CAP]; TRY(sb_path_join(full,sizeof(full),root,e->path)); TRY(hash_file(full,e,p,s)); }
        ++p->entries; TRY(update(p,e->path));
    }
    unsigned char digest[32]; sb_sha256_finish(&s->hash,digest); return sb_file_write(s->file,digest,32);
}
SBStatus sb_backup_create(const SBProject *project,const char *archive,SBBackupCallback callback,void *userdata) {
    char root[SB_PATH_CAP],destination[SB_PATH_CAP],parent[SB_PATH_CAP],temp[SB_PATH_CAP];
    Manifest m={0},again={0}; Progress p={0}; p.callback=callback; p.userdata=userdata; p.phase=SB_BACKUP_SCAN;
    if (!sb_id_valid(project->id) || sb_fs_kind(project->root)!=2) return sb_error(SB_INVALID,"Ungültiges Projekt.");
    TRY(sb_fs_absolute(project->root,root,sizeof(root))); TRY(sb_fs_absolute(archive,destination,sizeof(destination)));
    size_t n=strlen(root); bool inside=true;
    for (size_t i=0;i<n;++i) if (!destination[i] || lower((unsigned char)root[i])!=lower((unsigned char)destination[i])) { inside=false; break; }
    if (inside && (destination[n]=='/' || !destination[n])) return sb_error(SB_INVALID,"Speichere die Sicherung außerhalb dieses Projektordners.");
    if (sb_fs_kind(destination)!=0) return sb_error(SB_EXISTS,"Sicherungsdatei existiert bereits.");
    strcpy(parent,destination); char *slash=strrchr(parent,'/'); if (!slash) return sb_error(SB_INVALID,"Ungültiges Sicherungsziel."); *slash=0;
    if (sb_fs_kind(parent)!=2) return sb_error(SB_NOT_FOUND,"Sicherungsordner ist nicht erreichbar.");
    SBStatus status=metadata_file(root,m.name); strcpy(m.id,project->id);
    if (status.code==SB_OK) status=scan(root,&m,&p);
    if (status.code!=SB_OK) { dispose(&m); return status; }
    Stream stream={0}; sb_sha256_init(&stream.hash); status=temporary(parent,temp,false,&stream.file);
    if (status.code!=SB_OK) { dispose(&m); return status; }
    p.phase=SB_BACKUP_WRITE; p.entries=p.bytes=0; p.total=m.count; p.bytes_total=m.bytes;
    status=create_file(root,&m,&stream,&p);
    SBStatus closed=sb_file_close(stream.file,status.code==SB_OK); if (status.code==SB_OK) status=closed;
    if (status.code==SB_OK) {
        p.phase=SB_BACKUP_RECHECK; p.entries=p.bytes=0;
        status=scan(root,&again,&p);
        if (status.code==SB_OK && (m.count!=again.count || m.bytes!=again.bytes)) status=sb_error(SB_CONFLICT,"Projekt wurde während der Sicherung geändert.");
        for (size_t i=0;status.code==SB_OK && i<m.count;++i)
            if (strcmp(m.items[i].path,again.items[i].path) || m.items[i].kind!=again.items[i].kind || m.items[i].size!=again.items[i].size || memcmp(m.items[i].digest,again.items[i].digest,32)) status=sb_error(SB_CONFLICT,"Projekt wurde während der Sicherung geändert.");
    }
    if (status.code==SB_OK) { SBBackupInfo info; status=sb_backup_inspect(temp,&info,callback,userdata); }
    if (status.code==SB_OK) { p.phase=SB_BACKUP_PUBLISH; status=update(&p,destination); }
    if (status.code==SB_OK) status=sb_fs_publish_new(temp,destination);
    dispose(&m); dispose(&again);
    return status.code==SB_OK ? status : cleanup(status,temp,false);
}
static SBStatus archive_read(const char *archive,Manifest *m,unsigned char archive_digest[32],Progress *p,const char *stage) {
    Stream s={0}; sb_sha256_init(&s.hash); TRY(sb_file_open(archive,false,&s.file));
    SBStatus status=sb_ok(); unsigned char header[21];
#define READ(data,length,hash) do { status=read_exact(&s,data,length,hash); if (status.code!=SB_OK) goto done; } while (0)
    READ(header,sizeof(header),true);
    size_t count=(size_t)integer(header+8,4); uint64_t expected=integer(header+12,8); size_t id_length=header[20];
    if (memcmp(header,magic,8) || !count || count>SB_BACKUP_ENTRIES || expected>SB_BACKUP_BYTES || !id_length || id_length>64) { status=sb_error(SB_INVALID,"Ungültige Sicherung oder unbekannte Version."); goto done; }
    READ(m->id,id_length,true); m->id[id_length]=0;
    if (strlen(m->id)!=id_length || !sb_id_valid(m->id)) { status=sb_error(SB_INVALID,"Ungültige Projektkennung in Sicherung."); goto done; }
    p->entries=p->bytes=0; p->total=count; p->bytes_total=expected;
    bool metadata=false;
    for (size_t i=0;i<count;++i) {
        unsigned char record[39],buffer[65536]; char path[SB_PATH_CAP],full[SB_PATH_CAP]; Entry *e;
        READ(record,sizeof(record),true); size_t path_length=(size_t)integer(record+1,2); uint64_t size=integer(record+3,4);
        if (!path_length || path_length>=SB_PATH_CAP || size>SB_TEXT_LIMIT || m->bytes+size>expected || (record[0]!=1 && record[0]!=2) || (record[0]==2 && size)) { status=sb_error(SB_INVALID,"Ungültiger Sicherungseintrag."); goto done; }
        READ(path,path_length,true); path[path_length]=0;
        if (strlen(path)!=path_length) { status=sb_error(SB_INVALID,"Ungültiger Sicherungspfad."); goto done; }
        status=entry_add(m,path,record[0],&e); if (status.code!=SB_OK) goto done;
        e->size=(uint32_t)size; memcpy(e->digest,record+7,32); m->bytes+=size;
        SBFile *output=NULL; char *json=NULL;
        if (!strcmp(path,"brain.json") && e->kind==1) { json=malloc((size_t)size+1); if (!json) { status=sb_error(SB_MEMORY,"Nicht genug Arbeitsspeicher."); goto done; } }
        if (stage) {
            status=sb_path_join(full,sizeof(full),stage,path);
            if (status.code==SB_OK) status=e->kind==2 ? sb_fs_mkdir(full) : sb_file_open(full,true,&output);
            if (status.code!=SB_OK) { free(json); goto done; }
        }
        SBSha256 hash; sb_sha256_init(&hash); uint64_t at=0;
        while (at<size && status.code==SB_OK) {
            size_t amount=size-at<sizeof(buffer) ? (size_t)(size-at) : sizeof(buffer);
            status=read_exact(&s,buffer,amount,true); if (status.code!=SB_OK) break;
            sb_sha256_update(&hash,buffer,amount); if (json) memcpy(json+(size_t)at,buffer,amount);
            if (output) status=sb_file_write(output,buffer,amount);
            at+=amount; p->bytes+=amount;
            if (status.code==SB_OK) status=update(p,path);
        }
        SBStatus closed=sb_file_close(output,status.code==SB_OK); if (status.code==SB_OK) status=closed;
        unsigned char digest[32]; sb_sha256_finish(&hash,digest);
        if (status.code==SB_OK && e->kind==1 && memcmp(digest,e->digest,32)) status=sb_error(SB_INVALID,"Dateiprüfsumme stimmt nicht: %s",path);
        if (status.code==SB_OK && e->kind==2) { unsigned char zero[32]={0}; if (memcmp(e->digest,zero,32)) status=sb_error(SB_INVALID,"Ungültige Ordnerprüfsumme."); }
        if (json && status.code==SB_OK) { json[size]=0; status=sb_metadata_validate(json,(size_t)size,m->name); metadata=status.code==SB_OK; }
        free(json); if (status.code!=SB_OK) goto done;
        ++p->entries; status=update(p,path); if (status.code!=SB_OK) goto done;
    }
    if (!metadata || m->bytes!=expected) { status=sb_error(SB_INVALID,"Projektmetadaten fehlen oder Sicherungsgröße stimmt nicht."); goto done; }
    status=validate(m); if (status.code!=SB_OK) goto done;
    unsigned char stored[32]; READ(stored,32,false); sb_sha256_finish(&s.hash,archive_digest);
    if (memcmp(stored,archive_digest,32)) { status=sb_error(SB_INVALID,"Gesamtprüfsumme der Sicherung stimmt nicht."); goto done; }
    size_t extra=0; status=sb_file_read(s.file,stored,1,&extra);
    if (status.code==SB_OK && extra) status=sb_error(SB_INVALID,"Sicherung enthält zusätzliche Daten.");
done:
    { SBStatus closed=sb_file_close(s.file,false); if (status.code==SB_OK) status=closed; }
#undef READ
    return status;
}
static void information(const Manifest *m,SBBackupInfo *info) {
    memset(info,0,sizeof(*info)); strcpy(info->id,m->id); strcpy(info->name,m->name); info->bytes=m->bytes;
    for (size_t i=0;i<m->count;++i) if (m->items[i].kind==1) ++info->files; else ++info->directories;
}
SBStatus sb_backup_inspect(const char *archive,SBBackupInfo *info,SBBackupCallback callback,void *userdata) {
    Manifest m={0}; unsigned char digest[32]; Progress p={0}; p.callback=callback; p.userdata=userdata; p.phase=SB_BACKUP_VERIFY;
    SBStatus status=archive_read(archive,&m,digest,&p,NULL);
    if (status.code==SB_OK) information(&m,info);
    dispose(&m); return status;
}
SBStatus sb_backup_restore(const char *archive,const char *workspace,const char *id,SBProject *out,SBBackupCallback callback,void *userdata) {
    if (!sb_id_valid(id)) return sb_error(SB_INVALID,"Wähle einen gültigen, freien Projektordnernamen.");
    char parent[SB_PATH_CAP],destination[SB_PATH_CAP],stage[SB_PATH_CAP];
    TRY(sb_fs_absolute(workspace,parent,sizeof(parent)));
    if (sb_fs_kind(parent)!=2) return sb_error(SB_NOT_FOUND,"Arbeitsordner ist nicht erreichbar.");
    TRY(sb_path_join(destination,sizeof(destination),parent,id));
    if (sb_fs_kind(destination)!=0) return sb_error(SB_EXISTS,"Projektordner existiert bereits. Wähle einen anderen Namen.");
    Manifest first={0},second={0}; unsigned char original[32],current[32]; Progress p={0};
    p.callback=callback; p.userdata=userdata; p.phase=SB_BACKUP_VERIFY;
    SBStatus status=archive_read(archive,&first,original,&p,NULL);
    if (status.code!=SB_OK) { dispose(&first); return status; }
    status=temporary(parent,stage,true,NULL);
    if (status.code!=SB_OK) { dispose(&first); return status; }
    p.phase=SB_BACKUP_RESTORE;
    status=archive_read(archive,&second,current,&p,stage);
    if (status.code==SB_OK && memcmp(original,current,32)) status=sb_error(SB_CONFLICT,"Sicherung wurde während der Wiederherstellung geändert.");
    if (status.code==SB_OK) {
        char path[SB_PATH_CAP],temp[SB_PATH_CAP],*json=NULL,*changed=NULL; size_t length=0,changed_length=0;
        status=sb_path_join(path,sizeof(path),stage,"brain.json");
        if (status.code==SB_OK) status=sb_fs_read(path,&json,&length);
        if (status.code==SB_OK) status=sb_metadata_reidentify(json,length,id,&changed,&changed_length);
        SBFile *identity=NULL;
        if (status.code==SB_OK) status=temporary(stage,temp,false,&identity);
        if (status.code==SB_OK) status=sb_file_write(identity,changed,changed_length);
        SBStatus closed=sb_file_close(identity,status.code==SB_OK); if (status.code==SB_OK) status=closed;
        if (status.code==SB_OK) status=sb_fs_replace(temp,path);
        free(json); free(changed);
    }
    if (status.code==SB_OK) { p.phase=SB_BACKUP_PUBLISH; status=update(&p,id); }
    if (status.code==SB_OK) status=sb_fs_publish_new(stage,destination);
    if (status.code==SB_OK && out) { memset(out,0,sizeof(*out)); strcpy(out->id,id); strcpy(out->name,first.name); strcpy(out->root,destination); }
    dispose(&first); dispose(&second);
    return status.code==SB_OK ? status : cleanup(status,stage,true);
}
