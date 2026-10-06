#define NK_IMPLEMENTATION
#define NK_SDL3_RENDERER_IMPLEMENTATION
#include "ui.h"
#define SDL_MAIN_HANDLED
#include <SDL3/SDL_main.h>
#include "platform.h"
#include <stdlib.h>
#include <string.h>

static void paste(nk_handle user, struct nk_text_edit *edit) {
    char *text;
    size_t length;
    (void)user;
    text = SDL_GetClipboardText();
    if (!text) return;
    length = strlen(text);
    if (length <= SB_TEXT_LIMIT && sb_utf8_valid(text, length))
        nk_textedit_paste(edit, text, (int)length);
    SDL_free(text);
}

SBStatus sb_ui_fonts(SBUi *ui, float scale) {
    struct nk_font_atlas *atlas;
    struct nk_font_config config = nk_font_config(0);
    static const nk_rune ranges[] = {32, 0x024f, 0x0370, 0x052f, 0x2000, 0x206f, 0x2190, 0x22ff, 0};
    char *bytes = NULL;
    size_t length = 0;
    SBStatus result = sb_fs_read(ui->font_path, &bytes, &length);
    if (result.code != SB_OK) return result;
    ui->density = SDL_GetWindowPixelDensity(ui->window);
    if (ui->density < 1) ui->density = 1;
    ui->scale = scale;
    if (ui->normal) {
        struct nk_sdl *backend = (struct nk_sdl *)ui->ctx->userdata.ptr;
        nk_font_atlas_clear(&backend->atlas);
    }
    ui->code = NULL;
    atlas = nk_sdl_font_stash_begin(ui->ctx);
    config.range = ranges;
    config.oversample_h = 2; config.oversample_v = 2;
    ui->normal = nk_font_atlas_add_from_memory(atlas, bytes, length, 15 * scale * ui->density, &config);
    ui->body = nk_font_atlas_add_from_memory(atlas, bytes, length, 18 * scale * ui->density, &config);
    ui->heading = nk_font_atlas_add_from_memory(atlas, bytes, length, 26 * scale * ui->density, &config);
    {
        char mono_path[SB_PATH_CAP], *slash;
        char *mono = NULL;
        size_t mono_length;
        strcpy(mono_path, ui->font_path); slash = strrchr(mono_path, '/');
        if (slash) {
            *slash = 0;
            if (sb_path_join(mono_path, sizeof(mono_path), mono_path, "NotoSansMono-Regular.ttf").code == SB_OK &&
                sb_fs_read(mono_path, &mono, &mono_length).code == SB_OK) {
                ui->code = nk_font_atlas_add_from_memory(atlas, mono, mono_length, 17 * scale * ui->density, &config);
                free(mono);
            }
        }
        if (!ui->code) ui->code = ui->body;
    }
    if (!ui->normal || !ui->body || !ui->heading) { free(bytes); return sb_error(SB_IO, "Schrift konnte nicht geladen werden."); }
    nk_sdl_font_stash_end(ui->ctx);
    ui->normal->handle.height = 15 * scale;
    ui->body->handle.height = 18 * scale;
    ui->heading->handle.height = 26 * scale;
    if (ui->code != ui->body) ui->code->handle.height = 17 * scale;
    nk_style_set_font(ui->ctx, &ui->normal->handle);
    free(bytes);
    return sb_ok();
}

