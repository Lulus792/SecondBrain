#include "model.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"METADATA GUARD FAIL %d: %s\n",__LINE__,#x); return 1; } } while (0)
#define OK(x) CHECK((x).code==SB_OK)
static SBStatus replace(const char *path,const char *bytes,size_t length) {
    char temporary[SB_PATH_CAP];snprintf(temporary,sizeof(temporary),"%s.tmp",path);
    SBStatus s=sb_fs_write_new(temporary,bytes,length);return s.code==SB_OK ? sb_fs_replace(temporary,path) : s;
}
int main(int argc,char **argv) {
    CHECK(argc==2);char root[SB_PATH_CAP],suffix[90],meta[SB_PATH_CAP],note[SB_PATH_CAP],new_note[SB_PATH_CAP],archived[SB_PATH_CAP];
    snprintf(suffix,sizeof(suffix),"run-%lu-%lu",sb_process_id(),(unsigned long)time(NULL));OK(sb_path_join(root,sizeof(root),argv[1],suffix));
    SBApp app;OK(sb_app_init(&app,root));OK(sb_app_new_project(&app,"projekt","Bestandsschutz",NULL));OK(sb_app_new_note(&app,"knowledge","note","Notiz"));
    OK(sb_path_join(meta,sizeof(meta),app.project.root,"brain.json"));OK(sb_path_join(note,sizeof(note),app.project.root,app.path));OK(sb_path_join(new_note,sizeof(new_note),app.project.root,"inbox/blockiert.md"));
    char *original_meta=NULL,*original_note=NULL,*actual=NULL;size_t meta_length=0,note_length=0,length=0;
    OK(sb_fs_read(meta,&original_meta,&meta_length));OK(sb_fs_read(note,&original_note,&note_length));SBRevision revision=app.revision;
    strcpy(app.editor,"# Notiz\n\nEntwurf bleibt erhalten.\n");
    const char *invalid[]={"{\"name\":\"Projekt\",\"schema_version\":2}","{\"name\":true}",NULL};
    for (unsigned i=0;i<4;++i) {
        if (i==3) { OK(sb_fs_remove(meta)); OK(sb_fs_mkdir(meta)); }
        else if (invalid[i]) OK(replace(meta,invalid[i],strlen(invalid[i])));else OK(sb_fs_remove(meta));
        SBCode expected=i==2 ? SB_NOT_FOUND : SB_INVALID;
        CHECK(sb_app_save(&app).code==expected && sb_app_dirty(&app));CHECK(sb_app_save_copy(&app).code==expected && sb_app_dirty(&app));
        CHECK(sb_note_create(&app.project,"inbox","blockiert","Blockiert",NULL).code==expected && sb_fs_kind(new_note)==0);
        strcpy(archived,"erhalten");CHECK(sb_note_archive(&app.project,app.path,revision,archived,sizeof(archived)).code==expected && !strcmp(archived,"erhalten"));
        OK(sb_app_request(&app,SB_ACT_NOTE,"STATE.md"));CHECK(app.guard);
        CHECK(sb_app_decide(&app,SB_SAVE_CHANGES).code==expected && app.guard && sb_app_dirty(&app));OK(sb_app_decide(&app,SB_KEEP_EDITING));
        CHECK(strstr(app.editor,"Entwurf bleibt erhalten") && !strcmp(app.path,"knowledge/note.md"));
        OK(sb_fs_read(note,&actual,&length));CHECK(length==note_length && !memcmp(actual,original_note,length));free(actual);actual=NULL;
        if (i<2) { OK(sb_fs_read(meta,&actual,&length));CHECK(length==strlen(invalid[i]) && !memcmp(actual,invalid[i],length));free(actual);actual=NULL; }
        if (i==3) { CHECK(sb_fs_kind(meta)==2); OK(sb_fs_rmdir(meta)); }
        OK(replace(meta,original_meta,meta_length));
    }
    OK(sb_app_save(&app));CHECK(!sb_app_dirty(&app));OK(sb_fs_read(note,&actual,&length));CHECK(strstr(actual,"Entwurf bleibt erhalten"));free(actual);
    OK(sb_fs_read(meta,&actual,&length));CHECK(length==meta_length && !memcmp(actual,original_meta,length));free(actual);
    free(original_meta);free(original_note);sb_app_free(&app);
    printf("%u live metadata guard assertions passed: save, copies, creation, archive, guards and recovery.\n",checks);return 0;
}
