/* Shared paragraph analysis, logical wrapping and actual glyph geometry. */
#include "scripts.h"
#include <SDL3_ttf/SDL_ttf.h>
typedef SBStyledLine StyledLine;
typedef SBStyledPlan StyledPlan;
static void styled_plan_free(StyledPlan *plan){for(size_t i=0;i<plan->count;++i){sb_shape_line_free(&plan->lines[i].shape);free(plan->lines[i].boxes);}free(plan->lines);sb_native_text_free(&plan->native);memset(plan,0,sizeof(*plan));}
static bool styled_line_add(StyledPlan *plan,SBShapedLine *line,SBUi *ui,const struct nk_user_font *base,float gap,SBNativeCharBox *boxes,size_t box_count) {
    if(ui->layout_budget){
        size_t bytes=line->capacity*sizeof(*line->glyphs)+line->run_capacity*sizeof(*line->runs)+line->count+sizeof(uint8_t *)+box_count*sizeof(*boxes);
        if(plan->count==plan->capacity)bytes+=(plan->capacity ? plan->capacity : 16)*sizeof(*plan->lines);
        if(bytes>*ui->layout_budget)return false;*ui->layout_budget-=bytes;
    }
    if(plan->count==plan->capacity){size_t next=plan->capacity ? plan->capacity*2 : 16;StyledLine *grown=realloc(plan->lines,next*sizeof(*grown));if(!grown)return false;plan->lines=grown;plan->capacity=next;}
    plan->lines[plan->count++]=(StyledLine){.shape=*line,.y=plan->height,.boxes=boxes,.box_count=box_count};memset(line,0,sizeof(*line));
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
        if(ui->layout_cancel && SDL_GetAtomicInt(ui->layout_cancel)){free(fonts);sb_script_plan_free(&scripts);return false;}
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
static bool styled_boxes(SBUi *ui,const struct nk_user_font *base,const SBShapeParagraph *paragraph,const SBShapedLine *line,const char *source,size_t consumed,SBNativeCharBox **out,size_t *count){
    SBCaretPlan carets={0};size_t end=line->byte+line->length;
    if(line->length && sb_caret_plan(paragraph,line,&carets).code!=SB_OK)return false;
    SBGrapheme reader;SBGraphemeBoundary b;size_t extra=0;
    if(!sb_grapheme_init(&reader,source+end,consumed-end)){sb_caret_plan_free(&carets);return false;}
    sb_grapheme_next(&reader,&b);while(sb_grapheme_next(&reader,&b))++extra;
    size_t n=carets.cluster_count+extra;if(!n)n=1;
    if(ui->layout_budget && n>*ui->layout_budget/sizeof(SBNativeCharBox)){sb_caret_plan_free(&carets);return false;}
    SBNativeCharBox *boxes=calloc(n,sizeof(*boxes));if(!boxes){sb_caret_plan_free(&carets);return false;}
    float height=fmaxf(base->height,(line->ascent+line->descent)/ui->density),edge=0;size_t at=0;
    for(size_t i=0;i<carets.cluster_count;++i){SBCaretCluster c=carets.clusters[i];boxes[at++]=(SBNativeCharBox){c.byte,c.length,c.left/ui->density,0,(c.right-c.left)/ui->density,height,c.level};}
    SBCaret caret;if(sb_caret_find(&carets,end,SB_CARET_AFTER,line->base_level,&caret))edge=caret.x/ui->density;
    sb_grapheme_init(&reader,source+end,consumed-end);size_t previous=0;sb_grapheme_next(&reader,&b);
    while(sb_grapheme_next(&reader,&b)){boxes[at++]=(SBNativeCharBox){end+previous,b.byte-previous,edge,0,0,height,line->base_level};previous=b.byte;}
    if(!at)boxes[at++]=(SBNativeCharBox){end,0,edge,0,0,height,line->base_level};
    sb_caret_plan_free(&carets);*out=boxes;*count=at;return true;
}
static SBStatus styled_trial(SBUi *ui,const SBShapeParagraph *paragraph,size_t byte,size_t length,float *advance){
    if(!ui->styled_full_geometry_trials)return sb_shape_paragraph_advance(paragraph,byte,length,advance);
    SBShapedLine line={0};SBStatus status=sb_shape_paragraph_line(paragraph,byte,length,&line);
    if(status.code==SB_OK)*advance=line.advance;sb_shape_line_free(&line);return status;
}
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
        if(ui->layout_cancel && SDL_GetAtomicInt(ui->layout_cancel)){success=false;break;}
        size_t low=begin+1,high=low;SBShapedLine trial={0};float advance=0;
        if(prefix){size_t lo=begin+1,hi=terminal+1;
            while(lo<hi){size_t mid=lo+(hi-lo)/2;if(prefix[mid]-prefix[begin]<=available)lo=mid+1;else hi=mid;}
            low=high=lo>begin+1 ? lo-1 : begin+1;}
        success=styled_trial(ui,prepared,boundaries[begin],boundaries[high]-boundaries[begin],&advance).code==SB_OK;
        if(!success)break;
        bool fits=advance/ui->density<=available;
        if(prefix){
            while(success && !fits && low>begin+1){--low;
                success=styled_trial(ui,prepared,boundaries[begin],boundaries[low]-boundaries[begin],&advance).code==SB_OK;
                fits=success && advance/ui->density<=available;}
            while(success && fits && low<terminal){
                success=styled_trial(ui,prepared,boundaries[begin],boundaries[low+1]-boundaries[begin],&advance).code==SB_OK;
                fits=success && advance/ui->density<=available;if(fits)++low;}
        }else if(fits){
            for(;;){if(high==terminal){low=high;break;}size_t next=begin+2*(high-begin);if(next>terminal)next=terminal;
                success=styled_trial(ui,prepared,boundaries[begin],boundaries[next]-boundaries[begin],&advance).code==SB_OK;
                if(!success)break;fits=advance/ui->density<=available;if(!fits){low=high;high=next;break;}high=next;}
            while(success && high>low+1){size_t mid=low+(high-low)/2;success=styled_trial(ui,prepared,boundaries[begin],boundaries[mid]-boundaries[begin],&advance).code==SB_OK;
                if(!success)break;fits=advance/ui->density<=available;if(fits)low=mid;else high=mid;}
        }
        size_t end=low;
        if(end<terminal && end>begin+1){size_t space=end;while(space>begin && !styled_blank(text->text+start,boundaries[space-1],boundaries[space]))--space;if(space>begin)end=space;}
        size_t visible=end;while(visible>begin+1 && styled_blank(text->text+start,boundaries[visible-1],boundaries[visible]))--visible;
        if(success)success=sb_shape_paragraph_line(prepared,boundaries[begin],boundaries[visible]-boundaries[begin],&trial).code==SB_OK;
        begin=end;while(begin<terminal && styled_blank(text->text+start,boundaries[begin],boundaries[begin+1]))++begin;
        size_t consumed=begin==terminal ? length : boundaries[begin];SBNativeCharBox *boxes=NULL;size_t box_count=0;
        if(success)success=styled_boxes(ui,base,prepared,&trial,text->text+start,consumed,&boxes,&box_count);
        if(success){success=styled_line_add(plan,&trial,ui,base,gap,boxes,box_count);if(success)plan->lines[plan->count-1].source_offset=start;}
        if(!success)free(boxes);sb_shape_line_free(&trial);
    }
    if(success && !terminal){SBShapedLine empty={.base_level=sb_bidi_paragraph_level(bidi)};SBNativeCharBox *boxes=NULL;size_t box_count=0;
        success=styled_boxes(ui,base,prepared,&empty,text->text+start,length,&boxes,&box_count) && styled_line_add(plan,&empty,ui,base,gap,boxes,box_count);
        if(success)plan->lines[plan->count-1].source_offset=start;else free(boxes);}
    free(prefix);free(boundaries);free(fonts);sb_shape_paragraph_free(prepared);sb_bidi_paragraph_free(bidi);return success;
}
static bool styled_native_build(SBUi *ui,const SBStyledText *text,StyledPlan *plan){
    size_t count=0;for(size_t i=0;i<plan->count;++i){if(SIZE_MAX-count<plan->lines[i].box_count)return false;count+=plan->lines[i].box_count;}
    if(!count || count>SIZE_MAX/sizeof(SBNativeCharBox))return false;
    SBNativeCharBox *boxes=malloc(count*sizeof(*boxes));if(!boxes)return false;size_t at=0;
    for(size_t i=0;i<plan->count;++i){StyledLine *line=&plan->lines[i];for(size_t j=0;j<line->box_count;++j){SBNativeCharBox box=line->boxes[j];box.byte+=line->source_offset;box.y+=line->y;boxes[at++]=box;}}
    bool ok=sb_native_text_geometry(text->text,strlen(text->text),text->spans,text->count,boxes,count,&plan->native).code==SB_OK;free(boxes);
    if(ok && plan->native.count<plan->native.capacity){SBNativeTextRun *compact=realloc(plan->native.runs,plan->native.count*sizeof(*compact));if(compact){plan->native.runs=compact;plan->native.capacity=plan->native.count;}}
    if(ok && ui->layout_budget){size_t bytes=plan->native.capacity*sizeof(*plan->native.runs);if(bytes>*ui->layout_budget)ok=false;else *ui->layout_budget-=bytes;}
    return ok;
}
static bool styled_build(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,float width,struct nk_vec2 padding,float gap,bool terminal_line,StyledPlan *plan) {
    SBStyledText display={.text=text->text};if(!sb_ui_styled_spans(text,&display.spans,&display.count))return false;
    size_t length=strlen(text->text),start=0;bool ok=true;float available=fmaxf(1,width-2*padding.x);
    while(start<length && ok){if(ui->layout_cancel && SDL_GetAtomicInt(ui->layout_cancel)){ok=false;break;}size_t end=styled_paragraph_end(text->text,start,length);ok=styled_paragraph(ui,base,&display,start,end-start,available,gap,plan);start=end;}
    const char *last=text->text+length;
    bool trailing=terminal_line && length && sb_bidi_separator(SDL_StepBackUTF8(text->text,&last));
    if(ok && (!length || trailing)){SBShapedLine empty={0};SBNativeCharBox *box=calloc(1,sizeof(*box));if(box)*box=(SBNativeCharBox){0,0,0,0,0,base->height,0};
        ok=box && styled_line_add(plan,&empty,ui,base,gap,box,1);if(ok)plan->lines[plan->count-1].source_offset=length;else free(box);}
    if(ok)ok=styled_native_build(ui,&display,plan);
    free(display.spans);if(!ok){styled_plan_free(plan);return false;}plan->height+=2*padding.y;return true;
}
bool sb_ui_styled_plan(SBUi *ui,const struct nk_user_font *font,const SBStyledText *text,float width,struct nk_vec2 padding,float gap,bool terminal_line,SBStyledPlan *out){
    if(!ui || !font || sb_ui_font_owner(font)!=ui || !text || !text->text || !out || out->lines || out->count || out->capacity || out->native.runs || out->native.count || out->native.capacity || !isfinite(width) || width<=0 || !isfinite(padding.x) || !isfinite(padding.y) || padding.x<0 || padding.y<0 || !isfinite(gap) || gap<0)return false;
    return styled_build(ui,font,text,width,padding,gap,terminal_line,out);
}
void sb_ui_styled_plan_free(SBStyledPlan *plan){if(plan)styled_plan_free(plan);}
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
    next.bytes+=next.plan.native.capacity*sizeof(*next.plan.native.runs);
    for(size_t i=0;i<next.plan.count;++i)next.bytes+=next.plan.lines[i].shape.capacity*sizeof(SBShapeGlyph)+next.plan.lines[i].shape.run_capacity*sizeof(SBShapeRun)+next.plan.lines[i].box_count*sizeof(SBNativeCharBox);
    if(next.bytes>SB_STYLED_BYTES){styled_memo_free(&next);return NULL;}
    if(cache->bytes+next.bytes>SB_STYLED_BYTES){sb_ui_styled_cache_clear(ui);ui->styled_cache=calloc(1,sizeof(*ui->styled_cache));if(!ui->styled_cache){styled_memo_free(&next);return NULL;}cache=ui->styled_cache;slot=&cache->entries[hash%SB_STYLED_SETS][0];}
    cache->bytes-=slot->bytes;styled_memo_free(slot);next.stamp=++cache->stamp;*slot=next;cache->bytes+=next.bytes;return slot;
}
static const StyledPlan *styled_lookup(SBUi *ui,const struct nk_user_font *font,const SBStyledText *text,float width,struct nk_vec2 padding,float gap,bool terminal_line){
    if(!ui->styled_cache_disabled && ui->prepared_lookup){const StyledPlan *plan=ui->prepared_lookup(ui->prepared_data,font,text,width,padding,gap,terminal_line);if(plan)return plan;}
    StyledMemo *memo=styled_memo(ui,font,text,width,padding,gap,terminal_line);return memo ? &memo->plan : NULL;
}
float sb_ui_styled_height(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,float width){const StyledPlan *memo=styled_lookup(ui,base,text,width,ui->ctx->style.text.padding,4*ui->scale,false);if(memo)return memo->height;StyledPlan plan={0};if(!styled_build(ui,base,text,width,ui->ctx->style.text.padding,4*ui->scale,false,&plan))return 0;float height=plan.height;styled_plan_free(&plan);return height;}
void sb_ui_styled_draw(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text){sb_ui_styled_aligned(ui,base,text,NK_TEXT_LEFT);}
bool sb_ui_styled_geometry(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,float width,SBStyledGeometryVisitor visitor,void *user) {
    if(!visitor)return false;const StyledPlan *memo=styled_lookup(ui,base,text,width,ui->ctx->style.text.padding,4*ui->scale,false);StyledPlan fresh={0};
    if(!memo && !styled_build(ui,base,text,width,ui->ctx->style.text.padding,4*ui->scale,false,&fresh))return false;
    const StyledPlan *plan=memo ? memo : &fresh;bool ok=true;
    for(size_t i=0;i<plan->count && ok;++i)ok=visitor(user,&plan->lines[i].shape,plan->lines[i].source_offset,plan->lines[i].y);
    styled_plan_free(&fresh);return ok;
}
static bool styled_native_export(SBUi *ui,const struct nk_user_font *font,const SBStyledText *text,const StyledPlan *plan,struct nk_rect bounds,struct nk_vec2 padding,nk_flags alignment,SBNativeText *out){
    if(!plan->native.count)return false;SBNativeText native=plan->native;native.capacity=native.count;native.runs=malloc(native.count*sizeof(*native.runs));if(!native.runs)return false;
    memcpy(native.runs,plan->native.runs,native.count*sizeof(*native.runs));float available=fmaxf(1,bounds.w-2*padding.x);size_t line_index=0;
    for(size_t i=0;i<native.count;++i){SBNativeTextRun *run=&native.runs[i];if(!run->geometry)continue;
        while(line_index+1<plan->count && plan->lines[line_index+1].y<=run->y)++line_index;
        const StyledLine *line=&plan->lines[line_index];float measured=line->shape.advance/ui->density,shift=0;
        if(alignment&NK_TEXT_ALIGN_RIGHT)shift=fmaxf(0,available-measured);else if(alignment&NK_TEXT_ALIGN_CENTERED)shift=fmaxf(0,(available-measured)/2);
        run->x+=bounds.x+padding.x+shift;run->y+=bounds.y+padding.y;
    }
    /* Literal code changes native style without changing its wrapping/font. */
    if(text->count==1 && !text->spans[0].offset && text->spans[0].length==strlen(text->text))for(size_t i=0;i<native.count;++i)native.runs[i].span.style=text->spans[0].style;
    *out=native;(void)font;return true;
}
bool sb_ui_styled_native(SBUi *ui,const struct nk_user_font *font,const SBStyledText *text,struct nk_rect bounds,nk_flags alignment,SBNativeText *out){
    if(!ui || sb_ui_font_owner(font)!=ui || !ui->ctx || !text || !text->text || !out || !isfinite(bounds.x) || !isfinite(bounds.y) || !isfinite(bounds.w) || !isfinite(bounds.h) || bounds.w<=0 || bounds.h<0)return false;
    struct nk_vec2 padding=ui->ctx->style.text.padding;const StyledPlan *memo=styled_lookup(ui,font,text,bounds.w,padding,4*ui->scale,false);StyledPlan fresh={0};
    if(!memo && !styled_build(ui,font,text,bounds.w,padding,4*ui->scale,false,&fresh))return false;
    bool ok=styled_native_export(ui,font,text,memo ? memo : &fresh,bounds,padding,alignment,out);styled_plan_free(&fresh);return ok;
}
void sb_ui_styled_aligned(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,nk_flags alignment) {
    struct nk_rect bounds;if(nk_widget(&bounds,ui->ctx)==NK_WIDGET_INVALID)return;
    const StyledPlan *memo=styled_lookup(ui,base,text,bounds.w,ui->ctx->style.text.padding,4*ui->scale,false);StyledPlan fresh={0};
    if(!memo && !styled_build(ui,base,text,bounds.w,ui->ctx->style.text.padding,4*ui->scale,false,&fresh))return;
    const StyledPlan *plan=memo ? memo : &fresh;struct nk_vec2 padding=ui->ctx->style.text.padding;float available=fmaxf(1,bounds.w-2*padding.x);
    for(size_t i=0;i<plan->count;++i){const StyledLine *line=&plan->lines[i];float measured=line->shape.advance/ui->density,shift=0;
        if(alignment&NK_TEXT_ALIGN_RIGHT)shift=fmaxf(0,available-measured);else if(alignment&NK_TEXT_ALIGN_CENTERED)shift=fmaxf(0,(available-measured)/2);
        sb_ui_shaped_draw(ui,&line->shape,bounds.x+padding.x+shift,bounds.y+padding.y+line->y+line->shape.ascent/ui->density,ui->ctx->style.text.color);
    }
    styled_plan_free(&fresh);
}
