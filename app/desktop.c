#include "desktop.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static const char *sections[] = {"all", "overview", "knowledge", "inbox", "journal", "archive"};
static const char *section_names[] = {"Alle Dokumente", "Orientierung", "Wissen", "Eingang", "Übergaben", "Archiv"};
static const char *new_sections[] = {"knowledge", "inbox", "journal"};
static const char *new_names[] = {"Wissen", "Eingang", "Übergaben"};

static void target(SBDesktop *d, const char *id) {
    if (d->target_count < sizeof(d->targets) / sizeof(d->targets[0])) {
        SBTarget *item = &d->targets[d->target_count++];
        snprintf(item->id, sizeof(item->id), "%s", id);
        item->bounds = nk_widget_bounds(d->ui.ctx);
    }
}
static bool button(SBDesktop *d, const char *id, const char *label) {
    target(d, id);
    return nk_button_label(d->ui.ctx, label) != 0;
}
static void command(SBDesktop *d, SBCommand cmd) { if (d->command == SB_CMD_NONE) d->command = cmd; }
static void result(SBDesktop *d, SBStatus status, const char *success) {
    d->message = status;
    if (status.code == SB_OK && success) snprintf(d->message.message, sizeof(d->message.message), "%s", success);
}
static void field(SBDesktop *d, const char *tag, const char *label, char *text, int capacity, int focus) {
    struct nk_context *ctx = d->ui.ctx;
    int length = (int)strlen(text);
    nk_layout_row_dynamic(ctx, 22 * d->ui.scale, 1);
    nk_label(ctx, label, NK_TEXT_LEFT);
    nk_layout_row_dynamic(ctx, 36 * d->ui.scale, 1);
    target(d, tag);
    if (d->form_focus == focus) { nk_edit_focus(ctx, 0); d->form_focus = 0; }
    nk_flags state = nk_edit_string(ctx, NK_EDIT_FIELD, text, &length, capacity - 1, nk_filter_default);
    if (state & NK_EDIT_ACTIVE) d->active_form_field = focus;
    text[length] = 0;
}
static void request(SBDesktop *d, SBActionKind kind, const char *value) {
    if (d->navigation.kind != SB_ACT_NONE) return;
    if (value && strlen(value) >= sizeof(d->navigation.value)) {
        d->message = sb_error(SB_LIMIT, "Zielpfad zu lang."); return;
    }
    d->navigation.kind = kind;
    if (value) strcpy(d->navigation.value, value);
    else d->navigation.value[0] = 0;
}
SBStatus sb_desktop_init(SBDesktop *d, const char *workspace, const char *font, bool testing) {
    SBStatus status;
    memset(d, 0, sizeof(*d));
    d->sidebar = true; d->test = testing; strcpy(d->section, "all");
    status = sb_ui_init(&d->ui, font, 1336, 840, testing);
    if (status.code != SB_OK) return status;
    status = sb_app_init(&d->model, workspace);
    if (status.code != SB_OK) {
        sb_fs_absolute(workspace, d->model.workspace, sizeof(d->model.workspace));
        d->message = status;
    }
    d->generation = d->model.generation;
    return sb_ok();
}
void sb_desktop_free(SBDesktop *d) {
    sb_notes_free(&d->hits); free(d->context); sb_app_free(&d->model);
    sb_ui_shutdown(&d->ui); memset(d, 0, sizeof(*d));
}
static void search_refresh(SBDesktop *d) {
    if (!strcmp(d->search, d->searched)) return;
    SBNotes hits = {0};
    if (d->model.has_project && d->search[0]) {
        SBStatus status = sb_search(&d->model.project, d->search, &hits);
        if (status.code != SB_OK) { d->message = status; return; }
    }
    sb_notes_free(&d->hits); d->hits = hits;
    strcpy(d->searched, d->search);
}
static void synchronize(SBDesktop *d) {
    if (d->generation != d->model.generation) {
        d->generation = d->model.generation;
        d->editing = d->next_edit; d->next_edit = false;
        d->focus_editor = d->editing;
        d->reset_reader = true;
        sb_ui_reset_editor(&d->ui);
        d->search[0] = 0; d->searched[0] = 0; sb_notes_free(&d->hits);
        if (!strncmp(d->model.path, "archive/", 8)) strcpy(d->section, "archive");
    }
}
void sb_desktop_apply(SBDesktop *d) {
    SBCommand cmd = d->command;
    SBStatus status = sb_ok();
    d->command = SB_CMD_NONE;
    if (cmd == SB_CMD_NEW_PROJECT || cmd == SB_CMD_NEW_NOTE) {
        d->form = cmd == SB_CMD_NEW_PROJECT ? SB_FORM_PROJECT : SB_FORM_NOTE;
        d->name[0] = 0; d->id[0] = 0; d->repository[0] = 0; d->id_manual = false;
        d->note_section = 0; d->form_focus = 1; d->active_form_field = 1;
    } else if (cmd == SB_CMD_WORKSPACE) {
        d->form = SB_FORM_WORKSPACE;
        strcpy(d->folder, d->model.workspace); d->form_focus = 1;
    } else if (cmd == SB_CMD_SUBMIT) {
        if (d->form == SB_FORM_PROJECT) {
            status = sb_app_new_project(&d->model, d->id, d->name, d->repository);
            if (status.code == SB_OK) strcpy(d->section, "all");
            result(d, status, "Projekt angelegt.");
        } else if (d->form == SB_FORM_NOTE) {
            status = sb_app_new_note(&d->model, new_sections[d->note_section], d->id, d->name);
            if (status.code == SB_OK) { d->next_edit = true; strcpy(d->section, new_sections[d->note_section]); }
            result(d, status, "Notiz angelegt.");
        } else if (d->form == SB_FORM_WORKSPACE) {
            status = sb_app_request(&d->model, SB_ACT_WORKSPACE, d->folder);
            result(d, status, "Arbeitsordner geöffnet.");
        }
        if (status.code == SB_OK) d->form = SB_FORM_NONE;
    } else if (cmd == SB_CMD_CANCEL) d->form = SB_FORM_NONE;
    else if (cmd == SB_CMD_SAVE) {
        status = sb_app_save(&d->model); result(d, status, "Gespeichert.");
        d->searched[0] = 0;
    } else if (cmd == SB_CMD_COPY) {
        status = sb_app_save_copy(&d->model); result(d, status, "Eigene Fassung als neue Wissensnotiz gespeichert.");
    } else if (cmd == SB_CMD_CONTEXT) {
        char *text = NULL;
        status = sb_context_build(&d->model.project, &text);
        if (status.code == SB_OK) {
            free(d->context); d->context = text; d->form = SB_FORM_CONTEXT;
            result(d, status, sb_app_dirty(&d->model) ? "Kontext verwendet gespeicherte Dateien. Aktuelle Änderungen sind noch ungespeichert." : "Kontext aus den gespeicherten Kerninformationen.");
        } else result(d, status, NULL);
    } else if (cmd == SB_CMD_GUARD_SAVE || cmd == SB_CMD_GUARD_DISCARD || cmd == SB_CMD_GUARD_CANCEL) {
        status = sb_app_decide(&d->model, cmd == SB_CMD_GUARD_SAVE ? SB_SAVE_CHANGES :
                              cmd == SB_CMD_GUARD_DISCARD ? SB_DISCARD_CHANGES : SB_KEEP_EDITING);
        result(d, status, NULL);
    } else if (cmd == SB_CMD_ARCHIVE || cmd == SB_CMD_RELOAD) {
        status = sb_app_request(&d->model, cmd == SB_CMD_ARCHIVE ? SB_ACT_ARCHIVE : SB_ACT_RELOAD, NULL);
        result(d, status, cmd == SB_CMD_ARCHIVE ? "Dokument archiviert." : "Dokument neu geladen.");
    } else if (cmd == SB_CMD_THEME) sb_ui_theme(&d->ui, !d->ui.dark);
    else if (cmd == SB_CMD_SCALE) result(d, sb_ui_fonts(&d->ui, d->next_scale), NULL);
    else if (cmd == SB_CMD_SOURCE) {
        result(d, sb_app_source(&d->model, d->command_value), NULL);
        if (d->message.code == SB_OK) d->reset_reader = true;
    }
    if (d->navigation.kind != SB_ACT_NONE) {
        SBAction action = d->navigation;
        memset(&d->navigation, 0, sizeof(d->navigation));
        result(d, sb_app_request(&d->model, action.kind, action.value), NULL);
        if (action.kind == SB_ACT_PROJECT && d->form == SB_FORM_SETTINGS && d->message.code == SB_OK)
            d->form = SB_FORM_NONE;
    }
    synchronize(d);
}
void sb_desktop_event(SBDesktop *d, const SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
        request(d, SB_ACT_QUIT, NULL); return;
    }
    if (event->type == SDL_EVENT_KEY_DOWN && !event->key.repeat) {
        SDL_Keycode key = event->key.key;
#ifdef __APPLE__
        bool modifier = (event->key.mod & SDL_KMOD_GUI) != 0;
#else
        bool modifier = (event->key.mod & SDL_KMOD_CTRL) != 0;
#endif
        bool shift = (event->key.mod & SDL_KMOD_SHIFT) != 0;
        if (key == SDLK_ESCAPE) {
            if (d->model.guard) command(d, SB_CMD_GUARD_CANCEL);
            else if (d->form != SB_FORM_NONE) command(d, SB_CMD_CANCEL);
            else if (d->model.source) { sb_app_source_close(&d->model); d->reset_reader = true; }
            else { d->search[0] = 0; d->focus_search = false; }
            return;
        }
        if (d->model.guard) {
            if (modifier && key == SDLK_S) command(d, SB_CMD_GUARD_SAVE);
            return;
        }
        if (d->form == SB_FORM_PROJECT || d->form == SB_FORM_NOTE || d->form == SB_FORM_WORKSPACE) {
            if (key == SDLK_TAB) {
                int max = d->form == SB_FORM_WORKSPACE ? 1 : d->form == SB_FORM_NOTE ? 2 : 3;
                int next = d->active_form_field + (shift ? -1 : 1);
                d->form_focus = next > max ? 1 : next < 1 ? max : next;
                return;
            }
            if (key == SDLK_RETURN) { command(d, SB_CMD_SUBMIT); return; }
        } else if (d->form == SB_FORM_NONE && modifier) {
            if (key == SDLK_S) { command(d, SB_CMD_SAVE); return; }
            if (key == SDLK_N) { command(d, shift ? SB_CMD_NEW_PROJECT : SB_CMD_NEW_NOTE); return; }
            if (key == SDLK_O) { command(d, SB_CMD_WORKSPACE); return; }
            if (key == SDLK_F) { d->focus_search = true; return; }
            if (key == SDLK_E && d->model.editor && !d->model.source) { d->editing = !d->editing; d->focus_editor = d->editing; return; }
            if (key == SDLK_R) { command(d, SB_CMD_RELOAD); return; }
            if (key == SDLK_C && shift) { command(d, SB_CMD_CONTEXT); return; }
        }
    }
    sb_ui_event(&d->ui, event);
}

