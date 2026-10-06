#include "backup.h"
#include "platform.h"
#include "sha256.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifndef _WIN32
#include <unistd.h>
#endif
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"BACKUP FAIL %d: %s\n",__LINE__,#x); exit(1); } } while (0)
#define OK(x) CHECK((x).code==SB_OK)
static void sha_vector(const char *text,const char *expected,unsigned repeat) {
    SBSha256 hash; unsigned char digest[32]; char hex[65]; sb_sha256_init(&hash);
    for (unsigned i=0;i<repeat;++i) sb_sha256_update(&hash,text,strlen(text));
    sb_sha256_finish(&hash,digest);
    for (unsigned i=0;i<32;++i) snprintf(hex+i*2,3,"%02x",digest[i]);
    CHECK(!strcmp(hex,expected));
}
typedef struct { const char *a,*b; } Pair;
static SBStatus compare_visit(const char *name,int kind,void *userdata) {
    Pair *pair=userdata; char a[SB_PATH_CAP],b[SB_PATH_CAP];
    OK(sb_path_join(a,sizeof(a),pair->a,name)); OK(sb_path_join(b,sizeof(b),pair->b,name)); CHECK(sb_fs_kind(b)==kind);
    if (kind==2) { Pair next={a,b}; OK(sb_fs_list(a,compare_visit,&next)); }
    else {
        char *first,*second; size_t x,y; OK(sb_fs_read(a,&first,&x)); OK(sb_fs_read(b,&second,&y));
        CHECK(x==y && !memcmp(first,second,x)); free(first); free(second);
    }
    return sb_ok();
}
static SBStatus no_temporary(const char *name,int kind,void *userdata) {
    (void)kind; (void)userdata; CHECK(strncmp(name,".sb-backup-",11) && strncmp(name,".sb-restore-",12)); return sb_ok();
}
typedef struct { SBBackupPhase phase; bool fired,mutate; const char *path; } Control;
static bool interrupt(const SBBackupProgress *p,void *userdata) {
    Control *c=userdata;
    if (p->phase!=c->phase || c->fired) return true;
    if ((p->phase==SB_BACKUP_WRITE || p->phase==SB_BACKUP_RESTORE) && !p->bytes) return true;
    c->fired=true;
    if (c->mutate) {
        char *text; size_t length; char temp[SB_PATH_CAP];
        OK(sb_fs_read(c->path,&text,&length)); CHECK(length>1); text[0]=text[0]=='#' ? '!' : '#';
        snprintf(temp,sizeof(temp),"%s-change",c->path); OK(sb_fs_write_new(temp,text,length)); OK(sb_fs_replace(temp,c->path)); free(text);
        return true;
    }
    return false;
}
typedef struct { const char *path,*data; unsigned kind; } Fixture;
static void little(unsigned char *p,uint64_t value,unsigned n) { for (unsigned i=0;i<n;++i) p[i]=(unsigned char)(value>>(i*8)); }
static void forged(const char *path,const Fixture *entries,size_t count,bool bad_file,bool bad_archive) {
    unsigned char bytes[32768]; size_t at=0; uint64_t total=0;
    for (size_t i=0;i<count;++i) if (entries[i].data) total+=strlen(entries[i].data);
    memcpy(bytes,"SBBRAIN\1",8); little(bytes+8,count,4); little(bytes+12,total,8); bytes[20]=7; memcpy(bytes+21,"projekt",7); at=28;
    for (size_t i=0;i<count;++i) {
        size_t n=strlen(entries[i].path),length=entries[i].data ? strlen(entries[i].data) : 0;
        bytes[at]=(unsigned char)entries[i].kind; little(bytes+at+1,n,2); little(bytes+at+3,length,4); memset(bytes+at+7,0,32);
        if (entries[i].kind==1) { SBSha256 hash; sb_sha256_init(&hash); sb_sha256_update(&hash,entries[i].data,length); sb_sha256_finish(&hash,bytes+at+7); }
        if (bad_file && i==0) bytes[at+7]^=1;
        at+=39; memcpy(bytes+at,entries[i].path,n); at+=n;
        if (length) { memcpy(bytes+at,entries[i].data,length); at+=length; }
    }
    SBSha256 hash; sb_sha256_init(&hash); sb_sha256_update(&hash,bytes,at); sb_sha256_finish(&hash,bytes+at); if (bad_archive) bytes[at]^=1;
    OK(sb_fs_write_new(path,(const char *)bytes,at+32));
}
int main(int argc,char **argv) {
    CHECK(argc==2); char root[SB_PATH_CAP],workspace[SB_PATH_CAP],restore[SB_PATH_CAP],archive[SB_PATH_CAP],path[SB_PATH_CAP],other[SB_PATH_CAP],suffix[90];
    snprintf(suffix,sizeof(suffix),"run-%lu-%lu",sb_process_id(),(unsigned long)time(NULL));
    OK(sb_path_join(root,sizeof(root),argv[1],suffix)); OK(sb_fs_mkdirs(root));
    OK(sb_path_join(workspace,sizeof(workspace),root,"Original ü")); OK(sb_path_join(restore,sizeof(restore),root,"Wiederhergestellt ü")); OK(sb_fs_mkdir(restore));
    OK(sb_path_join(archive,sizeof(archive),root,"Sicherung ü.sbbackup"));
    sha_vector("","e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",1);
    sha_vector("abc","ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",1);
    sha_vector("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq","248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1",1);
    char thousand[1001]; memset(thousand,'a',1000); thousand[1000]=0;
    sha_vector(thousand,"cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0",1000);
    SBProject project,copy; OK(sb_project_create(workspace,"projekt","Projekt ü",NULL,&project));
    OK(sb_path_join(path,sizeof(path),project.root,"attachments")); OK(sb_fs_mkdir(path));
    OK(sb_path_join(path,sizeof(path),project.root,"attachments/leer")); OK(sb_fs_mkdir(path));
    OK(sb_path_join(path,sizeof(path),project.root,"attachments/Bild ü.bin"));
    unsigned char binary[200000]; for (size_t i=0;i<sizeof(binary);++i) binary[i]=(unsigned char)(i*19);
    OK(sb_fs_write_new(path,(const char *)binary,sizeof(binary)));
    OK(sb_backup_create(&project,archive,NULL,NULL)); SBBackupInfo info;
    OK(sb_backup_inspect(archive,&info,NULL,NULL)); CHECK(!strcmp(info.id,"projekt") && !strcmp(info.name,"Projekt ü") && info.files>10 && info.directories>=5 && info.bytes>=sizeof(binary));
    OK(sb_backup_restore(archive,restore,"projekt",&copy,NULL,NULL));
    Pair pair={project.root,copy.root}; OK(sb_fs_list(project.root,compare_visit,&pair));
    pair.a=copy.root; pair.b=project.root; OK(sb_fs_list(copy.root,compare_visit,&pair));
    CHECK(sb_backup_restore(archive,restore,"projekt",&copy,NULL,NULL).code==SB_EXISTS);
    OK(sb_path_join(path,sizeof(path),restore,"leer")); OK(sb_fs_mkdir(path)); CHECK(sb_backup_restore(archive,restore,"leer",NULL,NULL,NULL).code==SB_EXISTS && sb_fs_kind(path)==2);
    CHECK(sb_backup_create(&project,archive,NULL,NULL).code==SB_EXISTS);
    OK(sb_path_join(path,sizeof(path),project.root,"in-projekt.sbbackup")); CHECK(sb_backup_create(&project,path,NULL,NULL).code==SB_INVALID && !sb_fs_kind(path));
    OK(sb_backup_restore(archive,restore,"kopie",&copy,NULL,NULL)); OK(sb_path_join(path,sizeof(path),copy.root,"brain.json"));
    char *json; size_t length; OK(sb_fs_read(path,&json,&length)); CHECK(strstr(json,"\"id\": \"kopie\"")); free(json);
    OK(sb_path_join(path,sizeof(path),copy.root,"attachments/Bild ü.bin")); char *data; OK(sb_fs_read(path,&data,&length)); CHECK(length==sizeof(binary) && !memcmp(data,binary,length)); free(data);
    /* Abort before, during and immediately before publication; no partial result. */
    SBBackupPhase create_phases[]={SB_BACKUP_SCAN,SB_BACKUP_WRITE,SB_BACKUP_VERIFY,SB_BACKUP_PUBLISH,SB_BACKUP_RECHECK};
    for (unsigned i=0;i<5;++i) {
        OK(sb_path_join(other,sizeof(other),root,"abgebrochen.sbbackup")); Control c={create_phases[i],false,false,NULL};
        CHECK(sb_backup_create(&project,other,interrupt,&c).code==SB_CANCELLED && c.fired && sb_fs_kind(other)==0);
        OK(sb_fs_list(root,no_temporary,NULL));
    }
    SBBackupPhase restore_phases[]={SB_BACKUP_VERIFY,SB_BACKUP_RESTORE,SB_BACKUP_PUBLISH};
    for (unsigned i=0;i<3;++i) {
        Control c={restore_phases[i],false,false,NULL}; CHECK(sb_backup_restore(archive,restore,"abgebrochen",NULL,interrupt,&c).code==SB_CANCELLED && c.fired);
        OK(sb_path_join(path,sizeof(path),restore,"abgebrochen")); CHECK(!sb_fs_kind(path)); OK(sb_fs_list(restore,no_temporary,NULL));
    }
    static const char *meta="{\"schema_version\":1,\"id\":\"projekt\",\"name\":\"Sicherung\"}";
    Fixture valid[]={{"brain.json",meta,1},{"knowledge",NULL,2},{"knowledge/notiz.md","# Notiz\n",1}};
    OK(sb_path_join(other,sizeof(other),root,"minimal.sbbackup")); forged(other,valid,3,false,false); OK(sb_backup_inspect(other,&info,NULL,NULL));
    OK(sb_backup_restore(other,restore,"minimal",NULL,NULL,NULL));
    const char *unsafe[]={"../outside.md","/outside.md","C:/outside.md","knowledge\\outside.md","CON.txt","knowledge/trailing.","knowledge/../outside.md"};
    for (unsigned i=0;i<sizeof(unsafe)/sizeof(*unsafe);++i) {
        OK(sb_fs_remove(other)); Fixture bad[]={{"brain.json",meta,1},{unsafe[i],"x",1}}; forged(other,bad,2,false,false);
        CHECK(sb_backup_inspect(other,&info,NULL,NULL).code==SB_INVALID);
        CHECK(sb_backup_restore(other,restore,"unsafe",NULL,NULL,NULL).code==SB_INVALID); OK(sb_path_join(path,sizeof(path),restore,"unsafe")); CHECK(!sb_fs_kind(path));
    }
    OK(sb_fs_remove(other)); Fixture duplicate[]={{"A.md","# A",1},{"a.md","# a",1},{"brain.json",meta,1}}; forged(other,duplicate,3,false,false); CHECK(sb_backup_inspect(other,&info,NULL,NULL).code==SB_INVALID);
    OK(sb_fs_remove(other)); Fixture parent_file[]={{"brain.json",meta,1},{"knowledge","file",1},{"knowledge/notiz.md","note",1}}; forged(other,parent_file,3,false,false); CHECK(sb_backup_inspect(other,&info,NULL,NULL).code==SB_INVALID);
    for (unsigned i=0;i<2;++i) {
        OK(sb_fs_remove(other)); forged(other,valid,3,i==0,i==1); CHECK(sb_backup_inspect(other,&info,NULL,NULL).code==SB_INVALID);
        CHECK(sb_backup_restore(other,restore,"corrupt",NULL,NULL,NULL).code==SB_INVALID);
    }
    OK(sb_fs_remove(other)); forged(other,valid,3,false,false); OK(sb_fs_read(other,&data,&length)); OK(sb_fs_remove(other));
    OK(sb_fs_write_new(other,data,length-1)); CHECK(sb_backup_inspect(other,&info,NULL,NULL).code==SB_INVALID);
    OK(sb_fs_remove(other)); data[7]=2; OK(sb_fs_write_new(other,data,length)); CHECK(sb_backup_inspect(other,&info,NULL,NULL).code==SB_INVALID); free(data);
    OK(sb_fs_remove(other)); forged(other,valid,3,false,false); OK(sb_fs_read(other,&data,&length)); OK(sb_fs_remove(other));
    char *extra=malloc(length+1); CHECK(extra!=NULL); memcpy(extra,data,length); extra[length]='x'; OK(sb_fs_write_new(other,extra,length+1)); free(data); free(extra); CHECK(sb_backup_inspect(other,&info,NULL,NULL).code==SB_INVALID);
    /* A concurrent source edit is detected even when its byte length is unchanged. */
    OK(sb_path_join(path,sizeof(path),project.root,"PROJECT.md")); Control mutation={SB_BACKUP_WRITE,false,true,path};
    OK(sb_path_join(other,sizeof(other),root,"changed.sbbackup")); CHECK(sb_backup_create(&project,other,interrupt,&mutation).code==SB_CONFLICT && mutation.fired && !sb_fs_kind(other)); OK(sb_fs_list(root,no_temporary,NULL));
#ifndef _WIN32
    OK(sb_path_join(path,sizeof(path),project.root,"link")); CHECK(symlink("/",path)==0);
    CHECK(sb_backup_create(&project,other,NULL,NULL).code==SB_INVALID && !sb_fs_kind(other)); OK(sb_fs_remove(path));
#endif
    /* A regular file larger than the documented limit is rejected. */
    OK(sb_path_join(path,sizeof(path),project.root,"zu-gross.bin")); SBFile *large=NULL; OK(sb_file_open(path,true,&large));
    for (unsigned i=0;i<84;++i) OK(sb_file_write(large,binary,sizeof(binary))); OK(sb_file_close(large,true));
    CHECK(sb_backup_create(&project,other,NULL,NULL).code==SB_LIMIT && !sb_fs_kind(other)); OK(sb_fs_remove(path));
    /* Exclusively publishing a directory must reject an existing empty target. */
    char a[SB_PATH_CAP],b[SB_PATH_CAP]; OK(sb_path_join(a,sizeof(a),root,"stage")); OK(sb_path_join(b,sizeof(b),root,"occupied"));
    OK(sb_fs_mkdir(a)); OK(sb_fs_mkdir(b)); CHECK(sb_fs_publish_new(a,b).code==SB_EXISTS && sb_fs_kind(a)==2 && sb_fs_kind(b)==2);
    OK(sb_fs_rmdir(b)); OK(sb_fs_publish_new(a,b)); CHECK(!sb_fs_kind(a) && sb_fs_kind(b)==2);
    puts("Backup checks passed: complete roundtrip, SHA-256 vectors, metadata identity, corruption, unsafe paths, conflicts, aborts and exclusive publication.");
    printf("%u backup assertions passed.\n",checks); return 0;
}
