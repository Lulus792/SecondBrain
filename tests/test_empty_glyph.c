#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"EMPTY GLYPH %d: %s (%s)\n",__LINE__,#x,SDL_GetError());return 1;}}while(0)
int main(int argc,char **argv){CHECK(argc==2);CHECK(SDL_Init(0));CHECK(TTF_Init());
    for(unsigned size=0;size<3;++size){TTF_Font *font=TTF_OpenFont(argv[1],18+9*size);CHECK(font);const char *texts[]={"   "," A ","AV ffi ","\t A\t"};
        for(unsigned i=0;i<4;++i){SDL_Surface *surface=TTF_RenderText_Blended(font,texts[i],0,(SDL_Color){230,240,250,180});CHECK(surface && surface->w>0 && surface->h>0);if(i==0){SDL_Surface *rgba=SDL_ConvertSurface(surface,SDL_PIXELFORMAT_RGBA32);CHECK(rgba);for(int y=0;y<rgba->h;++y){Uint8 *p=(Uint8 *)rgba->pixels+y*rgba->pitch;for(int x=0;x<rgba->w;++x)CHECK(p[x*4+3]==0);}SDL_DestroySurface(rgba);}SDL_DestroySurface(surface);}
        TTF_CloseFont(font);
    }TTF_Quit();SDL_Quit();printf("%u empty glyph bitmap assertions passed.\n",checks);return 0;
}
