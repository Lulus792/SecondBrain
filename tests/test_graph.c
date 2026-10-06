#include "graph.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
int main(int argc, char **argv) {
    char path[SB_PATH_CAP], root[SB_PATH_CAP], suffix[80];
    SBProject project; SBNotes notes = {0}; SBGraph graph = {0}; SBNote note;
    unsigned checks = 0;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr, "Graph line %d: %s\n", __LINE__, #x); return 1; } } while (0)
    CHECK(argc == 2);
    CHECK(sb_graph_destination("knowledge/a.md", "../STATE.md#Stand", path, sizeof(path)) && !strcmp(path, "STATE.md"));
    CHECK(sb_graph_destination("knowledge/deep/a.md", "../b%20%C3%BC.md", path, sizeof(path)) && !strcmp(path, "knowledge/b ü.md"));
    CHECK(!sb_graph_destination("a.md", "../outside.md", path, sizeof(path)));
    CHECK(!sb_graph_destination("a.md", "https://example.com/a.md", path, sizeof(path)));
    CHECK(!sb_graph_destination("a.md", "%00.md", path, sizeof(path)));
    CHECK(!sb_graph_destination("a.md", "x%2Fy.md", path, 4));
    snprintf(suffix, sizeof(suffix), "graph-%lu-%lu", sb_process_id(), (unsigned long)time(NULL));
    CHECK(sb_path_join(root, sizeof(root), argv[1], suffix).code == SB_OK);
    CHECK(sb_project_create(root, "test", "Graph", "", &project).code == SB_OK);
    CHECK(sb_note_create(&project, "knowledge", "a", "A", &note).code == SB_OK);
    const char *text = "# A\n[Stand](../STATE.md)\n[Nochmals](../STATE.md)\n![Bild](../PROJECT.md)\n```\n[falsch](../QUESTIONS.md)\n```\n[Web](https://example.com)\n[Extern](../../other.md)\n";
    char *old = NULL; SBRevision revision;
    CHECK(sb_note_load(&project, note.path, &old, &revision).code == SB_OK); sb_text_free(old);
    CHECK(sb_note_save(&project, note.path, text, revision, NULL).code == SB_OK);
    CHECK(sb_notes_list(&project, &notes).code == SB_OK);
    CHECK(sb_graph_build(&project, &notes, &graph).code == SB_OK && graph.count == notes.count);
    size_t index = 0, links = 0;
    while (index < notes.count && strcmp(notes.items[index].path, "knowledge/a.md")) ++index;
    CHECK(index < notes.count);
    for (size_t i = 0; i < graph.edge_count; ++i) if (graph.edges[i].from == index) {
        ++links; CHECK(!strcmp(notes.items[graph.edges[i].to].path, "STATE.md"));
    }
    CHECK(links == 1);
    SBStar saved = graph.stars[index];
    CHECK(sb_graph_build(&project, &notes, &graph).code == SB_OK);
    CHECK(!memcmp(&saved, &graph.stars[index], sizeof(saved)));
    sb_graph_free(&graph); sb_notes_free(&notes);
    printf("%u graph assertions passed.\n", checks);
    return 0;
}