static void muted(SBDesktop *d, const char *text) {
    nk_label_colored(d->ui.ctx, text, NK_TEXT_LEFT,
                     d->ui.dark ? nk_rgb(164, 169, 181) : nk_rgb(99, 108, 123));
}
static void compact_label(SBDesktop *d, const char *text, char *out, size_t capacity, float width) {
    struct nk_user_font *font = &d->ui.normal->handle;
    snprintf(out, capacity, "%s", text);
    while (out[0] && !sb_utf8_valid(out, strlen(out))) out[strlen(out) - 1] = 0;
    if (font->width(font->userdata, font->height, out, (int)strlen(out)) <= width) return;
    size_t length = strlen(out);
    while (length) {
        --length;
        while (length && ((unsigned char)out[length] & 0xc0) == 0x80) --length;
        out[length] = 0;
        if (length + 4 <= capacity && font->width(font->userdata, font->height, out, (int)length) +
            font->width(font->userdata, font->height, "…", 3) <= width) { strcat(out, "…"); return; }
    }
}
static void project_rows(SBDesktop *d) {
    struct nk_context *ctx = d->ui.ctx;
    float scale = d->ui.scale;
    for (size_t i = 0; i < d->model.projects.count; ++i) {
        char tag[100];
        nk_bool selected = d->model.has_project && !strcmp(d->model.project.id, d->model.projects.items[i].id);
        nk_layout_row_dynamic(ctx, 36 * scale, 1);
        snprintf(tag, sizeof(tag), "project:%s", d->model.projects.items[i].id); target(d, tag);
        if (nk_selectable_label(ctx, d->model.projects.items[i].name, NK_TEXT_LEFT, &selected))
            request(d, SB_ACT_PROJECT, d->model.projects.items[i].id);
    }
}
static void sidebar(SBDesktop *d, float width, float height, nk_flags flags) {
    struct nk_context *ctx = d->ui.ctx;
    float s = d->ui.scale;
    struct nk_style_item old = ctx->style.window.fixed_background;
    ctx->style.window.fixed_background = nk_style_item_color(d->ui.dark ? nk_rgb(37, 38, 43) : nk_rgb(243, 244, 247));
    struct nk_style_item old_select = ctx->style.selectable.normal;
    ctx->style.selectable.normal = ctx->style.window.fixed_background;
    if (nk_begin(ctx, "Projects", nk_rect(0, 0, width, height), flags | NK_WINDOW_NO_SCROLLBAR)) {
        nk_style_set_font(ctx, &d->ui.body->handle);
        nk_layout_row_dynamic(ctx, 34 * s, 1); nk_label(ctx, "SecondBrain", NK_TEXT_LEFT);
        nk_style_set_font(ctx, &d->ui.normal->handle);
        nk_layout_row_dynamic(ctx, 22 * s, 1); muted(d, "PROJEKTE");
        nk_layout_row_dynamic(ctx, fminf(height * 0.25f, 180 * s), 1);
        if (nk_group_begin(ctx, "Project list", 0)) { project_rows(d); nk_group_end(ctx); }
        nk_layout_row_dynamic(ctx, 32 * s, 1);
        if (button(d, "new-project", "+ Neues Projekt")) command(d, SB_CMD_NEW_PROJECT);
        nk_layout_row_dynamic(ctx, 24 * s, 1); nk_spacer(ctx);
        nk_layout_row_dynamic(ctx, 22 * s, 1); muted(d, "WISSENSBEREICHE");
        for (size_t i = 0; i < sizeof(sections) / sizeof(*sections); ++i) {
            char tag[100];
            nk_bool selected = !strcmp(d->section, sections[i]);
            nk_layout_row_dynamic(ctx, 34 * s, 1);
            snprintf(tag, sizeof(tag), "section:%s", sections[i]); target(d, tag);
            if (nk_selectable_label(ctx, section_names[i], NK_TEXT_LEFT, &selected))
                strcpy(d->section, sections[i]);
        }
        nk_layout_row_dynamic(ctx, 24 * s, 1); nk_spacer(ctx);
        nk_layout_row_dynamic(ctx, 32 * s, 1);
        if (button(d, "workspace", "Arbeitsordner öffnen")) command(d, SB_CMD_WORKSPACE);
        nk_layout_row_dynamic(ctx, 32 * s, 1);
        if (button(d, "settings", "Darstellung")) d->form = SB_FORM_SETTINGS;
    }
    nk_end(ctx); ctx->style.window.fixed_background = old; ctx->style.selectable.normal = old_select;
}
static void document_list(SBDesktop *d, float x, float width, float height, nk_flags flags, bool compact) {
    struct nk_context *ctx = d->ui.ctx;
    float s = d->ui.scale;
    const SBNotes *notes = d->search[0] ? &d->hits : &d->model.notes;
    if (nk_begin(ctx, "Documents", nk_rect(x, 0, width, height), flags | (s <= 1.25f ? NK_WINDOW_NO_SCROLLBAR : 0))) {
        if (compact) {
            nk_layout_row_dynamic(ctx, 34 * s, 1);
            if (button(d, "project-picker", "Projekte")) d->form = SB_FORM_SETTINGS;
        }
        nk_layout_row_dynamic(ctx, 32 * s, 1);
        nk_label(ctx, d->model.has_project ? d->model.project.name : "Deine Projekte", NK_TEXT_LEFT);
        nk_layout_row_begin(ctx, NK_DYNAMIC, 34 * s, 2);
        nk_layout_row_push(ctx, 0.82f); target(d, "search");
        if (d->focus_search) { nk_edit_focus(ctx, NK_EDIT_AUTO_SELECT); d->focus_search = false; }
        int length = (int)strlen(d->search);
        nk_edit_string(ctx, NK_EDIT_FIELD, d->search, &length, (int)sizeof(d->search) - 1, nk_filter_default);
        d->search[length] = 0;
        nk_layout_row_push(ctx, 0.18f);
        if (button(d, "clear-search", "×")) d->search[0] = 0;
        nk_layout_row_end(ctx);
        nk_layout_row_dynamic(ctx, 20 * s, 1);
        muted(d, d->search[0] ? "Treffer in Titel und Inhalt" : "Suche im aktuellen Projekt");
        if (compact) {
            int selected = 0;
            for (int i = 0; i < 6; ++i) if (!strcmp(sections[i], d->section)) selected = i;
            nk_layout_row_dynamic(ctx, 32 * s, 1);
            selected = nk_combo(ctx, section_names, 6, selected, (int)(28 * s), nk_vec2(width, 240 * s));
            strcpy(d->section, sections[selected]);
        }
        float list_height = height - (compact ? 270 : 190) * s;
        if (list_height < 60) list_height = 60;
        nk_layout_row_dynamic(ctx, list_height, 1);
        if (nk_group_begin(ctx, "Note list", 0)) {
            size_t shown = 0;
            for (size_t i = 0; i < notes->count; ++i) {
                if (strcmp(d->section, "all") && strcmp(d->section, notes->items[i].section)) continue;
                char tag[100], title[SB_NAME_CAP], path[SB_NAME_CAP];
                nk_bool selected = !strcmp(d->model.path, notes->items[i].path);
                nk_layout_row_dynamic(ctx, 36 * s, 1);
                snprintf(tag, sizeof(tag), "note:%s", notes->items[i].path); target(d, tag);
                compact_label(d, notes->items[i].title, title, sizeof(title), width - 72);
                if (nk_selectable_label(ctx, title, NK_TEXT_LEFT, &selected))
                    request(d, SB_ACT_NOTE, notes->items[i].path);
                if (nk_widget_is_hovered(ctx)) nk_tooltip(ctx, notes->items[i].title);
                compact_label(d, notes->items[i].path, path, sizeof(path), width - 64);
                nk_layout_row_dynamic(ctx, 18 * s, 1); muted(d, path);
                if (nk_widget_is_hovered(ctx)) nk_tooltip(ctx, notes->items[i].path);
                ++shown;
            }
            if (!shown) {
                nk_layout_row_dynamic(ctx, 72 * s, 1);
                nk_label_wrap(ctx, d->search[0] ? "Keine Treffer. Ändere den Suchtext oder den Bereich." :
                              "Hier sind noch keine Dokumente. Lege eine neue Notiz an.");
            }
            nk_group_end(ctx);
        }
        nk_layout_row_dynamic(ctx, 34 * s, 1);
        if (button(d, "new-note", "+ Neue Notiz")) command(d, SB_CMD_NEW_NOTE);
        if (compact) {
            nk_layout_row_dynamic(ctx, 32 * s, 1);
            if (button(d, "new-project", "+ Neues Projekt")) command(d, SB_CMD_NEW_PROJECT);
        }
    }
    nk_end(ctx);
}

