#include "backup.h"
#include "platform.h"
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
    if (!strcmp(argv[1],"create") || !strcmp(argv[1],"create-fill")) {
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
