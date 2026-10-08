/* Shared paragraph analysis, logical wrapping and actual glyph geometry. */
#include "scripts.h"
#include <SDL3_ttf/SDL_ttf.h>
typedef struct {SBShapedLine shape;float y;size_t source_offset;} StyledLine;
typedef struct {StyledLine *lines;size_t count,capacity;float height;} StyledPlan;
static void styled_plan_free(StyledPlan *plan){for(size_t i=0;i<plan->count;++i)sb_shape_line_free(&plan->lines[i].shape);free(plan->lines);memset(plan,0,sizeof(*plan));}
static bool styled_line_add(StyledPlan *plan,SBShapedLine *line,SBUi *ui,const struct nk_user_font *base,float gap) {
    if(plan->count==plan->capacity){size_t next=plan->capacity ? plan->capacity*2 : 16;StyledLine *grown=realloc(plan->lines,next*sizeof(*grown));if(!grown)return false;plan->lines=grown;plan->capacity=next;}
    plan->lines[plan->count++]=(StyledLine){*line,plan->height};memset(line,0,sizeof(*line));
    const SBShapedLine *last=&plan->lines[plan->count-1].shape;
    plan->height+=fmaxf(base->height,(last->ascent+last->descent)/ui->density)+gap;return true;
}
bool sb_ui_styled_spans(const SBStyledText *text,SBTextSpan **spans,size_t *count) {
    *spans=NULL; *count=0;
    size_t length=strlen(text->text),covered=0;
    for (size_t i=0;i<text->count;++i) {
        SBTextSpan span=text->spans[i];
        if (span.offset!=covered || !span.length || span.length>length-covered) return false;
        covered+=span.length;
    }
    if (covered!=length) return false;
    SBGrapheme reader; SBGraphemeBoundary boundary;
    if (!sb_grapheme_init(&reader,text->text,length)) return false;
    SBStyledText display={.text=text->text};
    display.spans=malloc((text->count ? text->count : 1)*sizeof(*display.spans));
    if (!display.spans) return false;
    size_t start=0,index=0; sb_grapheme_next(&reader,&boundary);
    while (sb_grapheme_next(&reader,&boundary)) {
        while (index<text->count && text->spans[index].offset+text->spans[index].length<=start) ++index;
        unsigned style=index<text->count ? text->spans[index].style : 0;
        /* A grapheme keeps the style of its first scalar; marks never split it. */
        if (display.count && display.spans[display.count-1].style==style) display.spans[display.count-1].length+=boundary.byte-start;
        else display.spans[display.count++]=(SBTextSpan){start,boundary.byte-start,style};
        start=boundary.byte;
    }
    *spans=display.spans; *count=display.count; return true;
}

