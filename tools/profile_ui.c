/* Read-only desktop timing probe. Does not apply preferences or write notes. */
#include "desktop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct { double input,layout,draw,present,apply,total,tick_delta,observe; } Sample;
typedef struct { unsigned serial; Uint64 started; char note[SB_PATH_CAP]; } Request;
typedef struct { bool present,pending; unsigned text_commands,image_commands; } ReaderFrame;
/* The performance counter survives SDL_Quit; SDL ticks restart there. */
static double ticks_per_ms;
static double elapsed(Uint64 start) { return (double)(SDL_GetPerformanceCounter()-start)/ticks_per_ms; }
static int compare(const void *a,const void *b) { double x=*(const double *)a,y=*(const double *)b;return x<y ? -1 : x>y; }
static void csv_text(FILE *file,const char *text) {
    fputc('"',file);
    for(;*text;++text){if(*text=='"')fputc('"',file);fputc(*text,file);}
    fputc('"',file);
}
static ReaderFrame reader_frame(const SBDesktop *d){
    ReaderFrame result={0};struct nk_rect bounds={0};
    for(size_t i=0;i<d->target_count;++i)if(!strcmp(d->targets[i].id,"reader")){result.present=true;bounds=d->targets[i].bounds;break;}
    for(size_t i=0;i<d->passive_count;++i)if(!strcmp(d->passive[i].id,"reader:preparing"))result.pending=true;
    if(!result.present)return result;
    int width,height;SDL_GetWindowSize(d->ui.window,&width,&height);
    const struct nk_command *command;
    nk_foreach(command,d->ui.ctx){
        float x,y,w,h;
        if(command->type==NK_COMMAND_TEXT){const struct nk_command_text *text=(const struct nk_command_text *)command;if(!text->length)continue;x=text->x;y=text->y;w=text->w;h=text->h;}
        else if(command->type==NK_COMMAND_IMAGE){const struct nk_command_image *image=(const struct nk_command_image *)command;x=image->x;y=image->y;w=image->w;h=image->h;}
        else continue;
        if(x>=bounds.x+bounds.w || x+w<=bounds.x || y>=bounds.y+bounds.h || y+h<=bounds.y || x>=width || y>=height || x+w<=0 || y+h<=0)continue;
        if(command->type==NK_COMMAND_TEXT)++result.text_commands;else ++result.image_commands;
    }
    return result;
}
static bool phase(SBDesktop *d,const char *name,unsigned mode,char **notes,size_t count,FILE *trace,Request *request,unsigned switch_every) {
    Sample samples[68];
    Uint64 last_tick=SDL_GetPerformanceCounter();
    for(unsigned i=0;i<68;++i) {
        Uint64 start=SDL_GetPerformanceCounter(),t=start;
        char before[SB_PATH_CAP],requested[SB_PATH_CAP]={0};
        snprintf(before,sizeof(before),"%s",d->model.path);
        nk_input_begin(d->ui.ctx);SDL_Event e;while(SDL_PollEvent(&e))sb_desktop_event(d,&e);
        if(mode==1)d->scrolling[0].pending+=i<34 ? 3 : -3;
        if(mode==2)d->yaw+=0.01f;
        if(mode==3 && i%switch_every==0 && d->model.notes.count>1) {
            if(count>1) {
                const char *path=notes[(i/switch_every)%count];
                snprintf(requested,sizeof(requested),"%s",path);
                d->navigation.kind=SB_ACT_NOTE;snprintf(d->navigation.value,sizeof(d->navigation.value),"%s",path);
                d->ui.capture_pending=strcmp(path,d->model.path)!=0;d->follow_star=true;
            } else {
                SDL_Event key={0};key.type=SDL_EVENT_KEY_DOWN;key.key.key=SDLK_RIGHT;key.key.down=true;
                strcpy(d->focus,"galaxy");d->keyboard=true;sb_desktop_event(d,&key);
            }
        }
        if(!requested[0] && d->navigation.kind==SB_ACT_NOTE)snprintf(requested,sizeof(requested),"%s",d->navigation.value);
        if(requested[0]){++request->serial;request->started=start;snprintf(request->note,sizeof(request->note),"%s",requested);}
        Sample s={0};s.input=elapsed(t);t=SDL_GetPerformanceCounter();
        s.tick_delta=i ? (double)(t-last_tick)/ticks_per_ms : 1000.0/60;
        last_tick=t;
        sb_desktop_tick(d,(float)(s.tick_delta/1000));nk_input_end(d->ui.ctx);sb_desktop_frame(d);
        s.layout=elapsed(t);t=SDL_GetPerformanceCounter();ReaderFrame reader={0};
        char presented[SB_PATH_CAP]={0};bool transitioning=false;float transition=0;
        if(trace){reader=reader_frame(d);snprintf(presented,sizeof(presented),"%s",d->model.path);transitioning=d->ui.transitioning;transition=d->ui.transition;}
        s.observe=elapsed(t);
        t=SDL_GetPerformanceCounter();sb_ui_draw(&d->ui);s.draw=elapsed(t);
        t=SDL_GetPerformanceCounter();bool present_ok=SDL_RenderPresent(d->ui.renderer);s.present=elapsed(t);
        double request_elapsed=(double)(SDL_GetPerformanceCounter()-request->started)/ticks_per_ms;
        t=SDL_GetPerformanceCounter();sb_desktop_apply(d);s.apply=elapsed(t);s.total=elapsed(start);
        samples[i]=s;
        if(trace){
            fprintf(trace,"%s,%u,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,",name,i,s.input,s.layout,s.draw,s.present,s.apply,s.total);
            csv_text(trace,before);fputc(',',trace);csv_text(trace,requested);fputc(',',trace);csv_text(trace,d->model.path);
            fprintf(trace,",%zu,%zu,%zu,%zu,%.6f,",d->reader_source_builds,d->reader_source_hits,d->reader_plan_hits,d->reader_plan_misses,s.tick_delta);
            csv_text(trace,presented);fprintf(trace,",%u,%u,%u,%u,%u,%.6f,%u,",reader.present,reader.pending,reader.text_commands,reader.image_commands,transitioning,transition,request->serial);
            csv_text(trace,request->note);fprintf(trace,",%.6f,%u,%.6f\n",request_elapsed,present_ok,s.observe);
        }
        if(!present_ok){fprintf(stderr,"Presentation failed: %s\n",SDL_GetError());return false;}
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
    return true;
}
int main(int argc,char **argv) {
    if(argc<5 || (strcmp(argv[4],"software") && strcmp(argv[4],"native"))){fprintf(stderr,"Usage: profile WORKSPACE PROJECT FONT software|native [--trace NEW_CSV] [--switch-every FRAMES] [NOTE ...]\n");return 2;}
    ticks_per_ms=(double)SDL_GetPerformanceFrequency()/1000;
    if(ticks_per_ms<=0){fprintf(stderr,"Performance clock is unavailable.\n");return 1;}
    int first_note=5;FILE *trace=NULL;const char *trace_path=NULL;unsigned switch_every=10;bool interval_set=false;
    while(first_note<argc && (!strcmp(argv[first_note],"--trace") || !strcmp(argv[first_note],"--switch-every"))){
        if(first_note+1>=argc){fprintf(stderr,"Profile option requires a value.\n");return 2;}
        if(!strcmp(argv[first_note],"--trace")){
            if(trace_path){fprintf(stderr,"Duplicate trace option.\n");return 2;}trace_path=argv[first_note+1];
        }else{
            char *end=NULL;unsigned long value=strtoul(argv[first_note+1],&end,10);
            if(interval_set || !end || *end || value<1 || value>67){fprintf(stderr,"Switch interval must be 1-67 frames and specified once.\n");return 2;}
            switch_every=(unsigned)value;interval_set=true;
        }
        first_note+=2;
    }
    if(trace_path){
        trace=fopen(trace_path,"wx");if(!trace){perror(trace_path);return 1;}
        fprintf(trace,"phase,frame,input_ms,layout_ms,draw_ms,present_ms,apply_ms,total_ms,note_before,requested_note,note_after,source_builds,source_hits,plan_hits,plan_misses,tick_delta_ms,presented_note,reader_present,reader_pending,reader_text_commands,reader_image_commands,transitioning,transition_progress,request_serial,active_request,request_elapsed_ms,present_ok,observe_ms\n");
    }
    Uint64 start=SDL_GetPerformanceCounter();
    SBDesktop d;SBStatus s=sb_desktop_init(&d,argv[1],argv[3],!strcmp(argv[4],"software"));
    double init_ms=elapsed(start);
    if(s.code!=SB_OK){fprintf(stderr,"%s\n",s.message);if(trace)fclose(trace);return 1;}
    start=SDL_GetPerformanceCounter();Request request={.serial=1,.started=start};
    s=sb_app_request(&d.model,SB_ACT_PROJECT,argv[2]);
    double project_ms=elapsed(start),note_ms=0;
    if(s.code!=SB_OK){fprintf(stderr,"%s\n",s.message);sb_desktop_free(&d);if(trace)fclose(trace);return 1;}
    if(argc>first_note){start=SDL_GetPerformanceCounter();request.started=start;s=sb_app_request(&d.model,SB_ACT_NOTE,argv[first_note]);note_ms=elapsed(start);if(s.code!=SB_OK){fprintf(stderr,"%s\n",s.message);sb_desktop_free(&d);if(trace)fclose(trace);return 1;}}
    snprintf(request.note,sizeof(request.note),"%s",d.model.path);
    printf("Renderer: %s; notes: %zu; view: 1336x840\n",SDL_GetRendererName(d.ui.renderer),d.model.notes.count);
    printf("Startup: init %.2f ms; project %.2f ms; note %.2f ms (before first frame)\n",init_ms,project_ms,note_ms);
    bool success=phase(&d,"idle",0,argv+first_note,(size_t)(argc-first_note),trace,&request,switch_every) && phase(&d,"scroll",1,argv+first_note,(size_t)(argc-first_note),trace,&request,switch_every) && phase(&d,"camera",2,argv+first_note,(size_t)(argc-first_note),trace,&request,switch_every) && phase(&d,"switch",3,argv+first_note,(size_t)(argc-first_note),trace,&request,switch_every);
    start=SDL_GetPerformanceCounter();sb_desktop_free(&d);printf("Shutdown: %.2f ms\n",elapsed(start));
    if(trace){bool failed=ferror(trace)!=0;if(fclose(trace)!=0)failed=true;if(failed){fprintf(stderr,"Could not finish trace.\n");return 1;}}
    return success ? 0 : 1;
}
