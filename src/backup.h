#ifndef SB_BACKUP_H
#define SB_BACKUP_H
#include "sb.h"
#define SB_BACKUP_ENTRIES 4096u
#define SB_BACKUP_BYTES (256u*1024u*1024u)
typedef enum { SB_BACKUP_SCAN,SB_BACKUP_WRITE,SB_BACKUP_VERIFY,SB_BACKUP_RESTORE,SB_BACKUP_PUBLISH,SB_BACKUP_RECHECK } SBBackupPhase;
typedef struct {
    SBBackupPhase phase; size_t entries,entries_total; uint64_t bytes,bytes_total; const char *path;
} SBBackupProgress;
typedef bool (*SBBackupCallback)(const SBBackupProgress *progress,void *userdata);
typedef struct { char id[65],name[SB_NAME_CAP]; size_t files,directories; uint64_t bytes; } SBBackupInfo;
SBStatus sb_backup_create(const SBProject *project,const char *archive,SBBackupCallback progress,void *userdata);
SBStatus sb_backup_inspect(const char *archive,SBBackupInfo *info,SBBackupCallback progress,void *userdata);
SBStatus sb_backup_restore(const char *archive,const char *workspace,const char *id,SBProject *out,SBBackupCallback progress,void *userdata);
#endif
