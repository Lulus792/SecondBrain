#include "projection.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
SBStatus sb_projection_init(SBProjection *v,const char *source,size_t length) {
    if(!v)return sb_error(SB_INVALID,"Quellprojektion fehlt.");
    *v=(SBProjection){.source=source,.source_length=length};
    if((!source && length) || length>SB_TEXT_LIMIT)return sb_error(SB_INVALID,"Projektionsquelle ist ungültig oder zu groß.");
    v->text=malloc(1);
    if(!v->text)return sb_error(SB_MEMORY,"Kein Speicher für Quellprojektion.");
    v->text[0]=0;v->capacity=1;return sb_ok();
}
void sb_projection_free(SBProjection *v) {
    if(!v)return;
    free(v->text);free(v->spans);*v=(SBProjection){0};
}
static SBStatus reserve(SBProjection *v,size_t extra,bool merge) {
    if(extra>SB_TEXT_LIMIT+1-v->length)return sb_error(SB_LIMIT,"Die aufbereitete Quelle ist zu groß.");
    if(!merge && v->count==SB_PROJECTION_LIMIT)return sb_error(SB_LIMIT,"Die Quelle enthält zu viele getrennte Textbereiche.");
    size_t needed=v->length+extra+1;
    if(needed>v->capacity) {
        size_t capacity=v->capacity ? v->capacity : 1;
        while(capacity<needed) {
            if(capacity>(SB_TEXT_LIMIT+2)/2){capacity=SB_TEXT_LIMIT+2;break;}
            capacity*=2;
        }
        char *text=realloc(v->text,capacity);
        if(!text)return sb_error(SB_MEMORY,"Kein Speicher für aufbereiteten Text.");
        v->text=text;v->capacity=capacity;
    }
    if(!merge && v->count==v->span_capacity) {
        size_t capacity=v->span_capacity ? v->span_capacity*2 : 16;
        SBProjectionSpan *spans=realloc(v->spans,capacity*sizeof(*spans));
        if(!spans)return sb_error(SB_MEMORY,"Kein Speicher für Quellpositionen.");
        v->spans=spans;v->span_capacity=capacity;
    }
    return sb_ok();
}
static void span(SBProjection *v,size_t offset,size_t length,size_t source_length,bool copy,bool merge) {
    if(merge){v->spans[v->count-1].length+=length;v->spans[v->count-1].source_length+=source_length;}
    else v->spans[v->count++]=(SBProjectionSpan){v->length,length,offset,source_length,copy};
    v->length+=length;v->text[v->length]=0;
}
SBStatus sb_projection_copy(SBProjection *v,size_t offset,size_t length) {
    if(!v || !v->text || offset>v->source_length || length>v->source_length-offset)return sb_error(SB_INVALID,"Projektionsbereich ist ungültig.");
    if(!length)return sb_ok();
    SBProjectionSpan *last=v->count ? &v->spans[v->count-1] : NULL;
    bool merge=last && last->copy && last->source+last->source_length==offset;
    SBStatus status=reserve(v,length,merge);if(status.code!=SB_OK)return status;
    memcpy(v->text+v->length,v->source+offset,length);span(v,offset,length,length,true,merge);return sb_ok();
}
SBStatus sb_projection_spaces(SBProjection *v,size_t count,size_t source_tab) {
    if(!v || !v->text || source_tab>=v->source_length || v->source[source_tab]!='\t')return sb_error(SB_INVALID,"Tabulatorposition ist ungültig.");
    if(count>3)return sb_error(SB_INVALID,"Tabulatorrest ist ungültig.");
    if(!count)return sb_ok();
    SBStatus status=reserve(v,count,false);if(status.code!=SB_OK)return status;
    memset(v->text+v->length,' ',count);span(v,source_tab,count,1,false,false);return sb_ok();
}
SBStatus sb_projection_newline(SBProjection *v,size_t offset,size_t length) {
    if(!v || !v->text || offset>v->source_length || length>v->source_length-offset || length>2 ||
       (!length && offset!=v->source_length) ||
       (length==1 && v->source[offset]!='\n' && v->source[offset]!='\r') ||
       (length==2 && (v->source[offset]!='\r' || v->source[offset+1]!='\n')))
        return sb_error(SB_INVALID,"Zeilenende ist ungültig.");
    if(length==1 && v->source[offset]=='\n')return sb_projection_copy(v,offset,1);
    SBStatus status=reserve(v,1,false);if(status.code!=SB_OK)return status;
    v->text[v->length]='\n';span(v,offset,1,length,false,false);return sb_ok();
}
SBStatus sb_projection_source(const SBProjection *v,size_t offset,size_t *source) {
    if(!v || !v->text || !source || offset>v->length)return sb_error(SB_INVALID,"Projektionsposition ist ungültig.");
    if(offset==v->length){*source=v->count ? v->spans[v->count-1].source+v->spans[v->count-1].source_length : 0;return sb_ok();}
    size_t lo=0,hi=v->count;
    while(lo<hi){size_t mid=lo+(hi-lo)/2;if(v->spans[mid].view+v->spans[mid].length<=offset)lo=mid+1;else hi=mid;}
    if(lo==v->count)return sb_error(SB_INVALID,"Quellposition fehlt.");
    const SBProjectionSpan *s=&v->spans[lo];*source=s->source+(s->copy ? offset-s->view : 0);return sb_ok();
}
SBStatus sb_projection_cursor(SBProjectionCursor *cursor,size_t offset,size_t end,size_t column) {
    if(!cursor || offset>end)return sb_error(SB_INVALID,"Zeilenposition ist ungültig.");
    *cursor=(SBProjectionCursor){.byte=offset,.end=end,.column=column};return sb_ok();
}
SBStatus sb_projection_indent(const char *source,size_t length,SBProjectionCursor *cursor,size_t requested,size_t *consumed) {
    if((!source && length) || !cursor || !consumed || cursor->end>length || cursor->byte>cursor->end || cursor->pending>3 ||
       (cursor->pending && (cursor->anchor>=length || source[cursor->anchor]!='\t')))
        return sb_error(SB_INVALID,"Einrückungsposition ist ungültig.");
    SBProjectionCursor next=*cursor;size_t used=0;*consumed=0;
    while(used<requested) {
        if(next.pending) {
            size_t take=requested-used<next.pending ? requested-used : next.pending;
            if(take>SIZE_MAX-next.column)return sb_error(SB_LIMIT,"Einrückung ist zu groß.");
            next.pending-=take;next.column+=take;used+=take;continue;
        }
        if(next.byte==next.end || (source[next.byte]!=' ' && source[next.byte]!='\t'))break;
        size_t width=source[next.byte]=='\t' ? 4-next.column%4 : 1;
        size_t take=requested-used<width ? requested-used : width;
        if(take>SIZE_MAX-next.column)return sb_error(SB_LIMIT,"Einrückung ist zu groß.");
        next.anchor=next.byte++;next.pending=width-take;next.column+=take;used+=take;
    }
    *cursor=next;*consumed=used;return sb_ok();
}
SBStatus sb_projection_remainder(SBProjection *v,const SBProjectionCursor *cursor) {
    if(!v || !v->text || !cursor || cursor->end>v->source_length || cursor->byte>cursor->end || cursor->pending>3 ||
       (cursor->pending && (cursor->anchor>=v->source_length || v->source[cursor->anchor]!='\t')))
        return sb_error(SB_INVALID,"Restbereich ist ungültig.");
    size_t previous_length=v->length,previous_count=v->count;
    SBStatus status=cursor->pending ? sb_projection_spaces(v,cursor->pending,cursor->anchor) : sb_ok();
    if(status.code==SB_OK)status=sb_projection_copy(v,cursor->byte,cursor->end-cursor->byte);
    if(status.code!=SB_OK){v->length=previous_length;v->count=previous_count;v->text[v->length]=0;}
    return status;
}
