#include "ui.h"
#include "platform.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"MATERIAL %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
#include "material_fixture.h"
int main(int argc,char **argv){
    CHECK(argc==3 || (argc==4 && !strcmp(argv[3],"native")));SBUi ui;OK(sb_ui_init(&ui,argv[1],800,600,argc==3));
    printf("Renderer: %s; density %.2f\n",SDL_GetRendererName(ui.renderer),ui.density);
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
        if(!size && !scale && !mode){char path[SB_PATH_CAP];OK(sb_path_join(path,sizeof(path),argv[2],"material-panel.bmp"));CHECK(frame_pixels && SDL_SaveBMP(frame_pixels,path));}
    }
    SDL_DestroySurface(frame_pixels);frame_pixels=NULL;sb_ui_shutdown(&ui);printf("%u material/background-text/foreground/scale/resize/mode pixel assertions passed.\n",checks);return 0;
}
