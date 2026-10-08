#include "shaped.h"
#include "grapheme.h"
#include "ttf_shape.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

void sb_shape_line_free(SBShapedLine *line){if(line){free(line->glyphs);free(line->runs);memset(line,0,sizeof(*line));}}
static bool span_boundaries(const char *text,size_t length,const SBShapeFontSpan *spans,size_t count,size_t byte,size_t end) {
    SBGrapheme reader;SBGraphemeBoundary b;
    if(!sb_grapheme_init(&reader,text,length))return false;
    size_t i=0;bool start=false,finish=false;
    while(sb_grapheme_next(&reader,&b)) {
        start|=b.byte==byte;finish|=b.byte==end;
        while(i<count && spans[i].byte+spans[i].length==b.byte)++i;
        if(i<count && spans[i].byte+spans[i].length<b.byte)return false;
    }
    return i==count && start && finish;
}
static size_t span_at(const SBShapeFontSpan *spans,size_t count,size_t byte) {
    size_t lo=0,hi=count;
    while(lo<hi){size_t mid=lo+(hi-lo)/2;if(spans[mid].byte+spans[mid].length<=byte)lo=mid+1;else hi=mid;}
    return lo;
}
static void *reserve(void *old,size_t wanted,size_t *capacity,size_t element) {
    if(wanted<=*capacity)return old;
    size_t next=*capacity ? *capacity : 16,maximum=SIZE_MAX/element;
    if(wanted>maximum)return NULL;
    while(next<wanted){if(next>maximum/2){next=wanted;break;}next*=2;}
    void *grown=realloc(old,next*element);
    if(grown)*capacity=next;
    return grown;
}
static Uint32 shape_mirror(Uint32 cp){return (Uint32)sb_bidi_mirror(cp);}
static SBStatus append_range(SBShapedLine *line,const SBTextParagraph *p,size_t context_byte,size_t context_length,
    size_t begin,size_t length,const SBShapeFontSpan *span,unsigned char level) {
    SBTTFShape shaped={0};
    if(!sb_ttf_shape_range(span->font,sb_bidi_paragraph_text(p)+context_byte,context_length,
                          begin-context_byte,length,(level&1)!=0,span->script,shape_mirror,&shaped))
        return sb_error(SB_IO,"Schriftlauf konnte nicht geformt werden: %s",SDL_GetError());
    if(!isfinite(shaped.advance) || shaped.advance<0 ||
       shaped.count>SIZE_MAX/sizeof(*line->glyphs)-line->count || line->run_count==SIZE_MAX/sizeof(*line->runs)) {
        sb_ttf_shape_free(&shaped);return sb_error(SB_LIMIT,"Schriftlauf ist zu groß.");
    }
    SBShapeRun *runs=reserve(line->runs,line->run_count+1,&line->run_capacity,sizeof(*runs));
    if(!runs){sb_ttf_shape_free(&shaped);return sb_error(SB_MEMORY,"Zeilenlayout benötigt mehr Speicher.");}
    line->runs=runs;
    if(shaped.count) {
        SBShapeGlyph *glyphs=reserve(line->glyphs,line->count+shaped.count,&line->capacity,sizeof(*glyphs));
        if(!glyphs){sb_ttf_shape_free(&shaped);return sb_error(SB_MEMORY,"Zeilenlayout benötigt mehr Speicher.");}
        line->glyphs=glyphs;
    }
    runs[line->run_count++]=(SBShapeRun){begin,length,line->count,shaped.count,line->advance,shaped.advance,level};
    for(size_t i=0;i<shaped.count;++i) {
        const SBTTFGlyph *g=&shaped.glyphs[i];
        line->glyphs[line->count++]=(SBShapeGlyph){context_byte+g->byte,g->index,span->font,
            line->advance+g->x,g->y,g->advance,g->left,g->top,g->width,g->height,level};
    }
    line->advance+=shaped.advance;
    if(shaped.ascent>line->ascent)line->ascent=shaped.ascent;
    if(shaped.descent>line->descent)line->descent=shaped.descent;
    sb_ttf_shape_free(&shaped);return sb_ok();
}
SBStatus sb_shape_line(const SBTextParagraph *p,size_t byte,size_t length,const SBShapeFontSpan *spans,size_t count,SBShapedLine *out) {
    size_t total=sb_bidi_paragraph_length(p),covered=0;
    if(!p || !out || out->glyphs || out->runs || out->count || out->run_count || out->capacity || out->run_capacity || !spans || !count ||
       count>total || !length || byte>total || length>total-byte)return sb_error(SB_INVALID,"Ungültiger geformter Zeilenbereich.");
    for(size_t i=0;i<count;++i) {
        if(spans[i].byte!=covered || !spans[i].font || !spans[i].length || spans[i].length>total-covered)
            return sb_error(SB_INVALID,"Schriftbereiche decken den Absatz nicht vollständig ab.");
        covered+=spans[i].length;
    }
    if(covered!=total || !span_boundaries(sb_bidi_paragraph_text(p),total,spans,count,byte,byte+length))
        return sb_error(SB_INVALID,"Schrift- oder Zeilenbereiche trennen ein Graphem.");
    SBVisualLine visual={0};SBStatus status=sb_bidi_line(p,byte,length,&visual);
    if(status.code!=SB_OK)return status;
    SBShapedLine line={.byte=byte,.length=length,.base_level=visual.base_level};
    for(size_t r=0;r<visual.count && status.code==SB_OK;++r) {
        SBVisualRun run=visual.runs[r];size_t run_end=run.byte+run.length;
        size_t first=span_at(spans,count,run.byte),last=span_at(spans,count,run_end-1)+1;
        /* Odd-level font pieces must also be visited in visual order. */
        if(run.level&1) {
            for(size_t i=last;i>first && status.code==SB_OK;--i) {
                const SBShapeFontSpan *span=&spans[i-1];size_t begin=span->byte>run.byte ? span->byte : run.byte;
                size_t end=span->byte+span->length<run_end ? span->byte+span->length : run_end;
                if(begin<end)status=append_range(&line,p,byte,length,begin,end-begin,span,run.level);
            }
        } else {
            for(size_t i=first;i<last && status.code==SB_OK;++i) {
                const SBShapeFontSpan *span=&spans[i];size_t begin=span->byte>run.byte ? span->byte : run.byte;
                size_t end=span->byte+span->length<run_end ? span->byte+span->length : run_end;
                if(begin<end)status=append_range(&line,p,byte,length,begin,end-begin,span,run.level);
            }
        }
    }
    sb_bidi_line_free(&visual);
    if(status.code!=SB_OK){sb_shape_line_free(&line);return status;}
    *out=line;return sb_ok();
}
