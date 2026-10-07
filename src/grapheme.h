#ifndef SB_GRAPHEME_H
#define SB_GRAPHEME_H
#include "sb.h"
typedef struct {
    const char *text; size_t length,byte,characters;
    unsigned previous,regional;
    bool started,ended,emoji_chain,emoji_zwj,indic_link;
} SBGrapheme;
typedef struct { size_t byte,characters; } SBGraphemeBoundary;
typedef struct { size_t previous,floor,ceil,next; bool boundary; } SBGraphemePosition;
/* Extended grapheme clusters, UAX #29 revision 49 / Unicode 18.0.0.
   Positions count Unicode scalar values, preserving the existing editor ABI. */
bool sb_grapheme_init(SBGrapheme *reader,const char *text,size_t length);
bool sb_grapheme_next(SBGrapheme *reader,SBGraphemeBoundary *boundary);
bool sb_grapheme_position(const char *text,size_t length,size_t characters,SBGraphemePosition *position);
#endif
