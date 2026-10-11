#include "space.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
bool sb_space_reference_draw(SBSpace *,SDL_Renderer *,int,int);
void sb_space_reference_free(SBSpace *);
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"SPACE RASTER %d: %s\n",__LINE__,#x);return 1;}}while(0)
static unsigned random_state=17;
static unsigned next(void){random_state=random_state*1664525u+1013904223u;return random_state;}
static int compare(SBSpace *current,SBSpace *reference,SDL_Renderer *renderer,int w,int h){
    CHECK(sb_space_reference_draw(reference,renderer,w,h));CHECK(sb_space_draw(current,renderer,w,h));
    CHECK(current->width==reference->width && current->height==reference->height);
    size_t bytes=(size_t)current->width*current->height*4;
    CHECK(!memcmp(current->sky,reference->sky,bytes));CHECK(!memcmp(current->base,reference->base,bytes));
    if(memcmp(current->pixels,reference->pixels,bytes)){
        for(size_t i=0;i<bytes;++i)if(current->pixels[i]!=reference->pixels[i]){fprintf(stderr,"pixel byte %zu: %u != %u, view %dx%d dark=%d solid=%d contrast=%d panes=%u\n",i,current->pixels[i],reference->pixels[i],w,h,current->dark,current->solid,current->contrast,current->glass_count);break;}}
    CHECK(!memcmp(current->pixels,reference->pixels,bytes));return 0;
}
static void state(SBSpace *s,unsigned mode,unsigned panes,int w,int h){
    s->dark=(mode&1)!=0;s->solid=(mode&2)!=0;s->contrast=(mode&4)!=0;
    s->mouse_x=(float)(next()%(w+1));s->mouse_y=(float)(next()%(h+1));
    s->camera[0]=17;s->camera[1]=-9;s->camera[2]=11;s->camera[3]=0.4f;s->camera[4]=-0.2f;s->camera[5]=w*0.42f;s->camera[6]=h*0.45f;s->camera[7]=0.9f;
    s->glass_count=panes;
    for(unsigned i=0;i<panes;++i){float x=(float)(next()%(w+80))-40.25f,y=(float)(next()%(h+80))-40.75f;
        float width=1+(float)(next()%(w+30)),height=1+(float)(next()%(h+30));
        s->glass[i]=(SBGlass){{x,y,width+0.3f,height+0.7f},(float)(next()%80)};}
    if(panes)s->glass[0]=(SBGlass){{w*0.12f,h*0.1f,w*0.75f,h*0.8f},16};
    if(panes>1)s->glass[1]=(SBGlass){{w*0.27f,h*0.29f,w*0.5f,h*0.5f},32.3f};
}
int main(void){
    CHECK(SDL_Init(SDL_INIT_VIDEO));SDL_Window *window=SDL_CreateWindow("Raster parity",640,480,SDL_WINDOW_HIDDEN);CHECK(window);SDL_Renderer *renderer=SDL_CreateRenderer(window,"software");CHECK(renderer);
    SBSpace current={0},reference={0};current.count=reference.count=8;current.points=calloc(8,sizeof(*current.points));reference.points=calloc(8,sizeof(*reference.points));CHECK(current.points && reference.points);
    SBEdge edges[7];for(unsigned i=0;i<7;++i)edges[i]=(SBEdge){i,i+1};SBGraph graph={.edges=edges,.edge_count=7};current.graph=reference.graph=&graph;
    const int sizes[][2]={{64,48},{317,211},{800,600},{1336,840},{1920,1200},{2672,1680},{3840,2160}};
    for(unsigned size=0;size<7;++size)for(unsigned mode=0;mode<8;++mode){int w=sizes[size][0],h=sizes[size][1];state(&current,mode,mode%3 ? 4 : 0,w,h);
        for(unsigned p=0;p<8;++p)current.points[p]=(SBPoint){.x=w*(p+1)/9.0f,.y=h*(p%3+1)/4.0f,.depth=0.7f+p/10.0f,.group=p%5,.visible=true,.selected=p==3};
        memcpy(reference.points,current.points,8*sizeof(*current.points));reference.dark=current.dark;reference.solid=current.solid;reference.contrast=current.contrast;reference.mouse_x=current.mouse_x;reference.mouse_y=current.mouse_y;memcpy(reference.camera,current.camera,sizeof(current.camera));reference.glass_count=current.glass_count;memcpy(reference.glass,current.glass,sizeof(current.glass));
        CHECK(!compare(&current,&reference,renderer,w,h));CHECK(!compare(&current,&reference,renderer,w,h));
    }
    for(unsigned i=0;i<32;++i){int w=101+(int)(next()%1100),h=91+(int)(next()%750);state(&current,i%8,i==31 ? 64 : 1+next()%8,w,h);reference.dark=current.dark;reference.solid=current.solid;reference.contrast=current.contrast;reference.mouse_x=current.mouse_x;reference.mouse_y=current.mouse_y;memcpy(reference.camera,current.camera,sizeof(current.camera));reference.glass_count=current.glass_count;memcpy(reference.glass,current.glass,sizeof(current.glass));CHECK(!compare(&current,&reference,renderer,w,h));}
    sb_space_free(&current);sb_space_reference_free(&reference);SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();printf("%u exact CPU sky/scene/glass/cache/resize pixel assertions passed.\n",checks);return 0;
}
