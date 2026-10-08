#ifndef SB_WORD_H
#define SB_WORD_H
#include "sb.h"
/* Default word boundaries, UAX #29 revision 49 / Unicode 18.0.0.
   A boundary's significant flag describes the preceding segment: letters,
   numbers, identifier connectors or emoji. No dictionary-based tailoring. */
typedef struct {
    const char *text;size_t length,byte,characters;
    unsigned previous,significant,before,regional;
    bool started,ended,segment_significant;
} SBWord;
typedef struct {size_t byte,characters;bool significant;} SBWordBoundary;
bool sb_word_init(SBWord *reader,const char *text,size_t length);
bool sb_word_next(SBWord *reader,SBWordBoundary *boundary);
#endif
