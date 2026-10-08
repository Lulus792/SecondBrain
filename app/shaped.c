#include "shaped.h"
#include "grapheme.h"
#include "ttf_shape.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
struct SBShapeParagraph {const SBTextParagraph *bidi;SBShapeFontSpan *spans;size_t count;uint32_t *boundaries;size_t boundary_count;};
void sb_shape_paragraph_free(SBShapeParagraph *p){if(p){free(p->spans);free(p->boundaries);free(p);}}

void sb_shape_line_free(SBShapedLine *line){if(line){free(line->glyphs);free(line->runs);memset(line,0,sizeof(*line));}}
static SBStatus span_boundaries(SBShapeParagraph *p,const char *text,size_t length) {
    SBGrapheme reader;SBGraphemeBoundary b;
    if(!sb_grapheme_init(&reader,text,length))return sb_error(SB_INVALID,"Ungültiger Absatztext.");
    size_t i=0,capacity=0;
    while(sb_grapheme_next(&reader,&b)) {
        if(p->boundary_count==capacity){size_t next=capacity ? capacity*2 : 16;uint32_t *grown=realloc(p->boundaries,next*sizeof(*grown));if(!grown)return sb_error(SB_MEMORY,"Absatzgrenzen benötigen mehr Speicher.");p->boundaries=grown;capacity=next;}
        p->boundaries[p->boundary_count++]=(uint32_t)b.byte;
        while(i<p->count && p->spans[i].byte+p->spans[i].length==b.byte)++i;
        if(i<p->count && p->spans[i].byte+p->spans[i].length<b.byte)return sb_error(SB_INVALID,"Schriftbereiche trennen ein Graphem.");
    }
    return i==p->count ? sb_ok() : sb_error(SB_INVALID,"Unvollständige Absatzgrenzen.");
}
static bool cluster_boundary(const SBShapeParagraph *p,size_t byte){size_t lo=0,hi=p->boundary_count;while(lo<hi){size_t mid=lo+(hi-lo)/2;if(p->boundaries[mid]<byte)lo=mid+1;else hi=mid;}return lo<p->boundary_count && p->boundaries[lo]==byte;}
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
SBStatus sb_shape_paragraph_create(const SBTextParagraph *p,const SBShapeFontSpan *spans,size_t count,SBShapeParagraph **out) {
    size_t total=sb_bidi_paragraph_length(p),covered=0;
    if(!out)return sb_error(SB_INVALID,"Absatzausgabe fehlt.");
    *out=NULL;
    if(!p || !spans || !count || count>total || count>SIZE_MAX/sizeof(*spans))return sb_error(SB_INVALID,"Ungültige Schriftbereiche.");
    for(size_t i=0;i<count;++i) {
        if(spans[i].byte!=covered || !spans[i].font || !spans[i].length || spans[i].length>total-covered)
            return sb_error(SB_INVALID,"Schriftbereiche decken den Absatz nicht vollständig ab.");
        covered+=spans[i].length;
    }
    if(covered!=total)return sb_error(SB_INVALID,"Unvollständige Schriftbereiche.");
    SBShapeParagraph *prepared=calloc(1,sizeof(*prepared));
    if(!prepared)return sb_error(SB_MEMORY,"Absatzlayout benötigt mehr Speicher.");
    prepared->bidi=p;prepared->count=count;prepared->spans=malloc(count*sizeof(*spans));
    if(!prepared->spans){sb_shape_paragraph_free(prepared);return sb_error(SB_MEMORY,"Absatzlayout benötigt mehr Speicher.");}
    memcpy(prepared->spans,spans,count*sizeof(*spans));
    SBStatus status=span_boundaries(prepared,sb_bidi_paragraph_text(p),total);
    if(status.code!=SB_OK){sb_shape_paragraph_free(prepared);return status;}
    *out=prepared;return sb_ok();
}
SBStatus sb_shape_paragraph_line(const SBShapeParagraph *prepared,size_t byte,size_t length,SBShapedLine *out) {
    if(!prepared || !out || out->glyphs || out->runs || out->count || out->run_count || out->capacity || out->run_capacity)
        return sb_error(SB_INVALID,"Ungültige Zeilenausgabe.");
    const SBTextParagraph *p=prepared->bidi;size_t total=sb_bidi_paragraph_length(p);
    if(!length || byte>total || length>total-byte || !cluster_boundary(prepared,byte) || !cluster_boundary(prepared,byte+length))
        return sb_error(SB_INVALID,"Ungültiger geformter Zeilenbereich.");
    const SBShapeFontSpan *spans=prepared->spans;size_t count=prepared->count;
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
SBStatus sb_shape_line(const SBTextParagraph *p,size_t byte,size_t length,const SBShapeFontSpan *spans,size_t count,SBShapedLine *out) {
    SBShapeParagraph *prepared=NULL;SBStatus status=sb_shape_paragraph_create(p,spans,count,&prepared);
    if(status.code==SB_OK)status=sb_shape_paragraph_line(prepared,byte,length,out);
    sb_shape_paragraph_free(prepared);return status;
}
