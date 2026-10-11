#include "desktop.h"
#include "platform.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"READER SCROLL %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
/* Layout-only frames keep the worker race observable without adding a timer
   or changing the production worker. Rendering happens at the final snapshot. */
static void frame(SBDesktop *d){nk_input_begin(d->ui.ctx);SDL_Event e;while(SDL_PollEvent(&e))sb_desktop_event(d,&e);nk_input_end(d->ui.ctx);sb_desktop_tick(d,1.0f/60);sb_desktop_frame(d);nk_clear(d->ui.ctx);sb_desktop_apply(d);}
static bool loading(SBDesktop *d){for(size_t i=0;i<d->passive_count;++i)if(!strcmp(d->passive[i].id,"reader:preparing"))return true;return false;}
static void key(SBDesktop *d,SDL_Keycode key){SDL_Event e={0};e.type=SDL_EVENT_KEY_DOWN;e.key.windowID=SDL_GetWindowID(d->ui.window);e.key.down=true;e.key.key=key;sb_desktop_event(d,&e);frame(d);}
static void wheel(SBDesktop *d,float amount,bool layout){
    SDL_Event e={0};e.type=SDL_EVENT_MOUSE_WHEEL;e.wheel.windowID=SDL_GetWindowID(d->ui.window);e.wheel.y=amount;
    for(size_t i=0;i<d->target_count;++i)if(!strcmp(d->targets[i].id,"reader")){struct nk_rect r=d->targets[i].bounds;e.wheel.mouse_x=r.x+r.w/2;e.wheel.mouse_y=r.y+r.h/2;break;}
    sb_desktop_event(d,&e);if(layout)frame(d);
}
static bool ready(SBDesktop *d){Uint64 limit=SDL_GetTicksNS()+UINT64_C(15000000000);while(loading(d) && SDL_GetTicksNS()<limit){SDL_Delay(1);frame(d);}return !loading(d);}
static int capture(SBDesktop *d,const char *root,const char *name){
    nk_input_begin(d->ui.ctx);nk_input_end(d->ui.ctx);sb_desktop_tick(d,1.0f/60);sb_desktop_frame(d);sb_ui_draw(&d->ui);SDL_RenderPresent(d->ui.renderer);sb_desktop_apply(d);
    char path[SB_PATH_CAP];OK(sb_path_join(path,sizeof(path),root,name));OK(sb_ui_capture(&d->ui,path));printf("Reader scroll snapshot: %s\n",path);return 0;
}
static int exercise(SBDesktop *d,unsigned slot){
    strcpy(d->focus,"reader");d->keyboard=true;CHECK(loading(d));
    float step=slot==2 ? 250 : 220;
    if(slot==2)wheel(d,-5,true);else key(d,SDLK_PAGEDOWN);CHECK(d->scrolling[slot].pending+d->scrolling[slot].destination>=step);
    if(slot==2)wheel(d,-5,true);else key(d,SDLK_PAGEDOWN);CHECK(d->scrolling[slot].pending+d->scrolling[slot].destination>=2*step);
    if(slot==2)wheel(d,5,true);else key(d,SDLK_PAGEUP);CHECK(fabsf(d->scrolling[slot].pending+d->scrolling[slot].destination-step)<1);
    CHECK(ready(d));frame(d);CHECK(d->scrolling[slot].maximum>1000 && fabsf(d->scrolling[slot].position-step)<1 && !d->scrolling[slot].pending);
    return 0;
}
int main(int argc,char **argv){
    CHECK(argc==3);char root[SB_PATH_CAP],suffix[80];snprintf(suffix,sizeof(suffix),"reader-scroll-%lu",sb_process_id());OK(sb_path_join(root,sizeof(root),argv[2],suffix));
    SBDesktop d;OK(sb_desktop_init(&d,root,argv[1],true));OK(sb_app_new_project(&d.model,"scroll","Scroll",NULL));OK(sb_app_new_note(&d.model,"knowledge","long","Long"));
    char *text=calloc(1,100000);CHECK(text);strcpy(text,"# Long\n\n");for(unsigned i=0;i<300;++i)strcat(text,"Absatz mit **Wissen**, אבג ببب und 👩‍💻. Weitere Wörter für eine umgebrochene Zeile und ein längeres Dokument.\n\n");
    free(d.model.editor);d.model.editor=malloc(strlen(text)+1);CHECK(d.model.editor);strcpy(d.model.editor,text);OK(sb_app_save(&d.model));d.card=d.expanded=true;d.reduced_motion=true;frame(&d);CHECK(!exercise(&d,0));CHECK(!capture(&d,root,"reader-ready.bmp"));
    d.context=malloc(strlen(text)+1);CHECK(d.context);strcpy(d.context,text);d.form=SB_FORM_CONTEXT;frame(&d);CHECK(!exercise(&d,2));CHECK(!capture(&d,root,"context-ready.bmp"));
    d.form=SB_FORM_NONE;frame(&d);CHECK(ready(&d));
    OK(sb_app_new_note(&d.model,"knowledge","short","Short"));frame(&d);CHECK(!loading(&d));
    OK(sb_app_request(&d.model,SB_ACT_NOTE,"knowledge/long.md"));frame(&d);CHECK(loading(&d));strcpy(d.focus,"reader");d.keyboard=true;
    wheel(&d,-5,false);CHECK(d.scrolling[0].pending==250);
    OK(sb_app_request(&d.model,SB_ACT_NOTE,"knowledge/short.md"));frame(&d);CHECK(!loading(&d) && !d.scrolling[0].pending && d.scrolling[0].destination==0);
    OK(sb_app_request(&d.model,SB_ACT_NOTE,"knowledge/long.md"));frame(&d);CHECK(loading(&d));wheel(&d,-1000000,true);
    CHECK(loading(&d) || d.scrolling[0].position<=d.scrolling[0].maximum);
    CHECK(ready(&d));CHECK(d.scrolling[0].position<=d.scrolling[0].maximum);frame(&d);CHECK(d.scrolling[0].position==d.scrolling[0].maximum && !d.scrolling[0].pending);
    float bottom=d.scrolling[0].position;d.reduced_motion=false;wheel(&d,5,true);CHECK(d.scrolling[0].position<bottom && d.scrolling[0].position>bottom-250);
    for(unsigned i=0;i<60;++i)frame(&d);CHECK(fabsf(d.scrolling[0].position-(bottom-250))<1);
    free(text);sb_desktop_free(&d);printf("%u reader early-scroll/direction/readiness/context assertions passed.\n",checks);return 0;
}
