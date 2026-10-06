#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr, "FAIL %d: %s\n", __LINE__, #x); return 1; } } while (0)
static bool equal(struct nk_text_edit *edit, const char *text) {
    int length = nk_str_len_char(&edit->string);
    return (size_t)length == strlen(text) && !memcmp(nk_str_get_const(&edit->string), text, (size_t)length);
}
int main(int argc, char **argv) {
    SBUi ui;
    struct nk_text_edit edit;
    SBStatus result;
    char fixed[16] = {0};
    const char *original = "Wissen ü Äther";
    const char *replacement = "Énergie";
    if (argc != 3) return 2;
    result = sb_ui_init(&ui, argv[1], 1000, 720, true);
    if (result.code != SB_OK) { fprintf(stderr, "%s\n", result.message); return 1; }
    {
        SDL_Event event = {0};
        nk_input_begin(ui.ctx);
        event.type = SDL_EVENT_TEXT_INPUT;
        event.text.windowID = SDL_GetWindowID(ui.window);
        event.text.text = "Üß";
        sb_ui_event(&ui, &event);
        nk_input_end(ui.ctx);
        CHECK(ui.ctx->input.keyboard.text_len == 4);
        CHECK(!memcmp(ui.ctx->input.keyboard.text, "Üß", 4));
        nk_input_begin(ui.ctx);
        event.text.text = "/Ein/langer/Projektpfad/mit/Leerzeichen und ü/SecondBrain";
        sb_ui_event(&ui, &event); nk_input_end(ui.ctx);
        CHECK((size_t)ui.ctx->input.keyboard.text_len == strlen(event.text.text));
        CHECK(!memcmp(ui.ctx->input.keyboard.text, event.text.text, strlen(event.text.text)));
    }
    nk_textedit_init_default(&edit);
    edit.mode = NK_TEXT_EDIT_MODE_INSERT;
    CHECK(nk_textedit_paste(&edit, original, (int)strlen(original)));
    CHECK(equal(&edit, original));
    CHECK(edit.cursor == 14 && edit.string.len == 14);
    edit.select_start = 0; edit.select_end = edit.string.len;
    CHECK(nk_textedit_paste(&edit, replacement, (int)strlen(replacement)));
    CHECK(equal(&edit, replacement));
    nk_textedit_undo(&edit);
    CHECK(equal(&edit, original));
    CHECK(edit.select_start == edit.cursor && edit.select_end == edit.cursor);
    nk_textedit_redo(&edit);
    CHECK(equal(&edit, replacement));
    CHECK(edit.select_start == edit.cursor && edit.select_end == edit.cursor);
    CHECK(!nk_textedit_paste(&edit, "\xc0\x80", 2));
    CHECK(equal(&edit, replacement));
    nk_textedit_free(&edit);

    /* Cursor offsets and inserted lengths beyond signed 16-bit remain correct. */
    nk_textedit_init_default(&edit);
    edit.mode = NK_TEXT_EDIT_MODE_INSERT;
    {
        char *large = malloc(40001);
        CHECK(large != NULL);
        memset(large, 'a', 40000); large[40000] = 0;
        CHECK(nk_textedit_paste(&edit, large, 40000));
        CHECK(equal(&edit, large));
        CHECK(nk_textedit_paste(&edit, "ü", 2));
        nk_textedit_undo(&edit);
        CHECK(edit.cursor == 40000 && equal(&edit, large));
        nk_textedit_redo(&edit);
        CHECK(edit.cursor == 40001);
        nk_textedit_undo(&edit);
        nk_textedit_undo(&edit);
        CHECK(edit.string.len == 0);
        free(large);
    }
    nk_textedit_free(&edit);

    nk_textedit_init_fixed(&edit, fixed, sizeof(fixed));
    edit.mode = NK_TEXT_EDIT_MODE_INSERT;
    CHECK(nk_textedit_paste(&edit, "1234567890", 10));
    edit.select_start = 1; edit.select_end = 2;
    CHECK(!nk_textedit_paste(&edit, "too-much-new-content", 20));
    CHECK(equal(&edit, "1234567890"));
    CHECK(edit.select_start == 1 && edit.select_end == 2);
    nk_textedit_free(&edit);

    nk_input_begin(ui.ctx); nk_input_end(ui.ctx);
    if (nk_begin(ui.ctx, "Darstellung", nk_rect(0, 0, 1000, 720), NK_WINDOW_NO_SCROLLBAR)) {
        nk_style_set_font(ui.ctx, &ui.heading->handle);
        nk_layout_row_dynamic(ui.ctx, 48, 1);
        nk_label(ui.ctx, "Dein Wissen. Deine Projekte.", NK_TEXT_LEFT);
        nk_style_set_font(ui.ctx, &ui.body->handle);
        nk_layout_row_dynamic(ui.ctx, 40, 1);
        nk_label(ui.ctx, "Beispielprojekt · Ziele, Entscheidungen und Erkenntnisse", NK_TEXT_LEFT);
        nk_style_set_font(ui.ctx, &ui.normal->handle);
        nk_layout_row_dynamic(ui.ctx, 36, 3);
        nk_button_label(ui.ctx, "Lesen"); nk_button_label(ui.ctx, "Bearbeiten"); nk_button_label(ui.ctx, "Speichern");
    }
    nk_end(ui.ctx);
    sb_ui_draw(&ui);
    CHECK(sb_ui_capture(&ui, argv[2]).code == SB_OK);
    SDL_RenderPresent(ui.renderer);
    ui.space.dark = true;
    ui.space.points = calloc(1, sizeof(*ui.space.points)); CHECK(ui.space.points != NULL);
    ui.space.count = 1;
    ui.space.points[0] = (SBPoint){450,300,1,1,true,true,false};
    ui.space.glass[0] = (SBGlass){{400,250,250,250},30}; ui.space.glass_count = 1;
    sb_ui_draw(&ui);
    CHECK(ui.space.cached && ui.space.width == 1000);
    size_t pixel = ((size_t)300 * ui.space.width + 450) * 4;
    CHECK(memcmp(ui.space.pixels + pixel, ui.space.base + pixel, 3));
    uint64_t image = sb_hash((const char *)ui.space.pixels,(size_t)ui.space.width*ui.space.height*4);
    ui.space.points[0].x += 60; sb_ui_draw(&ui);
    CHECK(image != sb_hash((const char *)ui.space.pixels,(size_t)ui.space.width*ui.space.height*4));
    ui.space.solid = true; sb_ui_draw(&ui);
    unsigned char solid[3]; memcpy(solid,ui.space.pixels+pixel,3);
    ui.space.points[0].x -= 60; sb_ui_draw(&ui);
    CHECK(!memcmp(solid,ui.space.pixels+pixel,3));
    CHECK(sb_ui_fonts(&ui, 1.5f).code == SB_OK);
    sb_ui_theme(&ui, true);
    sb_ui_shutdown(&ui);
    printf("%u UI assertions passed: Unicode paste, replacement, undo, redo, capacity, rendering, fonts.\n", checks);
    return 0;
}
