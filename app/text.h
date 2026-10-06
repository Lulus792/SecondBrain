#ifndef SB_TEXT_H
#define SB_TEXT_H
#include "ui.h"
SBStatus sb_ui_text_fonts(SBUi *ui, float scale, float density);
void sb_ui_text_frame_end(SBUi *ui);
void sb_ui_text_free(SBUi *ui);
void sb_ui_text_draw(struct nk_draw_list *list, const struct nk_command_text *command);
#endif
