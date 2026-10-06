#include "model.h"
#include "platform.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define TRY(call) do { SBStatus result_ = (call); if (result_.code != SB_OK) return result_; } while (0)

void sb_app_source_close(SBApp *app) {
    free(app->source); app->source = NULL; app->source_path[0] = 0; app->source_title[0] = 0;
}
void sb_app_free(SBApp *app) {
    sb_projects_free(&app->projects); sb_notes_free(&app->notes);
    free(app->editor); sb_app_source_close(app);
    memset(app, 0, sizeof(*app));
}
bool sb_app_dirty(const SBApp *app) {
    size_t length;
    if (!app->editor || !app->revision.exists) return false;
    length = strlen(app->editor);
    return length != app->revision.length || sb_hash(app->editor, length) != app->revision.hash;
}
static SBStatus load_text(const SBProject *project, const char *path, char **editor, SBRevision *revision) {
    char *loaded = NULL, *buffer;
    SBStatus result = sb_note_load(project, path, &loaded, revision);
    if (result.code != SB_OK) return result;
    buffer = calloc(SB_TEXT_LIMIT + 1, 1);
    if (!buffer) { free(loaded); return sb_error(SB_MEMORY, "Nicht genug Speicher für die Bearbeitung."); }
    memcpy(buffer, loaded, revision->length);
    free(loaded); *editor = buffer;
    return sb_ok();
}
static void title_for(SBApp *app) {
    sb_markdown_title(app->editor ? app->editor : "", app->title, sizeof(app->title));
    if (!app->title[0]) {
        const char *name = strrchr(app->path, '/');
        snprintf(app->title, sizeof(app->title), "%s", name ? name + 1 : app->path);
        while (app->title[0] && !sb_utf8_valid(app->title, strlen(app->title))) app->title[strlen(app->title) - 1] = 0;
    }
}
static SBStatus open_note(SBApp *app, const char *path) {
    SBRevision revision = {0};
    char next_path[SB_PATH_CAP];
    char *editor = NULL;
    SBStatus result;
    if (!app->has_project) return sb_error(SB_INVALID, "Wähle zuerst ein Projekt.");
    if (strlen(path) >= sizeof(next_path)) return sb_error(SB_LIMIT, "Dokumentpfad zu lang.");
    strcpy(next_path, path);
    result = load_text(&app->project, path, &editor, &revision);
    if (result.code != SB_OK) return result;
    free(app->editor); app->editor = editor; app->revision = revision;
    strcpy(app->path, next_path);
    title_for(app); sb_app_source_close(app); ++app->generation;
    return sb_ok();
}
static SBStatus set_project(SBApp *app, const SBProject *project) {
    SBNotes notes = {0};
    SBRevision revision = {0};
    char *editor = NULL;
    const char *path = NULL;
    SBStatus result = sb_notes_list(project, &notes);
    if (result.code != SB_OK) return result;
    for (size_t i = 0; i < notes.count; ++i)
        if (!strcmp(notes.items[i].path, "PROJECT.md")) { path = notes.items[i].path; break; }
    if (!path && notes.count) path = notes.items[0].path;
    if (path) {
        result = load_text(project, path, &editor, &revision);
        if (result.code != SB_OK) { sb_notes_free(&notes); return result; }
    }
    snprintf(app->path, sizeof(app->path), "%s", path ? path : "");
    sb_notes_free(&app->notes); app->notes = notes;
    app->project = *project; app->has_project = true;
    free(app->editor); app->editor = editor; app->revision = revision;
    title_for(app); sb_app_source_close(app); ++app->generation;
    return sb_ok();
}
static SBStatus open_workspace(SBApp *app, const char *workspace) {
    char absolute[SB_PATH_CAP];
    SBProjects projects = {0};
    SBStatus result;
    TRY(sb_fs_absolute(workspace, absolute, sizeof(absolute)));
    TRY(sb_projects_list(absolute, &projects));
    if (projects.count) {
        result = set_project(app, &projects.items[0]);
        if (result.code != SB_OK) { sb_projects_free(&projects); return result; }
    } else {
        sb_notes_free(&app->notes); free(app->editor); app->editor = NULL;
        app->has_project = false; app->path[0] = 0; app->title[0] = 0;
        memset(&app->revision, 0, sizeof(app->revision)); sb_app_source_close(app); ++app->generation;
    }
    sb_projects_free(&app->projects); app->projects = projects;
    strcpy(app->workspace, absolute);
    return sb_ok();
}
SBStatus sb_app_init(SBApp *app, const char *workspace) {
    memset(app, 0, sizeof(*app));
    return open_workspace(app, workspace);
}
SBStatus sb_app_save(SBApp *app) {
    SBRevision saved = {0};
    if (!app->editor || !app->has_project) return sb_error(SB_INVALID, "Kein Dokument zum Speichern.");
    TRY(sb_note_save(&app->project, app->path, app->editor, app->revision, &saved));
    app->revision = saved; title_for(app);
    for (size_t i = 0; i < app->notes.count; ++i)
        if (!strcmp(app->notes.items[i].path, app->path)) strcpy(app->notes.items[i].title, app->title);
    return sb_ok();
}
static SBStatus apply(SBApp *app, SBAction action) {
    if (action.kind == SB_ACT_QUIT) { app->quit = true; return sb_ok(); }
    if (action.kind == SB_ACT_WORKSPACE) return open_workspace(app, action.value);
    if (action.kind == SB_ACT_PROJECT) {
        for (size_t i = 0; i < app->projects.count; ++i)
            if (!strcmp(app->projects.items[i].id, action.value)) return set_project(app, &app->projects.items[i]);
        return sb_error(SB_NOT_FOUND, "Projekt wurde nicht gefunden.");
    }
    if (action.kind == SB_ACT_NOTE) return open_note(app, action.value);
    if (action.kind == SB_ACT_RELOAD) {
        if (!app->editor) return open_workspace(app, app->workspace);
        return open_note(app, app->path);
    }
    if (action.kind == SB_ACT_ARCHIVE) {
        char archived[SB_PATH_CAP];
        SBNotes notes = {0};
        SBStatus result;
        TRY(sb_note_archive(&app->project, app->path, app->revision, archived, sizeof(archived)));
        strcpy(app->path, archived);
        result = open_note(app, archived);
        if (result.code != SB_OK) return result;
        result = sb_notes_list(&app->project, &notes);
        if (result.code == SB_OK) { sb_notes_free(&app->notes); app->notes = notes; }
        sb_app_source_close(app); ++app->generation;
        return result;
    }
    return sb_ok();
}
SBStatus sb_app_request(SBApp *app, SBActionKind kind, const char *value) {
    SBAction action = {0};
    if (app->guard) return sb_error(SB_INVALID, "Entscheide zuerst über die ungespeicherten Änderungen.");
    if (value && strlen(value) >= sizeof(action.value)) return sb_error(SB_LIMIT, "Zielpfad zu lang.");
    action.kind = kind;
    if (value) strcpy(action.value, value);
    if (kind == SB_ACT_PROJECT && app->has_project && !strcmp(action.value, app->project.id)) return sb_ok();
    if (kind == SB_ACT_NOTE && app->editor && !strcmp(action.value, app->path)) {
        sb_app_source_close(app); return sb_ok();
    }
    if (sb_app_dirty(app) && kind != SB_ACT_NONE) {
        app->pending = action; app->guard = true; return sb_ok();
    }
    return apply(app, action);
}
SBStatus sb_app_decide(SBApp *app, SBDecision decision) {
    SBAction pending = app->pending;
    if (!app->guard) return sb_error(SB_INVALID, "Es gibt keinen ausstehenden Wechsel.");
    if (decision == SB_KEEP_EDITING) {
        app->guard = false; memset(&app->pending, 0, sizeof(app->pending)); return sb_ok();
    }
    if (decision == SB_SAVE_CHANGES) TRY(sb_app_save(app));
    app->guard = false; memset(&app->pending, 0, sizeof(app->pending));
    return apply(app, pending);
}
static SBStatus refresh_projects(SBApp *app) {
    SBProjects projects = {0};
    TRY(sb_projects_list(app->workspace, &projects));
    sb_projects_free(&app->projects); app->projects = projects;
    return sb_ok();
}
static SBStatus refresh_notes(SBApp *app) {
    SBNotes notes = {0};
    TRY(sb_notes_list(&app->project, &notes));
    sb_notes_free(&app->notes); app->notes = notes;
    return sb_ok();
}
SBStatus sb_app_new_project(SBApp *app, const char *id, const char *name, const char *repository) {
    SBProject created;
    if (!app->workspace[0]) return sb_error(SB_INVALID, "Öffne zuerst einen Arbeitsordner.");
    TRY(sb_project_create(app->workspace, id, name, repository, &created));
    TRY(refresh_projects(app));
    return sb_app_request(app, SB_ACT_PROJECT, created.id);
}
SBStatus sb_app_new_note(SBApp *app, const char *section, const char *id, const char *title) {
    SBNote created;
    if (!app->has_project) return sb_error(SB_INVALID, "Wähle zuerst ein Projekt.");
    TRY(sb_note_create(&app->project, section, id, title, &created));
    TRY(refresh_notes(app));
    return sb_app_request(app, SB_ACT_NOTE, created.path);
}
SBStatus sb_app_save_copy(SBApp *app) {
    SBNote note;
    SBRevision initial = {0};
    char id[65], *text = NULL;
    static unsigned serial = 0;
    SBStatus result;
    if (!app->editor || !app->has_project) return sb_error(SB_INVALID, "Kein Dokument für eine Kopie.");
    for (unsigned attempt = 0; attempt < 100; ++attempt) {
        snprintf(id, sizeof(id), "kopie-%lu-%u", (unsigned long)time(NULL), ++serial);
        result = sb_note_create(&app->project, "knowledge", id, app->title, &note);
        if (result.code == SB_CONFLICT || result.code == SB_EXISTS) continue;
        if (result.code != SB_OK) return result;
        result = sb_note_load(&app->project, note.path, &text, &initial);
        free(text);
        if (result.code == SB_OK) result = sb_note_save(&app->project, note.path, app->editor, initial, NULL);
        if (result.code != SB_OK) return result;
        TRY(refresh_notes(app));
        /* Preserve any pending target; the copy resolves the dirty source safely. */
        return open_note(app, note.path);
    }
    return sb_error(SB_EXISTS, "Es konnte keine eindeutige Kopie angelegt werden.");
}
static int hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
SBStatus sb_app_source(SBApp *app, const char *link) {
    char decoded[SB_PATH_CAP], joined[SB_PATH_CAP], absolute[SB_PATH_CAP], base[SB_PATH_CAP];
    char *text = NULL, *slash;
    const char *input = link;
    size_t count = 0, length = 0;
    SBStatus result;
    if (!app->has_project) return sb_error(SB_INVALID, "Wähle zuerst ein Projekt.");
    if (!strncmp(input, "file://", 7)) {
        input += 7;
        if (*input != '/') return sb_error(SB_INVALID, "Nur lokale Datei-URLs werden in der Anwendung geöffnet.");
#ifdef _WIN32
        if (input[0] == '/' && input[1] && input[2] == ':') ++input;
#endif
    } else if (strstr(input, "://") || !strncmp(input, "mailto:", 7))
        return sb_error(SB_INVALID, "Diese Quelle ist keine lokale Textdatei.");
    while (*input && *input != '#') {
        if (count + 1 >= sizeof(decoded)) return sb_error(SB_LIMIT, "Quellenpfad zu lang.");
        if (*input == '%') {
            int high, low;
            if (!input[1] || !input[2] || (high = hex_digit(input[1])) < 0 ||
                (low = hex_digit(input[2])) < 0 || (!high && !low))
                return sb_error(SB_INVALID, "Ungültige Pfadkodierung.");
            decoded[count++] = (char)((high << 4) | low); input += 3;
        } else decoded[count++] = *input++;
    }
    decoded[count] = 0;
    if (!count || !sb_utf8_valid(decoded, count)) return sb_error(SB_INVALID, "Quellenpfad fehlt oder ist ungültig.");
    if (decoded[0] == '/' || (count >= 3 && decoded[1] == ':')) strcpy(joined, decoded);
    else {
        if (app->source) strcpy(base, app->source_path);
        else TRY(sb_path_join(base, sizeof(base), app->project.root, app->path));
        slash = strrchr(base, '/');
        if (!slash) return sb_error(SB_INVALID, "Quellenbasis ist ungültig.");
        *slash = 0;
        TRY(sb_path_join(joined, sizeof(joined), base, decoded));
    }
    TRY(sb_fs_absolute(joined, absolute, sizeof(absolute)));
    result = sb_fs_read(absolute, &text, &length);
    if (result.code != SB_OK) return result;
    if (!sb_utf8_valid(text, length)) { free(text); return sb_error(SB_INVALID, "Die Quelle ist keine UTF-8-Textdatei."); }
    free(app->source); app->source = text;
    strcpy(app->source_path, absolute);
    sb_markdown_title(text, app->source_title, sizeof(app->source_title));
    if (!app->source_title[0]) {
        const char *name = strrchr(absolute, '/');
        snprintf(app->source_title, sizeof(app->source_title), "%s", name ? name + 1 : absolute);
        while (app->source_title[0] && !sb_utf8_valid(app->source_title, strlen(app->source_title)))
            app->source_title[strlen(app->source_title) - 1] = 0;
    }
    return sb_ok();
}
void sb_app_slug(const char *title, char *out, size_t capacity) {
    size_t n = 0;
    const unsigned char *p = (const unsigned char *)title;
    if (!capacity) return;
    while (*p && n + 1 < capacity && n < 64) {
        unsigned char c = *p++;
        if (c >= 'A' && c <= 'Z') c = (unsigned char)(c + 32);
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) out[n++] = (char)c;
        else if (c == 0xc3 && *p) {
            unsigned char next = *p++;
            const char *replacement = next == 0xbc || next == 0x9c ? "ue" :
                next == 0xa4 || next == 0x84 ? "ae" : next == 0xb6 || next == 0x96 ? "oe" :
                next == 0x9f ? "ss" : NULL;
            if (replacement && n + 2 < capacity && n + 2 <= 64) {
                out[n++] = replacement[0]; out[n++] = replacement[1];
            } else if (n && out[n - 1] != '-') out[n++] = '-';
        } else if ((c & 0xc0) != 0x80 && n && out[n - 1] != '-') out[n++] = '-';
    }
    while (n && out[n - 1] == '-') --n;
    out[n] = 0;
    if (!sb_id_valid(out)) snprintf(out, capacity, "projekt");
}
