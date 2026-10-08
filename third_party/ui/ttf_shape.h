#ifndef SB_TTF_SHAPE_H
#define SB_TTF_SHAPE_H
#include <SDL3_ttf/SDL_ttf.h>
/* Pixels at the font's backing density; x/y relative to the baseline.
   Source clusters remain UTF-8 offsets in the complete context string. */
typedef struct {
    Uint32 index,byte;
    float x,y,advance;
    int left,top,width,height;
} SBTTFGlyph;
typedef struct {
    SBTTFGlyph *glyphs;
    size_t count;
    float advance;
    int ascent,descent;
} SBTTFShape;
/* Optional paragraph engine mirror data, returning 0 when no partner exists. */
typedef Uint32 (*SBTTFMirror)(Uint32 codepoint);
/* Valid UTF-8, a scalar-aligned range and an initially empty output are
   required. Context is one complete logical line, not a substring copy.
   Font/script/style boundaries may limit the range without losing context.
   Shape and rasterize on the font-owning thread. */
bool sb_ttf_shape_range(TTF_Font *font,const char *context,size_t length,
    size_t byte,size_t range,bool rtl,Uint32 script,SBTTFMirror mirror,SBTTFShape *out);
void sb_ttf_shape_free(SBTTFShape *shape);
/* Borrowed shaping-language tag; read only on the font-owning thread. */
const char *sb_ttf_font_language(TTF_Font *font);
/* Original font GDEF caret coordinates in backing pixels. Reports the total
   count, writes at most capacity positions. No shape or font state changes. */
bool sb_ttf_ligature_carets(TTF_Font *font,Uint32 glyph,bool rtl,float *positions,
    size_t capacity,size_t *count);
#endif
