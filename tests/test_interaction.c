#include "desktop.h"
#include "platform.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
static unsigned checks;
#define CHECK(x) do {++checks;if(!(x)){fprintf(stderr,"INTERACTION %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
static void frame(SBDesktop *d) {
    nk_input_begin(d->ui.ctx);SDL_Event e;while(SDL_PollEvent(&e))sb_desktop_event(d,&e);
    sb_desktop_tick(d,1.0f/60);nk_input_end(d->ui.ctx);sb_desktop_frame(d);sb_ui_draw(&d->ui);SDL_RenderPresent(d->ui.renderer);sb_desktop_apply(d);
}
static uint64_t pixels(SBUi *ui,SDL_Texture *texture) {
    SDL_Texture *target=NULL;
    if(texture) {
        float w,h;if(!SDL_GetTextureSize(texture,&w,&h))return 0;
        target=SDL_CreateTexture(ui->renderer,SDL_PIXELFORMAT_RGBA32,SDL_TEXTUREACCESS_TARGET,(int)w,(int)h);
        if(!target || !SDL_SetRenderTarget(ui->renderer,target)){SDL_DestroyTexture(target);return 0;}
        SDL_SetRenderLogicalPresentation(ui->renderer,(int)w,(int)h,SDL_LOGICAL_PRESENTATION_DISABLED);
        SDL_RenderTexture(ui->renderer,texture,NULL,NULL);
    }
    SDL_Surface *raw=SDL_RenderReadPixels(ui->renderer,NULL),*view=raw ? SDL_ConvertSurface(raw,SDL_PIXELFORMAT_RGBA32) : NULL;
    uint64_t hash=0;if(view)for(int y=0;y<view->h;++y)hash=hash*1099511628211ULL^sb_hash((const char *)view->pixels+y*view->pitch,(size_t)view->w*4);
    SDL_DestroySurface(view);SDL_DestroySurface(raw);SDL_SetRenderTarget(ui->renderer,NULL);SDL_DestroyTexture(target);return hash;
}

