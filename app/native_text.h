#ifndef SB_NATIVE_TEXT_H
#define SB_NATIVE_TEXT_H
#include "inline.h"
/* AccessKit's byte-length and word-index arrays use uint8_t. Keep each run
   within 255 selectable units. Source positions remain Unicode scalars. */
#define SB_NATIVE_CHAR_LIMIT 255
typedef struct {size_t byte,length;float x,y,width,height;unsigned char level;} SBNativeCharBox;
typedef struct {
    SBTextSpan span;
    size_t characters,word_count;
    uint8_t lengths[SB_NATIVE_CHAR_LIMIT],words[SB_NATIVE_CHAR_LIMIT];
    uint32_t scalars[SB_NATIVE_CHAR_LIMIT+1];
    float x,y,width,height,positions[SB_NATIVE_CHAR_LIMIT],widths[SB_NATIVE_CHAR_LIMIT];
    bool geometry;unsigned char level;
} SBNativeTextRun;
typedef struct {SBNativeTextRun *runs;size_t count,capacity;bool scalar_fallback;} SBNativeText;
SBStatus sb_native_text(const char *text,size_t length,const SBTextSpan *styles,size_t style_count,SBNativeText *out);
SBStatus sb_native_text_geometry(const char *text,size_t length,const SBTextSpan *styles,size_t style_count,const SBNativeCharBox *boxes,size_t box_count,SBNativeText *out);
void sb_native_text_free(SBNativeText *text);
#endif
