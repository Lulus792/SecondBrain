#ifndef SB_TEXT_BIDI_H
#define SB_TEXT_BIDI_H
#include "sb.h"
typedef enum { SB_BIDI_AUTO_LTR, SB_BIDI_AUTO_RTL, SB_BIDI_LTR, SB_BIDI_RTL } SBTextDirection;
typedef struct SBTextParagraph SBTextParagraph;
typedef struct { size_t byte,length; unsigned char level; } SBVisualRun;
typedef struct {
    SBVisualRun *runs;
    size_t count,byte,length;
    unsigned char base_level;
} SBVisualLine;
/* Owns an immutable UTF-8 copy. Processes the first paragraph (including its
   separator); length reports consumption so callers can continue with P1.
   Embedded NUL is allowed for normative tests, not for stored app documents. */
SBStatus sb_bidi_paragraph_create(const char *text,size_t length,SBTextDirection direction,SBTextParagraph **out);
void sb_bidi_paragraph_free(SBTextParagraph *paragraph);
size_t sb_bidi_paragraph_length(const SBTextParagraph *paragraph);
const char *sb_bidi_paragraph_text(const SBTextParagraph *paragraph);
unsigned char sb_bidi_paragraph_level(const SBTextParagraph *paragraph);
/* UTF-8 code-unit levels before L1; callers obtain post-L1 levels from runs.
   Mirroring is a shaping operation, never a mutation of this source copy. */
const unsigned char *sb_bidi_paragraph_levels(const SBTextParagraph *paragraph);
/* After logical wrapping, apply L1/L2 to one nonempty scalar-aligned range.
   Output must be empty; free it before reuse. Runs are in visual left-to-right
   order, with their source bytes still in logical order. Odd levels are RTL. */
SBStatus sb_bidi_line(const SBTextParagraph *paragraph,size_t byte,size_t length,SBVisualLine *out);
void sb_bidi_line_free(SBVisualLine *line);
uint32_t sb_bidi_mirror(uint32_t codepoint);
#endif
