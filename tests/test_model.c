#include "model.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr, "FAIL %d: %s\n", __LINE__, #x); return 1; } } while (0)
#define OK(call) do { SBStatus s_ = (call); ++checks; if (s_.code != SB_OK) { fprintf(stderr, "FAIL %d: %s\n", __LINE__, s_.message); return 1; } } while (0)

int main(int argc, char **argv) {
    SBApp app;
    char workspace[SB_PATH_CAP], path[SB_PATH_CAP], temporary[SB_PATH_CAP], slug[65];
    char suffix[100], *text = NULL;
    SBRevision revision = {0};
    SBNotes results = {0};
    if (argc != 2) return 2;
    snprintf(suffix, sizeof(suffix), "run-%lu-%lu/Basis ü", sb_process_id(), (unsigned long)time(NULL));
    OK(sb_path_join(workspace, sizeof(workspace), argv[1], suffix));
    OK(sb_app_init(&app, workspace));
    CHECK(!app.has_project && app.projects.count == 0 && !app.editor);
    OK(sb_app_new_project(&app, "first", "Erstes Projekt ü", NULL));
    CHECK(app.has_project && !strcmp(app.project.id, "first") && !strcmp(app.path, "PROJECT.md"));
    CHECK(!sb_app_dirty(&app));
    strcpy(app.editor, "# Erster Auftrag\n\nMein Wissen ü.\n");
    CHECK(sb_app_dirty(&app));
    OK(sb_app_request(&app, SB_ACT_NOTE, app.path));
    CHECK(!app.guard && sb_app_dirty(&app));
    OK(sb_app_new_note(&app, "knowledge", "energie", "Energie"));
    CHECK(app.guard && !strcmp(app.path, "PROJECT.md"));
    OK(sb_app_decide(&app, SB_KEEP_EDITING));
    CHECK(!app.guard && sb_app_dirty(&app));
    OK(sb_app_request(&app, SB_ACT_NOTE, "knowledge/energie.md"));
    CHECK(app.guard);
    OK(sb_app_decide(&app, SB_SAVE_CHANGES));
    CHECK(!strcmp(app.path, "knowledge/energie.md") && !sb_app_dirty(&app));
    OK(sb_note_load(&app.project, "PROJECT.md", &text, NULL));
    CHECK(strstr(text, "Mein Wissen ü"));
    free(text); text = NULL;
    strcpy(app.editor, "# Energie\n\nGespeicherter Befund.\n");
    OK(sb_app_save(&app));
    CHECK(!sb_app_dirty(&app));
    OK(sb_app_request(&app, SB_ACT_RELOAD, NULL));
    CHECK(!strcmp(app.path, "knowledge/energie.md") && strstr(app.editor, "Gespeicherter Befund"));
    OK(sb_search(&app.project, "befund", &results));
    CHECK(results.count == 1);
    sb_notes_free(&results);

    OK(sb_path_join(path, sizeof(path), app.workspace, "quelle ü.md"));
    OK(sb_fs_write_new(path, "# Originalquelle\n\nRelevantes Wissen.\n", strlen("# Originalquelle\n\nRelevantes Wissen.\n")));
    strcat(app.editor, "Ungespeicherter Zusatz.\n");
    OK(sb_app_source(&app, "../../quelle%20%C3%BC.md"));
    CHECK(app.source && strstr(app.source, "Relevantes Wissen") && sb_app_dirty(&app));
    CHECK(sb_app_source(&app, "fehlt.md").code == SB_NOT_FOUND);
    CHECK(app.source && strstr(app.source, "Relevantes Wissen"));
    sb_app_source_close(&app);
    CHECK(!app.source && strstr(app.editor, "Ungespeicherter Zusatz"));
    CHECK(sb_app_source(&app, "%00.md").code == SB_INVALID);
    CHECK(sb_app_source(&app, "https://example.com").code == SB_INVALID);
    OK(sb_app_request(&app, SB_ACT_ARCHIVE, NULL));
    CHECK(app.guard);
    OK(sb_app_decide(&app, SB_DISCARD_CHANGES));
    CHECK(!strcmp(app.path, "archive/energie.md") && !sb_app_dirty(&app));
    CHECK(strstr(app.editor, "Gespeicherter Befund") && !strstr(app.editor, "Ungespeicherter Zusatz"));
    CHECK(sb_fs_kind(path) == 1);

    strcat(app.editor, "Noch ungespeichert.\n");
    OK(sb_app_new_project(&app, "second", "Zweites Projekt", NULL));
    CHECK(app.guard && !strcmp(app.project.id, "first") && app.projects.count == 2);
    OK(sb_app_decide(&app, SB_DISCARD_CHANGES));
    CHECK(!strcmp(app.project.id, "second") && !sb_app_dirty(&app));
    OK(sb_app_new_note(&app, "knowledge", "fehler", "Fehleranalyse"));
    strcpy(app.editor, "# Fehleranalyse\n\nMeine eigene Fassung.\n");
    OK(sb_path_join(path, sizeof(path), app.project.root, app.path));
    OK(sb_path_join(temporary, sizeof(temporary), app.project.root, "external.tmp"));
    OK(sb_fs_write_new(temporary, "# Fremde Fassung\n", strlen("# Fremde Fassung\n")));
    OK(sb_fs_replace(temporary, path));
    CHECK(sb_app_save(&app).code == SB_CONFLICT);
    CHECK(sb_app_dirty(&app) && strstr(app.editor, "Meine eigene Fassung"));
    OK(sb_app_request(&app, SB_ACT_PROJECT, "first"));
    CHECK(app.guard);
    CHECK(sb_app_decide(&app, SB_SAVE_CHANGES).code == SB_CONFLICT);
    CHECK(app.guard && !strcmp(app.project.id, "second"));
    OK(sb_app_save_copy(&app));
    CHECK(app.guard && !sb_app_dirty(&app) && strstr(app.path, "knowledge/kopie-"));
    CHECK(strstr(app.editor, "Meine eigene Fassung"));
    OK(sb_fs_read(path, &text, &revision.length));
    CHECK(!strcmp(text, "# Fremde Fassung\n"));
    free(text); text = NULL;
    OK(sb_app_decide(&app, SB_SAVE_CHANGES));
    CHECK(!app.guard && !strcmp(app.project.id, "first"));
    strcpy(app.editor, "# Nicht verlieren\n");
    OK(sb_app_request(&app, SB_ACT_WORKSPACE, workspace));
    CHECK(app.guard);
    OK(sb_app_decide(&app, SB_KEEP_EDITING));
    CHECK(strstr(app.editor, "Nicht verlieren"));
    OK(sb_app_request(&app, SB_ACT_NOTE, "fehlt.md"));
    OK(sb_app_decide(&app, SB_DISCARD_CHANGES).code == SB_NOT_FOUND ? sb_ok() : sb_error(SB_INVALID, "Fehlendes Dokument muss erkannt werden."));
    CHECK(sb_app_dirty(&app) && strstr(app.editor, "Nicht verlieren"));
    OK(sb_app_request(&app, SB_ACT_QUIT, NULL));
    CHECK(app.guard && !app.quit);
    OK(sb_app_decide(&app, SB_KEEP_EDITING));
    CHECK(!app.quit && sb_app_dirty(&app));
    OK(sb_app_request(&app, SB_ACT_QUIT, NULL));
    OK(sb_app_decide(&app, SB_SAVE_CHANGES));
    CHECK(app.quit && !sb_app_dirty(&app));
    sb_app_free(&app);

    sb_app_slug("Über Kräfte & Größe", slug, sizeof(slug));
    CHECK(!strcmp(slug, "ueber-kraefte-groesse") && sb_id_valid(slug));
    sb_app_slug("CON", slug, sizeof(slug));
    CHECK(!strcmp(slug, "projekt"));
    sb_app_slug("中文", slug, sizeof(slug));
    CHECK(sb_id_valid(slug));
    printf("%u application assertions passed: navigation, dirty guard, sources, archive, conflict copies, quit.\n", checks);
    return 0;
}
