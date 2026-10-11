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
static SBPrepareState wait_layout(SBPrepareLayout *job){Uint64 end=SDL_GetTicksNS()+UINT64_C(15000000000);SBPrepareState state;while((state=sb_prepare_layout_state(job,NULL))==SB_PREPARE_PENDING && SDL_GetTicksNS()<end)SDL_Delay(1);return state;}
typedef struct {const SBStyledPlan *plan;size_t at;bool valid;} LayoutCompare;
static bool compare_layout(void *data,const SBShapedLine *line,size_t offset,float y){LayoutCompare *p=data;if(p->at>=p->plan->count){p->valid=false;return false;}const SBStyledLine *expected=&p->plan->lines[p->at++];Compare compare={.result=&expected->shape,.valid=true};
    /* Reuse numeric glyph comparison, with this paragraph's actual origin. */
    p->valid=offset==expected->source_offset && y==expected->y && compare_line(&compare,line,0,0);return p->valid;}
static int layout_case(SBUi *ui,unsigned role,float width,bool terminal){
    const char *source="AV **ffi é** אב12גד *ببب سلام* `code` 👩‍💻 words words\r\n\nאבג AB more words\n";SBInline reader;SBStyledText styled={0};OK(sb_inline_init(&reader,source,strlen(source)));OK(sb_inline_styled(&reader,0,strlen(source),&styled));
    const struct nk_user_font *font=role==0 ? &ui->normal->handle : role==1 ? &ui->body->handle : role==2 ? &ui->heading->handle : &ui->code->handle;
    struct nk_vec2 padding=ui->ctx->style.text.padding;float gap=4*ui->scale;SBTextSnapshot *snapshot=sb_ui_text_snapshot(ui);CHECK(snapshot);
    SBPrepareLayout *job=sb_prepare_layout_start(snapshot,&styled,role,width,padding,gap,terminal,77);CHECK(job);CHECK(wait_layout(job)==SB_PREPARE_READY);SBStyledPlan result={0};
    CHECK(sb_prepare_layout_take(job,ui,&styled,role,width+1,padding,gap,terminal,77,&result).code==SB_CONFLICT && !result.lines);
    CHECK(sb_prepare_layout_take(job,ui,&styled,role,width,padding,gap,terminal,78,&result).code==SB_CONFLICT && !result.lines);
    unsigned old_style=styled.spans[0].style;styled.spans[0].style^=SB_TEXT_BOLD;CHECK(sb_prepare_layout_take(job,ui,&styled,role,width,padding,gap,terminal,77,&result).code==SB_CONFLICT && !result.lines);styled.spans[0].style=old_style;
    OK(sb_prepare_layout_take(job,ui,&styled,role,width,padding,gap,terminal,77,&result));CHECK(sb_prepare_layout_take(job,ui,&styled,role,width,padding,gap,terminal,77,&result).code==SB_INVALID);sb_prepare_layout_free(job);
    SBStyledPlan reference={0};CHECK(sb_ui_styled_plan(ui,font,&styled,width,padding,gap,terminal,&reference));CHECK(reference.count==result.count && reference.height==result.height);
    for(size_t i=0;i<result.count;++i){CHECK(result.lines[i].y==reference.lines[i].y && result.lines[i].source_offset==reference.lines[i].source_offset);Compare compare={.result=&result.lines[i].shape,.valid=true};CHECK(compare_line(&compare,&reference.lines[i].shape,0,0));}
    if(!terminal){LayoutCompare compare={.plan=&result,.valid=true};CHECK(sb_ui_styled_geometry(ui,font,&styled,width,compare_layout,&compare));CHECK(compare.valid && compare.at==result.count);CHECK(sb_ui_styled_height(ui,font,&styled,width)==result.height);}
    sb_ui_styled_plan_free(&reference);sb_ui_styled_plan_free(&result);sb_styled_free(&styled);sb_inline_free(&reader);return 0;
}
static int layout_invalidation(SBUi *ui){
    OK(sb_ui_fonts(ui,1));const char *source="office ffi אבג ببب more text";SBTextSpan span={0,strlen(source),SB_TEXT_BOLD};SBStyledText text={.text=(char *)source,.spans=&span,.count=1};struct nk_vec2 padding=ui->ctx->style.text.padding;float gap=4*ui->scale;
    SBTextSnapshot *snapshot=sb_ui_text_snapshot(ui);CHECK(snapshot);SBPrepareLayout *job=sb_prepare_layout_start(snapshot,&text,1,120,padding,gap,false,6);CHECK(job);CHECK(wait_layout(job)==SB_PREPARE_READY);
    /* This style was absent at capture and created/mutated before adoption. */
    const struct nk_user_font *bold=sb_ui_text_style(ui,&ui->body->handle,SB_TEXT_BOLD);TTF_Font *changed=sb_ui_cluster_font(bold,"A",1,NULL);CHECK(changed && TTF_SetFontOutline(changed,3));SBStyledPlan plan={0};
    CHECK(sb_prepare_layout_take(job,ui,&text,1,120,padding,gap,false,6,&plan).code==SB_CONFLICT && !plan.lines);sb_prepare_layout_free(job);
    snapshot=sb_ui_text_snapshot(ui);CHECK(snapshot);job=sb_prepare_layout_start(snapshot,&text,1,120,padding,gap,false,6);CHECK(job);sb_prepare_layout_cancel(job);CHECK(wait_layout(job)==SB_PREPARE_CANCELLED);CHECK(sb_prepare_layout_take(job,ui,&text,1,120,padding,gap,false,6,&plan).code==SB_INVALID && !plan.lines);sb_prepare_layout_free(job);
    snapshot=sb_ui_text_snapshot(ui);CHECK(snapshot);job=sb_prepare_layout_start(snapshot,&text,1,120,padding,gap,false,6);CHECK(job);OK(sb_ui_fonts(ui,1.5f));CHECK(wait_layout(job)==SB_PREPARE_READY);CHECK(sb_prepare_layout_take(job,ui,&text,1,120,padding,gap,false,6,&plan).code==SB_CONFLICT && !plan.lines);sb_prepare_layout_free(job);
    SBTextSpan invalid_span={1,strlen(source)-1,0};SBStyledText invalid={.text=(char *)source,.spans=&invalid_span,.count=1};snapshot=sb_ui_text_snapshot(ui);CHECK(snapshot);job=sb_prepare_layout_start(snapshot,&invalid,1,120,padding,gap,false,6);CHECK(job);CHECK(wait_layout(job)==SB_PREPARE_FAILED);sb_prepare_layout_free(job);
    return 0;
}
static SBPrepareState wait_batch(SBPrepareBatch *batch){Uint64 end=SDL_GetTicksNS()+UINT64_C(15000000000);SBPrepareState state;while((state=sb_prepare_batch_state(batch,NULL))==SB_PREPARE_PENDING && SDL_GetTicksNS()<end)SDL_Delay(1);return state;}
static bool retire_batch(SBPrepareBatch *batch){Uint64 end=SDL_GetTicksNS()+UINT64_C(15000000000);while(!sb_prepare_batch_try_free(batch)){if(SDL_GetTicksNS()>=end)return false;SDL_Delay(1);}return true;}
typedef struct {SBPrepareBatch *batch;SBUi *ui;unsigned hits;} BatchLookup;
static const SBStyledPlan *batch_lookup(void *data,const struct nk_user_font *font,const SBStyledText *text,float width,struct nk_vec2 padding,float gap,bool terminal){
    BatchLookup *p=data;unsigned role=font==&p->ui->normal->handle ? 0 : font==&p->ui->body->handle ? 1 : font==&p->ui->heading->handle ? 2 : 3;
    SBPrepareRequest r={.text=*text,.role=role,.width=width,.padding=padding,.gap=gap,.terminal_line=terminal};
    const SBStyledPlan *plan=sb_prepare_batch_lookup(p->batch,p->ui,&r);if(plan)++p->hits;return plan;
}
static int batch_cases(SBUi *ui){
    OK(sb_ui_fonts(ui,1));
    const char *source="**AV ffi** אבג *ببب* 👩‍💻 words words\nlast";
    SBInline reader;SBStyledText styled={0};OK(sb_inline_init(&reader,source,strlen(source)));OK(sb_inline_styled(&reader,0,strlen(source),&styled));
    SBPrepareRequest requests[4];struct nk_vec2 padding=ui->ctx->style.text.padding;
    size_t remaining=1;SBStyledPlan rejected={0};ui->layout_budget=&remaining;
    CHECK(!sb_ui_styled_plan(ui,&ui->body->handle,&styled,100,padding,4,false,&rejected));
    CHECK(!rejected.lines && !rejected.count && remaining==1);ui->layout_budget=NULL;
    for(unsigned i=0;i<4;++i)requests[i]=(SBPrepareRequest){.text=styled,.role=i,.width=173+i*31,.padding=padding,.gap=4*ui->scale};
    SBTextSnapshot *fonts=sb_ui_text_snapshot(ui);CHECK(fonts);SBPrepareBatch *b=sb_prepare_batch_start(fonts,requests,4,23);CHECK(b);
    CHECK(wait_batch(b)==SB_PREPARE_READY);CHECK(!sb_prepare_batch_lookup(b,ui,&requests[0]));
    CHECK(sb_prepare_batch_bind(b,ui,24).code==SB_CONFLICT);OK(sb_prepare_batch_bind(b,ui,23));
    CHECK(sb_prepare_batch_bytes(b)>strlen(source)*4 && sb_prepare_batch_bytes(b)<SB_PREPARE_INPUT_BYTES+SB_PREPARE_OUTPUT_BYTES);
    for(unsigned i=0;i<4;++i){const SBStyledPlan *plan=sb_prepare_batch_lookup(b,ui,&requests[i]);CHECK(plan);
        const struct nk_user_font *font=i==0 ? &ui->normal->handle : i==1 ? &ui->body->handle : i==2 ? &ui->heading->handle : &ui->code->handle;
        SBStyledPlan reference={0};CHECK(sb_ui_styled_plan(ui,font,&styled,requests[i].width,padding,4,false,&reference));CHECK(reference.count==plan->count && reference.height==plan->height);
        for(size_t l=0;l<reference.count;++l){Compare c={.result=&plan->lines[l].shape,.valid=true};CHECK(compare_line(&c,&reference.lines[l].shape,0,0));CHECK(plan->lines[l].source_offset==reference.lines[l].source_offset && plan->lines[l].y==reference.lines[l].y);}
        sb_ui_styled_plan_free(&reference);
    }
    SBPrepareRequest changed=requests[0];changed.width+=1;CHECK(!sb_prepare_batch_lookup(b,ui,&changed));changed=requests[0];changed.terminal_line=true;CHECK(!sb_prepare_batch_lookup(b,ui,&changed));
    BatchLookup lookup={b,ui,0};ui->prepared_lookup=batch_lookup;ui->prepared_data=&lookup;sb_ui_styled_cache_clear(ui);
    const SBStyledPlan *plan=sb_prepare_batch_lookup(b,ui,&requests[1]);LayoutCompare compare={.plan=plan,.valid=true};
    CHECK(sb_ui_styled_height(ui,&ui->body->handle,&styled,requests[1].width)==plan->height);
    CHECK(sb_ui_styled_geometry(ui,&ui->body->handle,&styled,requests[1].width,compare_layout,&compare));CHECK(compare.valid && compare.at==plan->count && lookup.hits==2 && !ui->styled_cache);
    ui->prepared_lookup=NULL;ui->prepared_data=NULL;
    OK(sb_ui_fonts(ui,1.25f));CHECK(!sb_prepare_batch_lookup(b,ui,&requests[0]));CHECK(sb_prepare_batch_bind(b,ui,23).code==SB_CONFLICT);CHECK(retire_batch(b));
    fonts=sb_ui_text_snapshot(ui);CHECK(fonts);b=sb_prepare_batch_start(fonts,requests,4,25);CHECK(b);sb_prepare_batch_cancel(b);CHECK(wait_batch(b)==SB_PREPARE_CANCELLED);CHECK(sb_prepare_batch_bind(b,ui,25).code==SB_INVALID);CHECK(retire_batch(b));
    fonts=sb_ui_text_snapshot(ui);CHECK(fonts);CHECK(!sb_prepare_batch_start(fonts,requests,0,0));CHECK(!sb_prepare_batch_start(fonts,requests,SB_PREPARE_REQUEST_LIMIT+1,0));
    changed=requests[0];changed.role=4;CHECK(!sb_prepare_batch_start(fonts,&changed,1,0));sb_text_snapshot_free(fonts);
    /* More plans than the general memo cache can retain, all pinned together. */
    OK(sb_ui_fonts(ui,1));size_t count=2200;SBPrepareRequest *many=calloc(count,sizeof(*many));char (*labels)[40]=calloc(count,sizeof(*labels));SBTextSpan *spans=calloc(count,sizeof(*spans));CHECK(many && labels && spans);
    for(size_t i=0;i<count;++i){snprintf(labels[i],40,"block %zu AV ffi",i);spans[i]=(SBTextSpan){0,strlen(labels[i]),0};many[i]=(SBPrepareRequest){.text={.text=labels[i],.spans=&spans[i],.count=1},.role=1,.width=200,.padding=padding,.gap=4};}
    fonts=sb_ui_text_snapshot(ui);CHECK(fonts);b=sb_prepare_batch_start(fonts,many,count,9);CHECK(b);CHECK(wait_batch(b)==SB_PREPARE_READY);OK(sb_prepare_batch_bind(b,ui,9));
    for(size_t i=0;i<count;++i){plan=sb_prepare_batch_lookup(b,ui,&many[i]);CHECK(plan && plan->count==1 && plan->height>0);}
    /* Requests are copied; changes to caller memory cannot change a bound plan. */
    const SBStyledPlan *first=sb_prepare_batch_lookup(b,ui,&many[0]);labels[0][0]='X';CHECK(!sb_prepare_batch_lookup(b,ui,&many[0]));labels[0][0]='b';CHECK(sb_prepare_batch_lookup(b,ui,&many[0])==first);
    CHECK(retire_batch(b));free(many);free(labels);free(spans);
    /* Reject oversized aggregate input before starting a worker. */
    char *large=malloc(SB_TEXT_LIMIT+1);CHECK(large);memset(large,'a',SB_TEXT_LIMIT);large[SB_TEXT_LIMIT]=0;SBTextSpan large_span={0,SB_TEXT_LIMIT,0};
    for(size_t i=0;i<3;++i)requests[i]=(SBPrepareRequest){.text={.text=large,.spans=&large_span,.count=1},.role=1,.width=200,.padding=padding};
    fonts=sb_ui_text_snapshot(ui);CHECK(fonts);CHECK(!sb_prepare_batch_start(fonts,requests,3,0));sb_text_snapshot_free(fonts);free(large);
    sb_styled_free(&styled);sb_inline_free(&reader);return 0;
}
int main(int argc,char **argv){CHECK(argc==2);SBUi ui;OK(sb_ui_init(&ui,argv[1],900,600,true));
    for(unsigned scale=0;scale<3;++scale){OK(sb_ui_fonts(&ui,1+0.5f*scale));const struct nk_user_font *base=&ui.body->handle;
        CHECK(!shape_case(&ui,base,"AB é ffi AV אב12גד ببب سلام 👩‍💻",0));CHECK(!shape_case(&ui,base,"אבג AB 12 (דה)",0));
        for(unsigned style=1;style<8;++style){const struct nk_user_font *font=sb_ui_text_style(&ui,base,style);CHECK(font);CHECK(!shape_case(&ui,font,"AV ffi é ببب אבג 👩‍💻",style));}
        CHECK(!shape_case(&ui,base,"",0));
    }
    TTF_Font *font=sb_ui_cluster_font(&ui.body->handle,"п",2,NULL);CHECK(font && TTF_SetFontLanguage(font,"bg"));sb_ui_styled_cache_clear(&ui);CHECK(!shape_case(&ui,&ui.body->handle,"птб AV ffi",0));
    CHECK(!immutable_assets(&ui));CHECK(!thread_ownership(&ui));CHECK(!cancelled(&ui));CHECK(!stale_fonts(&ui));
    for(unsigned scale=0;scale<3;++scale){OK(sb_ui_fonts(&ui,1+0.5f*scale));for(unsigned role=0;role<4;++role){CHECK(!layout_case(&ui,role,125,false));CHECK(!layout_case(&ui,role,430,true));}}
    CHECK(!layout_invalidation(&ui));CHECK(!batch_cases(&ui));sb_ui_shutdown(&ui);printf("%u private-worker-font/source/glyph/wrap/style/batch/cancellation/epoch assertions passed.\n",checks);return 0;
}
