struct nk_draw_list;
struct nk_command_text;
void sb_ui_text_draw(struct nk_draw_list *, const struct nk_command_text *);
#define NK_DRAW_TEXT_CUSTOM sb_ui_text_draw
#define NK_IMPLEMENTATION
#define NK_SDL3_RENDERER_IMPLEMENTATION
#include "ui.h"
#include "text.h"
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
    if (!ui->text) {
        /* Nuklear's geometry renderer needs a white atlas texel, not a glyph atlas. */
        char *bytes=NULL; size_t length=0;
        SBStatus status=sb_fs_read(ui->font_path,&bytes,&length);
        if (status.code!=SB_OK) return status;
        static const nk_rune range[]={32,33,0};
        struct nk_font_config config=nk_font_config(0); config.range=range;
        struct nk_font_atlas *atlas=nk_sdl_font_stash_begin(ui->ctx);
        struct nk_font *dummy=nk_font_atlas_add_from_memory(atlas,bytes,length,8,&config);
        free(bytes);
        if (!dummy) return sb_error(SB_IO,"Darstellungsatlas konnte nicht geladen werden.");
        nk_sdl_font_stash_end(ui->ctx);
    }
    float density=SDL_GetWindowPixelDensity(ui->window);
    if (density<1) density=1;
    return sb_ui_text_fonts(ui,scale,density);
}

void sb_ui_theme(SBUi *ui, bool dark) {
    struct nk_color colors[NK_COLOR_COUNT];
    struct nk_color bg = dark ? nk_rgb(12, 22, 37) : nk_rgb(255, 255, 255);
    struct nk_color side = dark ? nk_rgba(110, 144, 184, 22) : nk_rgba(255,255,255,30);
    struct nk_color text = dark ? nk_rgb(231, 237, 246) : nk_rgb(32, 36, 44);
    struct nk_color border = dark ? nk_rgba(162, 184, 217, 56) : nk_rgb(221, 224, 230);
    for (int i = 0; i < NK_COLOR_COUNT; ++i) colors[i] = side;
    colors[NK_COLOR_TEXT] = text; colors[NK_COLOR_WINDOW] = bg;
    colors[NK_COLOR_HEADER] = side; colors[NK_COLOR_BORDER] = border;
    colors[NK_COLOR_BUTTON] = side;
    colors[NK_COLOR_BUTTON_HOVER] = dark ? nk_rgba(159, 189, 229, 48) : nk_rgb(226, 233, 243);
    colors[NK_COLOR_BUTTON_ACTIVE] = nk_rgb(59, 97, 142);
    colors[NK_COLOR_TOGGLE] = border; colors[NK_COLOR_TOGGLE_HOVER] = colors[NK_COLOR_BUTTON_HOVER];
    colors[NK_COLOR_TOGGLE_CURSOR] = nk_rgb(59, 97, 142);
    colors[NK_COLOR_SELECT] = bg; colors[NK_COLOR_SELECT_ACTIVE] = dark ? nk_rgba(113, 157, 212, 64) : nk_rgb(222, 236, 254);
    colors[NK_COLOR_SLIDER] = border; colors[NK_COLOR_SLIDER_CURSOR] = nk_rgb(59, 97, 142);
    colors[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgb(36, 114, 215);
    colors[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgb(59, 97, 142);
    colors[NK_COLOR_EDIT] = dark ? nk_rgba(10,21,36,180) : nk_rgba(249,253,255,200); colors[NK_COLOR_EDIT_CURSOR] = text;
    colors[NK_COLOR_PROPERTY] = side; colors[NK_COLOR_CHART] = bg;
    colors[NK_COLOR_CHART_COLOR] = nk_rgb(59, 97, 142); colors[NK_COLOR_CHART_COLOR_HIGHLIGHT] = text;
    colors[NK_COLOR_SCROLLBAR] = bg; colors[NK_COLOR_SCROLLBAR_CURSOR] = border;
    colors[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = colors[NK_COLOR_BUTTON_HOVER];
    colors[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgb(128, 133, 145);
    colors[NK_COLOR_TAB_HEADER] = side;
    if (ui->contrast) {
        bg=dark ? nk_rgb(0,0,0) : nk_rgb(255,255,255);
        text=dark ? nk_rgb(255,255,255) : nk_rgb(0,0,0);
        for (int i=0;i<NK_COLOR_COUNT;++i) colors[i]=bg;
        colors[NK_COLOR_TEXT]=text; colors[NK_COLOR_BORDER]=text;
        colors[NK_COLOR_BUTTON_HOVER]=dark ? nk_rgb(35,35,35) : nk_rgb(230,230,230);
        colors[NK_COLOR_BUTTON_ACTIVE]=colors[NK_COLOR_BUTTON_HOVER];
        colors[NK_COLOR_EDIT_CURSOR]=text; colors[NK_COLOR_TOGGLE_CURSOR]=text;
        colors[NK_COLOR_SCROLLBAR_CURSOR]=text;
        colors[NK_COLOR_SELECT_ACTIVE]=dark ? nk_rgb(80,80,80) : nk_rgb(175,175,175);
    }
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
    ui->ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(0, 0, 0, 0));
    ui->ctx->style.window.header.normal = nk_style_item_color(nk_rgba(0, 0, 0, 0));
    ui->ctx->style.window.header.active = ui->ctx->style.window.header.normal;
    ui->ctx->style.window.header.hover = ui->ctx->style.window.header.normal;
    ui->ctx->style.window.rounding = 16;
    ui->ctx->style.window.group_padding = nk_vec2(8, 10);
    ui->ctx->style.button.rounding = 9;
    ui->ctx->style.button.border = 0.8f;
    ui->ctx->style.button.border_color = dark ? nk_rgba(161,193,230,48) : nk_rgba(103,139,181,64);
    if (ui->contrast) { ui->ctx->style.button.border_color=text; ui->ctx->style.button.border=1.5f; }
    ui->ctx->style.button.padding = nk_vec2(10, 4);
    ui->ctx->style.edit.rounding = 10;
    ui->ctx->style.edit.padding = nk_vec2(10, 8);
    ui->ctx->style.edit.row_padding = 5;
    ui->ctx->style.selectable.rounding = 8;
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
    sb_ui_theme(ui, true);
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
    if (!sb_space_draw(&ui->space, ui->renderer, width, height)) {
        SDL_SetRenderDrawColor(ui->renderer, 7, 14, 26, 255); SDL_RenderClear(ui->renderer);
    }
    nk_sdl_render(ui->ctx, NK_ANTI_ALIASING_ON);
    sb_ui_text_frame_end(ui);
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
    sb_space_free(&ui->space);
    sb_ui_text_free(ui);
    if (ui->ctx) nk_sdl_shutdown(ui->ctx);
    if (ui->renderer) SDL_DestroyRenderer(ui->renderer);
    if (ui->window) SDL_DestroyWindow(ui->window);
    SDL_Quit();
    memset(ui, 0, sizeof(*ui));
}
