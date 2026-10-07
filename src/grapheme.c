#include "grapheme.h"
typedef struct { uint32_t lo,hi; unsigned char value; } Range;
#include "grapheme_data.inc"
enum { OTHER,CR,LF,CONTROL,EXTEND,ZWJ,RI,PREPEND,SPACING,L,V,T,LV,LVT };
static unsigned property(const Range *ranges,size_t count,uint32_t codepoint) {
    size_t lo=0,hi=count;
    while (lo<hi) { size_t mid=lo+(hi-lo)/2; if (ranges[mid].hi<codepoint) lo=mid+1; else hi=mid; }
    return lo<count && ranges[lo].lo<=codepoint ? ranges[lo].value : 0;
}
#define PROP(name,cp) property(name,sizeof(name)/sizeof(*name),cp)
static uint32_t decode(const char *text,size_t *byte) {
    const unsigned char *p=(const unsigned char *)text+*byte;
    unsigned length=*p<0x80 ? 1 : *p<0xe0 ? 2 : *p<0xf0 ? 3 : 4;
    uint32_t cp=length==1 ? *p : *p&((1u<<(7-length))-1);
    for (unsigned i=1;i<length;++i) cp=(cp<<6)|(p[i]&0x3f);
    *byte+=length; return cp;
}
/* Segmentation also handles U+0000 from the normative test corpus. File and
   input validators retain their independent NUL rejection. */
static bool valid(const char *text,size_t length) {
    size_t at=0;
    while (at<length) {
        unsigned char first=(unsigned char)text[at++]; if (first<0x80) continue;
        unsigned extra; uint32_t cp,minimum;
        if (first>=0xc2 && first<=0xdf) { extra=1;cp=first&31;minimum=0x80; }
        else if (first>=0xe0 && first<=0xef) { extra=2;cp=first&15;minimum=0x800; }
        else if (first>=0xf0 && first<=0xf4) { extra=3;cp=first&7;minimum=0x10000; }
        else return false;
        if (extra>length-at) return false;
        while (extra--) { unsigned char next=(unsigned char)text[at++]; if ((next&0xc0)!=0x80) return false; cp=(cp<<6)|(next&63); }
        if (cp<minimum || cp>0x10ffff || (cp>=0xd800 && cp<=0xdfff)) return false;
    }
    return true;
}
bool sb_grapheme_init(SBGrapheme *r,const char *text,size_t length) {
    if (!r) return false;
    *r=(SBGrapheme){0};
    if ((!text && length) || length>SB_TEXT_LIMIT || !valid(text,length)) return false;
    r->text=text; r->length=length; return true;
}
static bool breaks(const SBGrapheme *r,unsigned current,unsigned indic,bool emoji) {
    unsigned previous=r->previous;
    if (previous==CR && current==LF) return false;
    if (previous==CR || previous==LF || previous==CONTROL || current==CR || current==LF || current==CONTROL) return true;
    if (previous==L && (current==L || current==V || current==LV || current==LVT)) return false;
    if ((previous==LV || previous==V) && (current==V || current==T)) return false;
    if ((previous==LVT || previous==T) && current==T) return false;
    if (current==EXTEND || current==ZWJ || current==SPACING || previous==PREPEND) return false;
    if (indic==1 && r->indic_link) return false;
    if (emoji && previous==ZWJ && r->emoji_zwj) return false;
    if (previous==RI && current==RI && (r->regional&1)) return false;
    return true;
}
bool sb_grapheme_next(SBGrapheme *r,SBGraphemeBoundary *boundary) {
    if (!r || !boundary || r->ended) return false;
    if (!r->started) { r->started=true; *boundary=(SBGraphemeBoundary){0}; if (!r->length) r->ended=true; return true; }
    while (r->byte<r->length) {
        size_t start=r->byte; uint32_t cp=decode(r->text,&r->byte);
        unsigned current=PROP(gcb,cp),indic=PROP(incb,cp); bool emoji=PROP(pictographic,cp)!=0;
        bool boundary_here=r->characters && breaks(r,current,indic,emoji);
        r->emoji_zwj=current==ZWJ && r->emoji_chain;
        r->emoji_chain=emoji || (current==EXTEND && r->emoji_chain);
        r->indic_link=indic==3 || (indic==2 && r->indic_link);
        r->regional=current==RI ? r->regional+1 : 0;
        r->previous=current;
        size_t characters=r->characters++;
        if (boundary_here) { *boundary=(SBGraphemeBoundary){start,characters}; return true; }
    }
    r->ended=true; *boundary=(SBGraphemeBoundary){r->length,r->characters}; return true;
}
bool sb_grapheme_position(const char *text,size_t length,size_t characters,SBGraphemePosition *position) {
    if (!position) return false;
    *position=(SBGraphemePosition){0}; SBGrapheme reader;
    if (!sb_grapheme_init(&reader,text,length)) return false;
    SBGraphemeBoundary b; size_t before=0,previous=0;
    while (sb_grapheme_next(&reader,&b)) {
        if (b.characters>characters) {
            *position=(SBGraphemePosition){.previous=characters==before ? previous : before,.floor=before,.ceil=characters==before ? before : b.characters,.next=b.characters,.boundary=characters==before};
            return true;
        }
        previous=before; before=b.characters;
    }
    *position=(SBGraphemePosition){.previous=previous,.floor=before,.ceil=before,.next=before,.boundary=true}; return true;
}
