#ifndef SB_INLINE_H
#define SB_INLINE_H
#include "sb.h"
#define SB_INLINE_LIMIT 65536u
typedef enum { SB_INLINE_TEXT,SB_INLINE_ESCAPE,SB_INLINE_CODE,SB_INLINE_LINK,SB_INLINE_IMAGE,SB_INLINE_FORMAT,SB_INLINE_RAW,SB_INLINE_AUTOLINK,SB_INLINE_ENTITY } SBInlineKind;
enum { SB_TEXT_ITALIC=1,SB_TEXT_BOLD=2,SB_TEXT_CODE=4 };
typedef struct { size_t offset,length; unsigned style; } SBTextSpan;
typedef struct { char *text; SBTextSpan *spans; size_t count,capacity; } SBStyledText;
typedef struct {
    SBInlineKind kind;
    size_t offset,length,content,content_length,destination,destination_length;
    bool email;
} SBInlineToken;
struct SBInlineRun; struct SBInlinePair; struct SBInlineMark;
typedef struct {
    const char *text;
    size_t length,cursor;
    struct SBInlineRun *runs; size_t run_count,run_capacity;
    struct SBInlinePair *pairs; size_t pair_count,pair_capacity;
    struct SBInlineMark *marks; size_t mark_count,mark_capacity;
    bool marks_ready;
    SBInlineToken *opaque; size_t opaque_count,opaque_capacity;
} SBInline;
/* Initialize fresh storage, then free it even after an error. Source is borrowed. */
SBStatus sb_inline_init(SBInline *reader,const char *text,size_t length);
void sb_inline_free(SBInline *reader);
bool sb_inline_next(SBInline *reader,SBInlineToken *token);
SBStatus sb_inline_text(const SBInline *reader,size_t offset,size_t length,char **out);
SBStatus sb_inline_styled(const SBInline *reader,size_t offset,size_t length,SBStyledText *out);
void sb_styled_free(SBStyledText *text);
SBStatus sb_inline_destination(const SBInline *reader,const SBInlineToken *token,char *out,size_t capacity);
#endif
