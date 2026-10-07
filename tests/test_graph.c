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
    const char *text = "# A\n[Stand](../STATE.md)\n[Nochmals](../STATE.md)\n![Bild](../PROJECT.md)\n```\n[falsch](../QUESTIONS.md)\n~~~\n[weiter falsch](../PROJECT.md)\n~~~\n```\n[Entscheidung](../DECISIONS.md)\n`[inline](../PROJECT.md)`\n\\[escaped](../QUESTIONS.md)\n[Web](https://example.com)\n[Extern](../../other.md)\n";
    char *old = NULL; SBRevision revision;
    CHECK(sb_note_load(&project, note.path, &old, &revision).code == SB_OK); sb_text_free(old);
    CHECK(sb_note_save(&project, note.path, text, revision, NULL).code == SB_OK);
    CHECK(sb_notes_list(&project, &notes).code == SB_OK);
    CHECK(sb_graph_build(&project, &notes, &graph).code == SB_OK && graph.count == notes.count);
    size_t index = 0, links = 0;
    while (index < notes.count && strcmp(notes.items[index].path, "knowledge/a.md")) ++index;
    CHECK(index < notes.count);
    for (size_t i = 0; i < graph.edge_count; ++i) if (graph.edges[i].from == index) {
        ++links; CHECK(!strcmp(notes.items[graph.edges[i].to].path, "STATE.md") || !strcmp(notes.items[graph.edges[i].to].path, "DECISIONS.md"));
    }
    CHECK(links == 2);
    SBStar saved = graph.stars[index];
    CHECK(sb_graph_build(&project, &notes, &graph).code == SB_OK);
    CHECK(!memcmp(&saved, &graph.stars[index], sizeof(saved)));
    char named_path[SB_PATH_CAP];
    CHECK(sb_path_join(named_path,sizeof(named_path),project.root,"knowledge/Name ü (Teil).md").code==SB_OK);
    CHECK(sb_fs_write_new(named_path,"# Name ü\n",strlen("# Name ü\n")).code==SB_OK);
    const char *shared="# A\n\n[Stand](../STATE.md \"Titel\") [Entscheidung](../DECISIONS.md)\n[Name](<Name ü (Teil).md>)\n`[Nur Code](../PROJECT.md)` \\[Maskiert](../SOURCES.md)\n![Bild](../PROJECT.md)\n[Außen [Stand](../STATE.md)](../PROJECT.md)\n\n    [Eingerückt](../PROJECT.md)\n\n```info`ungültiger Zaun\n[Frage](../QUESTIONS.md)\n\n~~~~\n[Falsch](../PROJECT.md)\n~~~\n[Noch falsch](../SOURCES.md)\n~~~~\n";
    CHECK(sb_note_load(&project,note.path,&old,&revision).code==SB_OK); sb_text_free(old);
    CHECK(sb_note_save(&project,note.path,shared,revision,NULL).code==SB_OK);
    sb_notes_free(&notes); CHECK(sb_notes_list(&project,&notes).code==SB_OK);
    CHECK(sb_graph_build(&project,&notes,&graph).code==SB_OK);
    index=0; links=0; while (index<notes.count && strcmp(notes.items[index].path,note.path)) ++index;
    CHECK(index<notes.count);
    for (size_t i=0;i<graph.edge_count;++i) if (graph.edges[i].from==index) {
        ++links; const char *to=notes.items[graph.edges[i].to].path;
        CHECK(!strcmp(to,"STATE.md") || !strcmp(to,"DECISIONS.md") || !strcmp(to,"QUESTIONS.md") || !strcmp(to,"knowledge/Name ü (Teil).md"));
    }
    CHECK(links==4);
    CHECK(sb_note_load(&project,note.path,&old,NULL).code==SB_OK && !strcmp(old,shared)); sb_text_free(old);
    const char *table_links="# A\n\n| Quelle | Text |\n| --- | --- |\n| [Stand](../STATE.md) | `[Code](../PROJECT.md)` | [Überzählig](../SOURCES.md) |\n| [Fragen](../QUESTIONS.md) | Wert |\n";
    CHECK(sb_note_load(&project,note.path,&old,&revision).code==SB_OK); sb_text_free(old);
    CHECK(sb_note_save(&project,note.path,table_links,revision,NULL).code==SB_OK);
    sb_notes_free(&notes); CHECK(sb_notes_list(&project,&notes).code==SB_OK);
    CHECK(sb_graph_build(&project,&notes,&graph).code==SB_OK); index=0;links=0;
    while (index<notes.count && strcmp(notes.items[index].path,note.path)) ++index;
    CHECK(index<notes.count);
    for (size_t i=0;i<graph.edge_count;++i) if (graph.edges[i].from==index) {
        ++links; const char *to=notes.items[graph.edges[i].to].path; CHECK(!strcmp(to,"STATE.md") || !strcmp(to,"QUESTIONS.md"));
    }
    CHECK(links==2);
    CHECK(sb_path_join(named_path,sizeof(named_path),project.root,"knowledge/föö.md").code==SB_OK);
    CHECK(sb_fs_write_new(named_path,"# Ziel &amp; Text\n",strlen("# Ziel &amp; Text\n")).code==SB_OK);
    const char *entity_links="# A &amp; B\n\n[Ziel](f&ouml;&ouml;.md) &#91;falsch&#93;(../PROJECT.md)\n`[Code](f&ouml;&ouml;.md)`\n";
    CHECK(sb_note_load(&project,note.path,&old,&revision).code==SB_OK);sb_text_free(old);
    CHECK(sb_note_save(&project,note.path,entity_links,revision,NULL).code==SB_OK);
    sb_notes_free(&notes);CHECK(sb_notes_list(&project,&notes).code==SB_OK);
    CHECK(sb_graph_build(&project,&notes,&graph).code==SB_OK);index=0;links=0;
    while (index<notes.count && strcmp(notes.items[index].path,note.path)) ++index;
    CHECK(index<notes.count && !strcmp(notes.items[index].title,"A & B"));
    for (size_t i=0;i<graph.edge_count;++i) if (graph.edges[i].from==index) {
        ++links;CHECK(!strcmp(notes.items[graph.edges[i].to].path,"knowledge/föö.md"));
        CHECK(!strcmp(notes.items[graph.edges[i].to].title,"Ziel & Text"));
    }
    CHECK(links==1);
    CHECK(sb_note_load(&project,note.path,&old,NULL).code==SB_OK && !strcmp(old,entity_links));sb_text_free(old);
    const char *reference_links="# [Start][Straße]\n\n[Ziel][STRASSE] [Straße][] [Straße]\n![Bild][Straße] [Unbekannt][fake]\n\n| Quelle |\n| --- |\n| [Zelle][straße] |\n\n[straße]: f&ouml;&ouml;.md\n[STRASSE]: ../PROJECT.md\n\n```\n[fake]: ../SOURCES.md\n```\n";
    CHECK(sb_note_load(&project,note.path,&old,&revision).code==SB_OK);sb_text_free(old);
    CHECK(sb_note_save(&project,note.path,reference_links,revision,NULL).code==SB_OK);
    sb_notes_free(&notes);CHECK(sb_notes_list(&project,&notes).code==SB_OK);
    CHECK(sb_graph_build(&project,&notes,&graph).code==SB_OK);index=0;links=0;
    while(index<notes.count && strcmp(notes.items[index].path,note.path))++index;
    CHECK(index<notes.count && !strcmp(notes.items[index].title,"Start"));
    for(size_t i=0;i<graph.edge_count;++i)if(graph.edges[i].from==index){++links;CHECK(!strcmp(notes.items[graph.edges[i].to].path,"knowledge/föö.md"));}
    CHECK(links==1);
    CHECK(sb_note_load(&project,note.path,&old,NULL).code==SB_OK && !strcmp(old,reference_links));sb_text_free(old);
    sb_graph_free(&graph); sb_notes_free(&notes);
    printf("%u graph assertions passed.\n", checks);
    return 0;
}
