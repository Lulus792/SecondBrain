/* Compile the actual space implementation with narrow failure interception.
   All other SDL/UI allocations remain real and unaffected. */
#include "space.h"
#include <stdlib.h>
static unsigned fault,allocation;
void sb_test_space_fault(unsigned next){fault=next;allocation=0;}
static void *space_malloc(size_t size){if(fault>=1 && fault<=3 && ++allocation==fault)return NULL;return malloc(size);}
static SDL_Texture *space_texture(SDL_Renderer *r,SDL_PixelFormat f,SDL_TextureAccess a,int w,int h){
    if(fault==4){SDL_SetError("Injected material texture allocation failure");return NULL;}return SDL_CreateTexture(r,f,a,w,h);}
static bool space_upload(SDL_Texture *t,const SDL_Rect *rect,const void *pixels,int pitch){
    if(fault==5)return SDL_SetError("Injected material upload failure");return SDL_UpdateTexture(t,rect,pixels,pitch);}
static bool space_render(SDL_Renderer *r,SDL_Texture *t,const SDL_FRect *src,const SDL_FRect *dst){
    if(fault==6)return SDL_SetError("Injected material draw failure");return SDL_RenderTexture(r,t,src,dst);}
#define malloc space_malloc
#define SDL_CreateTexture space_texture
#define SDL_UpdateTexture space_upload
#define SDL_RenderTexture space_render
#include "../../app/space.c"
