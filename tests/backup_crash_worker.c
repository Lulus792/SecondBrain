#include "backup.h"
#include "platform.h"
#include "settings.h"
#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

typedef struct { SBBackupPhase phase; const char *marker; bool pause,recorded; } Stop;
static bool checkpoint(const SBBackupProgress *progress,void *userdata) {
    Stop *stop=userdata;
    if (progress->phase!=stop->phase) return true;
    if (progress->phase!=SB_BACKUP_PUBLISH && progress->bytes<65536) return true;
    if (stop->recorded) return true;
    char text[100]; snprintf(text,sizeof(text),"%u %llu\n",(unsigned)progress->phase,(unsigned long long)progress->bytes);
    SBStatus status=sb_fs_write_new(stop->marker,text,strlen(text));
    if (status.code!=SB_OK) { fprintf(stderr,"%s\n",status.message); return false; }
    stop->recorded=true; if (!stop->pause) return true;
    /* Parent keeps stdin open, then kills this process without cleanup. */
    while (getchar()!=EOF) {}
    return false;
}
static int run(int argc,char **argv) {
    if (argc!=7) return 2;
    Stop stop={(SBBackupPhase)strtoul(argv[5],NULL,10),argv[6],!strstr(argv[1],"-fill"),false};
    SBStatus status;
    if (!strcmp(argv[1],"model-save")) {
        SBApp app={0};char *text=NULL;size_t length=0;
        status=sb_fs_read(argv[3],&text,&length);
        if (status.code==SB_OK) status=sb_app_init(&app,argv[2]);
        if (status.code==SB_OK) status=sb_app_request(&app,SB_ACT_PROJECT,argv[5]);
        if (status.code==SB_OK) status=sb_app_request(&app,SB_ACT_NOTE,argv[4]);
        if (status.code==SB_OK && app.editor && length<=SB_TEXT_LIMIT) {
            memcpy(app.editor,text,length+1);SBRevision before=app.revision;
            status=sb_app_request(&app,SB_ACT_QUIT,NULL);
            if (status.code==SB_OK && (!app.guard || !sb_app_dirty(&app))) status=sb_error(SB_INVALID,"Entwurf löst keinen Schutzdialog aus.");
            if (status.code==SB_OK) {
                status=sb_app_decide(&app,SB_SAVE_CHANGES);
                bool kept=app.editor && !strcmp(app.editor,text);
                if (status.code==SB_IO && (!kept || !sb_app_dirty(&app) || !app.guard || app.quit || app.pending.kind!=SB_ACT_QUIT || app.revision.hash!=before.hash || app.revision.length!=before.length))
                    status=sb_error(SB_INVALID,"Speicherfehler verliert Entwurf oder Schutzentscheidung.");
                if (status.code==SB_OK && (!kept || sb_app_dirty(&app) || app.guard || !app.quit))
                    status=sb_error(SB_INVALID,"Erneutes Speichern beendet den geschützten Wechsel nicht.");
            }
        } else if (status.code==SB_OK) status=sb_error(SB_INVALID,"Editor fehlt.");
        free(text);sb_app_free(&app);
    } else if (!strcmp(argv[1],"note-save")) {
        SBProject project={0};char *text=NULL,*old=NULL;size_t length=0;uint64_t draft_hash=0;SBRevision expected={0},saved={0};
        if (strlen(argv[2])>=sizeof(project.root) || strlen(argv[5])>=sizeof(project.id)) return 2;
        strcpy(project.root,argv[2]);strcpy(project.id,argv[5]);
        status=sb_fs_read(argv[3],&text,&length);
        if (status.code==SB_OK) draft_hash=sb_hash(text,length);
        if (status.code==SB_OK && strlen(text)!=length) status=sb_error(SB_INVALID,"Text enthält NUL.");
        if (status.code==SB_OK) {
            status=sb_note_load(&project,argv[4],&old,&expected);free(old);
            if (status.code==SB_NOT_FOUND) status=sb_ok();
        }
        if (status.code==SB_OK) status=sb_note_save(&project,argv[4],text,expected,&saved);
        if (status.code==SB_OK && (!saved.exists || saved.length!=length || saved.hash!=sb_hash(text,length)))
            status=sb_error(SB_INVALID,"Gespeicherte Textrevision stimmt nicht.");
        if (text && sb_hash(text,length)!=draft_hash) status=sb_error(SB_INVALID,"Übergebener Entwurf wurde verändert.");
        free(text);
    } else if (!strcmp(argv[1],"settings-init") || !strcmp(argv[1],"settings-update")) {
        SBSettings settings;SBRevision expected={0};status=sb_settings_load(argv[2],&settings,&expected);
        if (status.code==SB_OK && !strcmp(argv[1],"settings-init")) {
            if (expected.exists) status=sb_error(SB_EXISTS,"Einstellungen bestehen bereits.");
            else if (strlen(argv[3])>=sizeof(settings.workspace) || strlen(argv[4])>=sizeof(settings.project) || strlen(argv[5])>=sizeof(settings.note)) return 2;
            else {strcpy(settings.workspace,argv[3]);strcpy(settings.project,argv[4]);strcpy(settings.note,argv[5]);}
        } else if (status.code==SB_OK) {
            if (!expected.exists) status=sb_error(SB_NOT_FOUND,"Einstellungen fehlen.");
            else {settings.dark=!settings.dark;settings.font_percent=settings.font_percent==100 ? 150 : 100;}
        }
        if (status.code==SB_OK) status=sb_settings_save(argv[2],&settings,expected,NULL);
    } else if (!strcmp(argv[1],"create") || !strcmp(argv[1],"create-fill")) {
        SBProject project={0};
        if (strlen(argv[2])>=sizeof(project.root) || strlen(argv[4])>=sizeof(project.id)) return 2;
        strcpy(project.root,argv[2]); strcpy(project.id,argv[4]);
        status=sb_backup_create(&project,argv[3],checkpoint,&stop);
    } else if (!strcmp(argv[1],"restore") || !strcmp(argv[1],"restore-fill"))
        status=sb_backup_restore(argv[2],argv[3],argv[4],NULL,checkpoint,&stop);
    else return 2;
    if (status.code!=SB_OK) fprintf(stderr,"status=%u: %s\n",(unsigned)status.code,status.message);
    return status.code==SB_OK ? 0 : 1;
}
#ifdef _WIN32
int wmain(int argc,wchar_t **wide) {
    char **args=calloc((size_t)argc,sizeof(*args)); if (!args) return 1;
    bool valid=true;
    for (int i=0;i<argc;++i) {
        int n=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,wide[i],-1,NULL,0,NULL,NULL);
        if (n<=0 || !(args[i]=malloc((size_t)n)) || !WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,wide[i],-1,args[i],n,NULL,NULL)) { valid=false; break; }
    }
    int result=valid ? run(argc,args) : 1;
    for (int i=0;i<argc;++i) free(args[i]); free(args); return result;
}
#else
int main(int argc,char **argv) { return run(argc,argv); }
#endif
