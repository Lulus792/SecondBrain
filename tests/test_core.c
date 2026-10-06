#include "sb.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static unsigned assertions;
#define CHECK(condition) do { ++assertions; if (!(condition)) { \
    fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); return 1; } } while (0)
#define OK(call) do { SBStatus result_ = (call); ++assertions; if (result_.code != SB_OK) { \
    fprintf(stderr, "FAIL %s:%d: %s: %s\n", __FILE__, __LINE__, #call, result_.message); return 1; } } while (0)

int main(int argc, char **argv) {
    char workspace[SB_PATH_CAP], root[SB_PATH_CAP], path[SB_PATH_CAP], temporary[SB_PATH_CAP];
    char archived[SB_PATH_CAP], suffix[128], *text = NULL, *context = NULL;
    SBProject project;
    SBProjects projects = {0};
    SBNotes notes = {0};
    SBNote note;
    SBRevision first = {0}, saved = {0};
    const char *name = "Physim ü \"Wissen\" @DATE@";
    if (argc != 2) return 2;
    snprintf(suffix, sizeof(suffix), "run-%lu-%lu/Wissensbasis ü", sb_process_id(), (unsigned long)time(NULL));
    OK(sb_path_join(workspace, sizeof(workspace), argv[1], suffix));
    OK(sb_fs_absolute(workspace, root, sizeof(root)));

    CHECK(sb_id_valid("mein-projekt") && !sb_id_valid("../outside") && !sb_id_valid("con"));
    CHECK(!sb_id_valid("com9") && !sb_id_valid("lpt1") && !sb_id_valid("a--b"));
    CHECK(sb_utf8_valid("Äther ü 中文", strlen("Äther ü 中文")));
    CHECK(!sb_utf8_valid("\xc0\x80", 2) && !sb_utf8_valid("\xed\xa0\x80", 3));
    CHECK(!sb_utf8_valid("\xf4\x90\x80\x80", 4) && !sb_utf8_valid("a\0b", 3));
    CHECK(sb_project_create(workspace, "invalid/name", "Test", NULL, NULL).code == SB_INVALID);
    CHECK(sb_project_create(workspace, "test", "Zeile\nBefehl", NULL, NULL).code == SB_INVALID);
    CHECK(sb_project_create(workspace, "test", "Test", "/missing-secondbrain-repository", NULL).code == SB_INVALID);

    OK(sb_project_create(workspace, "physim", name, NULL, &project));
    OK(sb_note_load(&project, "PROJECT.md", &text, &first));
    CHECK(strstr(text, name) != NULL);
    free(text); text = NULL;
    CHECK(sb_project_create(workspace, "physim", "Ersetzen", NULL, NULL).code == SB_EXISTS);
    OK(sb_note_load(&project, "PROJECT.md", &text, &saved));
    CHECK(first.hash == saved.hash && first.length == saved.length);
    CHECK(sb_note_archive(&project, "PROJECT.md", saved, archived, sizeof(archived)).code == SB_INVALID);
    free(text); text = NULL;
    OK(sb_projects_list(workspace, &projects));
    CHECK(projects.count == 1 && !strcmp(projects.items[0].name, name));
    sb_projects_free(&projects);
    OK(sb_notes_list(&project, &notes));
    CHECK(notes.count == 11);
    sb_notes_free(&notes);

    OK(sb_note_create(&project, "knowledge", "energy", "Äther und Energie", &note));
    CHECK(sb_note_create(&project, "knowledge", "energy", "Überschreiben", NULL).code == SB_CONFLICT);
    OK(sb_note_load(&project, note.path, &text, &first));
    CHECK(strstr(text, "Äther und Energie") != NULL);
    free(text); text = NULL;
    OK(sb_note_save(&project, note.path, "# Äther und Energie\n\nMessung mit Unicode ü 中文.\n", first, &saved));
    OK(sb_note_load(&project, note.path, &text, &first));
    CHECK(strstr(text, "中文") && saved.hash == first.hash);
    free(text); text = NULL;
    OK(sb_search(&project, "äTHER", &notes));
    CHECK(notes.count == 1 && !strcmp(notes.items[0].path, note.path));
    sb_notes_free(&notes);
    OK(sb_search(&project, "UNICODE Ü", &notes));
    CHECK(notes.count == 1);
    sb_notes_free(&notes);
    OK(sb_search(&project, "fehlt-vollständig", &notes));
    CHECK(notes.count == 0);
    sb_notes_free(&notes);
    CHECK(sb_search(&project, "\xff", &notes).code == SB_INVALID);

    OK(sb_path_join(path, sizeof(path), project.root, note.path));
    OK(sb_path_join(temporary, sizeof(temporary), project.root, "external.tmp"));
    OK(sb_fs_write_new(temporary, "# Externe Änderung\n", strlen("# Externe Änderung\n")));
    OK(sb_fs_replace(temporary, path));
    CHECK(sb_note_save(&project, note.path, "Mein ungespeicherter Text", first, NULL).code == SB_CONFLICT);
    OK(sb_note_load(&project, note.path, &text, &saved));
    CHECK(!strcmp(text, "# Externe Änderung\n"));
    free(text); text = NULL;
    CHECK(sb_note_save(&project, "../../outside.md", "Ungültig", saved, NULL).code == SB_INVALID);
    CHECK(sb_note_save(&project, "knowledge/bad.md", "\xff", (SBRevision){0}, NULL).code == SB_INVALID);
    OK(sb_note_archive(&project, note.path, saved, archived, sizeof(archived)));
    CHECK(!strcmp(archived, "archive/energy.md"));
    CHECK(sb_note_load(&project, note.path, &text, NULL).code == SB_NOT_FOUND);
    OK(sb_note_load(&project, archived, &text, &first));
    free(text); text = NULL;
    CHECK(sb_note_archive(&project, archived, first, archived, sizeof(archived)).code == SB_INVALID);

    OK(sb_note_create(&project, "inbox", "capture", "Neue Idee", &note));
    OK(sb_note_load(&project, note.path, &text, &first));
    free(text); text = NULL;
    OK(sb_path_join(path, sizeof(path), project.root, note.path));
    OK(sb_fs_remove(path));
    CHECK(sb_note_save(&project, note.path, "Text bleibt beim Nutzer", first, NULL).code == SB_CONFLICT);
    CHECK(sb_fs_kind(path) == 0);
    OK(sb_context_build(&project, &context));
    CHECK(strstr(context, "STATE.md") && strstr(context, "SOURCES.md") && strstr(context, name));
    free(context); context = NULL;

    OK(sb_path_join(path, sizeof(path), project.root, "brain.json"));
    OK(sb_path_join(temporary, sizeof(temporary), project.root, "metadata.tmp"));
    {
        const char *json = "{\"name\":\"Physim \\u00fc \\ud83d\\ude80\",\"other\":[true,null,{\"a\":12.5e2}]}";
        OK(sb_fs_write_new(temporary, json, strlen(json)));
        OK(sb_fs_replace(temporary, path));
        OK(sb_projects_list(workspace, &projects));
        CHECK(projects.count == 1 && !strcmp(projects.items[0].name, "Physim ü 🚀"));
        sb_projects_free(&projects);
    }
    {
        const char *bad[] = {"{\"name\":\"Test\",}", "{\"name\":\"\\ud800\"}", "{\"name\":\"A\",\"name\":\"B\"}", "{\"name\":\"A\",\"x\":01}"};
        for (size_t i = 0; i < sizeof(bad) / sizeof(*bad); ++i) {
            OK(sb_fs_write_new(temporary, bad[i], strlen(bad[i])));
            OK(sb_fs_replace(temporary, path));
            CHECK(sb_projects_list(workspace, &projects).code == SB_INVALID);
            CHECK(projects.items == NULL && projects.count == 0);
        }
    }
    printf("%u assertions passed: create, persistence, UTF-8, search, conflicts, archive, context, metadata.\n", assertions);
    return 0;
}
