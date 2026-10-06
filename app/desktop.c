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

static void compact_label(SBDesktop *d, const char *text, char *out, size_t capacity, float width);
static void glass(SBDesktop *d, struct nk_rect r, float radius);

static SBTarget *target_add(SBDesktop *d, const char *id, struct nk_rect rect, SBFocusKind kind, int group) {
    if (d->target_count == d->target_capacity) {
        size_t capacity = d->target_capacity ? d->target_capacity * 2 : 64;
        SBTarget *items = realloc(d->targets,capacity*sizeof(*items));
        if (!items) { d->message = sb_error(SB_MEMORY,"Kein Speicher für Tastaturziele."); return NULL; }
        d->targets = items; d->target_capacity = capacity;
    }
    SBTarget *item = &d->targets[d->target_count++];
    snprintf(item->id,sizeof(item->id),"%s",id);
    item->bounds = rect; item->kind = kind; item->group = group;
    return item;
}
static void target(SBDesktop *d, const char *id) {
    SBTarget *item = target_add(d,id,nk_widget_bounds(d->ui.ctx),SB_FOCUS_BUTTON,d->focus_group);
    if (!item) return;
    struct nk_panel *panel = d->ui.ctx->current->layout;
    struct nk_rect reveal = item->bounds;
    if (d->keyboard && d->focus_scroll_frames && !strcmp(d->focus,id)) {
        /* When a short group is clipped by its parent, reveal the group first. */
        while (panel && (panel->clip.h < item->bounds.h || !panel->offset_y)) { reveal = panel->bounds; panel = panel->parent; }
        if (panel) {
            float delta = reveal.y < panel->clip.y ? reveal.y-panel->clip.y :
                reveal.y+reveal.h > panel->clip.y+panel->clip.h ?
                reveal.y+reveal.h-panel->clip.y-panel->clip.h : 0;
            if (delta) *panel->offset_y = (nk_uint)fmaxf(0,(float)*panel->offset_y+delta);
        }
    }
}
static bool focused(SBDesktop *d, const char *id) { return d->keyboard && !strcmp(d->focus, id); }
static void ring(SBDesktop *d, const char *id) {
    if (focused(d, id) && d->target_count && !strcmp(d->targets[d->target_count-1].id,id)) nk_stroke_rect(nk_window_get_canvas(d->ui.ctx),
        d->targets[d->target_count - 1].bounds, 14, 2, nk_rgb(142,191,255));
}
static bool activation(SBDesktop *d, const char *id) {
    if (strcmp(d->activate, id)) return false;
    d->activate[0] = 0; return true;
}
static bool button(SBDesktop *d, const char *id, const char *label) {
    struct nk_rect bounds = nk_widget_bounds(d->ui.ctx);
    target(d, id);
    struct nk_rect clip = d->ui.ctx->current->layout->clip;
    if (bounds.y >= clip.y && bounds.y+bounds.h <= clip.y+clip.h) glass(d,bounds,12);
    char shown[SB_NAME_CAP];
    compact_label(d,label,shown,sizeof(shown), bounds.w - 2*d->ui.ctx->style.button.padding.x - 2*d->ui.ctx->style.button.border);
    bool clicked = nk_button_label(d->ui.ctx, shown) != 0;
    if (strcmp(shown,label) && nk_input_is_mouse_hovering_rect(&d->ui.ctx->input,bounds)) nk_tooltip(d->ui.ctx,label);
    ring(d, id);
    return clicked || activation(d, id);
}
static bool selectable(SBDesktop *d, const char *id, const char *label, nk_bool *selected) {
    target(d, id);
    bool clicked = nk_selectable_label(d->ui.ctx, label, NK_TEXT_LEFT, selected) != 0;
    ring(d, id);
    return clicked || activation(d, id);
}
static void text_target(SBDesktop *d, const char *id) {
    target(d, id);
    if (d->target_count && !strcmp(d->targets[d->target_count-1].id,id)) d->targets[d->target_count - 1].kind = SB_FOCUS_TEXT;
    if (focused(d, id) && !(d->ui.ctx->current->layout->flags & NK_WINDOW_NO_INPUT)) {
        nk_window_set_focus(d->ui.ctx, d->ui.ctx->current->name_string);
        /* Closing a higher window can leave Nuklear's lower window read-only. */
        d->ui.ctx->current->flags &= ~(nk_flags)NK_WINDOW_ROM;
        d->ui.ctx->current->layout->flags &= ~(nk_flags)NK_WINDOW_ROM;
        /* Widget sequence numbers can change when another field scrolls offscreen. */
        nk_edit_focus(d->ui.ctx, NK_EDIT_ALWAYS_INSERT_MODE); d->focus_changed = false;
    }
}
static void command(SBDesktop *d, SBCommand cmd) { if (d->command == SB_CMD_NONE) d->command = cmd; }
static void result(SBDesktop *d, SBStatus status, const char *success) {
    d->message = status;
    if (status.code != SB_OK) d->card = true;
    if (status.code == SB_OK && success) snprintf(d->message.message, sizeof(d->message.message), "%s", success);
}
static void field(SBDesktop *d, const char *tag, const char *label, char *text, int capacity, int focus) {
    struct nk_context *ctx = d->ui.ctx;
    int length = (int)strlen(text);
    nk_layout_row_dynamic(ctx, 22 * d->ui.scale, 1);
    nk_label(ctx, label, NK_TEXT_LEFT);
    nk_layout_row_dynamic(ctx, 36 * d->ui.scale, 1);
    text_target(d, tag);
    if (d->form_focus == focus) { nk_edit_focus(ctx, NK_EDIT_ALWAYS_INSERT_MODE); d->form_focus = 0; }
    nk_flags state = nk_edit_string(ctx, NK_EDIT_FIELD, text, &length, capacity, nk_filter_default);
    if (state & NK_EDIT_ACTIVE) d->active_form_field = focus;
    text[length] = 0;
    ring(d, tag);
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
    d->sidebar = true; d->test = testing; d->card = true; d->zoom = 1; d->graph_dirty = true;
    strcpy(d->focus, "project-picker"); strcpy(d->section, "all");
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
    if (d->text_edit_ready) nk_textedit_free(&d->text_edit);
    free(d->targets);
    sb_graph_free(&d->graph);
    sb_notes_free(&d->hits); free(d->context); sb_app_free(&d->model);
    sb_ui_shutdown(&d->ui); memset(d, 0, sizeof(*d));
}
static void search_refresh(SBDesktop *d) {
    if (!strcmp(d->search, d->searched)) return;
    SBNotes hits = {0}; d->page = 0;
    if (d->model.has_project && d->search[0]) {
        SBStatus status = sb_search(&d->model.project, d->search, &hits);
        if (status.code != SB_OK) { d->message = status; return; }
    }
    sb_notes_free(&d->hits); d->hits = hits;
    strcpy(d->searched, d->search);
}
static void editor_reset(SBDesktop *d) {
    if (d->text_edit_ready) nk_textedit_free(&d->text_edit);
    d->text_edit_ready = false;
    if (!d->model.editor) return;
    nk_textedit_init_fixed(&d->text_edit,d->model.editor,SB_TEXT_LIMIT);
    d->text_edit.string.buffer.allocated = strlen(d->model.editor);
    d->text_edit.string.len = 0;
    for (const unsigned char *p = (const unsigned char *)d->model.editor; *p; ++p)
        if ((*p & 0xc0) != 0x80) ++d->text_edit.string.len;
    d->text_edit.mode = NK_TEXT_EDIT_MODE_INSERT;
    d->text_edit_ready = true;
}
static void synchronize(SBDesktop *d) {
    if (d->generation != d->model.generation) {
        d->generation = d->model.generation;
        d->editing = d->next_edit; d->next_edit = false;
        d->focus_editor = d->editing;
        d->reset_reader = true; d->card = true; d->graph_dirty = true; d->page = 0;
        sb_ui_reset_editor(&d->ui); editor_reset(d);
        d->search[0] = 0; d->searched[0] = 0; sb_notes_free(&d->hits);
        if (!strncmp(d->model.path, "archive/", 8)) strcpy(d->section, "archive");
    }
    if (!d->text_edit_ready && d->model.editor) editor_reset(d);
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
        if (status.code == SB_OK) d->graph_dirty = true;
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
        if (cmd == SB_CMD_GUARD_CANCEL) d->next_edit = false;
        result(d, status, NULL);
    } else if (cmd == SB_CMD_ARCHIVE || cmd == SB_CMD_RELOAD) {
        status = sb_app_request(&d->model, cmd == SB_CMD_ARCHIVE ? SB_ACT_ARCHIVE : SB_ACT_RELOAD, NULL);
        result(d, status, cmd == SB_CMD_ARCHIVE ? "Dokument archiviert." : "Dokument neu geladen.");
    } else if (cmd == SB_CMD_THEME) sb_ui_theme(&d->ui, !d->ui.dark);
    else if (cmd == SB_CMD_SCALE) result(d, sb_ui_fonts(&d->ui, d->next_scale), NULL);
    else if (cmd == SB_CMD_SOURCE) {
        if (!d->model.source) snprintf(d->source_focus,sizeof(d->source_focus),"%s",d->focus);
        result(d, sb_app_source(&d->model, d->command_value), NULL);
        if (d->message.code == SB_OK) d->reset_reader = true;
    }
    if (d->navigation.kind != SB_ACT_NONE) {
        SBAction action = d->navigation;
        memset(&d->navigation, 0, sizeof(d->navigation));
        result(d, sb_app_request(&d->model, action.kind, action.value), NULL);
        if (action.kind == SB_ACT_PROJECT && (d->form == SB_FORM_SETTINGS || d->form == SB_FORM_PROJECTS) && d->message.code == SB_OK)
            d->form = SB_FORM_NONE;
    }
    synchronize(d);
}
static SBTarget *focus_target(SBDesktop *d) {
    for (size_t i = 0; i < d->target_count; ++i) if (!strcmp(d->focus, d->targets[i].id)) return &d->targets[i];
    return NULL;
}
static void focus_set(SBDesktop *d, const char *id) {
    snprintf(d->focus, sizeof(d->focus), "%s", id);
    d->focus_changed = true; d->keyboard = true; d->focus_scroll_frames = 3;
    d->ui.ctx->text_edit.active = 0;
    for (struct nk_window *w = d->ui.ctx->begin; w; w = w->next) w->edit.active = 0;
}
static void focus_step(SBDesktop *d, int direction) {
    if (!d->target_count) return;
    size_t index = direction > 0 ? d->target_count - 1 : 0;
    for (size_t i = 0; i < d->target_count; ++i) if (!strcmp(d->focus, d->targets[i].id)) { index = i; break; }
    index = direction > 0 ? (index + 1) % d->target_count : (index + d->target_count - 1) % d->target_count;
    focus_set(d, d->targets[index].id);
}
static bool inside(float x, float y, struct nk_rect r) {
    return x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h;
}
static bool map_input(SBDesktop *d, float x, float y) {
    if (d->form != SB_FORM_NONE || d->model.guard || !inside(x, y, d->map_bounds)) return false;
    for (unsigned i = 0; i < d->ui.space.glass_count; ++i) {
        SDL_FRect r = d->ui.space.glass[i].rect;
        if (inside(x, y, nk_rect(r.x, r.y, r.w, r.h))) return false;
    }
    return true;
}
static void camera_reset(SBDesktop *d) { d->yaw = d->pitch = d->pan_x = d->pan_y = 0; d->zoom = 1; }
static void star_step(SBDesktop *d, SDL_Keycode key) {
    SBSpace *s = &d->ui.space;
    if (!s->count) return;
    if (d->star >= s->count || !s->points[d->star].visible) {
        for (size_t i = 0; i < s->count; ++i) if (s->points[i].visible) { d->star = i; return; }
        return;
    }
    SBPoint current = s->points[d->star];
    float best = 1e20f; size_t found = d->star;
    for (size_t i = 0; i < s->count; ++i) if (i != d->star && s->points[i].visible) {
        float dx = s->points[i].x - current.x, dy = s->points[i].y - current.y;
        float along = key == SDLK_LEFT ? -dx : key == SDLK_RIGHT ? dx : key == SDLK_UP ? -dy : dy;
        float across = key == SDLK_LEFT || key == SDLK_RIGHT ? fabsf(dy) : fabsf(dx);
        float score = along + across * 2;
        if (along > 1 && score < best) { best = score; found = i; }
    }
    d->star = found;
}
void sb_desktop_event(SBDesktop *d, const SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
        request(d, SB_ACT_QUIT, NULL); return;
    }
    if (event->type == SDL_EVENT_MOUSE_MOTION) {
        d->ui.space.mouse_x = event->motion.x; d->ui.space.mouse_y = event->motion.y;
        if (d->dragging) {
            float dx = event->motion.x - d->drag_x, dy = event->motion.y - d->drag_y;
            if (fabsf(dx) + fabsf(dy) > 2) d->moved = true;
            if (SDL_GetModState() & SDL_KMOD_SHIFT) { d->pan_x += dx; d->pan_y += dy; }
            else { d->yaw += dx * 0.007f; d->pitch = fmaxf(-1.2f, fminf(1.2f, d->pitch + dy * 0.007f)); }
            d->drag_x = event->motion.x; d->drag_y = event->motion.y;
        }
    }
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        d->keyboard = false;
        for (size_t i = d->target_count; i > 0; --i) if (inside(event->button.x, event->button.y, d->targets[i-1].bounds)) {
            snprintf(d->focus, sizeof(d->focus), "%s", d->targets[i-1].id); break;
        }
        if (map_input(d, event->button.x, event->button.y)) {
            d->dragging = true; d->moved = false; d->drag_x = event->button.x; d->drag_y = event->button.y;
            strcpy(d->focus, "galaxy");
        }
    }
    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP && d->dragging) {
        d->dragging = false;
        if (!d->moved && event->button.button == SDL_BUTTON_LEFT) {
            float best = 22 * 22; size_t found = d->graph.count;
            for (size_t i = 0; i < d->ui.space.count; ++i) if (d->ui.space.points[i].visible) {
                float dx = d->ui.space.points[i].x - event->button.x, dy = d->ui.space.points[i].y - event->button.y;
                if (dx * dx + dy * dy < best) { best = dx * dx + dy * dy; found = i; }
            }
            if (found < d->model.notes.count) { d->star = found; d->card = true; request(d, SB_ACT_NOTE, d->model.notes.items[found].path); }
        }
    }
    if (event->type == SDL_EVENT_MOUSE_WHEEL && map_input(d, event->wheel.mouse_x, event->wheel.mouse_y)) {
        d->zoom = fmaxf(0.45f, fminf(3.5f, d->zoom * (1 + event->wheel.y * 0.08f))); return;
    }
    if (event->type == SDL_EVENT_KEY_DOWN) {
        SDL_Keycode key = event->key.key;
#ifdef __APPLE__
        bool modifier = (event->key.mod & SDL_KMOD_GUI) != 0;
#else
        bool modifier = (event->key.mod & SDL_KMOD_CTRL) != 0;
#endif
        bool shift = (event->key.mod & SDL_KMOD_SHIFT) != 0;
        if (key == SDLK_TAB) { focus_step(d, shift ? -1 : 1); return; }
        if (key == SDLK_F6 && d->form == SB_FORM_NONE && !d->model.guard) {
            SBTarget *item = focus_target(d); int group = item ? item->group : 0;
            int next = (group + (shift ? 2 : 1)) % 3;
            if (next == 2) d->card = true;
            if (next == 1) { d->card = false; d->browser = false; }
            focus_set(d, next == 0 ? "project-picker" : next == 1 ? "galaxy" : d->editing ? "editor" : "reader"); return;
        }
        if (key == SDLK_ESCAPE) {
            if (d->model.guard) command(d, SB_CMD_GUARD_CANCEL);
            else if (d->form != SB_FORM_NONE) command(d, SB_CMD_CANCEL);
            else if (d->model.source) { sb_app_source_close(&d->model); d->reset_reader = true; focus_set(d,d->source_focus[0] ? d->source_focus : "reader"); }
            else if (d->search[0]) { d->search[0] = 0; focus_set(d, "search"); }
            else if (d->editing) { d->editing = false; focus_set(d, "reader"); }
            else { d->card = false; focus_set(d, "galaxy"); }
            return;
        }
        if (modifier && key == SDLK_Q) { request(d, SB_ACT_QUIT, NULL); return; }
        if (d->model.guard) {
            if (modifier && key == SDLK_S) { command(d, SB_CMD_GUARD_SAVE); return; }
        } else if (d->form == SB_FORM_NONE && modifier) {
            if (key == SDLK_S) { command(d, shift ? SB_CMD_COPY : SB_CMD_SAVE); return; }
            if (key == SDLK_N) { command(d, shift ? SB_CMD_NEW_PROJECT : SB_CMD_NEW_NOTE); return; }
            if (key == SDLK_O) { command(d, SB_CMD_WORKSPACE); return; }
            if (key == SDLK_F) { d->browser = true; focus_set(d, "search"); return; }
            if (key == SDLK_E && d->model.editor && !d->model.source) { d->card = true; d->editing = !d->editing; d->focus_editor = d->editing; focus_set(d, d->editing ? "editor" : "reader"); return; }
            if (key == SDLK_R) { command(d, SB_CMD_RELOAD); return; }
            if (key == SDLK_C && shift) { command(d, SB_CMD_CONTEXT); return; }
            if (key == SDLK_COMMA) { d->form = SB_FORM_SETTINGS; return; }
        }
        if (key == SDLK_F1) { d->form = SB_FORM_HELP; return; }
        SBTarget *item = focus_target(d);
        bool text = item && item->kind == SB_FOCUS_TEXT;
        if (text && !strcmp(d->focus, "editor") && key == SDLK_I && (event->key.mod & SDL_KMOD_CTRL)) {
            nk_input_unicode(d->ui.ctx, '\t'); return;
        }
        if (!modifier && !text && (key == SDLK_RETURN || key == SDLK_SPACE)) {
            if (!strcmp(d->focus, "galaxy") && d->star < d->model.notes.count) {
                d->card = true; request(d, SB_ACT_NOTE, d->model.notes.items[d->star].path);
            } else snprintf(d->activate, sizeof(d->activate), "%s", d->focus);
            return;
        }
        if (text && key == SDLK_RETURN && d->form != SB_FORM_NONE) { command(d, SB_CMD_SUBMIT); return; }
        if (!strcmp(d->focus, "galaxy") && !modifier) {
            if (key == SDLK_LEFT || key == SDLK_RIGHT || key == SDLK_UP || key == SDLK_DOWN) {
                if (shift) {
                    if (key == SDLK_LEFT) d->yaw -= 0.1f;
                    if (key == SDLK_RIGHT) d->yaw += 0.1f;
                    if (key == SDLK_UP) d->pitch = fmaxf(-1.2f, d->pitch - 0.1f);
                    if (key == SDLK_DOWN) d->pitch = fminf(1.2f, d->pitch + 0.1f);
                } else star_step(d, key);
                d->keyboard = true; return;
            }
            if (key == SDLK_HOME) { camera_reset(d); return; }
            if (key == SDLK_EQUALS || key == SDLK_PLUS || key == SDLK_KP_PLUS) { d->zoom = fminf(3.5f, d->zoom * 1.15f); return; }
            if (key == SDLK_MINUS || key == SDLK_KP_MINUS) { d->zoom = fmaxf(0.45f, d->zoom / 1.15f); return; }
        }
        if (!text && (key == SDLK_PAGEDOWN || key == SDLK_PAGEUP || key == SDLK_DOWN || key == SDLK_UP)) {
            d->focus_scroll_frames = 0;
            d->scroll += key == SDLK_DOWN ? 50 : key == SDLK_UP ? -50 : key == SDLK_PAGEDOWN ? 220 : -220;
            return;
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
    size_t start = d->project_page * 6;
    if (start >= d->model.projects.count) { d->project_page = 0; start = 0; }
    for (size_t i = start; i < d->model.projects.count && i < start + 6; ++i) {
        char tag[100];
        nk_bool selected = d->model.has_project && !strcmp(d->model.project.id, d->model.projects.items[i].id);
        nk_layout_row_dynamic(ctx, 36 * scale, 1);
        snprintf(tag, sizeof(tag), "project:%s", d->model.projects.items[i].id);
        if (selectable(d, tag, d->model.projects.items[i].name, &selected))
            request(d, SB_ACT_PROJECT, d->model.projects.items[i].id);
    }
    if (d->model.projects.count > 6) {
        nk_layout_row_dynamic(ctx, 34 * scale, 2);
        if (button(d,"projects-prev","Vorherige Projekte")) d->project_page = d->project_page ? d->project_page-1 : (d->model.projects.count-1)/6;
        if (button(d,"projects-next","Weitere Projekte")) d->project_page = (d->project_page+1)%((d->model.projects.count-1)/6+1);
    }
}
static void glass(SBDesktop *d, struct nk_rect r, float radius) {
    SBSpace *space = &d->ui.space;
    if (space->glass_count < 64) space->glass[space->glass_count++] = (SBGlass){{r.x,r.y,r.w,r.h}, radius};
}
static void tools(SBDesktop *d, int width, float height, nk_flags flags) {
    struct nk_context *ctx = d->ui.ctx; float s = d->ui.scale;
    struct nk_rect rect = nk_rect(18, 16, width - 36.0f, height);
    glass(d, rect, 27); d->focus_group = 0;
    if (nk_begin(ctx, "Lumen tools", rect, flags | NK_WINDOW_NO_SCROLLBAR)) {
        bool compact = width < 1100 || s > 1.25f;
        nk_layout_row_begin(ctx, NK_DYNAMIC, 36 * s, compact ? 4 : 8);
        if (!compact) { nk_layout_row_push(ctx, 0.13f); nk_label(ctx, "SecondBrain", NK_TEXT_LEFT); }
        nk_layout_row_push(ctx, compact ? 0.30f : 0.18f);
        if (button(d, "project-picker", d->model.has_project ? d->model.project.name : "Projekte")) d->form = SB_FORM_PROJECTS;
        nk_layout_row_push(ctx, compact ? 0.47f : 0.25f); text_target(d, "search");
        int length = (int)strlen(d->search);
        nk_edit_string(ctx, NK_EDIT_FIELD, d->search, &length, sizeof(d->search), nk_filter_default);
        d->search[length] = 0;
        if (!length && !focused(d, "search")) {
            struct nk_rect hint = d->targets[d->target_count-1].bounds;
            hint.x += 12; hint.w -= 24;
            nk_draw_text(nk_window_get_canvas(ctx), hint, "Suche im Projekt", 16, &d->ui.normal->handle,
                nk_rgba(0,0,0,0), nk_rgb(154,176,204));
        }
        ring(d, "search");
        nk_layout_row_push(ctx, compact ? 0.07f : 0.04f);
        if (button(d, "clear-search", "×")) d->search[0] = 0;
        if (!compact) {
            nk_layout_row_push(ctx, 0.09f); if (button(d, "list", "Liste")) d->browser = !d->browser;
            nk_layout_row_push(ctx, 0.10f); if (button(d, "new-note", "+ Notiz")) command(d, SB_CMD_NEW_NOTE);
            nk_layout_row_push(ctx, 0.11f); if (button(d, "new-project", "+ Projekt")) command(d, SB_CMD_NEW_PROJECT);
        }
        nk_layout_row_push(ctx, compact ? 0.16f : 0.10f);
        if (button(d, "settings", compact ? "Design" : "Darstellung")) d->form = SB_FORM_SETTINGS;
        nk_layout_row_end(ctx);
        if (compact) {
            nk_layout_row_dynamic(ctx, 34 * s, 4);
            if (button(d, "list", "Liste")) d->browser = !d->browser;
            if (button(d, "new-note", "+ Notiz")) command(d, SB_CMD_NEW_NOTE);
            if (button(d, "new-project", "+ Projekt")) command(d, SB_CMD_NEW_PROJECT);
            if (button(d, "help", "Tastaturhilfe")) d->form = SB_FORM_HELP;
        }
    }
    nk_end(ctx);
}
static void document_list(SBDesktop *d, struct nk_rect rect, nk_flags flags) {
    struct nk_context *ctx = d->ui.ctx; float s = d->ui.scale;
    const SBNotes *notes = d->search[0] ? &d->hits : &d->model.notes;
    glass(d, rect, 26); d->focus_group = 0;
    if (nk_begin(ctx, "Documents", rect, flags)) {
        nk_layout_row_dynamic(ctx, 28 * s, 1); muted(d, d->search[0] ? "SUCHERGEBNISSE" : "DOKUMENTE");
        if (rect.h < 320*s) {
            int selected = 0;
            for (int k = 0; k < 6; ++k) if (!strcmp(d->section,sections[k])) selected = k;
            nk_layout_row_dynamic(ctx,28*s,1);
            if (button(d,"filter",section_names[selected])) d->form = SB_FORM_FILTER;
        } else {
            nk_layout_row_dynamic(ctx, 28 * s, 2);
            for (unsigned k = 0; k < 6; ++k) {
                char tag[100]; snprintf(tag,sizeof(tag),"section:%s",sections[k]);
                nk_bool selected = !strcmp(d->section,sections[k]);
                const char *short_names[] = {"Alle","Kern","Wissen","Eingang","Journal","Archiv"};
                if (selectable(d,tag,short_names[k],&selected)) { strcpy(d->section,sections[k]); d->page = 0; }
            }
        }
        size_t count = 0;
        for (size_t i = 0; i < notes->count; ++i) if (!strcmp(d->section, "all") || !strcmp(d->section, notes->items[i].section)) ++count;
        size_t per_page = (size_t)fmaxf(1, floorf((rect.h - (rect.h < 320*s ? 125 : 190) * s) / (40 * s)));
        if (d->page * per_page >= count) d->page = 0;
        size_t shown = 0;
        for (size_t i = 0; i < notes->count; ++i) {
            if (strcmp(d->section, "all") && strcmp(d->section, notes->items[i].section)) continue;
            size_t index = shown++;
            if (index < d->page * per_page || index >= (d->page + 1) * per_page) continue;
            char tag[100], title[SB_NAME_CAP];
            if (strlen(notes->items[i].path) < 90) snprintf(tag, sizeof(tag), "note:%s", notes->items[i].path);
            else snprintf(tag, sizeof(tag), "note-hash:%llu", (unsigned long long)sb_hash(notes->items[i].path, strlen(notes->items[i].path)));
            nk_bool selected = !strcmp(d->model.path, notes->items[i].path);
            compact_label(d, notes->items[i].title, title, sizeof(title), rect.w - 60);
            nk_layout_row_dynamic(ctx, 36 * s, 1);
            if (selectable(d, tag, title, &selected)) { d->card = true; request(d, SB_ACT_NOTE, notes->items[i].path); }
            if (nk_widget_is_hovered(ctx)) nk_tooltip(ctx, notes->items[i].path);
        }
        if (!count) { nk_layout_row_dynamic(ctx, 70 * s, 1); nk_label_wrap(ctx, "Keine Dokumente in diesem Bereich. Ändere Suche oder Bereich."); }
        if (count > per_page) {
            nk_layout_row_dynamic(ctx, 30 * s, 2);
            if (button(d, "page-prev", "Zurück")) d->page = d->page ? d->page - 1 : (count - 1) / per_page;
            if (button(d, "page-next", "Weiter")) d->page = (d->page + 1) % ((count - 1) / per_page + 1);
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
    target(d, "reader");
    if (d->target_count) d->targets[d->target_count - 1].kind = SB_FOCUS_READER;
    ring(d, "reader");
    if (d->scroll && (!strcmp(d->focus, "reader") || !strncmp(d->focus,"link:",5) || d->form == SB_FORM_CONTEXT)) {
        nk_uint x = 0, y = 0; nk_group_get_scroll(ctx, "Reader", &x, &y);
        nk_group_set_scroll(ctx, "Reader", x, (nk_uint)fmaxf(0, (float)y + d->scroll)); d->scroll = 0;
    }
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
    (void)width;
    if (button(d, "actions", "Aktionen")) d->form = SB_FORM_ACTIONS;
}
static void detail(SBDesktop *d, float x, float y, float width, float height, nk_flags flags) {
    struct nk_context *ctx = d->ui.ctx;
    float s = d->ui.scale;
    if (nk_begin(ctx, "Detail", nk_rect(x, y, width, height), flags)) {
        if (d->scroll && strcmp(d->focus,"reader") && strncmp(d->focus,"link:",5)) {
            nk_uint sx, sy; nk_window_get_scroll(ctx,&sx,&sy);
            nk_window_set_scroll(ctx,sx,(nk_uint)fmaxf(0,(float)sy+d->scroll)); d->scroll = 0;
        }
        if (d->reset_reader) { nk_group_set_scroll(ctx, "Reader", 0, 0); d->reset_reader = false; }
        nk_layout_row_dynamic(ctx, 28 * s, 1);
        if (button(d, "close-card", "Zur Sternkarte")) { d->card = false; focus_set(d, "galaxy"); }
        nk_style_set_font(ctx, &d->ui.heading->handle);
        const char *title = d->model.source ? d->model.source_title : d->model.editor ? d->model.title : "Projektwissen";
        float available = fmaxf(60, width - 60);
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
            if (button(d, "source-back", "Zurück zum Dokument")) { sb_app_source_close(&d->model); d->reset_reader = true; if (d->keyboard) focus_set(d,d->source_focus[0] ? d->source_focus : "reader"); }
        } else if (d->model.editor) {
            if (d->editing) {
                nk_layout_row_dynamic(ctx,34*s,3);
                if (button(d,"read","Lesen")) d->editing = false;
                if (button(d,"save","Speichern")) command(d,SB_CMD_SAVE);
            } else {
                nk_layout_row_dynamic(ctx,34*s,sb_app_dirty(&d->model) ? 3 : 2);
                if (button(d,"edit","Bearbeiten")) { d->editing = true; d->focus_editor = true; }
                if (sb_app_dirty(&d->model) && button(d,"save","Speichern")) command(d,SB_CMD_SAVE);
            }
            actions(d,width);
        }
        float tools = 34;
        float body_height = fmaxf(80*s, height - 32 - title_height - (24 + tools + 42 + 36) * s - 40);
        if (d->model.source) document(d, d->model.source, width, body_height);
        else if (d->model.editor && d->editing) {
            nk_style_set_font(ctx, &d->ui.body->handle);
            nk_layout_row_dynamic(ctx, body_height, 1); text_target(d, "editor");
            if (d->focus_editor) { nk_edit_focus(ctx, NK_EDIT_ALWAYS_INSERT_MODE); d->focus_editor = false; }
            nk_edit_buffer(ctx, NK_EDIT_BOX, &d->text_edit, nk_filter_default);
            size_t length = d->text_edit.string.buffer.allocated;
            d->model.editor[length] = 0; ring(d, "editor");
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
        nk_layout_row_dynamic(ctx, 42 * s, 1);
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
        glass(d, nk_rect((width-w)/2, (height-h)/2, w, h), 28);
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
        d->form == SB_FORM_CONTEXT ? "KI-Kontext" : d->form == SB_FORM_ACTIONS ? "Dokumentaktionen" :
        d->form == SB_FORM_PROJECTS ? "Projekt wählen" : d->form == SB_FORM_FILTER ? "Wissensbereich" : "Tastaturhilfe";
    glass(d, nk_rect((width-w)/2, (height-h)/2, w, h), 28);
    if (nk_begin(ctx, title, nk_rect((width - w) / 2, (height - h) / 2, w, h),
                 NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
        if (d->scroll && d->form != SB_FORM_CONTEXT) {
            nk_uint x, y; nk_window_get_scroll(ctx, &x, &y);
            nk_window_set_scroll(ctx, x, (nk_uint)fmaxf(0, (float)y + d->scroll)); d->scroll = 0;
        }
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
                if (button(d, "section-choice", new_names[d->note_section])) d->note_section = (d->note_section + 1) % 3;
            }
        } else if (d->form == SB_FORM_WORKSPACE) {
            nk_layout_row_dynamic(ctx, 72 * s, 1);
            nk_label_wrap(ctx, "Wähle den Ordner, der deine Projektgedächtnisse enthält. Ein neues Projekt wird darin als eigener Unterordner angelegt.");
            field(d, "form-folder", "Arbeitsordner", d->folder, sizeof(d->folder), 1);
        } else if (d->form == SB_FORM_FILTER) {
            nk_layout_row_dynamic(ctx,36*s,1);
            for (unsigned k = 0; k < 6; ++k) {
                char tag[100]; snprintf(tag,sizeof(tag),"section:%s",sections[k]);
                nk_bool selected = !strcmp(d->section,sections[k]);
                if (selectable(d,tag,section_names[k],&selected)) { strcpy(d->section,sections[k]); d->page = 0; d->form = SB_FORM_NONE; }
            }
        } else if (d->form == SB_FORM_PROJECTS) {
            project_rows(d);
            nk_layout_row_dynamic(ctx, 36 * s, 1);
            if (button(d, "new-project-settings", "Neues Projekt")) command(d, SB_CMD_NEW_PROJECT);
            if (button(d, "workspace-settings", "Arbeitsordner öffnen")) command(d, SB_CMD_WORKSPACE);
        } else if (d->form == SB_FORM_ACTIONS) {
            nk_layout_row_dynamic(ctx, 36 * s, 1);
            if (button(d, "context", "KI-Kontext")) command(d, SB_CMD_CONTEXT);
            if (button(d, "reload", "Neu laden")) { command(d, SB_CMD_RELOAD); d->form = SB_FORM_NONE; }
            if (d->model.editor && strchr(d->model.path, '/') && strncmp(d->model.path, "archive/", 8))
                if (button(d, "archive", "Archivieren")) { command(d, SB_CMD_ARCHIVE); d->form = SB_FORM_NONE; }
            if (button(d, "save-copy", "Als neue Notiz speichern")) { command(d, SB_CMD_COPY); d->form = SB_FORM_NONE; }
            if (button(d, "settings-actions", "Darstellung")) d->form = SB_FORM_SETTINGS;
            if (button(d, "help-actions", "Tastaturhilfe")) d->form = SB_FORM_HELP;
        } else if (d->form == SB_FORM_SETTINGS) {
            nk_layout_row_dynamic(ctx, 24 * s, 1); nk_label(ctx, "Projekte", NK_TEXT_LEFT);
            if (button(d, "project-settings", "Projekt wählen")) d->form = SB_FORM_PROJECTS;
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
            if (button(d, "transparency", d->solid ? "Glasdarstellung aktivieren" : "Transparenz reduzieren")) d->solid = !d->solid;
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
                "Tab / Umschalt+Tab: Fokus · Enter/Leertaste: aktivieren",
                "F6: Werkzeuge, Sterne, Dokument · F1: Hilfe",
                "Sterne: Pfeile wählen · Enter öffnet die Notiz",
                "Umschalt+Pfeile: Kamera drehen · +/-: Zoom · Pos1: zurück",
                "Ziehen: drehen · Umschalt+Ziehen: verschieben · Mausrad: Zoom",
                "Bild auf/ab: Lesen scrollen · Escape: zurück/schließen",
                "Im Editor: A/C/V/X/Z · Y oder Umschalt+Z · Ctrl+I: Tabulator einfügen"};
            for (size_t i = 0; i < sizeof(help) / sizeof(*help); ++i) {
                nk_layout_row_dynamic(ctx, 40 * s, 1); nk_label_wrap(ctx, help[i]);
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
static void graph_refresh(SBDesktop *d) {
    if (!d->graph_dirty) return;
    d->graph_dirty = false;
    if (strcmp(d->graph_project, d->model.project.root)) {
        snprintf(d->graph_project, sizeof(d->graph_project), "%s", d->model.project.root); camera_reset(d);
    }
    if (d->model.has_project) {
        SBStatus status = sb_graph_build(&d->model.project, &d->model.notes, &d->graph);
        if (status.code != SB_OK) { sb_graph_free(&d->graph); result(d, status, NULL); }
    } else sb_graph_free(&d->graph);
    free(d->ui.space.points); d->ui.space.points = NULL; d->ui.space.count = 0;
    if (d->graph.count) {
        d->ui.space.points = calloc(d->graph.count, sizeof(*d->ui.space.points));
        if (!d->ui.space.points) { result(d, sb_error(SB_MEMORY, "Kein Speicher für Sterne."), NULL); return; }
        d->ui.space.count = d->graph.count;
    }
    d->ui.space.graph = &d->graph;
    for (size_t i = 0; i < d->model.notes.count; ++i) if (!strcmp(d->model.path, d->model.notes.items[i].path)) d->star = i;
}
static void galaxy(SBDesktop *d, int width, int height, struct nk_rect card, struct nk_rect list) {
    struct nk_context *ctx = d->ui.ctx; SBSpace *space = &d->ui.space;
    float cy = d->map_bounds.y + d->map_bounds.h / 2;
    float left = d->browser ? list.x + list.w + 10 : 24;
    float right = d->card ? card.x - 12 : width - 24.0f;
    if (right - left < 180) { left = 24; right = width - 24.0f; }
    float cx = (left + right) / 2;
    float unit = fminf((right - left) / 760, d->map_bounds.h / 460) * d->zoom;
    for (size_t i = 0; i < space->count; ++i) {
        SBStar v = d->graph.stars[i];
        float x = v.x * cosf(d->yaw) + v.z * sinf(d->yaw);
        float z = -v.x * sinf(d->yaw) + v.z * cosf(d->yaw);
        float y = v.y * cosf(d->pitch) - z * sinf(d->pitch);
        z = v.y * sinf(d->pitch) + z * cosf(d->pitch);
        float perspective = 700 / (700 + z);
        SBPoint *p = &space->points[i];
        p->x = cx + x * perspective * unit + d->pan_x;
        p->y = cy + y * perspective * unit + d->pan_y;
        p->depth = perspective; p->group = v.group;
        p->selected = !strcmp(d->model.path, d->model.notes.items[i].path);
        p->focused = focused(d, "galaxy") && i == d->star;
        p->visible = inside(p->x, p->y, d->map_bounds) &&
            (!strcmp(d->section, "all") || !strcmp(d->section, d->model.notes.items[i].section));
        if (d->search[0]) {
            bool hit = false;
            for (size_t j = 0; j < d->hits.count; ++j) if (!strcmp(d->hits.items[j].path, d->model.notes.items[i].path)) { hit = true; break; }
            p->visible = p->visible && hit;
        }
    }
    d->focus_group = 1;
    if (nk_begin(ctx, "Galaxy", nk_rect(0,0,(float)width,(float)height), NK_WINDOW_NO_INPUT | NK_WINDOW_BACKGROUND | NK_WINDOW_NO_SCROLLBAR)) {
        target_add(d,"galaxy",d->map_bounds,SB_FOCUS_MAP,1);
        struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
        if (focused(d, "galaxy")) nk_stroke_rect(canvas, d->map_bounds, 24, 1, nk_rgba(142,191,255,90));
        struct nk_rect labels[80]; unsigned label_count = 0;
        for (size_t i = 0; i < space->count; ++i) {
            SBPoint p = space->points[i];
            if (!p.visible || (d->card && inside(p.x,p.y,card)) || (d->browser && inside(p.x,p.y,list))) continue;
            if (p.selected) nk_stroke_circle(canvas, nk_rect(p.x-8,p.y-8,16,16), 1, nk_rgba(142,191,255,145));
            if (p.focused) nk_stroke_circle(canvas, nk_rect(p.x-13,p.y-13,26,26), 2.5f, nk_rgb(190,223,255));
            char title[SB_NAME_CAP]; compact_label(d, d->model.notes.items[i].title, title, sizeof(title), 200);
            float tw = d->ui.normal->handle.width(d->ui.normal->handle.userdata, d->ui.normal->handle.height, title, (int)strlen(title));
            struct nk_rect r = nk_rect(p.x+12,p.y-8,tw+5,24*d->ui.scale);
            if (d->card && r.x < card.x+card.w && r.x+r.w > card.x && r.y < card.y+card.h && r.y+r.h > card.y)
                r.x = p.x-tw-12;
            if (d->browser && r.x < list.x+list.w && r.x+r.w > list.x && r.y < list.y+list.h && r.y+r.h > list.y) continue;
            bool collision = false;
            for (unsigned j = 0; j < label_count; ++j) if (r.x < labels[j].x+labels[j].w && r.x+r.w > labels[j].x && r.y < labels[j].y+labels[j].h && r.y+r.h > labels[j].y) collision = true;
            if ((!collision && label_count < 80) || p.selected || p.focused) {
                nk_draw_text(canvas,r,title,(int)strlen(title),&d->ui.normal->handle,nk_rgba(0,0,0,0),
                    d->ui.dark ? nk_rgb(220,233,251) : nk_rgb(40,61,89));
                if (label_count < 80) labels[label_count++] = r;
            }
        }
        if (!space->count) nk_draw_text(canvas, nk_rect(60,cy-20,width-120.0f,70),
            "Dein Projektwissen wird hier zur Sternkarte.", (int)strlen("Dein Projektwissen wird hier zur Sternkarte."),
            &d->ui.body->handle,nk_rgba(0,0,0,0),nk_rgb(166,185,211));
    }
    nk_end(ctx);
}
static void camera_tools(SBDesktop *d, int width, int height, float available, nk_flags flags) {
    struct nk_context *ctx = d->ui.ctx; float s = d->ui.scale;
    float w = fminf(available, fminf(width-36.0f,380*s));
    if (w < 220) return;
    struct nk_rect rect = nk_rect(18,height-80*s,w,64*s);
    glass(d,rect,26); d->focus_group = 1;
    if (nk_begin(ctx,"Camera",rect, flags | NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_begin(ctx,NK_DYNAMIC,28*s,5);
        nk_layout_row_push(ctx,0.15f);
        if (button(d,"zoom-out","-")) d->zoom = fmaxf(0.45f,d->zoom/1.15f);
        nk_layout_row_push(ctx,0.15f);
        if (button(d,"zoom-in","+")) d->zoom = fminf(3.5f,d->zoom*1.15f);
        nk_layout_row_push(ctx,0.15f);
        if (button(d,"rotate-left","<")) d->yaw -= 0.18f;
        nk_layout_row_push(ctx,0.15f);
        if (button(d,"rotate-right",">")) d->yaw += 0.18f;
        nk_layout_row_push(ctx,0.40f);
        if (button(d,"camera-home","Home")) camera_reset(d);
        nk_layout_row_end(ctx);
    }
    nk_end(ctx);
}
void sb_desktop_frame(SBDesktop *d) {
    int width, height; SDL_GetWindowSize(d->ui.window,&width,&height);
    synchronize(d);
    if (d->search[0] && strcmp(d->search,d->searched)) d->browser = true;
    search_refresh(d); graph_refresh(d);
    bool modal = d->form != SB_FORM_NONE || d->model.guard;
    bool before = d->focus_form != SB_FORM_NONE || d->focus_guard;
    bool enter = modal && (!before || d->form != d->focus_form || d->model.guard != d->focus_guard);
    if (modal && !before) snprintf(d->saved_focus,sizeof(d->saved_focus),"%s",d->focus);
    if (!modal && before) focus_set(d,d->saved_focus[0] ? d->saved_focus : "project-picker");
    if (!modal && d->focus_editor && d->editing) focus_set(d,"editor");
    d->focus_form = d->form; d->focus_guard = d->model.guard;
    float s = d->ui.scale;
    bool compact = width < 1100 || s > 1.25f;
    float header = compact ? 88*s+24 : 38*s+30;
    float top = header+28, body = height-top-20;
    float card_width = d->editing ? width-36.0f : fminf(540*s,width*0.43f);
    float card_height = d->editing ? body : fminf(body, 570*s);
    struct nk_rect card = nk_rect(width-card_width-18,top+body-card_height,card_width,card_height);
    struct nk_rect list = nk_rect(18,top,fminf(300*s,width*0.32f),body-70*s);
    if (d->browser && card.x < list.x + list.w + 12) { card.x = list.x + list.w + 12; card.w = width-card.x-18; }
    d->map_bounds = nk_rect(18,top,width-36.0f,body-70*s);
    d->target_count = 0; d->ui.space.glass_count = 0;
    d->ui.space.dark = d->ui.dark; d->ui.space.solid = d->solid;
    galaxy(d,width,height,card,list);
    nk_flags flags = modal ? NK_WINDOW_NO_INPUT : 0;
    tools(d,width,header,flags);
    if (d->browser) document_list(d,list,flags);
    if (d->card || !d->model.has_project) {
        glass(d,card,28); d->focus_group = 2;
        detail(d,card.x,card.y,card.w,card.h,flags);
    }
    camera_tools(d,width,height,d->card ? card.x-36 : width-36.0f,flags);
    if (modal) { d->target_count = 0; d->focus_group = 3; }
    struct nk_style_item old_background = d->ui.ctx->style.window.fixed_background;
    d->ui.ctx->style.window.fixed_background = nk_style_item_color(d->ui.dark ? nk_rgba(17,29,47,245) : nk_rgba(237,245,255,245));
    popup(d,width,height);
    d->ui.ctx->style.window.fixed_background = old_background;
    for (size_t i = 1; i < d->target_count; ++i) {
        SBTarget value = d->targets[i]; size_t j = i;
        while (j && d->targets[j-1].group > value.group) { d->targets[j] = d->targets[j-1]; --j; }
        d->targets[j] = value;
    }
    if (enter && d->target_count) {
        if (d->targets[0].kind == SB_FOCUS_TEXT) {
            /* The form has already activated its first field; keep SDL text input live. */
            snprintf(d->focus,sizeof(d->focus),"%s",d->targets[0].id);
            d->keyboard = true; d->focus_changed = true; d->focus_scroll_frames = 3;
        } else focus_set(d,d->targets[0].id);
    }
    if (!focus_target(d) && d->target_count) focus_set(d,d->targets[0].id);
    d->activate[0] = 0;
    if (d->focus_scroll_frames) --d->focus_scroll_frames;
    nk_style_set_font(d->ui.ctx,&d->ui.normal->handle);
    nk_sdl_update_TextInput(d->ui.ctx);
}
