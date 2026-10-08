#include "native_text.h"
#include "grapheme.h"
#include "word.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
void sb_native_text_free(SBNativeText *p){if(p){free(p->runs);memset(p,0,sizeof(*p));}}
static bool append(SBNativeText *p,size_t byte,size_t bytes,size_t first,size_t last,unsigned style,bool new_line,const SBNativeCharBox *box) {
    SBNativeTextRun *r=p->count ? &p->runs[p->count-1] : NULL;
    bool gap=false;
    if(box && r && r->characters && r->geometry){size_t last=r->characters-1;
        gap=fabsf((box->level&1) ? box->x+box->width-r->positions[last] : box->x-r->positions[last]-r->widths[last])>0.01f;}
    if(!r || new_line || gap || r->characters==SB_NATIVE_CHAR_LIMIT || r->span.style!=style || (box && (r->level!=box->level || r->y!=box->y || r->height!=box->height))){
        if(p->count==p->capacity){size_t next=p->capacity ? p->capacity*2 : 8;SBNativeTextRun *grown=realloc(p->runs,next*sizeof(*grown));if(!grown)return false;p->runs=grown;p->capacity=next;}
        r=&p->runs[p->count++];*r=(SBNativeTextRun){.span={byte,0,style}};r->scalars[0]=(uint32_t)first;
        if(box){r->geometry=true;r->level=box->level;r->x=box->x;r->y=box->y;r->height=box->height;}
    }
    if(box){float left=fminf(r->x,box->x),right=fmaxf(r->x+r->width,box->x+box->width);r->x=left;r->width=right-left;r->positions[r->characters]=box->x;r->widths[r->characters]=box->width;}
    r->lengths[r->characters++]=(uint8_t)bytes;r->scalars[r->characters]=(uint32_t)last;r->span.length+=bytes;return true;
}
static SBStatus build(const char *text,size_t length,const SBTextSpan *styles,size_t style_count,const SBNativeCharBox *boxes,size_t box_count,SBNativeText *out) {
    if(!out)return sb_error(SB_INVALID,"Native Textausgabe fehlt.");*out=(SBNativeText){0};
    SBGrapheme reader;if(!sb_grapheme_init(&reader,text,length))return sb_error(SB_INVALID,"Ungültiger nativer Text.");
    size_t covered=0;bool valid_styles=styles && style_count && style_count<=SB_INLINE_LIMIT;
    for(size_t i=0;valid_styles && i<style_count;++i){SBTextSpan s=styles[i];valid_styles=s.offset==covered && s.length && s.length<=length-covered;if(valid_styles)valid_styles=sb_utf8_valid(text+s.offset,s.length);if(valid_styles)covered+=s.length;}
    valid_styles=valid_styles && covered==length;
    SBNativeText p={0};SBGraphemeBoundary b;size_t byte=0,scalar=0,style=0,box_index=0;bool new_line=false;
    sb_grapheme_next(&reader,&b);
    while(sb_grapheme_next(&reader,&b)){
        if(valid_styles)while(style+1<style_count && styles[style].offset+styles[style].length<=byte)++style;
        if(boxes && box_index>=box_count){sb_native_text_free(&p);return sb_error(SB_INVALID,"Native Zeichenflächen sind zu kurz.");}
        const SBNativeCharBox *box=boxes ? &boxes[box_index++] : NULL;
        if(box && (box_index>box_count || box->byte!=byte || box->length!=b.byte-byte || !isfinite(box->x) || !isfinite(box->y) || !isfinite(box->width) || !isfinite(box->height) || box->width<0 || box->height<=0)){sb_native_text_free(&p);return sb_error(SB_INVALID,"Native Zeichenfläche passt nicht zur Quelle.");}
        unsigned format=valid_styles ? styles[style].style : 0;size_t bytes=b.byte-byte;
        if(bytes<=UINT8_MAX){if(!append(&p,byte,bytes,scalar,b.characters,format,new_line,box))goto memory;}
        else{
            /* The provider cannot encode a >255-byte selectable character.
               Preserve the complete source; UI selection still clamps it. */
            p.scalar_fallback=true;size_t at=byte,index=scalar;
            while(at<b.byte){unsigned char first=(unsigned char)text[at];size_t n=first<0x80 ? 1 : first<0xe0 ? 2 : first<0xf0 ? 3 : 4;
                if(!append(&p,at,n,index,index+1,format,new_line,NULL))goto memory;new_line=false;at+=n;++index;}
        }
        new_line=text[byte]=='\r' || text[byte]=='\n' || (bytes==2 && (unsigned char)text[byte]==0xc2 && (unsigned char)text[byte+1]==0x85) || (bytes==3 && !memcmp(text+byte,"\xe2\x80",2) && ((unsigned char)text[byte+2]==0xa8 || (unsigned char)text[byte+2]==0xa9));
        byte=b.byte;scalar=b.characters;
    }
    if(!p.count){p.runs=calloc(1,sizeof(*p.runs));if(!p.runs)goto memory;p.count=p.capacity=1;}
    else if(new_line){
        if(p.count==p.capacity){size_t next=p.capacity*2;SBNativeTextRun *grown=realloc(p.runs,next*sizeof(*grown));if(!grown)goto memory;p.runs=grown;p.capacity=next;}
        p.runs[p.count]=(SBNativeTextRun){.span={length,0,p.runs[p.count-1].span.style}};
        p.runs[p.count].scalars[0]=(uint32_t)scalar;++p.count;
    }
    if(boxes){
        if(box_index<box_count && boxes[box_index].byte==length && !boxes[box_index].length){SBNativeTextRun *last=&p.runs[p.count-1];const SBNativeCharBox *box=&boxes[box_index++];
            if(last->characters || !isfinite(box->x) || !isfinite(box->y) || !isfinite(box->width) || !isfinite(box->height) || box->width<0 || box->height<=0){sb_native_text_free(&p);return sb_error(SB_INVALID,"Ungültige leere native Zeichenfläche.");}
            last->geometry=true;last->x=box->x;last->y=box->y;last->width=box->width;last->height=box->height;last->level=box->level;}
        if(box_index!=box_count){sb_native_text_free(&p);return sb_error(SB_INVALID,"Unvollständige native Zeichenflächen.");}
        for(size_t i=0;i<p.count;++i){SBNativeTextRun *r=&p.runs[i];if(p.scalar_fallback){r->geometry=false;continue;}for(size_t j=0;j<r->characters;++j)r->positions[j]=(r->level&1) ? r->x+r->width-r->positions[j]-r->widths[j] : r->positions[j]-r->x;}
    }
    /* Word starts use whole-value context, including across style/run splits. */
    SBWord words;SBWordBoundary w;size_t first=0;SBGrapheme boundaries;SBGraphemeBoundary next={0};size_t floor=0,run=0;
    sb_word_init(&words,text,length);sb_grapheme_init(&boundaries,text,length);sb_grapheme_next(&boundaries,&next);
    while(sb_word_next(&words,&w)){
        if(w.significant && w.characters>first){
            while(next.characters<=first){floor=next.characters;if(!sb_grapheme_next(&boundaries,&next))break;}
            while(run+1<p.count && p.runs[run+1].scalars[0]<=floor)++run;
            SBNativeTextRun *r=&p.runs[run];size_t lo=0,hi=r->characters;
            while(lo<hi){size_t mid=lo+(hi-lo)/2;if(r->scalars[mid]<floor)lo=mid+1;else hi=mid;}
            if(lo<r->characters && (!r->word_count || r->words[r->word_count-1]!=lo))r->words[r->word_count++]=(uint8_t)lo;
        }first=w.characters;
    }
    *out=p;return sb_ok();
memory:sb_native_text_free(&p);return sb_error(SB_MEMORY,"Native Textläufe benötigen mehr Speicher.");
}

SBStatus sb_native_text(const char *text,size_t length,const SBTextSpan *styles,size_t style_count,SBNativeText *out){return build(text,length,styles,style_count,NULL,0,out);}
SBStatus sb_native_text_geometry(const char *text,size_t length,const SBTextSpan *styles,size_t style_count,const SBNativeCharBox *boxes,size_t box_count,SBNativeText *out){
    if(!boxes || !box_count){if(out)*out=(SBNativeText){0};return sb_error(SB_INVALID,"Native Zeichenflächen fehlen.");}return build(text,length,styles,style_count,boxes,box_count,out);
}
