/* Read-only desktop timing probe. Does not apply preferences or write notes. */
#include "desktop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct { double input,layout,draw,present,apply,total,tick_delta; } Sample;
/* The performance counter survives SDL_Quit; SDL ticks restart there. */
static double ticks_per_ms;
static double elapsed(Uint64 start) { return (double)(SDL_GetPerformanceCounter()-start)/ticks_per_ms; }
static int compare(const void *a,const void *b) { double x=*(const double *)a,y=*(const double *)b;return x<y ? -1 : x>y; }
static void csv_text(FILE *file,const char *text) {
    fputc('"',file);
    for(;*text;++text){if(*text=='"')fputc('"',file);fputc(*text,file);}
    fputc('"',file);
}
static void phase(SBDesktop *d,const char *name,unsigned mode,char **notes,size_t count,FILE *trace) {
    Sample samples[68];
    Uint64 last_tick=SDL_GetPerformanceCounter();
    for(unsigned i=0;i<68;++i) {
        Uint64 start=SDL_GetPerformanceCounter(),t=start;
        char before[SB_PATH_CAP],requested[SB_PATH_CAP]={0};
        snprintf(before,sizeof(before),"%s",d->model.path);
        nk_input_begin(d->ui.ctx);SDL_Event e;while(SDL_PollEvent(&e))sb_desktop_event(d,&e);
        if(mode==1)d->scrolling[0].pending+=i<34 ? 3 : -3;
        if(mode==2)d->yaw+=0.01f;
        if(mode==3 && i%10==0 && d->model.notes.count>1) {
            if(count>1) {
                const char *path=notes[(i/10)%count];
                snprintf(requested,sizeof(requested),"%s",path);
                d->navigation.kind=SB_ACT_NOTE;snprintf(d->navigation.value,sizeof(d->navigation.value),"%s",path);
                d->ui.capture_pending=strcmp(path,d->model.path)!=0;d->follow_star=true;
            } else {
                SDL_Event key={0};key.type=SDL_EVENT_KEY_DOWN;key.key.key=SDLK_RIGHT;key.key.down=true;
                strcpy(d->focus,"galaxy");d->keyboard=true;sb_desktop_event(d,&key);
            }
        }
        if(!requested[0] && d->navigation.kind==SB_ACT_NOTE)snprintf(requested,sizeof(requested),"%s",d->navigation.value);
        Sample s={0};s.input=elapsed(t);t=SDL_GetPerformanceCounter();
        s.tick_delta=i ? (double)(t-last_tick)/ticks_per_ms : 1000.0/60;
        last_tick=t;
        sb_desktop_tick(d,(float)(s.tick_delta/1000));nk_input_end(d->ui.ctx);sb_desktop_frame(d);
        s.layout=elapsed(t);t=SDL_GetPerformanceCounter();sb_ui_draw(&d->ui);s.draw=elapsed(t);
        t=SDL_GetPerformanceCounter();SDL_RenderPresent(d->ui.renderer);s.present=elapsed(t);t=SDL_GetPerformanceCounter();sb_desktop_apply(d);s.apply=elapsed(t);s.total=elapsed(start);
        samples[i]=s;
        if(trace){
            fprintf(trace,"%s,%u,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,",name,i,s.input,s.layout,s.draw,s.present,s.apply,s.total);
            csv_text(trace,before);fputc(',',trace);csv_text(trace,requested);fputc(',',trace);csv_text(trace,d->model.path);
            fprintf(trace,",%zu,%zu,%zu,%zu,%.6f\n",d->reader_source_builds,d->reader_source_hits,d->reader_plan_hits,d->reader_plan_misses,s.tick_delta);
        }
        double work=elapsed(start);if(work<1000.0/60)SDL_DelayNS((Uint64)((1000.0/60-work)*1e6));
    }
    double times[60],layout=0,draw=0,present=0,apply=0;unsigned worst=8;
    /* Retain the established settled summary, but expose every warm-up frame. */
    for(unsigned i=8;i<68;++i){times[i-8]=samples[i].total;if(samples[i].total>samples[worst].total)worst=i;layout+=samples[i].layout;draw+=samples[i].draw;present+=samples[i].present;apply+=samples[i].apply;}
    qsort(times,60,sizeof(double),compare);
    printf("%s: layout %.2f ms; draw %.2f ms; present %.2f ms; apply %.2f ms; frame median %.2f p95 %.2f max %.2f ms\n",name,layout/60,draw/60,present/60,apply/60,times[30],times[56],times[59]);
    Sample peak=samples[worst];printf("  slowest frame: layout %.2f; draw %.2f; present %.2f; apply %.2f ms\n",peak.layout,peak.draw,peak.present,peak.apply);fflush(stdout);
    unsigned initial_peak=0;for(unsigned i=1;i<8;++i)if(samples[i].total>samples[initial_peak].total)initial_peak=i;
    peak=samples[initial_peak];
    printf("  initial frames 0-7: max %.2f ms at %u; input %.2f; layout %.2f; draw %.2f; present %.2f; apply %.2f ms\n",peak.total,initial_peak,peak.input,peak.layout,peak.draw,peak.present,peak.apply);
    fflush(stdout);
}
int main(int argc,char **argv) {
    if(argc<5 || (strcmp(argv[4],"software") && strcmp(argv[4],"native"))){fprintf(stderr,"Usage: profile WORKSPACE PROJECT FONT software|native [--trace NEW_CSV] [NOTE ...]\n");return 2;}
    ticks_per_ms=(double)SDL_GetPerformanceFrequency()/1000;
    if(ticks_per_ms<=0){fprintf(stderr,"Performance clock is unavailable.\n");return 1;}
    int first_note=5;FILE *trace=NULL;
    if(argc>5 && !strcmp(argv[5],"--trace")){
        if(argc<7){fprintf(stderr,"--trace requires a new output file.\n");return 2;}
        trace=fopen(argv[6],"wx");if(!trace){perror(argv[6]);return 1;}
        fprintf(trace,"phase,frame,input_ms,layout_ms,draw_ms,present_ms,apply_ms,total_ms,note_before,requested_note,note_after,source_builds,source_hits,plan_hits,plan_misses,tick_delta_ms\n");
        first_note=7;
    }
    Uint64 start=SDL_GetPerformanceCounter();
    SBDesktop d;SBStatus s=sb_desktop_init(&d,argv[1],argv[3],!strcmp(argv[4],"software"));
    double init_ms=elapsed(start);
    if(s.code!=SB_OK){fprintf(stderr,"%s\n",s.message);if(trace)fclose(trace);return 1;}
    start=SDL_GetPerformanceCounter();
    s=sb_app_request(&d.model,SB_ACT_PROJECT,argv[2]);
    double project_ms=elapsed(start),note_ms=0;
    if(s.code!=SB_OK){fprintf(stderr,"%s\n",s.message);sb_desktop_free(&d);if(trace)fclose(trace);return 1;}
    if(argc>first_note){start=SDL_GetPerformanceCounter();s=sb_app_request(&d.model,SB_ACT_NOTE,argv[first_note]);note_ms=elapsed(start);if(s.code!=SB_OK){fprintf(stderr,"%s\n",s.message);sb_desktop_free(&d);if(trace)fclose(trace);return 1;}}
    printf("Renderer: %s; notes: %zu; view: 1336x840\n",SDL_GetRendererName(d.ui.renderer),d.model.notes.count);
    printf("Startup: init %.2f ms; project %.2f ms; note %.2f ms (before first frame)\n",init_ms,project_ms,note_ms);
    phase(&d,"idle",0,argv+first_note,(size_t)(argc-first_note),trace);phase(&d,"scroll",1,argv+first_note,(size_t)(argc-first_note),trace);phase(&d,"camera",2,argv+first_note,(size_t)(argc-first_note),trace);phase(&d,"switch",3,argv+first_note,(size_t)(argc-first_note),trace);
    start=SDL_GetPerformanceCounter();sb_desktop_free(&d);printf("Shutdown: %.2f ms\n",elapsed(start));
    if(trace){bool failed=ferror(trace)!=0;if(fclose(trace)!=0)failed=true;if(failed){fprintf(stderr,"Could not finish trace.\n");return 1;}}
    return 0;
}