static char *plain_inline(const char *text, size_t length) {
    char *out = malloc(length + 1);
    size_t n = 0, i = 0;
    if (!out) return NULL;
    while (i < length) {
        if (text[i] == '[') {
            size_t close = i + 1;
            while (close < length && text[close] != ']') ++close;
            if (close + 1 < length && text[close + 1] == '(') {
                size_t end = close + 2;
                int nesting = 1;
                while (end < length && nesting) {
                    if (text[end] == '(') ++nesting;
                    if (text[end] == ')') --nesting;
                    ++end;
                }
                if (!nesting) {
                    memcpy(out + n, text + i + 1, close - i - 1);
                    n += close - i - 1; i = end; continue;
                }
            }
        }
        if (text[i] == '*' && i + 1 < length && text[i + 1] == '*') { i += 2; continue; }
        if (text[i] == 96) { ++i; continue; }
        if (text[i] == '\r') { ++i; continue; }
        out[n++] = text[i] == '\n' ? ' ' : text[i];
        ++i;
    }
    out[n] = 0;
    return out;
}

static void document(SBDesktop *d, const char *text, float width, float height) {
    struct nk_context *ctx = d->ui.ctx;
    const char *line = text;
    const char *extension = d->model.source ? strrchr(d->model.source_path, '.') : NULL;
    bool whole_code = text == d->model.source && !d->model.source_directory && (!extension || strcmp(extension, ".md"));
    bool code = whole_code, first_heading = true;
    unsigned link_number = 0;
    nk_layout_row_dynamic(ctx, height, 1);
    if (!nk_group_begin(ctx, "Reader", 0)) return;
    while (*line) {
        const char *end = strchr(line, '\n');
        size_t length = end ? (size_t)(end - line) : strlen(line);
        if (length && line[length - 1] == '\r') --length;
        const char *content = line;
        unsigned heading = 0;
        if (!whole_code && length >= 3 && !strncmp(line, "```", 3)) code = !code;
        else if (!length) { nk_layout_row_dynamic(ctx, 9 * d->ui.scale, 1); nk_spacer(ctx); }
        else {
            if (!code) {
                while (heading < length && line[heading] == '#') ++heading;
                if (heading && heading < length && line[heading] == ' ') {
                    content += heading + 1; length -= heading + 1;
                    if (first_heading && heading == 1) { first_heading = false; goto next_line; }
                } else heading = 0;
            }
            first_heading = false;
            if (!code && !heading && length && content[0] != '-' && content[0] != '*' &&
                content[0] != '>' && content[0] != '|' && !(content[0] >= '0' && content[0] <= '9')) {
                while (end && end[1]) {
                    const char *next = end + 1, *next_end = strchr(next, '\n');
                    size_t next_length = next_end ? (size_t)(next_end - next) : strlen(next);
                    if (!next_length || *next == '\r' || *next == '#' || *next == '-' || *next == '*' ||
                        *next == '>' || *next == '|' || *next == 96 || (*next >= '0' && *next <= '9')) break;
                    end = next_end;
                    length = next_end ? (size_t)(next_end - content) : strlen(content);
                }
            }
            struct nk_user_font *font = heading ? &d->ui.heading->handle : code ? &d->ui.code->handle : &d->ui.body->handle;
            nk_style_set_font(ctx, font);
            char *plain = code ? NULL : plain_inline(content, length);
            const char *shown = plain ? plain : content;
            size_t shown_length = plain ? strlen(plain) : length;
            float available = fmaxf(60, width - 55);
            float measured = font->width(font->userdata, font->height, shown, (int)shown_length);
            float lines = measured < available * 0.92f ? 1 : ceilf(measured / available) + 1;
            nk_layout_row_dynamic(ctx, lines * (font->height + 4) + (heading ? 10 : 0), 1);
            nk_text_wrap(ctx, shown, (int)shown_length);
            free(plain);
            if (!code) {
                const char *p = content;
                const char *limit = content + length;
                while (p < limit) {
                    const char *open = memchr(p, '[', (size_t)(limit - p));
                    if (!open) break;
                    const char *close = memchr(open + 1, ']', (size_t)(limit - open - 1));
                    if (!close || close + 1 >= limit || close[1] != '(') { p = open + 1; continue; }
                    const char *finish = close + 2;
                    int nesting = 1;
                    while (finish < limit && nesting) {
                        if (*finish == '(') ++nesting;
                        if (*finish == ')') --nesting;
                        if (nesting) ++finish;
                    }
                    if (finish >= limit) break;
                    size_t target_length = (size_t)(finish - (close + 2));
                    if (target_length && target_length < SB_PATH_CAP && (open == content || open[-1] != '!')) {
                        char destination[SB_PATH_CAP], label[SB_NAME_CAP], tag[100];
                        size_t label_length = (size_t)(close - open - 1);
                        if (label_length >= sizeof(label)) label_length = sizeof(label) - 1;
                        memcpy(label, open + 1, label_length); label[label_length] = 0;
                        while (label[0] && !sb_utf8_valid(label, strlen(label))) label[strlen(label) - 1] = 0;
                        memcpy(destination, close + 2, target_length); destination[target_length] = 0;
                        if (destination[0] == '<' && target_length > 1 && destination[target_length - 1] == '>') {
                            memmove(destination, destination + 1, target_length - 2); destination[target_length - 2] = 0;
                        }
                        nk_style_set_font(ctx, &d->ui.normal->handle);
                        nk_layout_row_dynamic(ctx, 32 * d->ui.scale, 1);
                        snprintf(tag, sizeof(tag), "link:%u", link_number++);
                        if (button(d, tag, label)) {
                            if (!strncmp(destination, "https://", 8) || !strncmp(destination, "http://", 7))
                                result(d, SDL_OpenURL(destination) ? sb_ok() : sb_error(SB_IO, "%s", SDL_GetError()), "Webquelle im Browser geöffnet.");
                            else {
                                strcpy(d->command_value, destination); command(d, SB_CMD_SOURCE);
                            }
                        }
                    }
                    p = finish + 1;
                }
            }
        }
next_line:
        line = end ? end + 1 : line + strlen(line);
    }
    nk_style_set_font(ctx, &d->ui.normal->handle);
    nk_group_end(ctx);
}

