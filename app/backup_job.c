#include "backup_job.h"
#include "platform.h"
#include <string.h>
#include <stdio.h>
struct SBBackupJob {
    SDL_Thread *thread; SDL_Mutex *mutex; SDL_AtomicInt cancel;
    SBBackupJobState state; SBProject source;
    char archive[SB_PATH_CAP],workspace[SB_PATH_CAP],id[65];
    unsigned char expected[32]; bool checked;
};
static bool progress(const SBBackupProgress *p,void *userdata) {
    SBBackupJob *j=userdata;
    if (SDL_GetAtomicInt(&j->cancel)) return false;
    SDL_LockMutex(j->mutex); j->state.phase=p->phase; j->state.entries=p->entries; j->state.total=p->entries_total;
    j->state.bytes=p->bytes; j->state.bytes_total=p->bytes_total;
    snprintf(j->state.path,sizeof(j->state.path),"%s",p->path ? p->path : "");
    SDL_UnlockMutex(j->mutex); return !SDL_GetAtomicInt(&j->cancel);
}
static int run(void *userdata) {
    SBBackupJob *j=userdata; SBStatus status; SBBackupInfo info={0}; SBProject restored={0};
    if (SDL_GetAtomicInt(&j->cancel)) status=sb_error(SB_CANCELLED,"Abgebrochen.");
    else if (j->state.kind==SB_JOB_BACKUP) status=sb_backup_create(&j->source,j->archive,progress,j);
    else if (j->state.kind==SB_JOB_INSPECT) status=sb_backup_inspect(j->archive,&info,progress,j);
    else {
        status=sb_fs_mkdirs(j->workspace);
        if (status.code==SB_OK) status=sb_backup_restore_checked(j->archive,j->workspace,j->id,j->checked ? j->expected : NULL,&restored,progress,j);
    }
    SDL_LockMutex(j->mutex); j->state.status=status; j->state.info=info; j->state.project=restored; j->state.done=true;
    SDL_UnlockMutex(j->mutex); return 0;
}
SBBackupJob *sb_backup_job_start(SBBackupJobKind kind,const SBProject *project,const char *archive,const char *workspace,const char *id,const unsigned char expected[32]) {
    if (kind!=SB_JOB_BACKUP && kind!=SB_JOB_INSPECT && kind!=SB_JOB_RESTORE) return NULL;
    if (!archive || strlen(archive)>=SB_PATH_CAP || (workspace && strlen(workspace)>=SB_PATH_CAP) || (id && strlen(id)>64)) return NULL;
    SBBackupJob *j=SDL_calloc(1,sizeof(*j)); if (!j) return NULL;
    j->mutex=SDL_CreateMutex(); if (!j->mutex) { SDL_free(j); return NULL; }
    j->state.kind=kind; j->state.phase=kind==SB_JOB_BACKUP ? SB_BACKUP_SCAN : SB_BACKUP_VERIFY; if (project) j->source=*project;
    if (expected) { memcpy(j->expected,expected,32); j->checked=true; }
    strcpy(j->archive,archive); if (workspace) strcpy(j->workspace,workspace); if (id) strcpy(j->id,id);
    j->thread=SDL_CreateThread(run,"SecondBrain backup",j);
    if (!j->thread) { SDL_DestroyMutex(j->mutex); SDL_free(j); return NULL; }
    return j;
}
void sb_backup_job_snapshot(SBBackupJob *j,SBBackupJobState *out) {
    SDL_LockMutex(j->mutex); *out=j->state; SDL_UnlockMutex(j->mutex); out->cancel_requested=SDL_GetAtomicInt(&j->cancel)!=0;
}
void sb_backup_job_cancel(SBBackupJob *j) { if (j) SDL_SetAtomicInt(&j->cancel,1); }
void sb_backup_job_free(SBBackupJob *j) {
    if (!j) return;
    sb_backup_job_cancel(j); SDL_WaitThread(j->thread,NULL); SDL_DestroyMutex(j->mutex); SDL_free(j);
}
