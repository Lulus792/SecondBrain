#include "backup.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"BACKUP IO FAIL %d: %s\n",__LINE__,#x); return 1; } } while (0)
#define OK(x) CHECK((x).code==SB_OK)
static SBStatus clean(const char *name,int kind,void *data) {
    (void)kind; (void)data;
    if (!strncmp(name,".sb-backup-",11) || !strncmp(name,".sb-restore-",12)) return sb_error(SB_INVALID,"Temporäre Daten blieben zurück.");
    return sb_ok();
}
int main(int argc,char **argv) {
    CHECK(argc==2); char root[SB_PATH_CAP],workspace[SB_PATH_CAP],archive[SB_PATH_CAP],failed[SB_PATH_CAP],path[SB_PATH_CAP],restore[SB_PATH_CAP],suffix[90];
    snprintf(suffix,sizeof(suffix),"run-%lu-%lu",sb_process_id(),(unsigned long)time(NULL));
    OK(sb_path_join(root,sizeof(root),argv[1],suffix)); OK(sb_fs_mkdirs(root));
    OK(sb_path_join(workspace,sizeof(workspace),root,"workspace")); SBProject project;
    OK(sb_project_create(workspace,"projekt","Projekt",NULL,&project));
    OK(sb_path_join(path,sizeof(path),project.root,"attachment.bin"));
    char *bytes=malloc(200000); CHECK(bytes!=NULL); memset(bytes,42,200000); OK(sb_fs_write_new(path,bytes,200000)); free(bytes);
    OK(sb_path_join(archive,sizeof(archive),root,"valid.sbbackup")); OK(sb_backup_create(&project,archive,NULL,NULL));
    OK(sb_path_join(failed,sizeof(failed),root,"failed.sbbackup"));
    sb_test_file_fail_after(100000); CHECK(sb_backup_create(&project,failed,NULL,NULL).code==SB_IO); sb_test_file_fail_after(SIZE_MAX);
    CHECK(!sb_fs_kind(failed)); OK(sb_fs_list(root,clean,NULL));
    SBBackupInfo info; OK(sb_backup_inspect(archive,&info,NULL,NULL));
    OK(sb_path_join(restore,sizeof(restore),root,"restore")); OK(sb_fs_mkdir(restore));
    sb_test_file_fail_after(100000); CHECK(sb_backup_restore(archive,restore,"projekt",NULL,NULL,NULL).code==SB_IO); sb_test_file_fail_after(SIZE_MAX);
    OK(sb_path_join(path,sizeof(path),restore,"projekt")); CHECK(!sb_fs_kind(path)); OK(sb_fs_list(restore,clean,NULL));
    /* A failure while adapting the new identity also rolls back the entire stage. */
    sb_test_file_fail_after((size_t)info.bytes+1); CHECK(sb_backup_restore(archive,restore,"kopie",NULL,NULL,NULL).code==SB_IO); sb_test_file_fail_after(SIZE_MAX);
    OK(sb_path_join(path,sizeof(path),restore,"kopie")); CHECK(!sb_fs_kind(path)); OK(sb_fs_list(restore,clean,NULL));
    OK(sb_backup_restore(archive,restore,"projekt",NULL,NULL,NULL)); OK(sb_backup_inspect(archive,&info,NULL,NULL));
    printf("%u assertions passed for injected disk-full failures and retry; production binaries contain no injection API.\n",checks); return 0;
}
