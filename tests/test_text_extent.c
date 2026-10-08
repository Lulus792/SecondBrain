#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"TEXT EXTENT %d: %s\n",__LINE__,#x);return 1;}}while(0)
static uint64_t draw(SBUi *ui,const char *text,bool selected,unsigned *ink) {
 nk_input_begin(ui->ctx);nk_input_end(ui->ctx);nk_begin(ui->ctx,"Extent",nk_rect(0,0,1000,640),NK_WINDOW_NO_SCROLLBAR);
 const struct nk_user_font *font=&ui->normal->handle;struct nk_command_buffer *canvas=nk_window_get_canvas(ui->ctx);
 nk_push_scissor(canvas,nk_rect(10,10,900,40));float width=font->width(font->userdata,font->height,text,(int)strlen(text));
 struct nk_rect r=nk_rect(810-width,20,width,font->height);
 if(selected)nk_fill_rect(canvas,r,0,nk_rgb(37,71,106));
 nk_draw_text(canvas,r,text,(int)strlen(text),font,nk_rgba(0,0,0,0),nk_rgb(240,240,240));nk_end(ui->ctx);sb_ui_draw(ui);
 SDL_Surface *raw=SDL_RenderReadPixels(ui->renderer,NULL),*pixels=raw ? SDL_ConvertSurface(raw,SDL_PIXELFORMAT_RGBA32) : NULL;uint64_t hash=0;*ink=0;
 if(pixels){float sx=(float)pixels->w/1000,sy=(float)pixels->h/640;
  for(int y=(int)(20*sy);y<(int)(35*sy);++y)for(int x=(int)(20*sx);x<(int)(805*sx);++x){const unsigned char *p=(const unsigned char *)pixels->pixels+y*pixels->pitch+x*4;hash=hash*1099511628211ULL^sb_hash((const char *)p,4);if(p[0]>150 && p[1]>150 && p[2]>150)++*ink;}
 }
 SDL_DestroySurface(pixels);SDL_DestroySurface(raw);SDL_RenderPresent(ui->renderer);return hash;
}
int main(int argc,char **argv){if(argc!=2)return 2;SBUi ui;CHECK(sb_ui_init(&ui,argv[1],1000,640,true).code==SB_OK);
 char *longline=malloc(12004),*reference=malloc(1004);CHECK(longline && reference);
 memset(longline,'x',12000);memcpy(longline+12000,"END",4);memset(reference,'x',1000);memcpy(reference+1000,"END",4);
 CHECK(ui.normal->handle.width(ui.normal->handle.userdata,ui.normal->handle.height,longline,12003)>65535);
 for(unsigned selected=0;selected<2;++selected){unsigned a,b;uint64_t expected=draw(&ui,reference,selected!=0,&a),actual=draw(&ui,longline,selected!=0,&b);CHECK(a>500 && b>500);CHECK(expected && actual==expected);}
 free(reference);free(longline);sb_ui_shutdown(&ui);printf("%u text extent assertions passed: >65K logical width, visible suffix, selected/unselected independent reference.\n",checks);return 0;}
