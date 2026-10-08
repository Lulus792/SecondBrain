#include "prepare.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"PREPARE %d: %s (%s)\n",__LINE__,#x,SDL_GetError());return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
static SBPrepareState wait_job(SBPrepareJob *job){Uint64 end=SDL_GetTicksNS()+UINT64_C(15000000000);SBPrepareState state;while((state=sb_prepare_state(job,NULL))==SB_PREPARE_PENDING && SDL_GetTicksNS()<end)SDL_Delay(1);return state;}
typedef struct {const SBShapedLine *result;bool valid;unsigned calls;} Compare;
static bool compare_line(void *data,const SBShapedLine *line,size_t offset,float y){
    Compare *c=data;const SBShapedLine *r=c->result;++c->calls;
    c->valid=!offset && y==0 && line->byte==r->byte && line->length==r->length && line->base_level==r->base_level && line->advance==r->advance && line->ascent==r->ascent && line->descent==r->descent && line->count==r->count && line->run_count==r->run_count;
    for(size_t i=0;c->valid && i<line->count;++i){SBShapeGlyph a=line->glyphs[i],b=r->glyphs[i];c->valid=a.byte==b.byte && a.index==b.index && a.font==b.font && a.x==b.x && a.y==b.y && a.advance==b.advance && a.left==b.left && a.top==b.top && a.width==b.width && a.height==b.height && a.level==b.level;}
    for(size_t i=0;c->valid && i<line->run_count;++i){SBShapeRun a=line->runs[i],b=r->runs[i];c->valid=a.byte==b.byte && a.length==b.length && a.glyph_begin==b.glyph_begin && a.glyph_count==b.glyph_count && a.x==b.x && a.advance==b.advance && a.level==b.level;}
    return c->valid;
}
static int shape_case(SBUi *ui,const struct nk_user_font *font,const char *source,unsigned style){
    char caller[256];size_t length=strlen(source);CHECK(length<sizeof(caller));memcpy(caller,source,length+1);
    SBFontSnapshot *snapshot=sb_ui_font_snapshot(font);CHECK(snapshot);SBPrepareJob *job=sb_prepare_start(snapshot,caller,length,42);CHECK(job);
    memset(caller,'x',length);caller[length]=0;CHECK(wait_job(job)==SB_PREPARE_READY);SBShapedLine line={0};
    if(length)CHECK(sb_prepare_take(job,ui,caller,length,42,&line).code==SB_CONFLICT && !line.glyphs);
    CHECK(sb_prepare_take(job,ui,source,length,43,&line).code==SB_CONFLICT && !line.glyphs);
    OK(sb_prepare_take(job,ui,source,length,42,&line));CHECK(sb_prepare_take(job,ui,source,length,42,&line).code==SB_INVALID);
    sb_prepare_free(job); /* Transferred geometry and UI font binding stay valid. */
    SBTextSpan span={0,length,style};SBStyledText styled={.text=(char *)source,.spans=length ? &span : NULL,.count=length ? 1 : 0};Compare compare={.result=&line,.valid=true};
    CHECK(sb_ui_styled_geometry(ui,&ui->body->handle,&styled,100000,compare_line,&compare));CHECK(compare.valid && compare.calls==1);sb_shape_line_free(&line);return 0;
}
static int cancelled(SBUi *ui){
    char *source=malloc(1024*1024+1);CHECK(source);memset(source,'a',1024*1024);source[1024*1024]=0;
    SBFontSnapshot *snapshot=sb_ui_font_snapshot(&ui->body->handle);CHECK(snapshot);SBPrepareJob *job=sb_prepare_start(snapshot,source,1024*1024,1);CHECK(job);
    sb_prepare_cancel(job);CHECK(wait_job(job)==SB_PREPARE_CANCELLED);SBShapedLine line={0};CHECK(sb_prepare_take(job,ui,source,1024*1024,1,&line).code==SB_INVALID && !line.glyphs);sb_prepare_free(job);free(source);return 0;
}
static int stale_fonts(SBUi *ui){
    const char *source="AV ffi é אב12 ببب 👩‍💻";SBFontSnapshot *snapshot=sb_ui_font_snapshot(&ui->body->handle);CHECK(snapshot);SBPrepareJob *job=sb_prepare_start(snapshot,source,strlen(source),5);CHECK(job);
    OK(sb_ui_fonts(ui,1.25f));CHECK(wait_job(job)==SB_PREPARE_READY);SBShapedLine line={0};CHECK(sb_prepare_take(job,ui,source,strlen(source),5,&line).code==SB_CONFLICT && !line.glyphs);sb_prepare_free(job);
    snapshot=sb_ui_font_snapshot(&ui->body->handle);CHECK(snapshot);job=sb_prepare_start(snapshot,"\xc0\x80",2,5);CHECK(job);CHECK(wait_job(job)==SB_PREPARE_FAILED);SBStatus status;CHECK(sb_prepare_state(job,&status)==SB_PREPARE_FAILED && status.code==SB_INVALID);sb_prepare_free(job);return 0;
}
static int immutable_assets(SBUi *ui){
    SBFontSnapshot *snapshot=sb_ui_font_snapshot(&ui->body->handle);CHECK(snapshot);SBFontInstance *instance=NULL;OK(sb_font_snapshot_open(snapshot,&instance));sb_font_snapshot_free(snapshot);
    TTF_Font *font=sb_font_instance_select(instance,"A",1,NULL);CHECK(font && sb_font_instance_token(instance,font)==0);SBTextParagraph *p=NULL;SBShapedLine line={0};SBShapeFontSpan span={0,1,font,0};OK(sb_bidi_paragraph_create("A",1,SB_BIDI_AUTO_LTR,&p));OK(sb_shape_line(p,0,1,&span,1,&line));CHECK(line.count==1 && line.advance>0);sb_shape_line_free(&line);sb_bidi_paragraph_free(p);sb_font_instance_free(instance);return 0;
}
typedef struct {SBUi *ui;SBFontSnapshot *snapshot;bool rejected;} ForeignAccess;
static int foreign_access(void *data){ForeignAccess *p=data;p->rejected=!sb_ui_font_snapshot(&p->ui->body->handle) && !sb_font_snapshot_bind(p->snapshot,p->ui,0);return 0;}
static int thread_ownership(SBUi *ui){SBFontSnapshot *s=sb_ui_font_snapshot(&ui->body->handle);CHECK(s);ForeignAccess data={ui,s,false};SDL_Thread *thread=SDL_CreateThread(foreign_access,"Font owner probe",&data);CHECK(thread);SDL_WaitThread(thread,NULL);CHECK(data.rejected);CHECK(sb_font_snapshot_bind(s,ui,0));sb_font_snapshot_free(s);return 0;}
int main(int argc,char **argv){CHECK(argc==2);SBUi ui;OK(sb_ui_init(&ui,argv[1],900,600,true));
    for(unsigned scale=0;scale<3;++scale){OK(sb_ui_fonts(&ui,1+0.5f*scale));const struct nk_user_font *base=&ui.body->handle;
        CHECK(!shape_case(&ui,base,"AB é ffi AV אב12גד ببب سلام 👩‍💻",0));CHECK(!shape_case(&ui,base,"אבג AB 12 (דה)",0));
        for(unsigned style=1;style<8;++style){const struct nk_user_font *font=sb_ui_text_style(&ui,base,style);CHECK(font);CHECK(!shape_case(&ui,font,"AV ffi é ببب אבג 👩‍💻",style));}
        CHECK(!shape_case(&ui,base,"",0));
    }
    TTF_Font *font=sb_ui_cluster_font(&ui.body->handle,"п",2,NULL);CHECK(font && TTF_SetFontLanguage(font,"bg"));sb_ui_styled_cache_clear(&ui);CHECK(!shape_case(&ui,&ui.body->handle,"птб AV ffi",0));
    CHECK(!immutable_assets(&ui));CHECK(!thread_ownership(&ui));CHECK(!cancelled(&ui));CHECK(!stale_fonts(&ui));sb_ui_shutdown(&ui);printf("%u private-worker-font/source/glyph/cancellation/epoch assertions passed.\n",checks);return 0;
}
