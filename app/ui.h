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
#include "inline.h"
#include "shaped.h"
#include "carets.h"

void sb_ui_grapheme_clamp(struct nk_text_edit *edit);
typedef struct SBTextSystem SBTextSystem;
typedef struct SBStyledCache SBStyledCache;
typedef struct SBComposition SBComposition;
typedef struct SBEditCache SBEditCache;
typedef struct {
    SBTextSystem *text;
    SBStyledCache *styled_cache;
    bool styled_cache_disabled;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Cursor *text_cursor;
    Uint64 caret_epoch;
    SDL_Rect input_area;
    bool input_area_pending;
    SDL_Rect applied_input_area;
    bool input_area_applied;
    struct nk_window *input_window;
    nk_hash input_id;
    uint64_t input_hash;
    int input_cursor,input_start,input_end;
    unsigned char input_mode;
    struct nk_vec2 input_scrollbar;
    SBComposition *composition;
    SBEditCache *edit_cache;
    SBStatus input_status;
    bool pointer_text;
    struct nk_context *ctx;
    struct nk_font *normal, *body, *heading, *code;
    char font_path[SB_PATH_CAP];
    float scale, density;
    bool dark, testing,contrast;
    SBSpace space;
    SDL_Texture *outgoing_texture;
    int snapshot_width,snapshot_height;
    SDL_FRect card_bounds,outgoing_bounds;
    float transition;
    bool transitioning,capture_pending;
} SBUi;

SBStatus sb_ui_init(SBUi *ui, const char *font_path, int width, int height, bool testing);
SBStatus sb_ui_fonts(SBUi *ui, float scale);
void sb_ui_theme(SBUi *ui, bool dark);
void sb_ui_event(SBUi *ui, const SDL_Event *event);
bool sb_ui_composition_event(SBUi *ui,const SDL_Event *event);
void sb_ui_composition_cancel(SBUi *ui);
void sb_ui_composition_replay(SBUi *ui,void (*dispatch)(void *,const SDL_Event *),void *user);
bool sb_ui_composition_active(const SBUi *ui);
bool sb_ui_composition_committing(const SBUi *ui);
bool sb_ui_composition_working(const SBUi *ui);
void sb_ui_input_barrier(SBUi *ui);
void sb_ui_input_layout_changed(SBUi *ui);
void sb_ui_focus_input(SBUi *ui);
void sb_ui_draw(SBUi *ui);
void sb_ui_transition_begin(SBUi *ui);
void sb_ui_transition_tick(SBUi *ui,float seconds,bool reduced_motion);
float sb_ui_hint_height(SBUi *ui,const char *text,float width);
void sb_ui_hint_draw(SBUi *ui,struct nk_rect bounds,const char *text,float text_width,const char *shortcut);
void sb_ui_text_aligned(struct nk_context *ctx,const char *text,size_t length,nk_flags alignment);
float sb_ui_wrap_height(struct nk_context *ctx,const struct nk_user_font *font,const char *text,size_t length,float width);
/* Returned spans are owned by the caller; text remains borrowed. */
bool sb_ui_styled_spans(const SBStyledText *text,SBTextSpan **spans,size_t *count);
void sb_ui_styled_cache_clear(SBUi *ui);
float sb_ui_styled_height(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,float width);
void sb_ui_styled_draw(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text);
void sb_ui_styled_aligned(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,nk_flags alignment);
/* Geometry is borrowed only for the callback; offsets map paragraph bytes
   back to the complete displayed text. Positions are local window points. */
/* Borrowed line glyphs/runs are valid only during the callback. Glyph geometry
   is in backing pixels; y is local window points. source_offset + glyph.byte
   addresses the styled display text, not the original Markdown source. */
typedef bool (*SBStyledGeometryVisitor)(void *user,const SBShapedLine *line,size_t source_offset,float y);
/* Plain wrapped source slices keep paragraph context and explicit terminal lines. */
bool sb_ui_wrap_geometry(SBUi *ui,const struct nk_user_font *font,const char *text,size_t length,float width,SBStyledGeometryVisitor visitor,void *user);
bool sb_ui_styled_geometry(SBUi *ui,const struct nk_user_font *base,const SBStyledText *text,float width,SBStyledGeometryVisitor visitor,void *user);
SBStatus sb_ui_capture(SBUi *ui, const char *path);
void sb_ui_shutdown(SBUi *ui);
void sb_ui_reset_editor(SBUi *ui);
void sb_ui_edit_cache_clear(SBUi *ui);
/* Borrowed source and geometry only during the callback; x/glyph metrics in
   backing pixels, y in local window points. display includes current IME. */
typedef bool (*SBEditGeometryVisitor)(void *user,const char *source,size_t length,
    size_t offset,const SBShapedLine *line,const SBCaretPlan *carets,float y);
bool sb_ui_edit_geometry(SBUi *ui,struct nk_text_edit *edit,const struct nk_user_font *font,
    float row,bool display,SBEditGeometryVisitor visitor,void *user);

#endif
