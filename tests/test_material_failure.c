#include "ui.h"
#include "platform.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"MATERIAL FAILURE %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
void sb_test_space_fault(unsigned);
#include "material_fixture.h"
static bool color_at(int x,int y,unsigned char r,unsigned char g,unsigned char b){
    if(!frame_pixels || x<0 || y<0 || x>=frame_pixels->w || y>=frame_pixels->h)return false;
    unsigned char *p=(unsigned char *)frame_pixels->pixels+y*frame_pixels->pitch+x*4;
    return p[0]==r && p[1]==g && p[2]==b && p[3]==255;
}
int main(int argc,char **argv){
    CHECK(argc==3 || (argc==4 && !strcmp(argv[3],"native")));SBUi ui;OK(sb_ui_init(&ui,argv[1],800,600,argc==3));
    printf("Renderer: %s; density %.2f\n",SDL_GetRendererName(ui.renderer),ui.density);
    const int sizes[][2]={{800,600},{1336,840},{1000,700}};
    for(unsigned size=0;size<3;++size)for(unsigned mode_at=0;mode_at<6;++mode_at)for(unsigned failure=1;failure<=6;++failure){
        unsigned mode=mode_at^1u;
        CHECK(SDL_SetWindowSize(ui.window,sizes[size][0],sizes[size][1]));CHECK(SDL_SyncWindow(ui.window));
        ui.contrast=mode>=4;ui.space.contrast=ui.contrast;ui.space.solid=mode==2 || mode==3;sb_ui_theme(&ui,mode%2==0);ui.space.dark=ui.dark;
        struct nk_rect modal=nk_rect(60.5f,80.5f,sizes[size][0]-140.5f,sizes[size][1]-170.5f),inside=nk_rect(modal.x+24,modal.y+24,modal.w-48,modal.h-48);
        sb_space_free(&ui.space);ui.space.dark=ui.dark;ui.space.contrast=ui.contrast;ui.space.solid=mode==2 || mode==3;sb_test_space_fault(failure);
        paint(&ui,modal,false,false,true);CHECK(frame_pixels && !ui.material_ready);
        uint64_t foreground=region(&ui,inside);CHECK(foreground);
        /* The uncovered sky respects the selected theme, even without texture. */
        CHECK(color_at(0,0,ui.contrast ? ui.dark ? 0 : 255 : ui.dark ? 7 : 211,
                          ui.contrast ? ui.dark ? 0 : 255 : ui.dark ? 14 : 224,
                          ui.contrast ? ui.dark ? 0 : 255 : ui.dark ? 26 : 239));
        int logical_w,logical_h;SDL_GetWindowSize(ui.window,&logical_w,&logical_h);
        int card_x=30*frame_pixels->w/logical_w,card_y=40*frame_pixels->h/logical_h;
        CHECK(color_at(card_x,card_y,ui.contrast ? ui.dark ? 0 : 255 : ui.dark ? 17 : 237,
                                   ui.contrast ? ui.dark ? 0 : 255 : ui.dark ? 29 : 245,
                                   ui.contrast ? ui.dark ? 0 : 255 : ui.dark ? 47 : 255));
        if(failure<=4)CHECK(!ui.space.texture && !ui.space.pixels && !ui.space.base && !ui.space.sky && !ui.space.width && !ui.space.height);
        sb_test_space_fault(failure);paint(&ui,modal,true,false,true);CHECK(!ui.material_ready && foreground==region(&ui,inside));
        if(!size && (mode==0 || mode==1) && failure==1){char path[SB_PATH_CAP];OK(sb_path_join(path,sizeof(path),argv[2],mode ? "fallback-light.bmp" : "fallback-dark.bmp"));CHECK(SDL_SaveBMP(frame_pixels,path));}
        sb_test_space_fault(failure);paint(&ui,modal,true,false,false);CHECK(!ui.material_ready && foreground!=region(&ui,inside));
        sb_test_space_fault(0);paint(&ui,modal,true,false,true);CHECK(ui.material_ready && ui.space.texture && ui.space.pixels && ui.space.base && ui.space.sky);
        if(!ui.contrast)CHECK(foreground!=region(&ui,inside));
    }
    sb_test_space_fault(0);SDL_DestroySurface(frame_pixels);frame_pixels=NULL;sb_ui_shutdown(&ui);
    printf("%u material allocation/upload/draw/fallback/recovery assertions passed.\n",checks);return 0;
}
