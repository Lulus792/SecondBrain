#include "inline.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct SBInlineRun { size_t offset,width,close,short_close; } Run;
typedef struct SBInlinePair {
    size_t open,close,parent,end,destination,length;
    bool image,valid,has_link;
} Pair;
static bool punctuation(unsigned char c) {
    return (c>=33 && c<=47) || (c>=58 && c<=64) || (c>=91 && c<=96) || (c>=123 && c<=126);
}
static bool whitespace(char c) { return c==' ' || c=='\t' || c=='\r' || c=='\n'; }
static bool escaped(const char *text,size_t p) {
    size_t count=0; while (p && text[--p]=='\\') ++count;
    return count%2!=0;
}
static int run_width(const void *a,const void *b) {
    const Run *x=a,*y=b;
    return x->width<y->width ? -1 : x->width>y->width ? 1 : x->offset<y->offset ? -1 : x->offset>y->offset;
}
static int run_position(const void *a,const void *b) {
    const Run *x=a,*y=b; return x->offset<y->offset ? -1 : x->offset>y->offset;
}
static Run *find_run(const SBInline *r,size_t p) {
    size_t lo=0,hi=r->run_count;
    while (lo<hi) { size_t mid=lo+(hi-lo)/2; if (r->runs[mid].offset<p) lo=mid+1; else hi=mid; }
    return lo<r->run_count && r->runs[lo].offset==p ? &r->runs[lo] : NULL;
}
static bool code_at(const SBInline *r,size_t p,size_t *width,size_t *close) {
    Run *run=find_run(r,p);
    if (run) { *width=run->width; *close=run->close; return true; }
    run=p ? find_run(r,p-1) : NULL;
    if (run && run->width>1 && escaped(r->text,run->offset)) {
        *width=run->width-1; *close=run->short_close; return true;
    }
    return false;
}
static Pair *find_pair(const SBInline *r,size_t p) {
    size_t lo=0,hi=r->pair_count;
    while (lo<hi) { size_t mid=lo+(hi-lo)/2; if (r->pairs[mid].open<p) lo=mid+1; else hi=mid; }
    return lo<r->pair_count && r->pairs[lo].open==p ? &r->pairs[lo] : NULL;
}
static SBStatus reserve(void **items,size_t *capacity,size_t count,size_t size) {
    if (count>=SB_INLINE_LIMIT) return sb_error(SB_LIMIT,"Der Absatz enthält zu viele Markdown-Markierungen.");
    if (count<*capacity) return sb_ok();
    size_t next=*capacity ? *capacity*2 : 32;
    void *data=realloc(*items,next*size);
    if (!data) return sb_error(SB_MEMORY,"Markdown benötigt mehr Speicher.");
    *items=data; *capacity=next; return sb_ok();
}
static bool spend(size_t *budget) { if (!*budget) return false; --*budget; return true; }
#include "inline_raw.inc"
#include "inline_entities.inc"
#include "inline_emphasis.inc"
static bool spaces(const SBInline *r,size_t *p,size_t *budget) {
    unsigned endings=0;
    while (*p<r->length && whitespace(r->text[*p])) {
        if (!spend(budget)) return false;
        if (r->text[*p]=='\n' || r->text[*p]=='\r') {
            if (++endings>1) return false;
            if (r->text[*p]=='\r' && *p+1<r->length && r->text[*p+1]=='\n') ++*p;
        }
        ++*p;
    }
    return true;
}
static bool destination(SBInline *r,Pair *pair,size_t *budget) {
    size_t p=pair->close+1;
    if (p>=r->length || r->text[p++]!='(' || !spaces(r,&p,budget)) return false;
    pair->destination=p;
    if (p<r->length && r->text[p]=='<') {
        pair->destination=++p;
        while (p<r->length && r->text[p]!='>') {
            if (!spend(budget) || r->text[p]=='\r' || r->text[p]=='\n' || r->text[p]=='<') return false;
            if (r->text[p]=='\\' && p+1<r->length && punctuation((unsigned char)r->text[p+1])) ++p;
            ++p;
        }
        if (p==r->length) return false;
        pair->length=p-pair->destination; ++p;
    } else {
        unsigned nesting=0;
        while (p<r->length && !whitespace(r->text[p])) {
            if (!spend(budget)) return false;
            char c=r->text[p];
            if ((unsigned char)c<32 || c==127) return false;
            if (c=='\\' && p+1<r->length && punctuation((unsigned char)r->text[p+1])) { p+=2; continue; }
            if (c=='(' && ++nesting>32) return false;
            if (c==')') { if (!nesting) break; --nesting; }
            ++p;
        }
        if (nesting) return false;
        pair->length=p-pair->destination;
    }
    size_t before=p;
    if (!spaces(r,&p,budget) || p==r->length) return false;
    if (r->text[p]==')') { pair->end=p+1; return true; }
    /* A title must be separated from a nonempty destination. */
    if (before==p && pair->length) return false;
    char quote=r->text[p++]; if (quote!='\'' && quote!='"' && quote!='(') return false;
    char close=quote=='(' ? ')' : quote; unsigned endings=0;
    while (p<r->length && r->text[p]!=close) {
        if (!spend(budget)) return false;
        if (r->text[p]=='\\' && p+1<r->length && punctuation((unsigned char)r->text[p+1])) { p+=2; continue; }
        if (quote=='(' && r->text[p]=='(') return false;
        if (r->text[p]=='\r' || r->text[p]=='\n') {
            if (endings) return false;
            ++endings; if (r->text[p]=='\r' && p+1<r->length && r->text[p+1]=='\n') ++p;
        } else if (!whitespace(r->text[p])) endings=0;
        ++p;
    }
    if (p==r->length) return false;
    ++p;
    if (!spaces(r,&p,budget) || p==r->length || r->text[p]!=')') return false;
    pair->end=p+1; return true;
}
SBStatus sb_inline_init(SBInline *r,const char *text,size_t length) {
    if (!r) return sb_error(SB_INVALID,"Markdown-Leser fehlt.");
    *r=(SBInline){.text=text,.length=length};
    if ((!text && length) || length>SB_TEXT_LIMIT) return sb_error(SB_INVALID,"Markdown-Text ist ungültig oder zu groß.");
    for (size_t p=0;p<length;) {
        if (text[p]!='`') { ++p; continue; }
        size_t start=p; while (p<length && text[p]=='`') ++p;
        void *storage=r->runs;
        SBStatus status=reserve(&storage,&r->run_capacity,r->run_count,sizeof(Run)); r->runs=storage;
        if (status.code!=SB_OK) return status;
        r->runs[r->run_count++]=(Run){start,p-start,SIZE_MAX,SIZE_MAX};
    }
    if (r->run_count) {
        qsort(r->runs,r->run_count,sizeof(Run),run_width);
        for (size_t i=0;i+1<r->run_count;++i) if (r->runs[i].width==r->runs[i+1].width) r->runs[i].close=r->runs[i+1].offset;
        for (size_t i=0;i<r->run_count;++i) if (r->runs[i].width>1) {
            size_t width=r->runs[i].width-1,lo=0,hi=r->run_count;
            while (lo<hi) {
                size_t mid=lo+(hi-lo)/2; Run *candidate=&r->runs[mid];
                if (candidate->width<width || (candidate->width==width && candidate->offset<=r->runs[i].offset)) lo=mid+1;
                else hi=mid;
            }
            if (lo<r->run_count && r->runs[lo].width==width) r->runs[i].short_close=r->runs[lo].offset;
        }
        qsort(r->runs,r->run_count,sizeof(Run),run_position);
    }
    SBStatus opaque_status=opaque_init(r); if (opaque_status.code!=SB_OK) return opaque_status;
    size_t parent=SIZE_MAX;
    for (size_t p=0;p<length;) {
        if (text[p]=='\\' && p+1<length && punctuation((unsigned char)text[p+1])) { p+=2; continue; }
        SBInlineToken *opaque=opaque_at(r,p); if (opaque) { p+=opaque->length; continue; }
        size_t width=0,close=SIZE_MAX;
        if (text[p]=='`' && code_at(r,p,&width,&close)) { p=close!=SIZE_MAX ? close+width : p+width; continue; }
        if (text[p]=='[') {
            void *storage=r->pairs;
            SBStatus status=reserve(&storage,&r->pair_capacity,r->pair_count,sizeof(Pair)); r->pairs=storage;
            if (status.code!=SB_OK) return status;
            r->pairs[r->pair_count]=(Pair){.open=p,.close=SIZE_MAX,.parent=parent,.image=p>0 && text[p-1]=='!' && !escaped(text,p-1)};
            parent=r->pair_count++;
        } else if (text[p]==']' && parent!=SIZE_MAX) { r->pairs[parent].close=p; parent=r->pairs[parent].parent; }
        ++p;
    }
    size_t budget=length*32+64;
    for (size_t i=0;i<r->pair_count;++i) {
        Pair *pair=&r->pairs[i];
        if (pair->close!=SIZE_MAX) pair->valid=destination(r,pair,&budget);
        if (!budget) return sb_error(SB_LIMIT,"Markdown-Linkstruktur ist zu komplex.");
    }
    for (size_t i=r->pair_count;i>0;--i) {
        Pair *pair=&r->pairs[i-1];
        if (pair->has_link && !pair->image) pair->valid=false;
        if (pair->parent!=SIZE_MAX && !(pair->valid && pair->image) && ((pair->valid && !pair->image) || pair->has_link))
            r->pairs[pair->parent].has_link=true;
    }
    return emphasis_init(r);
}
void sb_inline_free(SBInline *r) { if (r) { free(r->runs); free(r->pairs); free(r->marks); free(r->opaque); *r=(SBInline){0}; } }
static bool at(const SBInline *r,size_t p,size_t limit,SBInlineToken *t) {
    if (p>=limit) return false;
    SBInlineToken *opaque=opaque_at(r,p);
    if (opaque && opaque->length<=limit-p) { *t=*opaque; return true; }
    *t=(SBInlineToken){.kind=SB_INLINE_TEXT,.offset=p,.length=1,.content=p,.content_length=1};
    char c=r->text[p];
    if (c=='\r' && p+1<limit && r->text[p+1]=='\n') t->length=t->content_length=2;
    if (c=='\\' && p+1<limit && punctuation((unsigned char)r->text[p+1])) {
        t->kind=SB_INLINE_ESCAPE; t->length=2; t->content=p+1; return true;
    }
    size_t width=0,close=SIZE_MAX;
    if (c=='`' && code_at(r,p,&width,&close)) {
        t->length=t->content_length=width;
        if (close!=SIZE_MAX && close+width<=limit) {
            t->kind=SB_INLINE_CODE; t->length=close+width-p;
            t->content=p+width; t->content_length=close-t->content;
        }
        return true;
    }
    if (c=='&' && !entity_opaque(r,p)) {
        char decoded[8]; size_t bytes=0,consumed=entity_decode(r->text+p,limit-p,decoded,&bytes);
        if (consumed) { t->kind=SB_INLINE_ENTITY; t->length=t->content_length=consumed; return true; }
    }
    Pair *pair=c=='[' ? find_pair(r,p) : c=='!' && p+1<limit ? find_pair(r,p+1) : NULL;
    if (pair && pair->valid && pair->end<=limit && pair->image==(c=='!')) {
        t->kind=pair->image ? SB_INLINE_IMAGE : SB_INLINE_LINK;
        t->length=pair->end-p; t->content=pair->open+1; t->content_length=pair->close-t->content;
        t->destination=pair->destination; t->destination_length=pair->length;
        return true;
    }
    Mark *mark=r->marks_ready ? mark_at(r,p) : NULL;
    if (mark && p>=mark->offset && p<mark->offset+mark->length) {
        t->kind=SB_INLINE_FORMAT; t->length=(mark->offset+mark->length-p<limit-p) ? mark->offset+mark->length-p : limit-p;
        t->content_length=0;
    }
    return true;
}
bool sb_inline_next(SBInline *r,SBInlineToken *t) {
    if (!r || !t || !at(r,r->cursor,r->length,t)) return false;
    r->cursor+=t->length; return true;
}
static void normalized(const char *text,size_t length,char *out,size_t *used) {
    for (size_t i=0;i<length;++i) {
        char c=text[i];
        if (c=='\r' && i+1<length && text[i+1]=='\n') ++i;
        out[(*used)++]=(c=='\r' || c=='\n') ? ' ' : c;
    }
}
static size_t normalized_length(const char *text,size_t length) {
    size_t result=length;
    for (size_t i=0;i<length;++i) if (text[i]=='\r' && i+1<length && text[i+1]=='\n') { --result; ++i; }
    return result;
}
static void code_range(const char **text,size_t *length) {
    if (*length<2) return;
    bool nonspace=false;
    for (size_t i=0;i<*length;++i) if ((*text)[i]!=' ' && (*text)[i]!='\r' && (*text)[i]!='\n') { nonspace=true; break; }
    char first=(*text)[0],last=(*text)[*length-1];
    if (!nonspace || !(first==' ' || first=='\r' || first=='\n') || !(last==' ' || last=='\r' || last=='\n')) return;
    size_t leading=first=='\r' && (*text)[1]=='\n' ? 2 : 1;
    size_t trailing=last=='\n' && (*text)[*length-2]=='\r' ? 2 : 1;
    *text+=leading; *length-=leading+trailing;
}
static SBStatus append_span(SBStyledText *out,size_t start,size_t length,unsigned style) {
    if (!length) return sb_ok();
    if (out->count && out->spans[out->count-1].style==style && out->spans[out->count-1].offset+out->spans[out->count-1].length==start) {
        out->spans[out->count-1].length+=length; return sb_ok();
    }
    void *storage=out->spans; SBStatus status=reserve(&storage,&out->capacity,out->count,sizeof(SBTextSpan)); out->spans=storage;
    if (status.code==SB_OK) out->spans[out->count++]=(SBTextSpan){start,length,style};
    return status;
}
static SBStatus output_space(SBStyledText *out,size_t used,size_t extra,size_t *capacity) {
    if (used>SB_TEXT_LIMIT || extra>SB_TEXT_LIMIT-used) return sb_error(SB_LIMIT,"Der aufbereitete Markdown-Text überschreitet die Textgrenze.");
    size_t needed=used+extra+1;
    if (needed<=*capacity) return sb_ok();
    size_t next=*capacity;
    while (next<needed) next=next>(SB_TEXT_LIMIT+1)/2 ? SB_TEXT_LIMIT+1 : next*2;
    char *text=realloc(out->text,next); if (!text) return sb_error(SB_MEMORY,"Markdown benötigt mehr Speicher.");
    out->text=text; *capacity=next; return sb_ok();
}
static SBStatus write_range(const SBInline *r,size_t p,size_t end,SBStyledText *out,size_t *used,size_t *capacity,unsigned depth,size_t *budget) {
    if (depth>32) return sb_error(SB_LIMIT,"Markdown-Textstruktur ist zu tief.");
    while (p<end) {
        if (!spend(budget)) return sb_error(SB_LIMIT,"Markdown-Textstruktur ist zu komplex.");
        SBInlineToken t; at(r,p,end,&t); size_t begin=*used;
        Mark *mark=mark_at(r,p); unsigned style=mark ? mark->active : 0;
        if (t.kind==SB_INLINE_LINK || t.kind==SB_INLINE_IMAGE) {
            SBStatus status=write_range(r,t.content,t.content+t.content_length,out,used,capacity,depth+1,budget);
            if (status.code!=SB_OK) return status;
        } else {
            char decoded[8]; size_t decoded_bytes=0;
            const char *data=r->text+t.content; size_t data_length=t.content_length;
            if (t.kind==SB_INLINE_ENTITY) { entity_decode(data,data_length,decoded,&decoded_bytes); data=decoded; data_length=decoded_bytes; }
            else if (t.kind==SB_INLINE_CODE) { code_range(&data,&data_length); style|=SB_TEXT_CODE; }
            size_t extra=t.kind==SB_INLINE_FORMAT ? 0 : normalized_length(data,data_length);
            SBStatus space=output_space(out,*used,extra,capacity); if (space.code!=SB_OK) return space;
            if (t.kind!=SB_INLINE_FORMAT) normalized(data,data_length,out->text,used);
        }
        if (t.kind!=SB_INLINE_LINK && t.kind!=SB_INLINE_IMAGE) {
            SBStatus status=append_span(out,begin,*used-begin,style); if (status.code!=SB_OK) return status;
        }
        p+=t.length;
    }
    return sb_ok();
}
void sb_styled_free(SBStyledText *text) { if (text) { free(text->text); free(text->spans); *text=(SBStyledText){0}; } }
SBStatus sb_inline_styled(const SBInline *r,size_t offset,size_t length,SBStyledText *out) {
    if (out) *out=(SBStyledText){0};
    if (!r || !out || offset>r->length || length>r->length-offset) return sb_error(SB_INVALID,"Markdown-Textbereich ist ungültig.");
    out->text=malloc(length+1); if (!out->text) return sb_error(SB_MEMORY,"Markdown benötigt mehr Speicher.");
    size_t used=0,capacity=length+1,budget=length*32+64;
    SBStatus status=write_range(r,offset,offset+length,out,&used,&capacity,0,&budget);
    if (status.code!=SB_OK) { sb_styled_free(out); return status; }
    out->text[used]=0; return sb_ok();
}
SBStatus sb_inline_text(const SBInline *r,size_t offset,size_t length,char **out) {
    if (out) *out=NULL;
    if (!out) return sb_error(SB_INVALID,"Markdown-Textausgabe fehlt.");
    SBStyledText styled; SBStatus status=sb_inline_styled(r,offset,length,&styled);
    if (status.code==SB_OK) { *out=styled.text; styled.text=NULL; sb_styled_free(&styled); }
    return status;
}
SBStatus sb_inline_destination(const SBInline *r,const SBInlineToken *t,char *out,size_t capacity) {
    if (!r || !t || !out || !capacity || (t->kind!=SB_INLINE_LINK && t->kind!=SB_INLINE_IMAGE && t->kind!=SB_INLINE_AUTOLINK) ||
        t->destination>r->length || t->destination_length>r->length-t->destination) return sb_error(SB_INVALID,"Markdown-Linkziel ist ungültig.");
    size_t used=0;
    for (size_t i=0;i<t->destination_length;) {
        char c=r->text[t->destination+i];
        if (t->kind!=SB_INLINE_AUTOLINK && c=='\\' && i+1<t->destination_length && punctuation((unsigned char)r->text[t->destination+i+1])) {
            c=r->text[t->destination+(++i)];
        } else if (t->kind!=SB_INLINE_AUTOLINK && c=='&') {
            char decoded[8]; size_t bytes=0,consumed=entity_decode(r->text+t->destination+i,t->destination_length-i,decoded,&bytes);
            if (consumed) {
                if (bytes>=capacity-used) { *out=0; return sb_error(SB_LIMIT,"Markdown-Linkziel ist zu lang."); }
                memcpy(out+used,decoded,bytes); used+=bytes; i+=consumed; continue;
            }
        }
        if (used+1>=capacity) { *out=0; return sb_error(SB_LIMIT,"Markdown-Linkziel ist zu lang."); }
        out[used++]=c; ++i;
    }
    out[used]=0; return sb_ok();
}
