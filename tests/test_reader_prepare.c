#include "desktop.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
static unsigned checks;
static double last_layout,first_pending,first_ready;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"READER PREPARE %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
static void frame(SBDesktop *d){nk_input_begin(d->ui.ctx);SDL_Event e;while(SDL_PollEvent(&e))sb_desktop_event(d,&e);nk_input_end(d->ui.ctx);sb_desktop_tick(d,1.0f/60);Uint64 start=SDL_GetTicksNS();sb_desktop_frame(d);last_layout=(double)(SDL_GetTicksNS()-start)/1e6;sb_ui_draw(&d->ui);SDL_RenderPresent(d->ui.renderer);sb_desktop_apply(d);}
static bool loading(SBDesktop *d){for(size_t i=0;i<d->passive_count;++i)if(!strcmp(d->passive[i].id,"reader:preparing"))return true;return false;}
static bool wait_reader(SBDesktop *d){Uint64 end=SDL_GetTicksNS()+UINT64_C(15000000000);while(loading(d) && SDL_GetTicksNS()<end){SDL_Delay(1);frame(d);}first_ready=last_layout;return !loading(d);}
static uint64_t geometry(SBDesktop *d){uint64_t hash=0;for(size_t i=0;i<d->passive_count;++i){SBPassiveText *p=&d->passive[i];if(strncmp(p->id,"reader:",7))continue;hash=hash*1099511628211ULL^sb_hash(p->id,strlen(p->id));hash=hash*1099511628211ULL^sb_hash(p->text,strlen(p->text));hash=hash*1099511628211ULL^sb_hash((const char *)&p->bounds,sizeof(p->bounds));hash=hash*1099511628211ULL^sb_hash((const char *)&p->document_y,sizeof(p->document_y));}return hash;}
static char *fixture(void){
    char *text=calloc(1,100000);if(!text)return NULL;strcpy(text,"# Long document\n\n");
    const char *block="A paragraph with **bold ffi**, *italic*, אבג and ببب with emoji 👩‍💻. More words for a wrapped line.\n\n> Quoted text with **style**.\n>\n> - Nested list with *words* and אבג.\n\n123. Numbered list words words words.\n124. Second item with **style**.\n\n| Name | Value |\n| --- | ---: |\n| **First** | אבג ببب |\n| Second | Text words words words |\n\n```c\nint value = 42; /* a long line with more words for wrapping */\n```\n\n";
    for(unsigned i=0;i<40;++i)strcat(text,block);return text;
}
int main(int argc,char **argv){
    CHECK(argc==3);char root[SB_PATH_CAP],suffix[80];snprintf(suffix,sizeof(suffix),"reader-prepare-%lu",sb_process_id());OK(sb_path_join(root,sizeof(root),argv[2],suffix));
    SBDesktop d;OK(sb_desktop_init(&d,root,argv[1],true));OK(sb_app_new_project(&d.model,"one","Reader",NULL));OK(sb_app_new_note(&d.model,"knowledge","long","Long document"));char *text=fixture();CHECK(text);
    free(d.model.editor);d.model.editor=malloc(strlen(text)+1);CHECK(d.model.editor);strcpy(d.model.editor,text);OK(sb_app_save(&d.model));d.card=true;d.editing=false;d.expanded=true;frame(&d);
    first_pending=last_layout;CHECK(d.reader_preparation && loading(&d) && sb_desktop_animating(&d));
    CHECK(wait_reader(&d));frame(&d);CHECK(d.reader_plan_hits>40);if(d.reader_plan_misses)fprintf(stderr,"prepared misses: %zu hits: %zu\n",d.reader_plan_misses,d.reader_plan_hits);CHECK(!d.reader_plan_misses);
    float maximum=d.scrolling[0].maximum;CHECK(maximum>1000);uint64_t prepared_geometry=geometry(&d);
    size_t source_builds=d.reader_source_builds;CHECK(d.reader_source && d.reader_source_bytes<=16u*1024u*1024u);frame(&d);CHECK(d.reader_source_builds==source_builds && geometry(&d)==prepared_geometry);
    d.ui.styled_cache_disabled=true;frame(&d);double reference_layout=last_layout;CHECK(geometry(&d)==prepared_geometry && d.scrolling[0].maximum==maximum);d.ui.styled_cache_disabled=false;frame(&d);
    printf("Fixture layout: initial pending %.2f ms; first ready %.2f ms; uncached synchronous reference %.2f ms.\n",first_pending,first_ready,reference_layout);
    d.scrolling[0].position=d.scrolling[0].destination=maximum/2;d.scrolling[0].active=true;frame(&d);float before=d.scrolling[0].position;
    /* A resized reader keeps its logical position while the new plan runs. */
    SDL_SetWindowSize(d.ui.window,750,900);frame(&d);CHECK(loading(&d));CHECK(fabsf(d.scrolling[0].position-before)<1);CHECK(wait_reader(&d));frame(&d);CHECK(d.reader_plan_hits>40 && !d.reader_plan_misses);CHECK(d.reader_source_builds==source_builds);
    /* A short replacement remains immediately usable; stale output is retired. */
    OK(sb_app_new_note(&d.model,"knowledge","short","Short"));frame(&d);CHECK(!loading(&d));
    OK(sb_app_request(&d.model,SB_ACT_NOTE,"knowledge/long.md"));frame(&d);CHECK(loading(&d));
    OK(sb_app_request(&d.model,SB_ACT_NOTE,"knowledge/short.md"));frame(&d);CHECK(!loading(&d));
    for(unsigned i=0;i<30 && d.reader_preparation;++i){SDL_Delay(1);frame(&d);}CHECK(!d.reader_preparation);
    OK(sb_app_request(&d.model,SB_ACT_NOTE,"knowledge/long.md"));frame(&d);CHECK(loading(&d));OK(sb_ui_fonts(&d.ui,1.5f));frame(&d);CHECK(wait_reader(&d));frame(&d);CHECK(d.reader_plan_hits>40 && !d.reader_plan_misses);
    d.editing=true;frame(&d);for(unsigned i=0;i<30 && d.reader_preparation;++i){SDL_Delay(1);frame(&d);}CHECK(!d.reader_preparation && !d.ui.prepared_lookup);CHECK(!d.reader_source && !d.reader_source_bytes);
    d.editing=false;char raw_path[SB_PATH_CAP];OK(sb_path_join(raw_path,sizeof(raw_path),d.model.project.root,"literal.txt"));
    char raw[16000]={0};for(unsigned i=0;i<120;++i)strcat(raw,"literal AV ffi אבג ببب text with words and code\r\n");
    OK(sb_fs_write_new(raw_path,raw,strlen(raw)));OK(sb_app_source(&d.model,raw_path));frame(&d);CHECK(loading(&d));CHECK(wait_reader(&d));frame(&d);CHECK(d.reader_plan_hits>100 && !d.reader_plan_misses);
    char picture[SB_PATH_CAP];OK(sb_path_join(picture,sizeof(picture),root,"reader-ready.bmp"));OK(sb_ui_capture(&d.ui,picture));printf("Reader snapshot: %s\n",picture);
    sb_app_source_close(&d.model);frame(&d);CHECK(wait_reader(&d));
    d.context=malloc(strlen(text)+1);CHECK(d.context);strcpy(d.context,text);d.form=SB_FORM_CONTEXT;frame(&d);CHECK(wait_reader(&d));frame(&d);CHECK(d.reader_plan_hits>40 && !d.reader_plan_misses);
    d.form=SB_FORM_NONE;frame(&d);CHECK(wait_reader(&d));
    free(text);sb_desktop_free(&d);printf("%u reader batch/retarget/resize/font/edit assertions passed.\n",checks);return 0;
}
