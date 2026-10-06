#ifndef SB_UI_H
#define SB_UI_H

#include <SDL3/SDL.h>
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_COMMAND_USERDATA
#define NK_INPUT_MAX 4096
#define NK_TEXTEDIT_UNDOCHARCOUNT 32000
#define NK_TEXTEDIT_UNDOSTATECOUNT 256
#include "nuklear.h"
#include "nuklear_sdl3_renderer.h"
#include "sb.h"
#include "space.h"

typedef struct SBTextSystem SBTextSystem;
typedef struct {
    SBTextSystem *text;
    SDL_Window *window;
    SDL_Renderer *renderer;
    struct nk_context *ctx;
    struct nk_font *normal, *body, *heading, *code;
    char font_path[SB_PATH_CAP];
    float scale, density;
    bool dark, testing,contrast;
    SBSpace space;
} SBUi;

SBStatus sb_ui_init(SBUi *ui, const char *font_path, int width, int height, bool testing);
SBStatus sb_ui_fonts(SBUi *ui, float scale);
void sb_ui_theme(SBUi *ui, bool dark);
void sb_ui_event(SBUi *ui, const SDL_Event *event);
void sb_ui_draw(SBUi *ui);
SBStatus sb_ui_capture(SBUi *ui, const char *path);
void sb_ui_shutdown(SBUi *ui);
void sb_ui_reset_editor(SBUi *ui);

#endif
