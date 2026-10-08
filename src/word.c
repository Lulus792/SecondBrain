#include "word.h"
#include "grapheme.h"
typedef struct {uint32_t lo,hi;unsigned char value;} WordRange;
#include "word_data.inc"
enum {OTHER,CR,LF,NEWLINE,EXTEND,ZWJ,RI,FORMAT,KATAKANA,HEBREW,ALETTER,SQUOTE,DQUOTE,MIDNUMLET,MIDLETTER,MIDNUM,EXTENDNUMLET,WSEGSPACE,NUMERIC};
static unsigned property(const WordRange *ranges,size_t count,uint32_t cp) {
    size_t lo=0,hi=count;while(lo<hi){size_t mid=lo+(hi-lo)/2;if(ranges[mid].hi<cp)lo=mid+1;else hi=mid;}
    return lo<count && ranges[lo].lo<=cp ? ranges[lo].value : 0;
}
#define PROP(name,cp) property(name,sizeof(name)/sizeof(*name),cp)
static uint32_t decode(const char *text,size_t *byte) {
    const unsigned char *p=(const unsigned char *)text+*byte;
    unsigned n=*p<0x80 ? 1 : *p<0xe0 ? 2 : *p<0xf0 ? 3 : 4;
    uint32_t cp=n==1 ? *p : *p&((1u<<(7-n))-1);
    for(unsigned i=1;i<n;++i)cp=(cp<<6)|(p[i]&63);
    *byte+=n;return cp;
}
static bool ignored(unsigned p){return p==EXTEND || p==FORMAT || p==ZWJ;}
static bool newline(unsigned p){return p==CR || p==LF || p==NEWLINE;}
static bool letter(unsigned p){return p==ALETTER || p==HEBREW;}
static bool middle_letter(unsigned p){return p==MIDLETTER || p==MIDNUMLET || p==SQUOTE;}
static bool middle_number(unsigned p){return p==MIDNUM || p==MIDNUMLET || p==SQUOTE;}
static bool extender(unsigned p){return letter(p) || p==NUMERIC || p==KATAKANA || p==EXTENDNUMLET;}
static unsigned following(const SBWord *r) {
    size_t at=r->byte;while(at<r->length){unsigned p=PROP(word_classes,decode(r->text,&at));if(!ignored(p))return p;}return OTHER;
}
static bool breaks(const SBWord *r,unsigned p,bool emoji) {
    unsigned raw=r->previous,left=r->significant,before=r->before;
    if(raw==CR && p==LF)return false;
    if(newline(raw) || newline(p))return true;
    if(raw==ZWJ && emoji)return false;
    if(raw==WSEGSPACE && p==WSEGSPACE)return false;
    if(ignored(p))return false;
    if(letter(left) && letter(p))return false;
    if(letter(left) && middle_letter(p) && letter(following(r)))return false;
    if(letter(before) && middle_letter(left) && letter(p))return false;
    if(left==HEBREW && p==SQUOTE)return false;
    if(left==HEBREW && p==DQUOTE && following(r)==HEBREW)return false;
    if(before==HEBREW && left==DQUOTE && p==HEBREW)return false;
    if(left==NUMERIC && p==NUMERIC)return false;
    if((letter(left) && p==NUMERIC) || (left==NUMERIC && letter(p)))return false;
    if(before==NUMERIC && middle_number(left) && p==NUMERIC)return false;
    if(left==NUMERIC && middle_number(p) && following(r)==NUMERIC)return false;
    if(left==KATAKANA && p==KATAKANA)return false;
    if((extender(left) && p==EXTENDNUMLET) || (left==EXTENDNUMLET && extender(p)))return false;
    if(left==RI && p==RI && (r->regional&1))return false;
    return true;
}
bool sb_word_init(SBWord *r,const char *text,size_t length) {
    if(!r)return false;*r=(SBWord){0};SBGrapheme validation;
    if(!sb_grapheme_init(&validation,text,length))return false;
    r->text=text;r->length=length;return true;
}
bool sb_word_next(SBWord *r,SBWordBoundary *boundary) {
    if(!r || !boundary || r->ended)return false;
    if(!r->started){r->started=true;*boundary=(SBWordBoundary){0};if(!r->length)r->ended=true;return true;}
    while(r->byte<r->length){size_t start=r->byte,characters=r->characters;
        uint32_t cp=decode(r->text,&r->byte);unsigned p=PROP(word_classes,cp);bool emoji=PROP(word_emoji,cp)!=0;
        bool here=characters && breaks(r,p,emoji),significant=r->segment_significant;
        if(here)r->segment_significant=false;
        r->segment_significant|=PROP(word_letters_numbers,cp)!=0 || p==EXTENDNUMLET || p==RI || emoji;
        if(!ignored(p)){r->regional=p==RI ? r->regional+1 : 0;r->before=r->significant;r->significant=p;}
        r->previous=p;++r->characters;
        if(here){*boundary=(SBWordBoundary){start,characters,significant};return true;}
    }
    r->ended=true;*boundary=(SBWordBoundary){r->length,r->characters,r->segment_significant};return true;
}
