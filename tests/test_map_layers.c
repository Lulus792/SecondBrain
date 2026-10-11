#include "desktop.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"MAP LAYERS %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
static void frame(SBDesktop *d,const char *focus){
    nk_input_begin(d->ui.ctx);SDL_Event e;while(SDL_PollEvent(&e))sb_desktop_event(d,&e);nk_input_end(d->ui.ctx);
    d->keyboard=true;snprintf(d->focus,sizeof(d->focus),"%s",focus);sb_desktop_frame(d);sb_ui_draw(&d->ui);SDL_RenderPresent(d->ui.renderer);
}
static uint64_t region(SBDesktop *d,struct nk_rect area){
    SDL_Surface *raw=SDL_RenderReadPixels(d->ui.renderer,NULL),*pixels=raw ? SDL_ConvertSurface(raw,SDL_PIXELFORMAT_RGBA32) : NULL;
    if(!pixels){SDL_DestroySurface(raw);return 0;}int w,h;SDL_GetWindowSize(d->ui.window,&w,&h);
    int x=(int)ceilf(area.x*pixels->w/w),y=(int)ceilf(area.y*pixels->h/h),right=(int)floorf((area.x+area.w)*pixels->w/w),bottom=(int)floorf((area.y+area.h)*pixels->h/h);
    uint64_t hash=1;if(x<0 || y<0 || right>pixels->w || bottom>pixels->h || right<=x || bottom<=y)hash=0;
    else for(int row=y;row<bottom;++row)hash=hash*1099511628211ULL^sb_hash((char *)pixels->pixels+row*pixels->pitch+x*4,(size_t)(right-x)*4);
    SDL_DestroySurface(pixels);SDL_DestroySurface(raw);return hash;
}
static bool has_map(SBDesktop *d){for(size_t i=0;i<d->target_count;++i)if(!strcmp(d->targets[i].id,"galaxy"))return true;return false;}
int main(int argc,char **argv){
    CHECK(argc==3);char root[SB_PATH_CAP],suffix[80];snprintf(suffix,sizeof(suffix),"map-layers-%lu",sb_process_id());OK(sb_path_join(root,sizeof(root),argv[2],suffix));
    SBDesktop d;OK(sb_desktop_init(&d,root,argv[1],true));OK(sb_app_new_project(&d.model,"review","Review",NULL));OK(sb_app_new_note(&d.model,"knowledge","note","Note"));
    strcpy(d.model.editor,"# Note\n\nText remains readable.\n");OK(sb_app_save(&d.model));d.card=true;d.reduced_motion=true;frame(&d,"galaxy");frame(&d,"galaxy");
    for(unsigned scale=0;scale<2;++scale)for(unsigned compact=0;compact<2;++compact)for(unsigned mode=0;mode<4;++mode){
        unsigned light=mode%2;d.ui.contrast=mode>=2;
        OK(sb_ui_fonts(&d.ui,scale ? 2 : 1));CHECK(SDL_SetWindowSize(d.ui.window,compact ? 780 : 1336,compact ? 640 : 840));CHECK(SDL_SyncWindow(d.ui.window));sb_ui_theme(&d.ui,!light);d.card=true;d.browser=false;d.expanded=false;
        frame(&d,"project-picker");frame(&d,"project-picker");SDL_FRect card=d.ui.card_bounds;
        struct nk_rect strip=nk_rect(card.x+40,d.map_bounds.y+d.map_bounds.h-3,fminf(80,card.w-80),6);
        struct nk_rect exposed=nk_rect(d.map_bounds.x,d.map_bounds.y+d.map_bounds.h/2,5,8);
        int window_w,window_h;SDL_GetWindowSize(d.ui.window,&window_w,&window_h);
        struct nk_rect camera_strip=nk_rect(48,window_h-80*d.ui.scale+8,60,6);uint64_t camera_before=region(&d,camera_strip);
        uint64_t free_before=region(&d,exposed),before=region(&d,strip);CHECK(before && free_before);frame(&d,"galaxy");uint64_t after=region(&d,strip);
        if(before!=after)fprintf(stderr,"Card focus leak at scale=%u compact=%u light=%u\n",scale,compact,light);
        CHECK(before==after && has_map(&d));
        CHECK(free_before!=region(&d,exposed));
        if(!scale && !compact && !mode){char picture[SB_PATH_CAP];OK(sb_path_join(picture,sizeof(picture),root,"map-card-focus.bmp"));OK(sb_ui_capture(&d.ui,picture));printf("Card snapshot: %s\n",picture);}
        if(scale && camera_before!=region(&d,camera_strip))fprintf(stderr,"Camera difference compact=%u mode=%u strip=%.1f,%.1f\n",compact,mode,camera_strip.x,camera_strip.y);
        if(scale)CHECK(camera_before && camera_before==region(&d,camera_strip));
        d.browser=true;frame(&d,"project-picker");struct nk_rect list_strip=nk_rect(48,d.map_bounds.y+d.map_bounds.h-3,90,6);before=region(&d,list_strip);frame(&d,"galaxy");CHECK(before && before==region(&d,list_strip));
    }
    d.browser=true;d.editing=true;d.expanded=false;frame(&d,"editor");CHECK(!has_map(&d));
    d.editing=false;d.browser=false;d.expanded=true;frame(&d,"reader");CHECK(!has_map(&d));
    /* Tab can reach the map while search has temporarily made room for it.
       Search dismissal restores the expanded reader and must keep traversal
       there rather than restarting at the toolbar. */
    d.search_session=true;d.search_browser_before=false;d.search_expanded_before=true;
    d.browser=true;d.expanded=false;frame(&d,"galaxy");
    CHECK(!d.search_session && !d.browser && d.expanded && !strcmp(d.focus,"reader"));
    SDL_Event key={0};key.type=SDL_EVENT_KEY_DOWN;key.key.windowID=SDL_GetWindowID(d.ui.window);key.key.key=SDLK_F6;key.key.mod=SDL_KMOD_SHIFT;key.key.down=true;
    sb_desktop_event(&d,&key);CHECK(!strcmp(d.focus,"galaxy"));frame(&d,"galaxy");CHECK(!d.card && has_map(&d));
    char snapshot[SB_PATH_CAP];OK(sb_path_join(snapshot,sizeof(snapshot),root,"map-focus.bmp"));OK(sb_ui_capture(&d.ui,snapshot));printf("Map snapshot: %s\n",snapshot);
    sb_desktop_free(&d);printf("%u foreground-card/focus/scale/list/F6 assertions passed.\n",checks);return 0;
}
