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
TTF_Font *sb_ui_cluster_font(const struct nk_user_font *font,const char *value,size_t length,TTF_Font *previous);
bool sb_ui_shaped_draw(SBUi *ui,const SBShapedLine *line,float x,float baseline,struct nk_color color);
#endif
