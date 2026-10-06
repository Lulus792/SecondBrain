#ifndef SB_GRAPH_H
#define SB_GRAPH_H
#include "sb.h"

/* Layout groups describe folders; edges describe actual saved Markdown links. */
typedef struct { float x, y, z; unsigned group; } SBStar;
typedef struct { size_t from, to; } SBEdge;
typedef struct {
    SBStar *stars;
    SBEdge *edges;
    size_t count, edge_count, edge_capacity;
} SBGraph;
SBStatus sb_graph_build(const SBProject *project, const SBNotes *notes, SBGraph *out);
void sb_graph_free(SBGraph *graph);
bool sb_graph_destination(const char *from, const char *link, char *out, size_t capacity);
#endif
