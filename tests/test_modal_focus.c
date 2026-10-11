#include "desktop.h"
#include "platform.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"MODAL FOCUS %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
static void frame(SBDesktop *d,bool keyboard){
    nk_input_begin(d->ui.ctx);SDL_Event e;while(SDL_PollEvent(&e))sb_desktop_event(d,&e);nk_input_end(d->ui.ctx);
    d->keyboard=keyboard;strcpy(d->focus,"reader");sb_desktop_frame(d);sb_ui_draw(&d->ui);SDL_RenderPresent(d->ui.renderer);sb_desktop_apply(d);
}
static struct nk_rect reader(SBDesktop *d){for(size_t i=0;i<d->target_count;++i)if(!strcmp(d->targets[i].id,"reader"))return d->targets[i].bounds;return nk_rect(0,0,0,0);}
static struct nk_rect strip(SBDesktop *d,struct nk_rect r){float inset=d->ui.contrast ? 4*d->ui.scale : 0;return nk_rect(r.x+r.w-inset-2,r.y+r.h/2-10,4,20);}
static uint64_t pixels(SBDesktop *d,struct nk_rect r){
    SDL_Surface *raw=SDL_RenderReadPixels(d->ui.renderer,NULL),*s=raw ? SDL_ConvertSurface(raw,SDL_PIXELFORMAT_RGBA32) : NULL;
    if(!s){SDL_DestroySurface(raw);return 0;}int w,h;SDL_GetWindowSize(d->ui.window,&w,&h);
    int x=(int)ceilf(r.x*s->w/w),y=(int)ceilf(r.y*s->h/h),right=(int)floorf((r.x+r.w)*s->w/w),bottom=(int)floorf((r.y+r.h)*s->h/h);uint64_t hash=1;
    if(x<0 || y<0 || right>s->w || bottom>s->h || right<=x || bottom<=y)hash=0;
    else for(int row=y;row<bottom;++row)hash=hash*1099511628211ULL^sb_hash((char *)s->pixels+row*s->pitch+x*4,(size_t)(right-x)*4);
    SDL_DestroySurface(s);SDL_DestroySurface(raw);return hash;
}
static int capture(SBDesktop *d,const char *root,const char *name){char path[SB_PATH_CAP];OK(sb_path_join(path,sizeof(path),root,name));OK(sb_ui_capture(&d->ui,path));printf("Modal focus snapshot: %s\n",path);return 0;}
int main(int argc,char **argv){
    CHECK(argc==3);char root[SB_PATH_CAP],suffix[80];snprintf(suffix,sizeof(suffix),"modal-focus-%lu",sb_process_id());OK(sb_path_join(root,sizeof(root),argv[2],suffix));
    SBDesktop d;OK(sb_desktop_init(&d,root,argv[1],true));OK(sb_app_new_project(&d.model,"focus","Focus",NULL));OK(sb_app_new_note(&d.model,"knowledge","note","Note"));
    strcpy(d.model.editor,"# Note\n\nReadable document.\n");OK(sb_app_save(&d.model));d.card=true;d.reduced_motion=true;
    d.context=malloc(80);d.notice=malloc(80);CHECK(d.context && d.notice);strcpy(d.context,"# Context\n\nA separate reading task.\n");strcpy(d.notice,"Original license text for this isolated test.\n");
    for(unsigned scale=0;scale<2;++scale)for(unsigned mode=0;mode<6;++mode){
        OK(sb_ui_fonts(&d.ui,scale ? 2 : 1));CHECK(SDL_SetWindowSize(d.ui.window,scale ? 1920 : 1336,scale ? 1080 : 840) && SDL_SyncWindow(d.ui.window));
        d.ui.contrast=mode>=4;d.solid=mode==2 || mode==3;sb_ui_theme(&d.ui,mode%2==0);d.form=SB_FORM_NONE;d.card=true;d.expanded=false;
        frame(&d,false);frame(&d,false);struct nk_rect background=strip(&d,reader(&d));uint64_t normal=pixels(&d,background);CHECK(normal);
        frame(&d,true);CHECK(pixels(&d,background)!=normal); /* Still visible outside a modal task. */
        SBForm forms[]={SB_FORM_CONTEXT,SB_FORM_NOTICE_TEXT};
        for(size_t i=0;i<2;++i){
            d.form=forms[i];frame(&d,false);frame(&d,false);struct nk_rect foreground=strip(&d,reader(&d));CHECK(foreground.w>0 && background.x>d.modal_bounds.x+d.modal_bounds.w);
            uint64_t behind=pixels(&d,background),active=pixels(&d,foreground);CHECK(behind && active);
            frame(&d,true);CHECK(!strcmp(d.focus,"reader"));
            for(size_t n=0;n<d.target_count;++n)CHECK(d.targets[n].group==3);
            CHECK(pixels(&d,foreground)!=active); /* Active reader retains its ring. */
            if(!scale && !mode && !i)CHECK(!capture(&d,root,"context-focus.bmp"));
            if(pixels(&d,background)!=behind)fprintf(stderr,"Background ring changed: scale %u mode %u form %d\n",scale,mode,forms[i]);
            CHECK(pixels(&d,background)==behind);
            SDL_Event e={0};e.type=SDL_EVENT_KEY_DOWN;e.key.key=SDLK_ESCAPE;e.key.down=true;sb_desktop_event(&d,&e);frame(&d,true);frame(&d,true);CHECK(d.form==SB_FORM_NONE && !strcmp(d.focus,"reader"));CHECK(pixels(&d,background)!=normal);
        }
    }
    sb_desktop_free(&d);printf("%u modal focus/background/foreground/return/scale/theme assertions passed.\n",checks);return 0;
}