static void actions(SBDesktop *d, float width) {
    struct nk_context *ctx = d->ui.ctx;
    if (nk_combo_begin_label(ctx, "Aktionen", nk_vec2(fminf(width, 340 * d->ui.scale), 280 * d->ui.scale))) {
        nk_layout_row_dynamic(ctx, 32 * d->ui.scale, 1);
        if (button(d, "context", "KI-Kontext")) { command(d, SB_CMD_CONTEXT); nk_combo_close(ctx); }
        if (button(d, "reload", "Neu laden")) { command(d, SB_CMD_RELOAD); nk_combo_close(ctx); }
        if (d->model.editor && strchr(d->model.path, '/') && strncmp(d->model.path, "archive/", 8))
            if (button(d, "archive", "Archivieren")) { command(d, SB_CMD_ARCHIVE); nk_combo_close(ctx); }
        if (button(d, "save-copy", "Als neue Notiz speichern")) { command(d, SB_CMD_COPY); nk_combo_close(ctx); }
        if (button(d, "settings", "Darstellung")) { d->form = SB_FORM_SETTINGS; nk_combo_close(ctx); }
        if (button(d, "help", "Tastaturhilfe")) { d->form = SB_FORM_HELP; nk_combo_close(ctx); }
        nk_combo_end(ctx);
    }
}
static void detail(SBDesktop *d, float x, float width, float height, nk_flags flags) {
    struct nk_context *ctx = d->ui.ctx;
    float s = d->ui.scale;
    if (nk_begin(ctx, "Detail", nk_rect(x, 0, width, height), flags | (s <= 1.25f ? NK_WINDOW_NO_SCROLLBAR : 0))) {
        if (d->reset_reader) { nk_group_set_scroll(ctx, "Reader", 0, 0); d->reset_reader = false; }
        nk_style_set_font(ctx, &d->ui.heading->handle);
        const char *title = d->model.source ? d->model.source_title : d->model.editor ? d->model.title : "Projektwissen";
        float available = fmaxf(60, width - 44);
        struct nk_user_font *heading = &d->ui.heading->handle;
        float measured = heading->width(heading->userdata, heading->height, title, (int)strlen(title));
        float title_height = measured <= available ? 42 * s : (ceilf(measured / available) + 1) * (heading->height + 4);
        nk_layout_row_dynamic(ctx, title_height, 1);
        nk_label_wrap(ctx, title);
        nk_style_set_font(ctx, &d->ui.normal->handle);
        nk_layout_row_dynamic(ctx, 24 * s, 1);
        muted(d, d->model.source ? "Schreibgeschützte Quelle" :
                 d->model.editor ? sb_app_dirty(&d->model) ? "Ungespeicherte Änderungen" : "Gespeichert" :
                 "Lege ein Projekt an oder öffne einen Arbeitsordner.");
        if (d->model.source) {
            nk_layout_row_dynamic(ctx, 34 * s, 1);
            if (button(d, "source-back", "Zurück zum Dokument")) { sb_app_source_close(&d->model); d->reset_reader = true; }
        } else if (d->model.editor) {
            nk_layout_row_dynamic(ctx, 34 * s, s > 1.25f ? 2 : 4);
            if (button(d, "read", "Lesen")) d->editing = false;
            if (button(d, "edit", "Bearbeiten")) { d->editing = true; d->focus_editor = true; }
            if (s > 1.25f) nk_layout_row_dynamic(ctx, 34 * s, 2);
            if (button(d, "save", "Speichern")) command(d, SB_CMD_SAVE);
            target(d, "actions"); actions(d, width);
        }
        float tools = d->model.source ? 34 : s > 1.25f ? 68 : 34;
        float body_height = fmaxf(80, height - 28 - title_height - (24 + tools + 60) * s - 40);
        if (d->model.source) document(d, d->model.source, width, body_height);
        else if (d->model.editor && d->editing) {
            nk_style_set_font(ctx, &d->ui.body->handle);
            nk_layout_row_dynamic(ctx, body_height, 1); target(d, "editor");
            if (d->focus_editor) { nk_edit_focus(ctx, 0); d->focus_editor = false; }
            int length = (int)strlen(d->model.editor);
            nk_edit_string(ctx, NK_EDIT_BOX, d->model.editor, &length, SB_TEXT_LIMIT + 1, nk_filter_default);
            d->model.editor[length] = 0;
            nk_style_set_font(ctx, &d->ui.normal->handle);
        } else if (d->model.editor) document(d, d->model.editor, width, body_height);
        else {
            nk_layout_row_dynamic(ctx, 110 * s, 1);
            nk_label_wrap(ctx, "Ziele, Entscheidungen, Quellen und Erkenntnisse bleiben bei deinem Projekt. SecondBrain zeigt und bearbeitet sie hier.");
            nk_layout_row_dynamic(ctx, 38 * s, 1);
            if (button(d, "new-project-detail", "Ein Projektgedächtnis anlegen")) command(d, SB_CMD_NEW_PROJECT);
            nk_layout_row_dynamic(ctx, 38 * s, 1);
            if (button(d, "workspace-detail", "Vorhandenen Arbeitsordner öffnen")) command(d, SB_CMD_WORKSPACE);
        }
        nk_layout_row_dynamic(ctx, 60 * s, 1);
        const char *feedback = d->message.message[0] ? d->message.message :
                               d->model.source ? d->model.source_path : d->model.path;
        if (d->message.code == SB_OK && sb_app_dirty(&d->model)) feedback = "Änderungen noch nicht gespeichert.";
        nk_text_wrap_colored(ctx, feedback, (int)strlen(feedback),
            d->message.code == SB_OK ? d->ui.dark ? nk_rgb(164, 169, 181) : nk_rgb(99, 108, 123) :
                                      d->ui.dark ? nk_rgb(255, 153, 153) : nk_rgb(155, 31, 41));
    }
    nk_end(ctx);
}
static void popup(SBDesktop *d, int width, int height) {
    struct nk_context *ctx = d->ui.ctx;
    float s = d->ui.scale, w = fminf(width - 40.0f, 580 * s), h = fminf(height - 40.0f, 680 * s);
    if (d->model.guard) {
        h = fminf(height - 40.0f, 430 * s);
        if (nk_begin(ctx, "Änderungen erhalten", nk_rect((width - w) / 2, (height - h) / 2, w, h),
                     NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
            nk_layout_row_dynamic(ctx, 76 * s, 1);
            nk_label_wrap(ctx, "Dieses Dokument enthält ungespeicherte Änderungen. Speichere sie vor dem Wechsel oder behalte die Bearbeitung bei.");
            nk_layout_row_dynamic(ctx, 36 * s, 1);
            if (button(d, "guard-save", "Speichern und weiter")) command(d, SB_CMD_GUARD_SAVE);
            if (button(d, "guard-discard", "Änderungen verwerfen")) command(d, SB_CMD_GUARD_DISCARD);
            if (button(d, "guard-cancel", "Weiter bearbeiten")) command(d, SB_CMD_GUARD_CANCEL);
            if (d->message.code == SB_CONFLICT)
                if (button(d, "guard-copy", "Eigene Fassung als neue Notiz sichern")) command(d, SB_CMD_COPY);
            nk_layout_row_dynamic(ctx, 60 * s, 1); nk_label_wrap(ctx, d->message.message);
        }
        nk_end(ctx); return;
    }
    if (d->form == SB_FORM_NONE) return;
    const char *title = d->form == SB_FORM_PROJECT ? "Neues Projekt" : d->form == SB_FORM_NOTE ? "Neue Notiz" :
        d->form == SB_FORM_WORKSPACE ? "Arbeitsordner öffnen" : d->form == SB_FORM_SETTINGS ? "Projekte und Darstellung" :
        d->form == SB_FORM_CONTEXT ? "KI-Kontext" : "Tastaturhilfe";
    if (nk_begin(ctx, title, nk_rect((width - w) / 2, (height - h) / 2, w, h),
                 NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
        if (d->form == SB_FORM_PROJECT || d->form == SB_FORM_NOTE) {
            char previous_id[65];
            field(d, "form-name", d->form == SB_FORM_PROJECT ? "Projektname" : "Titel", d->name, sizeof(d->name), 1);
            if (!d->id_manual) sb_app_slug(d->name, d->id, sizeof(d->id));
            strcpy(previous_id, d->id);
            field(d, "form-id", d->form == SB_FORM_PROJECT ? "Ordnername" : "Dateiname", d->id, sizeof(d->id), 2);
            if (strcmp(previous_id, d->id)) d->id_manual = true;
            nk_layout_row_dynamic(ctx, 44 * s, 1);
            nk_label_wrap(ctx, "Kleinbuchstaben, Zahlen und Bindestriche. Vorhandene Projekte und Notizen bleiben erhalten.");
            if (d->form == SB_FORM_PROJECT)
                field(d, "form-repo", "Projektordner verknüpfen (optional)", d->repository, sizeof(d->repository), 3);
            else {
                nk_layout_row_dynamic(ctx, 24 * s, 1); nk_label(ctx, "Wissensbereich", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 34 * s, 1);
                d->note_section = nk_combo(ctx, new_names, 3, d->note_section, (int)(28 * s), nk_vec2(w - 60, 130 * s));
            }
        } else if (d->form == SB_FORM_WORKSPACE) {
            nk_layout_row_dynamic(ctx, 72 * s, 1);
            nk_label_wrap(ctx, "Wähle den Ordner, der deine Projektgedächtnisse enthält. Ein neues Projekt wird darin als eigener Unterordner angelegt.");
            field(d, "form-folder", "Arbeitsordner", d->folder, sizeof(d->folder), 1);
        } else if (d->form == SB_FORM_SETTINGS) {
            nk_layout_row_dynamic(ctx, 24 * s, 1); nk_label(ctx, "Projekte", NK_TEXT_LEFT);
            project_rows(d);
            nk_layout_row_dynamic(ctx, 36 * s, 1);
            if (button(d, "workspace-settings", "Arbeitsordner öffnen")) command(d, SB_CMD_WORKSPACE);
            nk_layout_row_dynamic(ctx, 36 * s, 1);
            if (button(d, "new-project-settings", "Neues Projekt")) command(d, SB_CMD_NEW_PROJECT);
            nk_layout_row_dynamic(ctx, 32 * s, 1); nk_label(ctx, "Darstellung", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 36 * s, 1);
            if (button(d, "theme", d->ui.dark ? "Helle Darstellung" : "Dunkle Darstellung")) command(d, SB_CMD_THEME);
            nk_layout_row_dynamic(ctx, 36 * s, 2);
            if (button(d, "font-minus", "Kleinere Schrift")) { d->next_scale = fmaxf(1, d->ui.scale - 0.25f); command(d, SB_CMD_SCALE); }
            if (button(d, "font-plus", "Größere Schrift")) { d->next_scale = fminf(2, d->ui.scale + 0.25f); command(d, SB_CMD_SCALE); }
            nk_layout_row_dynamic(ctx, 36 * s, 1);
            if (button(d, "sidebar", d->sidebar ? "Seitenleiste ausblenden" : "Seitenleiste einblenden")) d->sidebar = !d->sidebar;
        } else if (d->form == SB_FORM_CONTEXT) {
            nk_layout_row_dynamic(ctx, 64 * s, 1);
            nk_label_wrap(ctx, "Dieser Kontext enthält die gespeicherten Kerninformationen. Die Originaldateien bleiben über den Projektordner zugänglich.");
            nk_layout_row_dynamic(ctx, 36 * s, 1);
            if (button(d, "copy-context", "Kontext kopieren"))
                result(d, SDL_SetClipboardText(d->context ? d->context : "") ? sb_ok() : sb_error(SB_IO, "%s", SDL_GetError()), "Kontext kopiert.");
            document(d, d->context ? d->context : "", w - 30, fmaxf(80, h - 310 * s));
        } else {
            const char *help[] = {"Command auf macOS, Control auf Windows und Linux:",
                "N: Neue Notiz · Umschalt+N: Neues Projekt", "S: Speichern · O: Arbeitsordner öffnen",
                "F: Suche · E: Lesen/Bearbeiten", "R: Neu laden · Umschalt+C: KI-Kontext",
                "Escape: Dialog schließen oder Quelle verlassen", "Im Editor: A/C/V/X/Z sowie Y oder Umschalt+Z"};
            for (size_t i = 0; i < sizeof(help) / sizeof(*help); ++i) {
                nk_layout_row_dynamic(ctx, 50 * s, 1); nk_label_wrap(ctx, help[i]);
            }
        }
        nk_layout_row_dynamic(ctx, 52 * s, 1); nk_label_wrap(ctx, d->message.message);
        nk_layout_row_dynamic(ctx, 36 * s, 2);
        if (d->form == SB_FORM_PROJECT || d->form == SB_FORM_NOTE || d->form == SB_FORM_WORKSPACE)
            if (button(d, "submit", d->form == SB_FORM_WORKSPACE ? "Öffnen" : "Anlegen")) command(d, SB_CMD_SUBMIT);
        if (button(d, "cancel", "Schließen")) command(d, SB_CMD_CANCEL);
    }
    nk_end(ctx);
}
void sb_desktop_frame(SBDesktop *d) {
    int width, height;
    SDL_GetWindowSize(d->ui.window, &width, &height);
    synchronize(d); search_refresh(d);
    d->ui.ctx->style.combo.button_padding = nk_vec2(0, 11 * d->ui.scale);
    d->ui.ctx->style.combo.button.padding = nk_vec2(2 * d->ui.scale, 2 * d->ui.scale);
    d->target_count = 0;
    nk_flags flags = d->form != SB_FORM_NONE || d->model.guard ? NK_WINDOW_NO_INPUT : 0;
    bool side = d->sidebar && width >= 1100 && d->ui.scale <= 1.25f;
    float sidebar_width = side ? 218 * d->ui.scale : 0;
    float list_width = side ? 288 * d->ui.scale : 244 * d->ui.scale;
    if (list_width > width * 0.37f) list_width = width * 0.37f;
    if (side) sidebar(d, sidebar_width, (float)height, flags);
    document_list(d, sidebar_width, list_width, (float)height, flags, !side);
    detail(d, sidebar_width + list_width, width - sidebar_width - list_width, (float)height, flags);
    popup(d, width, height);
    nk_style_set_font(d->ui.ctx, &d->ui.normal->handle);
    nk_sdl_update_TextInput(d->ui.ctx);
}
