#include "desktop.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"EXIT PREFS %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
static void frame(SBDesktop *d){
    nk_input_begin(d->ui.ctx);SDL_Event e;while(SDL_PollEvent(&e))sb_desktop_event(d,&e);nk_input_end(d->ui.ctx);
    sb_desktop_tick(d,1.0f/60);sb_desktop_frame(d);sb_ui_draw(&d->ui);SDL_RenderPresent(d->ui.renderer);sb_desktop_apply(d);sb_desktop_finish_quit(d);
}
static void key(SBDesktop *d,SDL_Keycode key){SDL_Event e={0};e.type=SDL_EVENT_KEY_DOWN;e.key.key=key;nk_input_begin(d->ui.ctx);sb_desktop_event(d,&e);nk_input_end(d->ui.ctx);frame(d);}
static SBTarget *target(SBDesktop *d,const char *id){for(size_t i=0;i<d->target_count;++i)if(!strcmp(d->targets[i].id,id))return &d->targets[i];return NULL;}
static bool inside(struct nk_rect a,struct nk_rect b){return a.x>=b.x && a.y>=b.y && a.x+a.w<=b.x+b.w+0.5f && a.y+a.h<=b.y+b.h+0.5f;}
int main(int argc,char **argv){
    CHECK(argc==3);char root[SB_PATH_CAP],name[80],good[SB_PATH_CAP],blocked[SB_PATH_CAP],bad[SB_PATH_CAP];
    snprintf(name,sizeof(name),"exit-prefs-%lu",sb_process_id());OK(sb_path_join(root,sizeof(root),argv[2],name));
    SBDesktop d;OK(sb_desktop_init(&d,root,argv[1],true));OK(sb_app_new_project(&d.model,"test","Test",NULL));OK(sb_app_new_note(&d.model,"knowledge","note","Notiz"));
    OK(sb_path_join(good,sizeof(good),root,"Einstellungen ü.conf"));OK(sb_desktop_preferences(&d,good,true));OK(sb_desktop_store_preferences(&d));
    char *old=NULL;size_t length=0;OK(sb_fs_read(good,&old,&length));uint64_t hash=sb_hash(old,length);free(old);
    OK(sb_path_join(blocked,sizeof(blocked),root,"blocked"));OK(sb_fs_write_new(blocked,"owned",5));OK(sb_path_join(bad,sizeof(bad),blocked,"settings.conf"));
    for(unsigned scale=0;scale<3;++scale){
        OK(sb_ui_fonts(&d.ui,1+0.5f*scale));CHECK(SDL_SetWindowSize(d.ui.window,780,520) && SDL_SyncWindow(d.ui.window));
        d.form=SB_FORM_NONE;d.editing=true;d.next_edit=true;d.card=true;strcpy(d.focus,"editor");frame(&d);strcpy(d.settings_path,bad);
        OK(sb_app_request(&d.model,SB_ACT_QUIT,NULL));CHECK(d.model.quit);CHECK(!sb_desktop_finish_quit(&d));frame(&d);frame(&d);
        CHECK(!d.model.quit && d.form==SB_FORM_EXIT_ERROR && d.message.code!=SB_OK && !strcmp(d.focus,"exit-retry"));
        const char *ids[]={"cancel","exit-retry","exit-skip","exit-back"};
        for(unsigned i=0;i<4;++i){SBTarget *t=target(&d,ids[i]);CHECK(t && t->group==3 && inside(t->bounds,d.modal_bounds));}
        CHECK(inside(d.modal_bounds,nk_rect(0,0,780,520)));
        key(&d,SDLK_DOWN);CHECK(!strcmp(d.focus,"exit-skip"));key(&d,SDLK_UP);CHECK(!strcmp(d.focus,"exit-retry"));
        key(&d,SDLK_RETURN);CHECK(d.form==SB_FORM_EXIT_ERROR && !d.model.quit);
        SDL_Event close={0};close.type=SDL_EVENT_QUIT;sb_desktop_event(&d,&close);frame(&d);CHECK(!d.model.quit);
        if(scale==2){char image[SB_PATH_CAP];OK(sb_path_join(image,sizeof(image),root,"error-200.bmp"));OK(sb_ui_capture(&d.ui,image));}
        OK(sb_fs_read(good,&old,&length));CHECK(sb_hash(old,length)==hash);free(old);
        key(&d,SDLK_ESCAPE);frame(&d);CHECK(d.form==SB_FORM_NONE && !d.model.quit && !strcmp(d.focus,"editor"));
    }
    /* A begun form survives cancellation with its actual return focus. */
    d.form=SB_FORM_NOTE;strcpy(d.name,"Begonnene Notiz");frame(&d);strcpy(d.focus,"form-name");frame(&d);
    OK(sb_app_request(&d.model,SB_ACT_QUIT,NULL));CHECK(!sb_desktop_finish_quit(&d));frame(&d);key(&d,SDLK_ESCAPE);frame(&d);
    CHECK(d.form==SB_FORM_NOTE && !strcmp(d.name,"Begonnene Notiz") && !strcmp(d.focus,"form-name"));
    key(&d,SDLK_F1);CHECK(d.form==SB_FORM_HELP && d.help_return);
    OK(sb_app_request(&d.model,SB_ACT_QUIT,NULL));CHECK(!sb_desktop_finish_quit(&d));frame(&d);
    CHECK(d.form==SB_FORM_EXIT_ERROR && d.help_return);
    key(&d,SDLK_ESCAPE);frame(&d);CHECK(d.form==SB_FORM_HELP && d.help_return);
    key(&d,SDLK_ESCAPE);frame(&d);CHECK(d.form==SB_FORM_NOTE && !strcmp(d.name,"Begonnene Notiz"));
    d.form=SB_FORM_SETTINGS;frame(&d);frame(&d);CHECK(d.scrolling[3].maximum>50);
    d.focus_scroll_frames=0;d.scrolling[3].position=d.scrolling[3].destination=d.scrolling[3].maximum/2;d.scrolling[3].active=true;frame(&d);
    float scroll=d.scrolling[3].position;
    OK(sb_app_request(&d.model,SB_ACT_QUIT,NULL));CHECK(!sb_desktop_finish_quit(&d));frame(&d);key(&d,SDLK_ESCAPE);frame(&d);frame(&d);
    CHECK(d.form==SB_FORM_SETTINGS && fabsf(d.scrolling[3].position-scroll)<1 && !d.scrolling[3].dragging);
    d.form=SB_FORM_NONE;frame(&d);OK(sb_app_request(&d.model,SB_ACT_QUIT,NULL));CHECK(!sb_desktop_finish_quit(&d));frame(&d);
    strcpy(d.settings_path,good);key(&d,SDLK_RETURN);CHECK(d.model.quit);
    OK(sb_fs_read(good,&old,&length));hash=sb_hash(old,length);free(old);
    d.model.quit=false;d.form=SB_FORM_NONE;strcpy(d.settings_path,bad);OK(sb_app_request(&d.model,SB_ACT_QUIT,NULL));CHECK(!sb_desktop_finish_quit(&d));frame(&d);
    key(&d,SDLK_DOWN);key(&d,SDLK_RETURN);CHECK(d.model.quit && d.preferences_skip && sb_desktop_finish_quit(&d));
    /* Successful retry intentionally updates preferences; skip must preserve it. */
    SBSettings saved;SBRevision revision;OK(sb_settings_load(good,&saved,&revision));CHECK(saved.font_percent==200);
    CHECK(sb_fs_kind(blocked)==1 && sb_fs_kind(bad)!=1);
    OK(sb_fs_read(good,&old,&length));CHECK(sb_hash(old,length)==hash);free(old);
    sb_desktop_free(&d);printf("%u quit/preferences/focus/form/layout/keyboard assertions passed.\n",checks);return 0;
}