static SBTarget *find(SBDesktop *d,const char *id) {for(size_t i=0;i<d->target_count;++i)if(!strcmp(d->targets[i].id,id))return &d->targets[i];return NULL;}
static SBStatus capture(SBDesktop *d,const char *root,const char *name) {char path[SB_PATH_CAP];SBStatus s=sb_path_join(path,sizeof(path),root,name);return s.code==SB_OK ? sb_ui_capture(&d->ui,path) : s;}
int main(int argc,char **argv) {
    CHECK(argc==3);char root[SB_PATH_CAP],name[64];snprintf(name,sizeof(name),"interaction-%lu-%lu",sb_process_id(),(unsigned long)time(NULL));OK(sb_path_join(root,sizeof(root),argv[2],name));
    SBDesktop d;OK(sb_desktop_init(&d,root,argv[1],true));OK(sb_app_new_project(&d.model,"test","UI-Prüfung",NULL));
    OK(sb_app_new_note(&d.model,"knowledge","a","Dokument A"));strcpy(d.model.editor,"# Dokument A\n\nAusgehender Inhalt.\n");OK(sb_app_save(&d.model));
    OK(sb_app_new_note(&d.model,"knowledge","b","Dokument B"));strcpy(d.model.editor,"# Dokument B\n\n");
    for(unsigned i=0;i<80;++i)strcat(d.model.editor,"Absatz zum flüssigen Scrollen mit sichtbarer Struktur.\n\n");OK(sb_app_save(&d.model));
    OK(sb_app_request(&d.model,SB_ACT_NOTE,"knowledge/a.md"));frame(&d);frame(&d);
    uint64_t previous=pixels(&d.ui,NULL);CHECK(previous);OK(capture(&d,root,"transition-old.bmp"));
    d.navigation.kind=SB_ACT_NOTE;strcpy(d.navigation.value,"knowledge/b.md");d.follow_star=true;CHECK(sb_desktop_animating(&d));
    d.ui.capture_pending=true;frame(&d);CHECK(!strcmp(d.model.path,"knowledge/b.md") && d.ui.transitioning && d.flight==0);
    CHECK(pixels(&d.ui,d.ui.outgoing_texture)==previous);
    float distance=sqrtf(powf(d.flight_to[0]-d.flight_from[0],2)+powf(d.flight_to[1]-d.flight_from[1],2));CHECK(distance>10);
    frame(&d);float moved=sqrtf(powf(d.focus_x-d.flight_from[0],2)+powf(d.focus_y-d.flight_from[1],2));CHECK(moved/distance>0.05f);
    OK(capture(&d,root,"transition-start.bmp"));for(unsigned i=0;i<4;++i)frame(&d);OK(capture(&d,root,"transition-middle.bmp"));
    for(unsigned i=0;i<30;++i)frame(&d);CHECK(!d.ui.transitioning && d.flight==1);OK(capture(&d,root,"transition-end.bmp"));
    CHECK(d.scrolling[0].maximum>100);d.scrolling[0].pending=7;frame(&d);CHECK(d.scrolling[0].position>0 && d.scrolling[0].position<7 && fabsf(d.scrolling[0].fractional)>0.01f);
    float first=d.scrolling[0].position;frame(&d);CHECK(d.scrolling[0].position>first);
    for(unsigned i=0;i<30;++i)frame(&d);CHECK(d.scrolling[0].position==7);
    SBScroll *scroll=&d.scrolling[0];SDL_Event e={0};e.type=SDL_EVENT_MOUSE_BUTTON_DOWN;e.button.button=SDL_BUTTON_LEFT;e.button.windowID=SDL_GetWindowID(d.ui.window);e.button.x=scroll->thumb.x+2;e.button.y=scroll->thumb.y+scroll->thumb.h/2;sb_desktop_event(&d,&e);CHECK(scroll->dragging);
    scroll->pending=31;float range=scroll->track.h-scroll->thumb.h;e.type=SDL_EVENT_MOUSE_MOTION;e.motion.windowID=SDL_GetWindowID(d.ui.window);e.motion.x=scroll->thumb.x;e.motion.y=scroll->track.y+scroll->grab+range*0.75f;sb_desktop_event(&d,&e);
    CHECK(scroll->pending==0 && fabsf(scroll->position-scroll->maximum*0.75f)<0.01f);frame(&d);CHECK(fabsf(scroll->position-scroll->maximum*0.75f)<0.6f);
    e.type=SDL_EVENT_MOUSE_BUTTON_UP;e.button.button=SDL_BUTTON_LEFT;sb_desktop_event(&d,&e);CHECK(!scroll->dragging);
    d.form=SB_FORM_FILTER;frame(&d);frame(&d);frame(&d);CHECK(d.modal_bounds.w<400 && d.modal_bounds.h<400);
    float last=0;for(size_t i=0;i<d.target_count;++i)if(!strncmp(d.targets[i].id,"section:",8)){SBTarget *t=&d.targets[i];CHECK(t->bounds.y>=d.modal_bounds.y && t->bounds.y+t->bounds.h<d.modal_bounds.y+d.modal_bounds.h);last=fmaxf(last,t->bounds.y+t->bounds.h);}
    OK(capture(&d,root,"filter.bmp"));CHECK(last>0 && d.modal_bounds.y+d.modal_bounds.h-last<45);
    d.form=SB_FORM_ACTIONS;frame(&d);frame(&d);frame(&d);CHECK(d.modal_bounds.w<450 && find(&d,"help-actions"));OK(capture(&d,root,"actions.bmp"));
    d.form=SB_FORM_NONE;d.search[0]=0;frame(&d);frame(&d);CHECK(!find(&d,"clear-search"));OK(capture(&d,root,"search-empty.bmp"));
    strcpy(d.search,"Dokument");frame(&d);frame(&d);CHECK(find(&d,"clear-search"));OK(capture(&d,root,"search-filled.bmp"));
    SBTarget *target=find(&d,"settings");CHECK(target);e.type=SDL_EVENT_MOUSE_MOTION;e.motion.windowID=SDL_GetWindowID(d.ui.window);e.motion.x=target->bounds.x+target->bounds.w/2;e.motion.y=target->bounds.y+target->bounds.h/2;
    nk_input_begin(d.ui.ctx);sb_desktop_event(&d,&e);nk_input_end(d.ui.ctx);frame(&d);CHECK(d.hover_label[0]);d.hover_started=SDL_GetTicksNS()-400000000u;frame(&d);OK(capture(&d,root,"hover.bmp"));
    target=find(&d,"edit");CHECK(target);e.motion.x=target->bounds.x+target->bounds.w/2;e.motion.y=target->bounds.y+target->bounds.h/2;
    nk_input_begin(d.ui.ctx);sb_desktop_event(&d,&e);nk_input_end(d.ui.ctx);frame(&d);CHECK(!strcmp(d.hover_label,"Bearbeiten"));d.hover_started=SDL_GetTicksNS()-400000000u;frame(&d);OK(capture(&d,root,"hover-shortcut.bmp"));
    CHECK(d.tooltip_bounds.x>=0 && d.tooltip_bounds.x+d.tooltip_bounds.w<=1336 && d.tooltip_bounds.y>=0 && d.tooltip_bounds.y+d.tooltip_bounds.h<=840);
    SDL_Surface *hint_surface=SDL_RenderReadPixels(d.ui.renderer,NULL);CHECK(hint_surface);Uint8 red,green,blue,alpha;
    CHECK(SDL_ReadSurfacePixel(hint_surface,(int)((d.tooltip_bounds.x+5)*hint_surface->w/1336),(int)((d.tooltip_bounds.y+5)*hint_surface->h/840),&red,&green,&blue,&alpha));
    if(red!=22 || green!=34 || blue!=51)fprintf(stderr,"Hint margin RGB %u %u %u, expected 22 34 51\n",red,green,blue);CHECK(red==22 && green==34 && blue==51);SDL_DestroySurface(hint_surface);
    e.type=SDL_EVENT_MOUSE_BUTTON_DOWN;e.button.windowID=SDL_GetWindowID(d.ui.window);e.button.button=SDL_BUTTON_LEFT;e.button.down=true;e.button.x=target->bounds.x+target->bounds.w/2;e.button.y=target->bounds.y+target->bounds.h/2;
    CHECK(SDL_PushEvent(&e));frame(&d);
    e.type=SDL_EVENT_MOUSE_BUTTON_UP;e.button.down=false;CHECK(SDL_PushEvent(&e));frame(&d);CHECK(d.editing && !sb_app_dirty(&d.model));
    d.editing=false;d.expanded=false;frame(&d);
    for(unsigned scale=0;scale<2;++scale) {
        OK(sb_ui_fonts(&d.ui,scale ? 2 : 1));CHECK(SDL_SetWindowSize(d.ui.window,scale ? 780 : 1336,scale ? 640 : 840));CHECK(SDL_SyncWindow(d.ui.window));
        SBForm forms[]={SB_FORM_FILTER,SB_FORM_ACTIONS,SB_FORM_PROJECTS,SB_FORM_NOTE,SB_FORM_PROJECT,SB_FORM_WORKSPACE,SB_FORM_SETTINGS,SB_FORM_ABOUT,SB_FORM_HELP,SB_FORM_BACKUP,SB_FORM_RESTORE,SB_FORM_NOTICE_LIST};
        for(size_t f=0;f<sizeof(forms)/sizeof(*forms);++f) {d.form=forms[f];frame(&d);frame(&d);frame(&d);CHECK(d.modal_bounds.w<= (scale ? 740 : 1296) && d.modal_bounds.h<=(scale ? 600 : 800));char artifact[64];snprintf(artifact,sizeof(artifact),"modal-%u-%u.bmp",scale,(unsigned)forms[f]);OK(capture(&d,root,artifact));}
    }
    d.form=SB_FORM_NONE;OK(sb_ui_fonts(&d.ui,1));CHECK(SDL_SetWindowSize(d.ui.window,1336,840));CHECK(SDL_SyncWindow(d.ui.window));frame(&d);frame(&d);
    d.navigation.kind=SB_ACT_NOTE;strcpy(d.navigation.value,"knowledge/a.md");d.follow_star=true;d.ui.capture_pending=true;frame(&d);frame(&d);CHECK(d.ui.transitioning);
    for(unsigned repeat=0;repeat<12;++repeat) {
        const char *path=repeat%2 ? "knowledge/a.md" : "knowledge/b.md";
        d.navigation.kind=SB_ACT_NOTE;strcpy(d.navigation.value,path);d.follow_star=true;d.ui.capture_pending=true;frame(&d);
        CHECK(!strcmp(d.model.path,path) && !sb_app_dirty(&d.model) && d.ui.transitioning);frame(&d);
    }
    e.type=SDL_EVENT_MOUSE_BUTTON_DOWN;e.button.windowID=SDL_GetWindowID(d.ui.window);e.button.button=SDL_BUTTON_LEFT;e.button.x=d.ui.card_bounds.x+30;e.button.y=d.ui.card_bounds.y+30;sb_desktop_event(&d,&e);CHECK(!d.ui.transitioning);
    sb_desktop_set_style(&d,(SBStyleChoice){.dark=true,.motion=true});frame(&d);CHECK(!d.ui.transitioning && d.flight==1 && !sb_app_dirty(&d.model));
    sb_desktop_free(&d);printf("%u interaction assertions passed. Artifacts: %s\n",checks,root);return 0;
}
