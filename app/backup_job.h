#ifndef SB_BACKUP_JOB_H
#define SB_BACKUP_JOB_H
#include "backup.h"
#include <SDL3/SDL.h>
typedef enum { SB_JOB_BACKUP,SB_JOB_INSPECT,SB_JOB_RESTORE } SBBackupJobKind;
typedef struct SBBackupJob SBBackupJob;
typedef struct {
    SBBackupJobKind kind; SBBackupPhase phase; size_t entries,total;
    uint64_t bytes,bytes_total; bool done,cancel_requested;
    char path[SB_PATH_CAP]; SBStatus status; SBBackupInfo info; SBProject project;
} SBBackupJobState;
SBBackupJob *sb_backup_job_start(SBBackupJobKind kind,const SBProject *project,const char *archive,const char *workspace,const char *id,const unsigned char expected[32]);
void sb_backup_job_snapshot(SBBackupJob *job,SBBackupJobState *out);
void sb_backup_job_cancel(SBBackupJob *job);
void sb_backup_job_free(SBBackupJob *job);
#endif
