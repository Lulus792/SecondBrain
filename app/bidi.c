#include "bidi.h"
#include "grapheme.h"
#include <SheenBidi/SheenBidi.h>
#include <stdlib.h>
#include <string.h>

struct SBTextParagraph {
    char *text;
    uint32_t *scalars,*bytes;
    unsigned char *levels;
    size_t length;
    size_t scalar_count;
    unsigned char empty_level;
    SBAlgorithmRef algorithm;
    SBParagraphRef paragraph;
};
void sb_bidi_paragraph_free(SBTextParagraph *p) {
    if(!p)return;
    if(p->paragraph)SBParagraphRelease(p->paragraph);
    if(p->algorithm)SBAlgorithmRelease(p->algorithm);
    free(p->scalars);free(p->bytes);free(p->levels);free(p->text);free(p);
}
/* Input was strictly validated by the shared grapheme reader. Keep the UBA
   in scalar coordinates: its UTF-8 L1 path can split a multibyte PDI after a
   segment separator. Convert only completed runs back to source bytes. */
static uint32_t decode(const char *text,size_t *at) {
    const unsigned char *p=(const unsigned char *)text+*at;
    unsigned n=*p<0x80 ? 1 : *p<0xe0 ? 2 : *p<0xf0 ? 3 : 4;
    uint32_t cp=n==1 ? *p : *p&((1u<<(7-n))-1);
    for(unsigned i=1;i<n;++i)cp=(cp<<6)|(p[i]&63);
    *at+=n;return cp;
}
SBStatus sb_bidi_paragraph_create(const char *text,size_t length,SBTextDirection direction,SBTextParagraph **out) {
    if(!out)return sb_error(SB_INVALID,"Absatzausgabe fehlt.");
    *out=NULL;
    SBGrapheme validation;
    if(direction<SB_BIDI_AUTO_LTR || direction>SB_BIDI_RTL || !sb_grapheme_init(&validation,text,length))
        return sb_error(SB_INVALID,"Ungültiger UTF-8-Absatz oder Schreibrichtung.");
    SBTextParagraph *p=calloc(1,sizeof(*p));
    if(!p)return sb_error(SB_MEMORY,"Absatzlayout benötigt mehr Speicher.");
    p->text=malloc(length+1);
    if(!p->text){sb_bidi_paragraph_free(p);return sb_error(SB_MEMORY,"Absatzlayout benötigt mehr Speicher.");}
    if(length)memcpy(p->text,text,length);
    p->text[length]=0;p->length=length;
    p->empty_level=(direction==SB_BIDI_RTL || direction==SB_BIDI_AUTO_RTL);
    if(length) {
        for(size_t i=0;i<length;++i)if(((unsigned char)text[i]&0xc0)!=0x80)++p->scalar_count;
        p->scalars=malloc(p->scalar_count*sizeof(*p->scalars));
        p->bytes=malloc((p->scalar_count+1)*sizeof(*p->bytes));
        p->levels=malloc(length);
        if(!p->scalars || !p->bytes || !p->levels){sb_bidi_paragraph_free(p);return sb_error(SB_MEMORY,"Absatzlayout benötigt mehr Speicher.");}
        size_t at=0;
        for(size_t i=0;i<p->scalar_count;++i){p->bytes[i]=(uint32_t)at;p->scalars[i]=decode(p->text,&at);}
        p->bytes[p->scalar_count]=(uint32_t)at;
        SBCodepointSequence sequence={SBStringEncodingUTF32,p->scalars,p->scalar_count};
        p->algorithm=SBAlgorithmCreate(&sequence);
        const SBLevel bases[]={SBLevelDefaultLTR,SBLevelDefaultRTL,0,1};
        if(p->algorithm)p->paragraph=SBAlgorithmCreateParagraph(p->algorithm,0,p->scalar_count,bases[direction]);
        if(!p->paragraph){sb_bidi_paragraph_free(p);return sb_error(SB_MEMORY,"Absatzlayout konnte nicht erstellt werden.");}
        p->scalar_count=SBParagraphGetLength(p->paragraph);
        p->length=p->bytes[p->scalar_count];
        const SBLevel *levels=SBParagraphGetLevelsPtr(p->paragraph);
        for(size_t i=0;i<p->scalar_count;++i)memset(p->levels+p->bytes[i],levels[i],p->bytes[i+1]-p->bytes[i]);
    }
    *out=p;return sb_ok();
}
size_t sb_bidi_paragraph_length(const SBTextParagraph *p){return p ? p->length : 0;}
const char *sb_bidi_paragraph_text(const SBTextParagraph *p){return p ? p->text : NULL;}
unsigned char sb_bidi_paragraph_level(const SBTextParagraph *p){return p && p->paragraph ? SBParagraphGetBaseLevel(p->paragraph) : p ? p->empty_level : 0;}
const unsigned char *sb_bidi_paragraph_levels(const SBTextParagraph *p){return p ? p->levels : NULL;}
void sb_bidi_line_free(SBVisualLine *line){if(line){free(line->runs);memset(line,0,sizeof(*line));}}
static bool boundary(const SBTextParagraph *p,size_t byte){return byte==p->length || ((unsigned char)p->text[byte]&0xc0)!=0x80;}
static size_t scalar(const SBTextParagraph *p,size_t byte) {
    size_t lo=0,hi=p->scalar_count;
    while(lo<hi){size_t mid=lo+(hi-lo)/2;if(p->bytes[mid]<byte)lo=mid+1;else hi=mid;}
    return lo;
}
SBStatus sb_bidi_line(const SBTextParagraph *p,size_t byte,size_t length,SBVisualLine *out) {
    if(!p || !out || out->runs || out->count || !length || byte>p->length || length>p->length-byte || !boundary(p,byte) || !boundary(p,byte+length))
        return sb_error(SB_INVALID,"Ungültiger Absatzbereich für eine Textzeile.");
    size_t first=scalar(p,byte),last=scalar(p,byte+length);
    SBLineRef line=SBParagraphCreateLine(p->paragraph,first,last-first);
    if(!line)return sb_error(SB_MEMORY,"Zeilenlayout konnte nicht erstellt werden.");
    size_t count=SBLineGetRunCount(line);
    if(count>SIZE_MAX/sizeof(*out->runs)){SBLineRelease(line);return sb_error(SB_LIMIT,"Zu viele Schriftläufe.");}
    SBVisualRun *runs=count ? malloc(count*sizeof(*runs)) : NULL;
    if(count && !runs){SBLineRelease(line);return sb_error(SB_MEMORY,"Zeilenlayout benötigt mehr Speicher.");}
    const SBRun *original=SBLineGetRunsPtr(line);
    for(size_t i=0;i<count;++i){size_t start=p->bytes[original[i].offset],end=p->bytes[original[i].offset+original[i].length];runs[i]=(SBVisualRun){start,end-start,original[i].level};}
    SBLineRelease(line);
    *out=(SBVisualLine){runs,count,byte,length,sb_bidi_paragraph_level(p)};
    return sb_ok();
}
uint32_t sb_bidi_mirror(uint32_t cp){return cp<=0x10ffff && !(cp>=0xd800 && cp<=0xdfff) ? SBCodepointGetMirror(cp) : 0;}
bool sb_bidi_separator(uint32_t cp){return cp<=0x10ffff && SBCodepointGetBidiType(cp)==SBBidiTypeB;}
