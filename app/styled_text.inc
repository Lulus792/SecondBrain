/* Shared measurement and drawing layout for own Markdown style spans. */
typedef struct { SBUi *ui; const struct nk_user_font *base; const SBStyledText *text; size_t span; } StyledCursor;
typedef struct { float *widths; size_t count,capacity; } StyledLines;
typedef struct {size_t offset,length,line;float x,y,width;const struct nk_user_font *font;} StyledPiece;
typedef struct {StyledPiece *pieces;size_t count,capacity;StyledLines lines;} StyledPlan;
static bool styled_piece(StyledPlan *plan,StyledPiece piece) {
    if(plan->count==plan->capacity){size_t capacity=plan->capacity ? plan->capacity*2 : 16;StyledPiece *next=realloc(plan->pieces,capacity*sizeof(*next));if(!next)return false;plan->pieces=next;plan->capacity=capacity;}
    plan->pieces[plan->count++]=piece;return true;
}
static bool styled_line(StyledLines *lines,size_t index,float width) {
    if (!lines) return true;
    if (index>=lines->capacity) {
        size_t capacity=lines->capacity ? lines->capacity*2 : 16;
        float *next=realloc(lines->widths,capacity*sizeof(*next)); if (!next) return false;
        lines->widths=next; lines->capacity=capacity;
    }
    if (index>=lines->count) { lines->widths[index]=0; lines->count=index+1; }
    lines->widths[index]=fmaxf(lines->widths[index],width); return true;
}
static const struct nk_user_font *styled_font(StyledCursor *cursor,size_t offset,size_t *end) {
    while (cursor->span<cursor->text->count && cursor->text->spans[cursor->span].offset+cursor->text->spans[cursor->span].length<=offset) ++cursor->span;
    if (cursor->span==cursor->text->count) { *end=strlen(cursor->text->text); return cursor->base; }
    const SBTextSpan *span=&cursor->text->spans[cursor->span]; *end=span->offset+span->length;
    return sb_ui_text_style(cursor->ui,cursor->base,span->style);
}
static size_t styled_chunk(const char *text,size_t begin,size_t end) {
    /* The layout validates UTF-8 once. These views start at complete clusters. */
    SBGrapheme reader={.text=text+begin,.length=end-begin}; SBGraphemeBoundary boundary; size_t previous=0;
    while (sb_grapheme_next(&reader,&boundary)) {
        if (boundary.byte>=2048) return begin+(previous ? previous : boundary.byte);
        previous=boundary.byte;
    }
    return end;
}
static float styled_width(StyledCursor cursor,size_t begin,size_t end,float maximum) {
    float width=0;
    while (begin<end) {
        size_t limit; const struct nk_user_font *font=styled_font(&cursor,begin,&limit);
        if (limit>end) limit=end;
        limit=styled_chunk(cursor.text->text,begin,limit);
        width+=font->width(font->userdata,font->height,cursor.text->text+begin,(int)(limit-begin)); begin=limit;
        if (width>maximum) break;
    }
    return width;
}
static float styled_layout_view(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,struct nk_rect bounds,bool draw,nk_flags alignment,StyledLines *lines,StyledPlan *plan) {
    struct nk_context *ctx=ui->ctx; struct nk_vec2 pad=ctx->style.text.padding;
    float width=fmaxf(1,bounds.w-2*pad.x),ascent=0,descent=0;
    for (size_t i=0;i<text->count;++i) {
        const SBTextSpan *span=&text->spans[i]; const struct nk_user_font *font=sb_ui_text_style(ui,base,span->style);
        float above,below;
        if (!sb_ui_text_metrics(font,text->text+span->offset,span->length,&above,&below)) return 0;
        ascent=fmaxf(ascent,above); descent=fmaxf(descent,below);
    }
    float line_height=fmaxf(base->height,ascent+descent)+4*ui->scale;
    float x=0,y=0; size_t begin=0,length=strlen(text->text),line=0;
    if (!sb_utf8_valid(text->text,length)) return 0;
    StyledCursor cursor={ui,base,text,0};
    while (begin<length) {
        size_t end=begin,previous=0;
        SBGrapheme words={.text=text->text+begin,.length=length-begin}; SBGraphemeBoundary boundary;
        sb_grapheme_next(&words,&boundary); bool spaces=false,first=true;
        while (sb_grapheme_next(&words,&boundary)) {
            bool blank=boundary.byte-previous==1 && (text->text[begin+previous]==' ' || text->text[begin+previous]=='\t');
            if (first) { spaces=blank; first=false; }
            else if (spaces!=blank) break;
            end=begin+boundary.byte; previous=boundary.byte;
        }
        float word=styled_width(cursor,begin,end,width);
        if (!spaces && x>0 && x+word>width) { x=0; y+=line_height; ++line; }
        if (spaces && x+word>width) { x=0; y+=line_height; ++line; begin=end; continue; }
        while (begin<end) {
            size_t limit; const struct nk_user_font *font=styled_font(&cursor,begin,&limit);
            if (limit>end) limit=end;
            limit=styled_chunk(text->text,begin,limit);
            size_t fitting=limit-begin; float measured=font->width(font->userdata,font->height,text->text+begin,(int)fitting);
            if (measured>width-x) {
                fitting=sb_ui_text_fit(font,text->text+begin,fitting,width-x,&measured);
                if (!fitting && x>0) { x=0; y+=line_height; ++line; continue; }
                if (!fitting) {
                    SBGrapheme reader={.text=text->text+begin,.length=limit-begin}; SBGraphemeBoundary boundary;
                    sb_grapheme_next(&reader,&boundary);
                    if (!sb_grapheme_next(&reader,&boundary)) return 0;
                    fitting=boundary.byte; measured=font->width(font->userdata,font->height,text->text+begin,(int)fitting);
                }
            }
            if (!fitting) return 0;
            float shift=0;
            if (draw && lines && line<lines->count) {
                if (alignment&NK_TEXT_ALIGN_RIGHT) shift=fmaxf(0,width-lines->widths[line]);
                else if (alignment&NK_TEXT_ALIGN_CENTERED) shift=fmaxf(0,(width-lines->widths[line])/2);
            }
            if (draw || plan) {
                float above,below;
                if (!sb_ui_text_metrics(font,text->text+begin,fitting,&above,&below)) return 0;
                if(plan && !styled_piece(plan,(StyledPiece){begin,fitting,line,pad.x+x,pad.y+y+ascent-above,fmaxf(measured,1),font}))return 0;
                if(draw)nk_draw_text(nk_window_get_canvas(ctx),nk_rect(bounds.x+pad.x+x+shift,bounds.y+pad.y+y+ascent-above,fmaxf(measured,1),font->height),
                    text->text+begin,(int)fitting,font,nk_rgba(0,0,0,0),ctx->style.text.color);
            }
            x+=measured; begin+=fitting;
            if (!draw && !styled_line(lines,line,x)) return 0;
            if (begin<length && (begin<limit || x>=width)) { x=0; y+=line_height; ++line; }
        }
    }
    return y+line_height+2*pad.y;
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
static float styled_layout(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,struct nk_rect bounds,bool draw,nk_flags alignment,StyledLines *lines) {
    SBStyledText display={.text=text->text};
    if (!sb_ui_styled_spans(text,&display.spans,&display.count)) return 0;
    float height=styled_layout_view(ui,base,&display,bounds,draw,alignment,lines,NULL); free(display.spans); return height;
}
#define SB_STYLED_SETS 512
#define SB_STYLED_WAYS 4
#define SB_STYLED_BYTES (32u*1024u*1024u)
typedef struct {
    SBStyledText text;StyledPlan plan;const struct nk_user_font *font;
    float width,height;struct nk_vec2 padding;uint64_t hash,stamp;size_t bytes;
} StyledMemo;
struct SBStyledCache {StyledMemo entries[SB_STYLED_SETS][SB_STYLED_WAYS];size_t bytes;uint64_t stamp;};
static void styled_memo_free(StyledMemo *memo){sb_styled_free(&memo->text);free(memo->plan.pieces);free(memo->plan.lines.widths);memset(memo,0,sizeof(*memo));}
void sb_ui_styled_cache_clear(SBUi *ui) {
    if(!ui->styled_cache)return;
    for(size_t i=0;i<SB_STYLED_SETS;++i)for(size_t j=0;j<SB_STYLED_WAYS;++j)styled_memo_free(&ui->styled_cache->entries[i][j]);
    free(ui->styled_cache);ui->styled_cache=NULL;
}
static StyledMemo *styled_memo(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,float width) {
    size_t length=strlen(text->text);if(ui->styled_cache_disabled || length>65536)return NULL;
    uint64_t hash=sb_hash(text->text,length);
    for(size_t i=0;i<text->count;++i){hash=hash*1099511628211ULL^text->spans[i].offset;hash=hash*1099511628211ULL^text->spans[i].length;hash=hash*1099511628211ULL^text->spans[i].style;}
    if(!ui->styled_cache)ui->styled_cache=calloc(1,sizeof(*ui->styled_cache));if(!ui->styled_cache)return NULL;
    SBStyledCache *cache=ui->styled_cache;StyledMemo *slot=&cache->entries[hash%SB_STYLED_SETS][0];
    for(size_t j=0;j<SB_STYLED_WAYS;++j) {
        StyledMemo *memo=&cache->entries[hash%SB_STYLED_SETS][j];bool spans=memo->text.count==text->count;
        if(spans)for(size_t i=0;i<text->count;++i)if(memo->text.spans[i].offset!=text->spans[i].offset || memo->text.spans[i].length!=text->spans[i].length || memo->text.spans[i].style!=text->spans[i].style){spans=false;break;}
        if(memo->text.text && memo->hash==hash && memo->font==base && memo->width==width && memo->padding.x==ui->ctx->style.text.padding.x && memo->padding.y==ui->ctx->style.text.padding.y && spans && !strcmp(memo->text.text,text->text)){memo->stamp=++cache->stamp;return memo;}
        if(memo->stamp<slot->stamp)slot=memo;
    }
    StyledMemo next={.font=base,.width=width,.padding=ui->ctx->style.text.padding,.hash=hash};
    next.text.text=malloc(length+1);next.text.spans=text->count ? malloc(text->count*sizeof(*text->spans)) : NULL;
    if(!next.text.text || (text->count && !next.text.spans)){styled_memo_free(&next);return NULL;}
    memcpy(next.text.text,text->text,length+1);next.text.count=text->count;if(text->count)memcpy(next.text.spans,text->spans,text->count*sizeof(*text->spans));
    SBStyledText display={.text=next.text.text};
    if(sb_ui_styled_spans(&next.text,&display.spans,&display.count))next.height=styled_layout_view(ui,base,&display,nk_rect(0,0,width,0),false,NK_TEXT_LEFT,&next.plan.lines,&next.plan);
    free(display.spans);
    if(next.height<=0){styled_memo_free(&next);return NULL;}
    next.bytes=length+1+text->count*sizeof(*text->spans)+next.plan.capacity*sizeof(*next.plan.pieces)+next.plan.lines.capacity*sizeof(float);
    if(next.bytes>SB_STYLED_BYTES){styled_memo_free(&next);return NULL;}
    if(cache->bytes+next.bytes>SB_STYLED_BYTES){sb_ui_styled_cache_clear(ui);ui->styled_cache=calloc(1,sizeof(*ui->styled_cache));if(!ui->styled_cache){styled_memo_free(&next);return NULL;}cache=ui->styled_cache;slot=&cache->entries[hash%SB_STYLED_SETS][0];}
    cache->bytes-=slot->bytes;styled_memo_free(slot);next.stamp=++cache->stamp;*slot=next;cache->bytes+=next.bytes;return slot;
}
float sb_ui_styled_height(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,float width) {
    StyledMemo *memo=styled_memo(ui,base,text,width);return memo ? memo->height : styled_layout(ui,base,text,nk_rect(0,0,width,0),false,NK_TEXT_LEFT,NULL);
}
void sb_ui_styled_draw(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text) {sb_ui_styled_aligned(ui,base,text,NK_TEXT_LEFT);}
void sb_ui_styled_aligned(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,nk_flags alignment) {
    struct nk_rect bounds;if(nk_widget(&bounds,ui->ctx)==NK_WIDGET_INVALID)return;
    StyledMemo *memo=styled_memo(ui,base,text,bounds.w);
    if(memo) {
        float available=fmaxf(1,bounds.w-2*memo->padding.x);
        for(size_t i=0;i<memo->plan.count;++i){StyledPiece p=memo->plan.pieces[i];float shift=0;
            if(p.line<memo->plan.lines.count){if(alignment&NK_TEXT_ALIGN_RIGHT)shift=fmaxf(0,available-memo->plan.lines.widths[p.line]);else if(alignment&NK_TEXT_ALIGN_CENTERED)shift=fmaxf(0,(available-memo->plan.lines.widths[p.line])/2);}
            nk_draw_text(nk_window_get_canvas(ui->ctx),nk_rect(bounds.x+p.x+shift,bounds.y+p.y,p.width,p.font->height),memo->text.text+p.offset,(int)p.length,p.font,nk_rgba(0,0,0,0),ui->ctx->style.text.color);
        }return;
    }
    StyledLines lines={0};bool aligned=(alignment&(NK_TEXT_ALIGN_RIGHT|NK_TEXT_ALIGN_CENTERED))!=0;
    if(aligned && !styled_layout(ui,base,text,bounds,false,alignment,&lines)){free(lines.widths);return;}
    styled_layout(ui,base,text,bounds,true,alignment,aligned ? &lines : NULL);free(lines.widths);
}
