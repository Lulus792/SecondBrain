/* Shared measurement and drawing layout for own Markdown style spans. */
typedef struct { SBUi *ui; const struct nk_user_font *base; const SBStyledText *text; size_t span; } StyledCursor;
typedef struct { float *widths; size_t count,capacity; } StyledLines;
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
static float styled_layout_view(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,struct nk_rect bounds,bool draw,nk_flags alignment,StyledLines *lines) {
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
            if (draw) {
                float above,below;
                if (!sb_ui_text_metrics(font,text->text+begin,fitting,&above,&below)) return 0;
                nk_draw_text(nk_window_get_canvas(ctx),nk_rect(bounds.x+pad.x+x+shift,bounds.y+pad.y+y+ascent-above,fmaxf(measured,1),font->height),
                    text->text+begin,(int)fitting,font,nk_rgba(0,0,0,0),ctx->style.text.color);
            }
            x+=measured; begin+=fitting;
            if (!draw && !styled_line(lines,line,x)) return 0;
            if (begin<length && (begin<limit || x>=width)) { x=0; y+=line_height; ++line; }
        }
    }
    return y+line_height+2*pad.y;
}
static float styled_layout(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,struct nk_rect bounds,bool draw,nk_flags alignment,StyledLines *lines) {
    size_t length=strlen(text->text),covered=0;
    for (size_t i=0;i<text->count;++i) {
        SBTextSpan span=text->spans[i];
        if (span.offset!=covered || !span.length || span.length>length-covered) return 0;
        covered+=span.length;
    }
    if (covered!=length) return 0;
    SBGrapheme reader; SBGraphemeBoundary boundary;
    if (!sb_grapheme_init(&reader,text->text,length)) return 0;
    SBStyledText display={.text=text->text};
    display.spans=malloc((text->count ? text->count : 1)*sizeof(*display.spans));
    if (!display.spans) return 0;
    size_t start=0,index=0; sb_grapheme_next(&reader,&boundary);
    while (sb_grapheme_next(&reader,&boundary)) {
        while (index<text->count && text->spans[index].offset+text->spans[index].length<=start) ++index;
        unsigned style=index<text->count ? text->spans[index].style : 0;
        /* A grapheme keeps the style of its first scalar; marks never split it. */
        if (display.count && display.spans[display.count-1].style==style) display.spans[display.count-1].length+=boundary.byte-start;
        else display.spans[display.count++]=(SBTextSpan){start,boundary.byte-start,style};
        start=boundary.byte;
    }
    float height=styled_layout_view(ui,base,&display,bounds,draw,alignment,lines); free(display.spans); return height;
}
float sb_ui_styled_height(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,float width) {
    return styled_layout(ui,base,text,nk_rect(0,0,width,0),false,NK_TEXT_LEFT,NULL);
}
void sb_ui_styled_draw(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text) {
    sb_ui_styled_aligned(ui,base,text,NK_TEXT_LEFT);
}
void sb_ui_styled_aligned(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,nk_flags alignment) {
    struct nk_rect bounds; nk_widget(&bounds,ui->ctx); StyledLines lines={0};
    bool aligned=(alignment&(NK_TEXT_ALIGN_RIGHT|NK_TEXT_ALIGN_CENTERED))!=0;
    if (aligned && !styled_layout(ui,base,text,bounds,false,alignment,&lines)) { free(lines.widths); return; }
    styled_layout(ui,base,text,bounds,true,alignment,aligned ? &lines : NULL); free(lines.widths);
}
