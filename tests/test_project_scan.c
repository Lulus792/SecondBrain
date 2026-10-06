#include "model.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"PROJECT SCAN FAIL %d: %s\n",__LINE__,#x); return 1; } } while (0)
#define OK(x) CHECK((x).code==SB_OK)
static SBStatus replace(const char *path,const char *text) {
    char temp[SB_PATH_CAP]; snprintf(temp,sizeof(temp),"%s.tmp",path);
    SBStatus status=sb_fs_write_new(temp,text,strlen(text));
    return status.code==SB_OK ? sb_fs_replace(temp,path) : status;
}
int main(int argc,char **argv) {
    CHECK(argc==2); char root[SB_PATH_CAP],suffix[90],path[SB_PATH_CAP],other[SB_PATH_CAP];
    snprintf(suffix,sizeof(suffix),"run-%lu-%lu",sb_process_id(),(unsigned long)time(NULL));
    OK(sb_path_join(root,sizeof(root),argv[1],suffix)); SBProject good,bad;
    OK(sb_project_create(root,"good","Z gültig",NULL,&good)); OK(sb_project_create(root,"bad","A defekt",NULL,&bad));
    OK(sb_path_join(path,sizeof(path),bad.root,"brain.json")); const char *invalid="{\"name\":\"Defekt\",\"schema_version\":2}";
    OK(replace(path,invalid)); SBProjects projects={0};
    CHECK(sb_projects_list(root,&projects).code==SB_INVALID && !projects.count);
    OK(sb_projects_scan(root,&projects)); CHECK(projects.count==2 && !strcmp(projects.items[0].id,"good") && projects.items[1].problem.code==SB_INVALID); sb_projects_free(&projects);
    SBApp app; OK(sb_app_init(&app,root)); CHECK(app.has_project && !strcmp(app.project.id,"good") && app.projects.count==2);
    CHECK(sb_app_request(&app,SB_ACT_PROJECT,"bad").code==SB_INVALID && !strcmp(app.project.id,"good"));
    strcat(app.editor,"Entwurf bleibt.\n"); OK(sb_app_refresh_projects(&app)); CHECK(sb_app_dirty(&app) && strstr(app.editor,"Entwurf bleibt"));
    CHECK(sb_app_request(&app,SB_ACT_PROJECT,"bad").code==SB_OK && app.guard); OK(sb_app_decide(&app,SB_KEEP_EDITING)); CHECK(sb_app_dirty(&app));
    OK(replace(path,"{\"name\":\"Repariert\"}")); OK(sb_app_refresh_projects(&app));
    OK(sb_app_request(&app,SB_ACT_PROJECT,"bad")); CHECK(app.guard); OK(sb_app_decide(&app,SB_SAVE_CHANGES)); CHECK(!strcmp(app.project.id,"bad"));
    char *saved=NULL; size_t size=0; OK(sb_fs_read(path,&saved,&size)); CHECK(!strcmp(saved,"{\"name\":\"Repariert\"}")); free(saved);
    /* Fresh metadata must be checked even after a successful scan. */
    OK(sb_path_join(other,sizeof(other),good.root,"brain.json")); OK(replace(other,invalid));
    CHECK(sb_app_request(&app,SB_ACT_PROJECT,"good").code==SB_INVALID && !strcmp(app.project.id,"bad"));
    OK(replace(path,invalid)); OK(sb_app_refresh_projects(&app)); sb_app_free(&app);
    OK(sb_app_init(&app,root)); CHECK(!app.has_project && app.projects.count==2); OK(sb_app_new_project(&app,"new","Neu",NULL)); CHECK(app.has_project && !strcmp(app.project.id,"new")); sb_app_free(&app);
    /* A project with invalid note text cannot block the next usable project. */
    OK(replace(other,"{\"name\":\"A Notizfehler\"}")); OK(sb_path_join(other,sizeof(other),good.root,"PROJECT.md"));
    const char nul[]={"# Text\0Rest"}; char temp[SB_PATH_CAP]; OK(sb_path_join(temp,sizeof(temp),good.root,"note.tmp")); OK(sb_fs_write_new(temp,nul,sizeof(nul)-1)); OK(sb_fs_replace(temp,other));
    OK(sb_app_init(&app,root)); CHECK(app.has_project && !strcmp(app.project.id,"new"));
    bool noted=false; for (size_t i=0;i<app.projects.count;++i) if (!strcmp(app.projects.items[i].id,"good")) noted=app.projects.items[i].problem.code==SB_INVALID;
    CHECK(noted); sb_app_free(&app);
    OK(sb_fs_read(path,&saved,&size)); CHECK(!strcmp(saved,invalid)); free(saved);
    OK(sb_fs_read(other,&saved,&size)); CHECK(size==sizeof(nul)-1 && !memcmp(saved,nul,size)); free(saved);
    printf("%u project discovery assertions passed: partial lists, guards, recovery, stale metadata and preserved files.\n",checks); return 0;
}