void sb_ui_theme(SBUi *ui, bool dark) {
    struct nk_color colors[NK_COLOR_COUNT];
    struct nk_color bg = dark ? nk_rgb(28, 29, 33) : nk_rgb(255, 255, 255);
    struct nk_color side = dark ? nk_rgb(37, 38, 43) : nk_rgb(243, 244, 247);
    struct nk_color text = dark ? nk_rgb(236, 237, 241) : nk_rgb(32, 36, 44);
    struct nk_color border = dark ? nk_rgb(65, 66, 73) : nk_rgb(221, 224, 230);
    for (int i = 0; i < NK_COLOR_COUNT; ++i) colors[i] = side;
    colors[NK_COLOR_TEXT] = text; colors[NK_COLOR_WINDOW] = bg;
    colors[NK_COLOR_HEADER] = side; colors[NK_COLOR_BORDER] = border;
    colors[NK_COLOR_BUTTON] = side;
    colors[NK_COLOR_BUTTON_HOVER] = dark ? nk_rgb(62, 65, 74) : nk_rgb(226, 233, 243);
    colors[NK_COLOR_BUTTON_ACTIVE] = nk_rgb(27, 93, 190);
    colors[NK_COLOR_TOGGLE] = border; colors[NK_COLOR_TOGGLE_HOVER] = colors[NK_COLOR_BUTTON_HOVER];
    colors[NK_COLOR_TOGGLE_CURSOR] = nk_rgb(27, 93, 190);
    colors[NK_COLOR_SELECT] = bg; colors[NK_COLOR_SELECT_ACTIVE] = dark ? nk_rgb(33, 73, 125) : nk_rgb(222, 236, 254);
    colors[NK_COLOR_SLIDER] = border; colors[NK_COLOR_SLIDER_CURSOR] = nk_rgb(27, 93, 190);
    colors[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgb(36, 114, 215);
    colors[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgb(27, 93, 190);
    colors[NK_COLOR_EDIT] = bg; colors[NK_COLOR_EDIT_CURSOR] = text;
    colors[NK_COLOR_PROPERTY] = side; colors[NK_COLOR_CHART] = bg;
    colors[NK_COLOR_CHART_COLOR] = nk_rgb(27, 93, 190); colors[NK_COLOR_CHART_COLOR_HIGHLIGHT] = text;
    colors[NK_COLOR_SCROLLBAR] = bg; colors[NK_COLOR_SCROLLBAR_CURSOR] = border;
    colors[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = colors[NK_COLOR_BUTTON_HOVER];
    colors[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgb(128, 133, 145);
    colors[NK_COLOR_TAB_HEADER] = side;
    nk_style_from_table(ui->ctx, colors);
    ui->ctx->style.edit.cursor_text_normal = bg;
    ui->ctx->style.edit.cursor_text_hover = bg;
    ui->ctx->style.edit.selected_normal = dark ? nk_rgb(38, 94, 164) : nk_rgb(189, 216, 252);
    ui->ctx->style.edit.selected_hover = ui->ctx->style.edit.selected_normal;
    ui->ctx->style.edit.selected_text_normal = text;
    ui->ctx->style.edit.selected_text_hover = text;
    ui->ctx->style.window.padding = nk_vec2(18, 14);
    ui->ctx->style.window.spacing = nk_vec2(8, 8);
    ui->ctx->style.window.border = 0;
    ui->ctx->style.window.group_padding = nk_vec2(8, 10);
    ui->ctx->style.button.rounding = 7;
    ui->ctx->style.button.border = 0;
    ui->ctx->style.button.padding = nk_vec2(10, 6);
    ui->ctx->style.edit.rounding = 7;
    ui->ctx->style.edit.padding = nk_vec2(10, 8);
    ui->ctx->style.edit.row_padding = 5;
    ui->ctx->style.selectable.rounding = 7;
    ui->ctx->style.selectable.text_normal = text;
    ui->ctx->style.selectable.text_normal_active = text;
    ui->ctx->style.selectable.text_hover_active = text;
    ui->ctx->style.selectable.text_pressed_active = text;
    ui->ctx->style.combo.rounding = 7;
    ui->ctx->style.combo.border = 0;
    ui->ctx->style.combo.content_padding = nk_vec2(10, 4);
    ui->ctx->style.combo.button.padding = nk_vec2(2, 2);
    ui->dark = dark;
}

SBStatus sb_ui_init(SBUi *ui, const char *font_path, int width, int height, bool testing) {
    SBStatus result;
    memset(ui, 0, sizeof(*ui));
    if (strlen(font_path) >= sizeof(ui->font_path)) return sb_error(SB_LIMIT, "Schriftpfad zu lang.");
    strcpy(ui->font_path, font_path);
    ui->testing = testing;
    SDL_SetMainReady();
    if (!SDL_Init(SDL_INIT_VIDEO)) return sb_error(SB_IO, "Fenstersystem: %s", SDL_GetError());
    ui->window = SDL_CreateWindow("SecondBrain", width, height,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | (testing ? SDL_WINDOW_HIDDEN : 0));
    if (!ui->window) { SDL_Quit(); return sb_error(SB_IO, "Fenster: %s", SDL_GetError()); }
    SDL_SetWindowMinimumSize(ui->window, 780, 520);
    ui->renderer = SDL_CreateRenderer(ui->window, testing ? "software" : NULL);
    if (!ui->renderer) { sb_ui_shutdown(ui); return sb_error(SB_IO, "Darstellung: %s", SDL_GetError()); }
    ui->ctx = nk_sdl_init(ui->window, ui->renderer, nk_sdl_allocator());
    ui->ctx->clip.paste = paste;
    result = sb_ui_fonts(ui, 1);
    if (result.code != SB_OK) { sb_ui_shutdown(ui); return result; }
    sb_ui_theme(ui, false);
    return sb_ok();
}

void sb_ui_event(SBUi *ui, const SDL_Event *event) {
    SDL_Event copy = *event;
    if (event->type == SDL_EVENT_TEXT_INPUT) {
        const char *text = event->text.text;
        if (text && sb_utf8_valid(text, strlen(text)))
            while (*text) nk_input_unicode(ui->ctx, (nk_rune)SDL_StepUTF8(&text, NULL));
        return;
    }
    if (event->type == SDL_EVENT_KEY_DOWN || event->type == SDL_EVENT_KEY_UP) {
#ifdef __APPLE__
        if (copy.key.mod & SDL_KMOD_GUI) copy.key.mod |= SDL_KMOD_CTRL;
#endif
        if ((copy.key.mod & SDL_KMOD_CTRL) &&
            (copy.key.key == SDLK_Y || (copy.key.key == SDLK_Z && (copy.key.mod & SDL_KMOD_SHIFT))))
            copy.key.key = SDLK_R;
    }
    nk_sdl_handle_event(ui->ctx, &copy);
}

void sb_ui_draw(SBUi *ui) {
    int width, height;
    SDL_GetWindowSize(ui->window, &width, &height);
    SDL_SetRenderLogicalPresentation(ui->renderer, width, height, SDL_LOGICAL_PRESENTATION_STRETCH);
    SDL_SetRenderDrawColor(ui->renderer, ui->dark ? 28 : 255, ui->dark ? 29 : 255, ui->dark ? 33 : 255, 255);
    SDL_RenderClear(ui->renderer);
    nk_sdl_render(ui->ctx, NK_ANTI_ALIASING_ON);
}
void sb_ui_reset_editor(SBUi *ui) {
    nk_textedit_clear_state(&ui->ctx->text_edit, NK_TEXT_EDIT_MULTI_LINE, nk_filter_default);
    ui->ctx->text_edit.active = 0;
    if (ui->ctx->active) ui->ctx->active->edit.active = 0;
}
SBStatus sb_ui_capture(SBUi *ui, const char *path) {
    SDL_Surface *surface = SDL_RenderReadPixels(ui->renderer, NULL);
    bool ok;
    if (!surface) return sb_error(SB_IO, "Darstellung konnte nicht aufgenommen werden.");
    ok = SDL_SaveBMP(surface, path);
    SDL_DestroySurface(surface);
    return ok ? sb_ok() : sb_error(SB_IO, "Bild konnte nicht gespeichert werden.");
}
void sb_ui_shutdown(SBUi *ui) {
    if (ui->ctx) nk_sdl_shutdown(ui->ctx);
    if (ui->renderer) SDL_DestroyRenderer(ui->renderer);
    if (ui->window) SDL_DestroyWindow(ui->window);
    SDL_Quit();
    memset(ui, 0, sizeof(*ui));
}
