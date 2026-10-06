#include "sb.h"
#include "platform.h"
#include "backup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"METADATA FAIL %d: %s\n",__LINE__,#x); return 1; } } while (0)
#define OK(x) CHECK((x).code==SB_OK)
int main(int argc,char **argv) {
    CHECK(argc==2); char name[SB_NAME_CAP],root[SB_PATH_CAP],path[SB_PATH_CAP],temp[SB_PATH_CAP],archive[SB_PATH_CAP],suffix[90];
    const char *valid[]={
        "{\"name\":\"Altformat ü\"}",
        "{\"schema_version\":1,\"id\":\"projekt\",\"name\":\"Altformat\"}",
        "{\"schema_version\":1,\"template_version\":4294967295,\"id\":\"projekt\",\"name\":\"Projekt\",\"created\":\"2026-10-06\",\"project_root\":null}",
        "{\"name\":\"Projekt\",\"project_root\":\"../Ordner ü\",\"custom\":{\"a\":[true,42,\"ü\"]}}"
    };
    for (size_t i=0;i<sizeof(valid)/sizeof(*valid);++i) OK(sb_metadata_validate(valid[i],strlen(valid[i]),name));
    const char *bad[]={
        "{\"name\":\"A\",\"schema_version\":2}","{\"name\":\"A\",\"schema_version\":0}",
        "{\"name\":\"A\",\"schema_version\":\"1\"}","{\"name\":\"A\",\"schema_version\":1.0}",
        "{\"name\":\"A\",\"schema_version\":1e0}","{\"name\":\"A\",\"schema_version\":-1}",
        "{\"name\":\"A\",\"schema_version\":01}","{\"name\":\"A\",\"schema_version\":1,\"schema_version\":1}",
        "{\"name\":\"A\",\"template_version\":4294967296}","{\"name\":\"A\",\"template_version\":null}",
        "{\"name\":\"A\",\"id\":\"../projekt\"}","{\"name\":\"A\",\"id\":\"con\"}",
        "{\"name\":\"A\",\"id\":\"a\",\"id\":\"b\"}","{\"name\":\"A\",\"created\":true}",
        "{\"name\":\"A\",\"project_root\":[\"x\"]}","{\"name\":\"A\",\"project_root\":\"x\\nY\"}",
        "{\"name\":\"A\",\"project_root\":null,\"project_root\":null}",
        "{\"name\":\"A\",\"na\\u006de\":\"B\"}","{\"name\":\"A\"} trailing",
        "{\"name\":\"A\",\"extra\":[true,]}","{\"name\":\"A\",\"extra\":{\"x\":1,}}"
    };
    for (size_t i=0;i<sizeof(bad)/sizeof(*bad);++i) {
        strcpy(name,"erhalten"); CHECK(sb_metadata_validate(bad[i],strlen(bad[i]),name).code==SB_INVALID);
        CHECK(!strcmp(name,"erhalten")); char *changed=(char *)1; size_t length=999;
        CHECK(sb_metadata_reidentify(bad[i],strlen(bad[i]),"kopie",&changed,&length).code==SB_INVALID);
        CHECK(!changed && !length);
    }
    const char nul[]={"{\"name\":\"A\"}\0{\"schema_version\":2}"};
    CHECK(sb_metadata_validate(nul,sizeof(nul)-1,name).code==SB_INVALID);
    CHECK(sb_metadata_validate(NULL,0,name).code==SB_INVALID);
    CHECK(sb_metadata_validate(NULL,(size_t)SB_TEXT_LIMIT+1,name).code==SB_LIMIT);
    /* The length-based API must not inspect a terminator outside this allocation. */
    size_t length=strlen(valid[3]); char *bounded=malloc(length); CHECK(bounded); memcpy(bounded,valid[3],length);
    OK(sb_metadata_validate(bounded,length,name)); char *changed=NULL; size_t changed_length=0;
    OK(sb_metadata_reidentify(bounded,length,"kopie",&changed,&changed_length)); free(bounded);
    CHECK(strstr(changed,"\"id\": \"kopie\"") && strstr(changed,"\"custom\":{\"a\":[true,42,\"ü\"]}"));
    OK(sb_metadata_validate(changed,changed_length,name)); free(changed);
    const char *escaped="{\"name\":\"A\",\"id\":\"proje\\u006bt\",\"extra\": [1,2]}";
    OK(sb_metadata_reidentify(escaped,strlen(escaped),"kopie",&changed,&changed_length));
    CHECK(strstr(changed,"\"id\":\"kopie\"") && strstr(changed,"\"extra\": [1,2]")); free(changed);
    snprintf(suffix,sizeof(suffix),"run-%lu-%lu",sb_process_id(),(unsigned long)time(NULL));
    OK(sb_path_join(root,sizeof(root),argv[1],suffix)); SBProject project; OK(sb_project_create(root,"projekt","Projekt",NULL,&project));
    char note_path[SB_PATH_CAP]; const char note_nul[]={"# Notiz\0Unsichtbarer Rest"};
    OK(sb_path_join(note_path,sizeof(note_path),project.root,"knowledge/nul.md"));
    OK(sb_fs_write_new(note_path,note_nul,sizeof(note_nul)-1));
    char *note_text=NULL; SBRevision revision={0};
    CHECK(sb_note_load(&project,"knowledge/nul.md",&note_text,&revision).code==SB_INVALID && !note_text);
    SBNotes notes={0}; CHECK(sb_notes_list(&project,&notes).code==SB_INVALID && !notes.items && !notes.count);
    CHECK(sb_note_save(&project,"knowledge/nul.md","# Ersetzen",revision,NULL).code==SB_INVALID);
    size_t note_length=0; OK(sb_fs_read(note_path,&note_text,&note_length));
    CHECK(note_length==sizeof(note_nul)-1 && !memcmp(note_text,note_nul,note_length)); free(note_text);
    OK(sb_fs_remove(note_path));
    OK(sb_path_join(path,sizeof(path),project.root,"brain.json")); OK(sb_path_join(temp,sizeof(temp),project.root,"meta.tmp"));
    OK(sb_fs_write_new(temp,nul,sizeof(nul)-1)); OK(sb_fs_replace(temp,path));
    SBProjects projects={0}; CHECK(sb_projects_list(root,&projects).code==SB_INVALID && !projects.items && !projects.count);
    char *original=NULL; size_t original_length=0; OK(sb_fs_read(path,&original,&original_length));
    CHECK(original_length==sizeof(nul)-1 && !memcmp(original,nul,original_length)); free(original);
    OK(sb_fs_write_new(temp,bad[0],strlen(bad[0]))); OK(sb_fs_replace(temp,path));
    OK(sb_path_join(archive,sizeof(archive),root,"unsupported.sbbackup"));
    CHECK(sb_backup_create(&project,archive,NULL,NULL).code==SB_INVALID && sb_fs_kind(archive)==0);
    OK(sb_fs_read(path,&original,&original_length)); CHECK(original_length==strlen(bad[0]) && !memcmp(original,bad[0],original_length)); free(original);
    printf("%u metadata contract assertions passed: versions, legacy input, bounded buffers, field types and original preservation.\n",checks); return 0;
}
