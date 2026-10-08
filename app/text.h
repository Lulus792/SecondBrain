#ifndef SB_TEXT_H
#define SB_TEXT_H
#include "ui.h"
#include "shaped.h"
SBStatus sb_ui_text_fonts(SBUi *ui, float scale, float density);
void sb_ui_text_frame_end(SBUi *ui);
void sb_ui_text_free(SBUi *ui);
void sb_ui_text_draw(struct nk_draw_list *list, const struct nk_command_text *command);
const struct nk_user_font *sb_ui_text_style(SBUi *ui,const struct nk_user_font *base,unsigned style);
size_t sb_ui_text_fit(const struct nk_user_font *font,const char *value,size_t length,float available,float *measured);
bool sb_ui_text_metrics(const struct nk_user_font *font,const char *value,size_t length,float *ascent,float *descent);
SBUi *sb_ui_font_owner(const struct nk_user_font *font);
TTF_Font *sb_ui_cluster_font(const struct nk_user_font *font,const char *value,size_t length,TTF_Font *previous);
/* Immutable font bytes/configuration, captured on the UI thread. Worker
   instances open/use/close their own fonts on one owning thread. */
typedef struct SBFontSnapshot SBFontSnapshot;
typedef struct SBFontInstance SBFontInstance;
SBFontSnapshot *sb_ui_font_snapshot(const struct nk_user_font *font);
void sb_font_snapshot_free(SBFontSnapshot *snapshot);
SBStatus sb_font_snapshot_open(const SBFontSnapshot *snapshot,SBFontInstance **out);
void sb_font_instance_free(SBFontInstance *instance);
TTF_Font *sb_font_instance_select(SBFontInstance *instance,const char *text,size_t length,TTF_Font *previous);
unsigned sb_font_instance_token(const SBFontInstance *instance,const TTF_Font *font);
TTF_Font *sb_font_snapshot_bind(const SBFontSnapshot *snapshot,SBUi *ui,unsigned token);
bool sb_ui_shaped_draw_canvas(SBUi *ui,struct nk_command_buffer *canvas,const SBShapedLine *line,float x,float baseline,struct nk_color color);
bool sb_ui_shaped_draw(SBUi *ui,const SBShapedLine *line,float x,float baseline,struct nk_color color);
#endif