static size_t styled_paragraph_end(const char *text,size_t begin,size_t length) {
    const char *at=text+begin;
    while((size_t)(at-text)<length){uint32_t cp=SDL_StepUTF8(&at,NULL);if(sb_bidi_separator(cp)){if(cp=='\r' && (size_t)(at-text)<length && *at=='\n')++at;break;}}
    return (size_t)(at-text);
}
static bool styled_fonts(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,size_t start,size_t length,SBShapeFontSpan **out,size_t *count) {
    SBScriptPlan scripts={0};if(sb_script_plan(text->text+start,length,&scripts).code!=SB_OK)return false;
    SBShapeFontSpan *fonts=NULL;size_t used=0,capacity=0,style=0,script=0,previous=0;TTF_Font *last=NULL;const struct nk_user_font *last_face=NULL;
    while(style<text->count && text->spans[style].offset+text->spans[style].length<=start)++style;
    SBGrapheme reader={.text=text->text+start,.length=length};SBGraphemeBoundary b;sb_grapheme_next(&reader,&b);
    while(sb_grapheme_next(&reader,&b)) {
        while(style<text->count && text->spans[style].offset+text->spans[style].length<=start+previous)++style;
        while(script+1<scripts.count && scripts.spans[script].byte+scripts.spans[script].length<=previous)++script;
        const struct nk_user_font *face=sb_ui_text_style(ui,base,style<text->count ? text->spans[style].style : 0);
        TTF_Font *font=sb_ui_cluster_font(face,text->text+start+previous,b.byte-previous,last_face==face ? last : NULL);uint32_t tag=scripts.count ? scripts.spans[script].script : SB_SCRIPT_COMMON;
        if(!font){free(fonts);sb_script_plan_free(&scripts);return false;}
        if(used && fonts[used-1].font==font && fonts[used-1].script==tag)fonts[used-1].length+=b.byte-previous;
        else{if(used==capacity){size_t next=capacity ? capacity*2 : 16;SBShapeFontSpan *grown=realloc(fonts,next*sizeof(*grown));if(!grown){free(fonts);sb_script_plan_free(&scripts);return false;}fonts=grown;capacity=next;}fonts[used++]=(SBShapeFontSpan){previous,b.byte-previous,font,tag};}
        previous=b.byte;last=font;last_face=face;
    }
    sb_script_plan_free(&scripts);*out=fonts;*count=used;return true;
}
static bool styled_blank(const char *text,size_t a,size_t b){return b==a+1 && (text[a]==' ' || text[a]=='\t');}
static bool styled_paragraph(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,size_t start,size_t length,float available,float gap,StyledPlan *plan) {
    SBTextParagraph *bidi=NULL;SBShapeParagraph *prepared=NULL;SBShapeFontSpan *fonts=NULL;size_t font_count=0;
    if(sb_bidi_paragraph_create(text->text+start,length,SB_BIDI_AUTO_LTR,&bidi).code!=SB_OK)return false;
    bool success=styled_fonts(ui,base,text,start,length,&fonts,&font_count) && sb_shape_paragraph_create(bidi,fonts,font_count,&prepared).code==SB_OK;
    uint32_t *boundaries=NULL;size_t count=0,capacity=0;
    if(success){SBGrapheme reader={.text=text->text+start,.length=length};SBGraphemeBoundary b;
        while(sb_grapheme_next(&reader,&b)){if(count==capacity){size_t next=capacity ? capacity*2 : 32;uint32_t *grown=realloc(boundaries,next*sizeof(*grown));if(!grown){success=false;break;}boundaries=grown;capacity=next;}boundaries[count++]=(uint32_t)b.byte;}}
    /* Paragraph separators belong to analysis, not to a visible glyph line. */
    size_t terminal=count ? count-1 : 0;
    if(terminal){const char *cp_at=text->text+start+boundaries[terminal-1];uint32_t cp=SDL_StepUTF8(&cp_at,NULL);if(sb_bidi_separator(cp))--terminal;}
    /* A full shape supplies only a wrap estimate. Every chosen line is shaped
       again with its own context and checked exactly; ligatures and joining
       at a wrap boundary may differ from the paragraph-wide estimate. */
    double *prefix=NULL;
    if(success && terminal && length<=65536){
        SBShapedLine full={0};
        if(sb_shape_paragraph_line(prepared,0,boundaries[terminal],&full).code==SB_OK){
            prefix=calloc(terminal+1,sizeof(*prefix));
            if(prefix){for(size_t g=0;g<full.count;++g){size_t lo=0,hi=terminal;
                    while(lo<hi){size_t mid=lo+(hi-lo)/2;if(boundaries[mid+1]<=full.glyphs[g].byte)lo=mid+1;else hi=mid;}
                    if(lo<terminal)prefix[lo+1]+=fmaxf(0,full.glyphs[g].advance)/ui->density;}
                for(size_t i=1;i<=terminal;++i)prefix[i]+=prefix[i-1];}
        }
        sb_shape_line_free(&full);
    }
    size_t begin=0;
    while(success && begin<terminal) {
        size_t low=begin+1,high=low;SBShapedLine trial={0};
        if(prefix){size_t lo=begin+1,hi=terminal+1;
            while(lo<hi){size_t mid=lo+(hi-lo)/2;if(prefix[mid]-prefix[begin]<=available)lo=mid+1;else hi=mid;}
            low=high=lo>begin+1 ? lo-1 : begin+1;}
        success=sb_shape_paragraph_line(prepared,boundaries[begin],boundaries[high]-boundaries[begin],&trial).code==SB_OK;
        if(!success)break;
        bool fits=trial.advance/ui->density<=available;sb_shape_line_free(&trial);
        if(prefix){
            while(success && !fits && low>begin+1){--low;
                success=sb_shape_paragraph_line(prepared,boundaries[begin],boundaries[low]-boundaries[begin],&trial).code==SB_OK;
                fits=success && trial.advance/ui->density<=available;sb_shape_line_free(&trial);}
            while(success && fits && low<terminal){
                success=sb_shape_paragraph_line(prepared,boundaries[begin],boundaries[low+1]-boundaries[begin],&trial).code==SB_OK;
                fits=success && trial.advance/ui->density<=available;sb_shape_line_free(&trial);if(fits)++low;}
        }else if(fits){
            for(;;){if(high==terminal){low=high;break;}size_t next=begin+2*(high-begin);if(next>terminal)next=terminal;
                success=sb_shape_paragraph_line(prepared,boundaries[begin],boundaries[next]-boundaries[begin],&trial).code==SB_OK;
                if(!success)break;fits=trial.advance/ui->density<=available;sb_shape_line_free(&trial);if(!fits){low=high;high=next;break;}high=next;}
            while(success && high>low+1){size_t mid=low+(high-low)/2;success=sb_shape_paragraph_line(prepared,boundaries[begin],boundaries[mid]-boundaries[begin],&trial).code==SB_OK;
                if(!success)break;fits=trial.advance/ui->density<=available;sb_shape_line_free(&trial);if(fits)low=mid;else high=mid;}
        }
        size_t end=low;
        if(end<terminal && end>begin+1){size_t space=end;while(space>begin && !styled_blank(text->text+start,boundaries[space-1],boundaries[space]))--space;if(space>begin)end=space;}
        size_t visible=end;while(visible>begin+1 && styled_blank(text->text+start,boundaries[visible-1],boundaries[visible]))--visible;
        if(success)success=sb_shape_paragraph_line(prepared,boundaries[begin],boundaries[visible]-boundaries[begin],&trial).code==SB_OK;
        if(success){success=styled_line_add(plan,&trial,ui,base,gap);if(success)plan->lines[plan->count-1].source_offset=start;}sb_shape_line_free(&trial);
        begin=end;while(begin<terminal && styled_blank(text->text+start,boundaries[begin],boundaries[begin+1]))++begin;
    }
    if(success && !terminal){SBShapedLine empty={.base_level=sb_bidi_paragraph_level(bidi)};success=styled_line_add(plan,&empty,ui,base,gap);if(success)plan->lines[plan->count-1].source_offset=start;}
    free(prefix);free(boundaries);free(fonts);sb_shape_paragraph_free(prepared);sb_bidi_paragraph_free(bidi);return success;
}
static bool styled_build(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,float width,struct nk_vec2 padding,float gap,bool terminal_line,StyledPlan *plan) {
    SBStyledText display={.text=text->text};if(!sb_ui_styled_spans(text,&display.spans,&display.count))return false;
    size_t length=strlen(text->text),start=0;bool ok=true;float available=fmaxf(1,width-2*padding.x);
    while(start<length && ok){size_t end=styled_paragraph_end(text->text,start,length);ok=styled_paragraph(ui,base,&display,start,end-start,available,gap,plan);start=end;}
    const char *last=text->text+length;
    bool trailing=terminal_line && length && sb_bidi_separator(SDL_StepBackUTF8(text->text,&last));
    if(ok && (!length || trailing)){SBShapedLine empty={0};ok=styled_line_add(plan,&empty,ui,base,gap);if(ok)plan->lines[plan->count-1].source_offset=length;}
    free(display.spans);if(!ok){styled_plan_free(plan);return false;}plan->height+=2*padding.y;return true;
}
#define SB_STYLED_SETS 512
#define SB_STYLED_WAYS 4
#define SB_STYLED_BYTES (32u*1024u*1024u)
typedef struct {SBStyledText text;StyledPlan plan;const struct nk_user_font *font;float width,gap;bool terminal_line;struct nk_vec2 padding;uint64_t hash,stamp;size_t bytes;} StyledMemo;
struct SBStyledCache {StyledMemo entries[SB_STYLED_SETS][SB_STYLED_WAYS];size_t bytes;uint64_t stamp;};
static void styled_memo_free(StyledMemo *memo){sb_styled_free(&memo->text);styled_plan_free(&memo->plan);memset(memo,0,sizeof(*memo));}
void sb_ui_styled_cache_clear(SBUi *ui){if(!ui->styled_cache)return;for(size_t i=0;i<SB_STYLED_SETS;++i)for(size_t j=0;j<SB_STYLED_WAYS;++j)styled_memo_free(&ui->styled_cache->entries[i][j]);free(ui->styled_cache);ui->styled_cache=NULL;}
static StyledMemo *styled_memo(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,float width,struct nk_vec2 padding,float gap,bool terminal_line) {
    size_t length=strlen(text->text);if(ui->styled_cache_disabled || length>65536)return NULL;
    uint64_t hash=sb_hash(text->text,length);
    for(size_t i=0;i<text->count;++i){hash=hash*1099511628211ULL^text->spans[i].offset;hash=hash*1099511628211ULL^text->spans[i].length;hash=hash*1099511628211ULL^text->spans[i].style;}
    if(!ui->styled_cache)ui->styled_cache=calloc(1,sizeof(*ui->styled_cache));if(!ui->styled_cache)return NULL;
    SBStyledCache *cache=ui->styled_cache;StyledMemo *slot=&cache->entries[hash%SB_STYLED_SETS][0];
    for(size_t j=0;j<SB_STYLED_WAYS;++j){StyledMemo *memo=&cache->entries[hash%SB_STYLED_SETS][j];bool spans=memo->text.count==text->count;
        if(spans)for(size_t i=0;i<text->count;++i)if(memo->text.spans[i].offset!=text->spans[i].offset || memo->text.spans[i].length!=text->spans[i].length || memo->text.spans[i].style!=text->spans[i].style){spans=false;break;}
        if(memo->text.text && memo->hash==hash && memo->font==base && memo->width==width && memo->terminal_line==terminal_line && memo->gap==gap && memo->padding.x==padding.x && memo->padding.y==padding.y && spans && !strcmp(memo->text.text,text->text)){memo->stamp=++cache->stamp;return memo;}
        if(memo->stamp<slot->stamp)slot=memo;
    }
    StyledMemo next={.font=base,.width=width,.padding=padding,.gap=gap,.terminal_line=terminal_line,.hash=hash};
    next.text.text=malloc(length+1);next.text.spans=text->count ? malloc(text->count*sizeof(*text->spans)) : NULL;
    if(!next.text.text || (text->count && !next.text.spans)){styled_memo_free(&next);return NULL;}
    memcpy(next.text.text,text->text,length+1);next.text.count=text->count;if(text->count)memcpy(next.text.spans,text->spans,text->count*sizeof(*text->spans));
    if(!styled_build(ui,base,&next.text,width,padding,gap,terminal_line,&next.plan)){styled_memo_free(&next);return NULL;}
    next.bytes=length+1+text->count*sizeof(*text->spans)+next.plan.capacity*sizeof(*next.plan.lines);
    for(size_t i=0;i<next.plan.count;++i)next.bytes+=next.plan.lines[i].shape.capacity*sizeof(SBShapeGlyph)+next.plan.lines[i].shape.run_capacity*sizeof(SBShapeRun);
    if(next.bytes>SB_STYLED_BYTES){styled_memo_free(&next);return NULL;}
    if(cache->bytes+next.bytes>SB_STYLED_BYTES){sb_ui_styled_cache_clear(ui);ui->styled_cache=calloc(1,sizeof(*ui->styled_cache));if(!ui->styled_cache){styled_memo_free(&next);return NULL;}cache=ui->styled_cache;slot=&cache->entries[hash%SB_STYLED_SETS][0];}
    cache->bytes-=slot->bytes;styled_memo_free(slot);next.stamp=++cache->stamp;*slot=next;cache->bytes+=next.bytes;return slot;
}
float sb_ui_styled_height(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,float width){StyledMemo *memo=styled_memo(ui,base,text,width,ui->ctx->style.text.padding,4*ui->scale,false);if(memo)return memo->plan.height;StyledPlan plan={0};if(!styled_build(ui,base,text,width,ui->ctx->style.text.padding,4*ui->scale,false,&plan))return 0;float height=plan.height;styled_plan_free(&plan);return height;}
void sb_ui_styled_draw(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text){sb_ui_styled_aligned(ui,base,text,NK_TEXT_LEFT);}
bool sb_ui_styled_geometry(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,float width,SBStyledGeometryVisitor visitor,void *user) {
    if(!visitor)return false;StyledMemo *memo=styled_memo(ui,base,text,width,ui->ctx->style.text.padding,4*ui->scale,false);StyledPlan fresh={0};
    if(!memo && !styled_build(ui,base,text,width,ui->ctx->style.text.padding,4*ui->scale,false,&fresh))return false;
    const StyledPlan *plan=memo ? &memo->plan : &fresh;bool ok=true;
    for(size_t i=0;i<plan->count && ok;++i)ok=visitor(user,&plan->lines[i].shape,plan->lines[i].source_offset,plan->lines[i].y);
    styled_plan_free(&fresh);return ok;
}
void sb_ui_styled_aligned(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,nk_flags alignment) {
    struct nk_rect bounds;if(nk_widget(&bounds,ui->ctx)==NK_WIDGET_INVALID)return;
    StyledMemo *memo=styled_memo(ui,base,text,bounds.w,ui->ctx->style.text.padding,4*ui->scale,false);StyledPlan fresh={0};
    if(!memo && !styled_build(ui,base,text,bounds.w,ui->ctx->style.text.padding,4*ui->scale,false,&fresh))return;
    const StyledPlan *plan=memo ? &memo->plan : &fresh;struct nk_vec2 padding=ui->ctx->style.text.padding;float available=fmaxf(1,bounds.w-2*padding.x);
    for(size_t i=0;i<plan->count;++i){const StyledLine *line=&plan->lines[i];float measured=line->shape.advance/ui->density,shift=0;
        if(alignment&NK_TEXT_ALIGN_RIGHT)shift=fmaxf(0,available-measured);else if(alignment&NK_TEXT_ALIGN_CENTERED)shift=fmaxf(0,(available-measured)/2);
        sb_ui_shaped_draw(ui,&line->shape,bounds.x+padding.x+shift,bounds.y+padding.y+line->y+line->shape.ascent/ui->density,ui->ctx->style.text.color);
    }
    styled_plan_free(&fresh);
}
