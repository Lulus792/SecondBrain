#include "desktop.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"WELCOME FAIL %d: %s\n",__LINE__,#x); return 1; } } while (0)
#define OK(x) CHECK((x).code==SB_OK)
static void frame(SBDesktop *d) {
    nk_input_begin(d->ui.ctx); SDL_Event event;
    while (SDL_PollEvent(&event)) sb_desktop_event(d,&event);
    nk_input_end(d->ui.ctx); sb_desktop_tick(d,1.0f/60); sb_desktop_frame(d);
    sb_ui_draw(&d->ui); SDL_RenderPresent(d->ui.renderer); sb_desktop_apply(d);
}
static void key(SBDesktop *d,SDL_Keycode code) {
    SDL_Event event={0}; event.type=SDL_EVENT_KEY_DOWN; event.key.key=code; event.key.down=true;
    nk_input_begin(d->ui.ctx); sb_desktop_event(d,&event); nk_input_end(d->ui.ctx);
    frame(d); frame(d); frame(d);
}
static SBTarget *target(SBDesktop *d,const char *id) {
    for (size_t i=0;i<d->target_count;++i) if (!strcmp(d->targets[i].id,id)) return &d->targets[i];
    return NULL;
}
static bool reach(SBDesktop *d,const char *id) {
    for (unsigned i=0;i<20 && strcmp(d->focus,id);++i) key(d,SDLK_TAB);
    return !strcmp(d->focus,id);
}
static bool capture(SBDesktop *d,const char *root,const char *name) {
    char path[SB_PATH_CAP];
    return sb_path_join(path,sizeof(path),root,name).code==SB_OK && sb_ui_capture(&d->ui,path).code==SB_OK;
}
int main(int argc,char **argv) {
    CHECK(argc==3); char root[SB_PATH_CAP],workspace[SB_PATH_CAP],empty[SB_PATH_CAP],suffix[90];
    snprintf(suffix,sizeof(suffix),"run-%lu-%lu",sb_process_id(),(unsigned long)time(NULL));
    OK(sb_path_join(root,sizeof(root),argv[2],suffix)); OK(sb_fs_mkdirs(root));
    OK(sb_path_join(workspace,sizeof(workspace),root,"Arbeitsordner ü"));
    OK(sb_path_join(empty,sizeof(empty),root,"Weiterer Ordner"));
    SBDesktop d; OK(sb_desktop_init(&d,workspace,argv[1],true)); frame(&d); frame(&d);
    CHECK(!d.model.has_project && d.model.projects.count==0 && !strcmp(d.focus,"new-project"));
    CHECK(target(&d,"new-project") && target(&d,"workspace-detail") && target(&d,"restore-project"));
    CHECK(!target(&d,"search") && !target(&d,"new-note") && !target(&d,"galaxy"));
    bool heading=false,path=false;
    for (size_t i=0;i<d.passive_count;++i) {
        if (!strcmp(d.passive[i].id,"welcome-title")) heading=d.passive[i].role==ACCESSKIT_ROLE_HEADING;
        if (!strcmp(d.passive[i].id,"welcome-workspace")) path=!strcmp(d.passive[i].text,d.model.workspace);
    }
    CHECK(heading && path); CHECK(capture(&d,root,"welcome-dark.bmp"));
    CHECK(reach(&d,"workspace-detail")); key(&d,SDLK_RETURN); CHECK(d.form==SB_FORM_WORKSPACE);
    key(&d,SDLK_ESCAPE); CHECK(d.form==SB_FORM_NONE && !d.model.has_project && !strcmp(d.focus,"workspace-detail"));
    CHECK(reach(&d,"restore-project")); key(&d,SDLK_RETURN); CHECK(d.form==SB_FORM_RESTORE);
    key(&d,SDLK_ESCAPE); CHECK(d.form==SB_FORM_NONE && !strcmp(d.focus,"restore-project"));
    CHECK(reach(&d,"help-actions")); key(&d,SDLK_RETURN); CHECK(d.form==SB_FORM_HELP);
    key(&d,SDLK_ESCAPE); CHECK(d.form==SB_FORM_NONE && !strcmp(d.focus,"help-actions"));
    OK(sb_ui_fonts(&d.ui,2)); CHECK(SDL_SetWindowSize(d.ui.window,780,520)); CHECK(SDL_SyncWindow(d.ui.window));
    sb_desktop_set_style(&d,(SBStyleChoice){.dark=false,.contrast=true,.motion=true}); frame(&d);
    CHECK(reach(&d,"settings-actions")); SBTarget *settings=target(&d,"settings-actions");
    int width,height; SDL_GetWindowSize(d.ui.window,&width,&height);
    CHECK(capture(&d,root,"welcome-200-light.bmp"));
    CHECK(settings && settings->bounds.x>=0 && settings->bounds.x+settings->bounds.w<=width && settings->bounds.y>=0 && settings->bounds.y+settings->bounds.h<=height);
    SDL_Event wheel={0}; wheel.type=SDL_EVENT_MOUSE_WHEEL; wheel.wheel.y=-1;
    wheel.wheel.mouse_x=width/2.0f; wheel.wheel.mouse_y=height/2.0f;
    float scroll=d.scrolling[2].position,zoom=d.zoom;
    sb_desktop_event(&d,&wheel); frame(&d);
    CHECK(d.scrolling[2].position>scroll && d.zoom==zoom);
    scroll=d.scrolling[2].position; wheel.wheel.mouse_x=5; wheel.wheel.mouse_y=5;
    sb_desktop_event(&d,&wheel); frame(&d);
    CHECK(d.scrolling[2].position==scroll && d.zoom==zoom);
    key(&d,SDLK_RETURN); CHECK(d.form==SB_FORM_SETTINGS);
    key(&d,SDLK_ESCAPE); CHECK(d.form==SB_FORM_NONE && !strcmp(d.focus,"settings-actions"));
    CHECK(reach(&d,"new-project")); key(&d,SDLK_RETURN); CHECK(d.form==SB_FORM_PROJECT);
    key(&d,SDLK_ESCAPE); CHECK(!d.model.has_project && d.model.projects.count==0);
    OK(sb_app_new_project(&d.model,"projekt","Mein Projekt",NULL)); frame(&d);
    CHECK(d.model.has_project && target(&d,"search") && target(&d,"new-note") && !target(&d,"workspace-detail"));
    OK(sb_fs_mkdirs(empty)); OK(sb_app_request(&d.model,SB_ACT_WORKSPACE,empty)); frame(&d);
    CHECK(!d.model.has_project && target(&d,"new-project"));
    OK(sb_app_request(&d.model,SB_ACT_WORKSPACE,workspace)); frame(&d);
    CHECK(d.model.has_project && !strcmp(d.model.project.id,"projekt"));
    sb_desktop_free(&d); printf("%u first-start assertions passed through keyboard events and actual project changes.\n",checks); return 0;
}
