#include "ui.h"
#include "platform.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"MATERIAL %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
static uint64_t region(SBUi *ui,struct nk_rect area){
    SDL_Surface *raw=SDL_RenderReadPixels(ui->renderer,NULL),*pixels=raw ? SDL_ConvertSurface(raw,SDL_PIXELFORMAT_RGBA32) : NULL;
    if(!pixels){SDL_DestroySurface(raw);return 0;}int w,h;SDL_GetWindowSize(ui->window,&w,&h);
    int left=(int)ceilf(area.x*pixels->w/w),top=(int)ceilf(area.y*pixels->h/h),right=(int)floorf((area.x+area.w)*pixels->w/w),bottom=(int)floorf((area.y+area.h)*pixels->h/h);
    uint64_t hash=1;if(left<0 || top<0 || right>pixels->w || bottom>pixels->h || right<=left || bottom<=top)hash=0;
    else for(int y=top;y<bottom;++y)hash=hash*1099511628211ULL^sb_hash((char *)pixels->pixels+y*pixels->pitch+left*4,(size_t)(right-left)*4);
    SDL_DestroySurface(pixels);SDL_DestroySurface(raw);return hash;
}
static void paint(SBUi *ui,struct nk_rect modal,bool background,bool legacy,bool foreground){
    int w,h;SDL_GetWindowSize(ui->window,&w,&h);struct nk_context *ctx=ui->ctx;
    ui->space.glass_count=2;ui->space.glass[0]=(SBGlass){{10,10,w-20,h-20},16};ui->space.glass[1]=(SBGlass){{modal.x,modal.y,modal.w,modal.h},16};
    nk_input_begin(ctx);SDL_Event event;while(SDL_PollEvent(&event)){}nk_input_end(ctx);
    ctx->style.window.fixed_background=nk_style_item_color(nk_rgba(0,0,0,0));
    if(nk_begin(ctx,"Underlay",nk_rect(10,10,w-20,h-20),NK_WINDOW_NO_SCROLLBAR)){
        for(unsigned i=0;i<30;++i){nk_layout_row_dynamic(ctx,25*ui->scale,1);
            if(background)nk_label(ctx,"BACKGROUND WORDS BACKGROUND WORDS BACKGROUND WORDS BACKGROUND WORDS",NK_TEXT_LEFT);else nk_spacer(ctx);}
        if(background)nk_fill_rect(nk_window_get_canvas(ctx),nk_rect(modal.x+1,modal.y+1,3,3),0,nk_rgb(220,50,80));
    }nk_end(ctx);
    if(legacy)ctx->style.window.fixed_background=nk_style_item_color(ui->contrast ? ui->dark ? nk_rgb(0,0,0) : nk_rgb(255,255,255) : ui->dark ? nk_rgba(17,29,47,245) : nk_rgba(237,245,255,245));
    if(nk_begin(ctx,"Foreground",modal,NK_WINDOW_NO_SCROLLBAR)){
        if(!legacy)sb_ui_material_panel(ui,modal,16);
        nk_layout_row_dynamic(ctx,36*ui->scale,1);if(foreground)nk_label(ctx,"Foreground text",NK_TEXT_LEFT);else nk_spacer(ctx);
    }nk_end(ctx);ctx->style.window.fixed_background=nk_style_item_color(nk_rgba(0,0,0,0));

    sb_ui_draw(ui);SDL_RenderPresent(ui->renderer);
}
int main(int argc,char **argv){
    CHECK(argc==3);SBUi ui;OK(sb_ui_init(&ui,argv[1],800,600,true));
    const int sizes[][2]={{800,600},{1336,840},{1920,1080}};
    for(unsigned size=0;size<3;++size)for(unsigned scale=0;scale<2;++scale)for(unsigned mode=0;mode<6;++mode){
        CHECK(SDL_SetWindowSize(ui.window,sizes[size][0],sizes[size][1]));CHECK(SDL_SyncWindow(ui.window));OK(sb_ui_fonts(&ui,scale ? 2 : 1));
        ui.contrast=mode>=4;ui.space.contrast=ui.contrast;ui.space.solid=mode==2 || mode==3;sb_ui_theme(&ui,mode%2==0);ui.space.dark=ui.dark;
        struct nk_rect modal=nk_rect(60.5f,80.5f,sizes[size][0]-140.5f,sizes[size][1]-170.5f),inside=nk_rect(modal.x+24,modal.y+24,modal.w-48,modal.h-48);
        paint(&ui,modal,false,true,true);uint64_t legacy=region(&ui,inside);CHECK(legacy);
        paint(&ui,modal,true,true,true);if(!ui.contrast)CHECK(legacy!=region(&ui,inside));
        paint(&ui,modal,false,false,true);uint64_t restored=region(&ui,inside);CHECK(restored && ui.material_ready);
        struct nk_rect corner=nk_rect(modal.x+1,modal.y+1,2,2);uint64_t outside=region(&ui,corner);CHECK(outside);
        paint(&ui,modal,true,false,true);
        if(outside==region(&ui,corner)){fprintf(stderr,"Corner unchanged size=%u scale=%u mode=%u\n",size,scale,mode);}CHECK(outside!=region(&ui,corner));
        if(restored!=region(&ui,inside))fprintf(stderr,"Material mismatch size=%u scale=%u mode=%u\n",size,scale,mode);CHECK(restored==region(&ui,inside));
        paint(&ui,modal,true,false,false);CHECK(restored!=region(&ui,inside));
        if(!size && !scale && !mode){char path[SB_PATH_CAP];OK(sb_path_join(path,sizeof(path),argv[2],"material-panel.bmp"));OK(sb_ui_capture(&ui,path));}
    }
    sb_ui_shutdown(&ui);printf("%u material/background-text/foreground/scale/resize/mode pixel assertions passed.\n",checks);return 0;
}
