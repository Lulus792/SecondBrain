/* Read-only desktop timing probe. Does not apply preferences or write notes. */
#include "desktop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct { double layout,draw,present,apply,total; } Sample;
static double elapsed(Uint64 start) { return (double)(SDL_GetTicksNS()-start)/1e6; }
static int compare(const void *a,const void *b) { double x=*(const double *)a,y=*(const double *)b;return x<y ? -1 : x>y; }
static void phase(SBDesktop *d,const char *name,unsigned mode) {
    Sample samples[60];
    for(unsigned i=0;i<68;++i) {
        nk_input_begin(d->ui.ctx);SDL_Event e;while(SDL_PollEvent(&e))sb_desktop_event(d,&e);
        if(mode==1)d->scrolling[0].pending+=i<34 ? 3 : -3;
        if(mode==2)d->yaw+=0.01f;
        if(mode==3 && i%10==0 && d->model.notes.count>1) {
            SDL_Event key={0};key.type=SDL_EVENT_KEY_DOWN;key.key.key=SDLK_RIGHT;key.key.down=true;
            strcpy(d->focus,"galaxy");d->keyboard=true;sb_desktop_event(d,&key);
        }
        Uint64 start=SDL_GetTicksNS(),t=start;sb_desktop_tick(d,1.0f/60);nk_input_end(d->ui.ctx);sb_desktop_frame(d);
        Sample s={0};s.layout=elapsed(t);t=SDL_GetTicksNS();sb_ui_draw(&d->ui);s.draw=elapsed(t);
        t=SDL_GetTicksNS();SDL_RenderPresent(d->ui.renderer);s.present=elapsed(t);t=SDL_GetTicksNS();sb_desktop_apply(d);s.apply=elapsed(t);s.total=elapsed(start);
        if(i>=8)samples[i-8]=s;
        Uint64 work=SDL_GetTicksNS()-start;if(work<16666667u)SDL_DelayNS(16666667u-work);
    }
    double times[60],layout=0,draw=0,present=0,apply=0;
    for(unsigned i=0;i<60;++i){times[i]=samples[i].total;layout+=samples[i].layout;draw+=samples[i].draw;present+=samples[i].present;apply+=samples[i].apply;}
    qsort(times,60,sizeof(double),compare);
    printf("%s: layout %.2f ms; draw %.2f ms; present %.2f ms; apply %.2f ms; frame median %.2f p95 %.2f max %.2f ms\n",name,layout/60,draw/60,present/60,apply/60,times[30],times[56],times[59]);fflush(stdout);
}
int main(int argc,char **argv) {
    if(argc!=5 && argc!=6){fprintf(stderr,"Usage: profile WORKSPACE PROJECT FONT software|native [NOTE]\n");return 2;}
    SBDesktop d;SBStatus s=sb_desktop_init(&d,argv[1],argv[3],!strcmp(argv[4],"software"));
    if(s.code!=SB_OK){fprintf(stderr,"%s\n",s.message);return 1;}
    s=sb_app_request(&d.model,SB_ACT_PROJECT,argv[2]);
    if(s.code!=SB_OK){fprintf(stderr,"%s\n",s.message);sb_desktop_free(&d);return 1;}
    if(argc==6){s=sb_app_request(&d.model,SB_ACT_NOTE,argv[5]);if(s.code!=SB_OK){fprintf(stderr,"%s\n",s.message);sb_desktop_free(&d);return 1;}}
    printf("Renderer: %s; notes: %zu; view: 1336x840\n",SDL_GetRendererName(d.ui.renderer),d.model.notes.count);
    phase(&d,"idle",0);phase(&d,"scroll",1);phase(&d,"camera",2);phase(&d,"switch",3);
    sb_desktop_free(&d);return 0;
}
