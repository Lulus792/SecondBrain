#ifndef SB_PROJECTION_H
#define SB_PROJECTION_H
#include "sb.h"
#define SB_PROJECTION_LIMIT 65536u
/* A read-only source and a separately owned, source-mapped parsing view. */
typedef struct {
    size_t view,length,source,source_length;
    bool copy;
} SBProjectionSpan;
typedef struct {
    const char *source;
    size_t source_length;
    char *text;
    size_t length,capacity;
    SBProjectionSpan *spans;
    size_t count,span_capacity;
} SBProjection;
typedef struct {
    size_t byte,end,column,pending,anchor;
} SBProjectionCursor;
SBStatus sb_projection_init(SBProjection *view,const char *source,size_t length);
void sb_projection_free(SBProjection *view);
SBStatus sb_projection_copy(SBProjection *view,size_t offset,size_t length);
SBStatus sb_projection_spaces(SBProjection *view,size_t count,size_t source_tab);
SBStatus sb_projection_newline(SBProjection *view,size_t offset,size_t length);
SBStatus sb_projection_source(const SBProjection *view,size_t offset,size_t *source);
SBStatus sb_projection_cursor(SBProjectionCursor *cursor,size_t offset,size_t end,size_t column);
/* Consume only indentation; a short prefix succeeds with consumed < requested.
   A tab can be partially consumed. Errors leave the cursor unchanged. */
SBStatus sb_projection_indent(const char *source,size_t length,SBProjectionCursor *cursor,size_t requested,size_t *consumed);
SBStatus sb_projection_remainder(SBProjection *view,const SBProjectionCursor *cursor);
#endif
