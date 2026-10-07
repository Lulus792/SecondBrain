#ifndef SB_INLINE_H
#define SB_INLINE_H
#include "sb.h"
#define SB_INLINE_LIMIT 65536u
typedef enum { SB_INLINE_TEXT,SB_INLINE_ESCAPE,SB_INLINE_CODE,SB_INLINE_LINK,SB_INLINE_IMAGE } SBInlineKind;
typedef struct {
    SBInlineKind kind;
    size_t offset,length,content,content_length,destination,destination_length;
} SBInlineToken;
struct SBInlineRun; struct SBInlinePair;
typedef struct {
    const char *text;
    size_t length,cursor;
    struct SBInlineRun *runs; size_t run_count,run_capacity;
    struct SBInlinePair *pairs; size_t pair_count,pair_capacity;
} SBInline;
/* Initialize fresh storage, then free it even after an error. Source is borrowed. */
SBStatus sb_inline_init(SBInline *reader,const char *text,size_t length);
void sb_inline_free(SBInline *reader);
bool sb_inline_next(SBInline *reader,SBInlineToken *token);
SBStatus sb_inline_text(const SBInline *reader,size_t offset,size_t length,char **out);
SBStatus sb_inline_destination(const SBInline *reader,const SBInlineToken *token,char *out,size_t capacity);
#endif
