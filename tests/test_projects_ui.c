#include "desktop.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"PROJECT UI FAIL %d: %s\n",__LINE__,#x); return 1; } } while (0)
#define OK(x) CHECK((x).code==SB_OK)
static void frame(SBDesktop *d) {
    nk_input_begin(d->ui.ctx); SDL_Event e; while (SDL_PollEvent(&e)) sb_desktop_event(d,&e); nk_input_end(d->ui.ctx);
    sb_desktop_tick(d,1.0f/60); sb_desktop_frame(d); sb_ui_draw(&d->ui); SDL_RenderPresent(d->ui.renderer); sb_desktop_apply(d);
}
static void key_mod(SBDesktop *d,SDL_Keycode code,SDL_Keymod mod) { SDL_Event e={0};e.type=SDL_EVENT_KEY_DOWN;e.key.key=code;e.key.mod=mod;nk_input_begin(d->ui.ctx);sb_desktop_event(d,&e);nk_input_end(d->ui.ctx);frame(d);frame(d);frame(d); }
static void key(SBDesktop *d,SDL_Keycode code) { key_mod(d,code,0); }
static SBTarget *target(SBDesktop *d,const char *id) { for (size_t i=0;i<d->target_count;++i) if (!strcmp(d->targets[i].id,id)) return &d->targets[i];return NULL; }
static bool activate(SBDesktop *d,const char *id) { for (unsigned i=0;i<25 && strcmp(d->focus,id);++i) key(d,SDLK_TAB);if (strcmp(d->focus,id))return false;key(d,SDLK_RETURN);return true; }
static SBStatus replace(const char *path,const char *text) { char temporary[SB_PATH_CAP];snprintf(temporary,sizeof(temporary),"%s.tmp",path);SBStatus s=sb_fs_write_new(temporary,text,strlen(text));return s.code==SB_OK ? sb_fs_replace(temporary,path) : s; }
static SBStatus capture(SBDesktop *d,const char *root,const char *name) { char path[SB_PATH_CAP]; SBStatus s=sb_path_join(path,sizeof(path),root,name);return s.code==SB_OK ? sb_ui_capture(&d->ui,path) : s; }
int main(int argc,char **argv) {
    CHECK(argc==3);char root[SB_PATH_CAP],workspace[SB_PATH_CAP],path[SB_PATH_CAP],good_path[SB_PATH_CAP],suffix[90];
    snprintf(suffix,sizeof(suffix),"run-%lu-%lu",sb_process_id(),(unsigned long)time(NULL));OK(sb_path_join(root,sizeof(root),argv[2],suffix));OK(sb_path_join(workspace,sizeof(workspace),root,"Arbeitsordner ü"));
    SBProject good,bad;OK(sb_project_create(workspace,"good","Gültiges Projekt",NULL,&good));OK(sb_project_create(workspace,"bad","Defekt",NULL,&bad));
    OK(sb_path_join(path,sizeof(path),bad.root,"brain.json"));const char *invalid="{\"name\":\"Defekt\",\"schema_version\":2}";OK(replace(path,invalid));
    SBDesktop d;OK(sb_desktop_init(&d,workspace,argv[1],true));frame(&d);CHECK(d.model.has_project && !strcmp(d.model.project.id,"good"));
    OK(sb_path_join(good_path,sizeof(good_path),good.root,"brain.json"));
    char *metadata=NULL;size_t metadata_length=0;OK(sb_fs_read(good_path,&metadata,&metadata_length));
    strcat(d.model.editor,"Entwurf nach Schemawechsel.\n");OK(replace(good_path,invalid));
#ifdef __APPLE__
    SDL_Keymod save_mod=SDL_KMOD_GUI;
#else
    SDL_Keymod save_mod=SDL_KMOD_CTRL;
#endif
    key_mod(&d,SDLK_S,save_mod);CHECK(d.message.code==SB_INVALID && sb_app_dirty(&d.model));
    CHECK(strstr(d.model.editor,"Entwurf nach Schemawechsel"));OK(capture(&d,root,"metadata-save-error.bmp"));
    OK(replace(good_path,metadata));free(metadata);key_mod(&d,SDLK_S,save_mod);CHECK(!sb_app_dirty(&d.model) && d.message.code==SB_OK);
    CHECK(activate(&d,"project-picker") && d.form==SB_FORM_PROJECTS);CHECK(target(&d,"project:good") && !target(&d,"project:bad"));
    bool reason=false;for(size_t i=0;i<d.passive_count;++i) if(strstr(d.passive[i].text,"Projektschema 2"))reason=true;CHECK(reason);OK(capture(&d,root,"partial-projects.bmp"));
    strcat(d.model.editor,"Entwurf bleibt.\n");OK(replace(path,"{\"name\":\"Repariert\"}"));CHECK(activate(&d,"refresh-projects"));CHECK(sb_app_dirty(&d.model) && target(&d,"project:bad"));
    CHECK(activate(&d,"project:bad") && d.model.guard);CHECK(activate(&d,"guard-cancel") && sb_app_dirty(&d.model) && !strcmp(d.model.project.id,"good"));
    OK(sb_app_save(&d.model));CHECK(activate(&d,"project-picker"));CHECK(activate(&d,"project:bad"));CHECK(!strcmp(d.model.project.id,"bad"));sb_desktop_free(&d);
    OK(sb_path_join(good_path,sizeof(good_path),good.root,"brain.json"));OK(replace(good_path,invalid));OK(replace(path,invalid));
    OK(sb_desktop_init(&d,workspace,argv[1],true));frame(&d);CHECK(!d.model.has_project && target(&d,"project-settings"));CHECK(activate(&d,"project-settings"));CHECK(d.form==SB_FORM_PROJECTS);
    OK(sb_ui_fonts(&d.ui,2));CHECK(SDL_SetWindowSize(d.ui.window,780,520));CHECK(SDL_SyncWindow(d.ui.window));sb_desktop_set_style(&d,(SBStyleChoice){.dark=false,.contrast=true,.motion=true});frame(&d);OK(capture(&d,root,"blocked-projects-200.bmp"));
    CHECK(activate(&d,"refresh-projects"));SBTarget *refresh=target(&d,"refresh-projects");int w,h;SDL_GetWindowSize(d.ui.window,&w,&h);CHECK(refresh && refresh->bounds.y>=0 && refresh->bounds.y+refresh->bounds.h<=h);OK(capture(&d,root,"blocked-projects-focus.bmp"));
    key(&d,SDLK_ESCAPE);CHECK(!d.model.has_project && d.form==SB_FORM_NONE);
    char *original=NULL;size_t length=0;OK(sb_fs_read(path,&original,&length));CHECK(!strcmp(original,invalid));free(original);sb_desktop_free(&d);
    printf("%u project UI assertions passed: diagnostics, keyboard recovery, draft guard, all blocked and large text.\n",checks);return 0;
}
