#ifndef SB_SHAPED_LINE_H
#define SB_SHAPED_LINE_H
#include "bidi.h"
typedef struct TTF_Font TTF_Font;
/* Covers the paragraph in logical order; font/style/script boundaries are
   complete grapheme boundaries. Fonts must outlive the resulting line. */
typedef struct {size_t byte,length;TTF_Font *font;uint32_t script;} SBShapeFontSpan;
typedef struct {
    size_t byte;
    uint32_t index;
    TTF_Font *font;
    float x,y,advance;
    int left,top,width,height;
    unsigned char level;
} SBShapeGlyph;
typedef struct {
    size_t byte,length,glyph_begin,glyph_count;
    float x,advance;
    unsigned char level;
} SBShapeRun;
typedef struct {
    SBShapeGlyph *glyphs;size_t count,capacity;
    SBShapeRun *runs;size_t run_count,run_capacity;
    size_t byte,length;
    float advance;
    int ascent,descent;
    unsigned char base_level;
} SBShapedLine;
/* Resolves this logical line using its existing paragraph, then subdivides
   visual runs by fonts/scripts without re-running paragraph direction.
   Shapes against the complete line context, preserving joining across styles.
   Output must be empty. Pixel geometry is at the supplied fonts' density. */
SBStatus sb_shape_line(const SBTextParagraph *paragraph,size_t byte,size_t length,
    const SBShapeFontSpan *spans,size_t count,SBShapedLine *out);
void sb_shape_line_free(SBShapedLine *line);
#endif
