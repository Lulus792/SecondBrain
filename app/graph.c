#include "graph.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

static int hex(unsigned char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
bool sb_graph_destination(const char *from, const char *link, char *out, size_t capacity) {
    char decoded[SB_PATH_CAP], path[SB_PATH_CAP];
    size_t n = 0, used = 0;
    if (!from || !link || !out || !capacity) return false;
    if (*link == '<') ++link;
    while (*link && *link != '#' && *link != '?' && *link != '>') {
        unsigned char c = (unsigned char)*link++;
        if (c == '%') {
            if (!link[0] || !link[1] || hex(link[0]) < 0 || hex(link[1]) < 0) return false;
            c = (unsigned char)(hex(link[0]) * 16 + hex(link[1])); link += 2;
        }
        if (!c || c == ':' || c == '\\' || n + 1 >= sizeof(decoded)) return false;
        decoded[n++] = (char)c;
    }
    decoded[n] = 0;
    if (!n || decoded[0] == '/' || !sb_utf8_valid(decoded, n)) return false;
    const char *slash = strrchr(from, '/');
    size_t prefix = slash ? (size_t)(slash - from + 1) : 0;
    if (prefix + n >= sizeof(path)) return false;
    memcpy(path, from, prefix); memcpy(path + prefix, decoded, n + 1);
    char *part = path;
    while (*part) {
        char *end = strchr(part, '/');
        size_t length = end ? (size_t)(end - part) : strlen(part);
        if (length == 2 && part[0] == '.' && part[1] == '.') {
            if (!used) return false;
            while (used && out[used - 1] != '/') --used;
            if (used) --used;
        } else if (length && !(length == 1 && part[0] == '.')) {
            if (used + length + 2 > capacity) return false;
            if (used) out[used++] = '/';
            memcpy(out + used, part, length); used += length;
        }
        if (!end) break;
        part = end + 1;
    }
    out[used] = 0;
    return used != 0;
}
void sb_graph_free(SBGraph *g) {
    if (!g) return;
    free(g->stars); free(g->edges); memset(g, 0, sizeof(*g));
}
static SBStatus edge(SBGraph *g, size_t from, size_t to) {
    if (from == to) return sb_ok();
    for (size_t i = 0; i < g->edge_count; ++i)
        if (g->edges[i].from == from && g->edges[i].to == to) return sb_ok();
    if (g->edge_count == 65536) return sb_error(SB_LIMIT, "Die Sternkarte unterstützt höchstens 65536 Verweise.");
    if (g->edge_count == g->edge_capacity) {
        size_t cap = g->edge_capacity ? g->edge_capacity * 2 : 64;
        SBEdge *items = realloc(g->edges, cap * sizeof(*items));
        if (!items) return sb_error(SB_MEMORY, "Kein Speicher für Verweise.");
        g->edges = items; g->edge_capacity = cap;
    }
    g->edges[g->edge_count++] = (SBEdge){from, to};
    return sb_ok();
}
SBStatus sb_graph_build(const SBProject *project, const SBNotes *notes, SBGraph *out) {
    SBGraph graph = {0};
    SBStatus status = sb_ok();
    static const char *groups[] = {"overview", "knowledge", "inbox", "journal", "archive"};
    if (notes->count > 4096) return sb_error(SB_LIMIT, "Die Sternkarte unterstützt höchstens 4096 Dokumente. Die Dokumentliste bleibt verfügbar.");
    graph.stars = calloc(notes->count ? notes->count : 1, sizeof(*graph.stars));
    if (!graph.stars) return sb_error(SB_MEMORY, "Kein Speicher für die Sternkarte.");
    graph.count = notes->count;
    for (size_t i = 0; i < notes->count; ++i) {
        unsigned group = 0;
        for (unsigned k = 0; k < 5; ++k) if (!strcmp(notes->items[i].section, groups[k])) group = k;
        uint64_t hash = sb_hash(notes->items[i].path, strlen(notes->items[i].path));
        float angle = (float)(hash % 6283) / 1000.0f;
        float orbit = 70.0f + (float)((hash >> 16) % 130);
        float center = (float)group * 1.256637f - 1.57f;
        graph.stars[i] = (SBStar){cosf(center) * 220 + cosf(angle) * orbit,
            sinf(center) * 165 + sinf(angle) * orbit * 0.7f,
            (float)((int)((hash >> 32) % 180) - 90), group};
        char *text = NULL;
        status = sb_note_load(project, notes->items[i].path, &text, NULL);
        if (status.code != SB_OK) goto fail;
        const char *p = text;
        bool fence = false, line_start = true;
        while (*p) {
            if (line_start && (!strncmp(p, "```", 3) || !strncmp(p, "~~~", 3))) fence = !fence;
            line_start = *p == '\n';
            if (!fence && *p == '[' && (p == text || p[-1] != '!')) {
                const char *close = strchr(p + 1, ']');
                if (close && close[1] == '(') {
                    const char *end = close + 2;
                    unsigned nesting = 1;
                    while (*end && *end != '\n' && nesting) {
                        if (*end == '(') ++nesting;
                        if (*end == ')') --nesting;
                        if (nesting) ++end;
                    }
                    if (!nesting && (size_t)(end - close - 2) < SB_PATH_CAP) {
                        char link[SB_PATH_CAP], path[SB_PATH_CAP];
                        size_t length = (size_t)(end - close - 2);
                        memcpy(link, close + 2, length); link[length] = 0;
                        if (sb_graph_destination(notes->items[i].path, link, path, sizeof(path))) {
                            for (size_t k = 0; k < notes->count; ++k) if (!strcmp(path, notes->items[k].path)) {
                                status = edge(&graph, i, k);
                                if (status.code != SB_OK) { free(text); goto fail; }
                                break;
                            }
                        }
                        p = end;
                    }
                }
            }
            ++p;
        }
        free(text);
    }
    sb_graph_free(out); *out = graph; return sb_ok();
fail:
    sb_graph_free(&graph); return status;
}
