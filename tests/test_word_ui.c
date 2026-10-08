#include "text.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"WORD UI %d: %s\n",__LINE__,#x);return 1;}}while(0)
#ifdef __APPLE__
#define WORD_MOD SDL_KMOD_ALT
#define EDIT_MOD SDL_KMOD_GUI
#else
#define WORD_MOD SDL_KMOD_CTRL
#define EDIT_MOD SDL_KMOD_CTRL
#endif
static struct nk_rect field;
static nk_flags flags=NK_EDIT_FIELD;
static void frame(SBUi *ui,struct nk_text_edit *edit,const SDL_Event *e){
 nk_input_begin(ui->ctx);if(e)sb_ui_event(ui,e);nk_input_end(ui->ctx);
 nk_begin(ui->ctx,"Word field",nk_rect(0,0,1000,500),NK_WINDOW_NO_SCROLLBAR);nk_style_set_font(ui->ctx,&ui->normal->handle);nk_layout_row_static(ui->ctx,flags&NK_EDIT_MULTILINE ? 350 : 70,850,1);field=nk_widget_bounds(ui->ctx);nk_edit_focus(ui->ctx,flags);nk_edit_buffer(ui->ctx,flags,edit,nk_filter_default);nk_end(ui->ctx);sb_ui_draw(ui);SDL_RenderPresent(ui->renderer);
}
static void key(SBUi *ui,struct nk_text_edit *edit,SDL_Keycode k,SDL_Keymod mod){SDL_Event e={0};e.type=SDL_EVENT_KEY_DOWN;e.key.key=k;e.key.mod=mod;e.key.windowID=SDL_GetWindowID(ui->window);frame(ui,edit,&e);e.type=SDL_EVENT_KEY_UP;e.key.mod=0;frame(ui,edit,&e);}
static void cursor(SBUi *ui,struct nk_text_edit *edit,int at){edit->cursor=edit->select_start=edit->select_end=at;edit->visual_valid=0;frame(ui,edit,NULL);}
typedef struct {size_t byte;float x;bool found;} Cluster;
static bool cluster(void *user,const char *text,size_t length,size_t offset,const SBShapedLine *line,const SBCaretPlan *carets,float y){Cluster *p=user;(void)text;(void)length;(void)line;(void)y;for(size_t i=0;i<carets->cluster_count;++i)if(offset+carets->clusters[i].byte==p->byte){SBCaretCluster c=carets->clusters[i];p->x=fminf(c.left,c.right)*0.2f+fmaxf(c.left,c.right)*0.8f;p->found=true;}return true;}
int main(int argc,char **argv){CHECK(argc==3);SBUi ui;CHECK(sb_ui_init(&ui,argv[1],1000,500,true).code==SB_OK);ui.ctx->style.edit.padding=nk_vec2(0,0);ui.ctx->style.edit.border=0;
 const char *source="Éther élan can't 3,456.7 👩‍👩‍👧‍👦 τέλος";
#ifdef __APPLE__
 const int right[]={5,11,17,25,33,39};
#else
 const int right[]={6,12,18,26,34,39};
#endif
 const int left[]={34,26,18,12,6,0};
 for(unsigned scale=0;scale<3;++scale){CHECK(sb_ui_fonts(&ui,1+0.5f*scale).code==SB_OK);struct nk_text_edit edit;nk_textedit_init_default(&edit);edit.mode=NK_TEXT_EDIT_MODE_INSERT;CHECK(nk_textedit_paste(&edit,source,(int)strlen(source)));CHECK(edit.string.len==39);cursor(&ui,&edit,0);
  for(unsigned i=0;i<6;++i){key(&ui,&edit,SDLK_RIGHT,WORD_MOD);CHECK(edit.cursor==right[i] && edit.select_start==edit.cursor && edit.select_end==edit.cursor);frame(&ui,&edit,NULL);CHECK(edit.cursor==right[i]);}
  for(unsigned i=0;i<6;++i){key(&ui,&edit,SDLK_LEFT,WORD_MOD);CHECK(edit.cursor==left[i]);}
  key(&ui,&edit,SDLK_LEFT,WORD_MOD);CHECK(edit.cursor==0);
  key(&ui,&edit,SDLK_RIGHT,WORD_MOD|SDL_KMOD_SHIFT);CHECK(edit.select_start==0 && edit.select_end==right[0]);
  key(&ui,&edit,SDLK_RIGHT,WORD_MOD|SDL_KMOD_SHIFT);CHECK(edit.select_start==0 && edit.select_end==right[1]);
  key(&ui,&edit,SDLK_LEFT,WORD_MOD|SDL_KMOD_SHIFT);CHECK(edit.select_start==0 && edit.select_end==6);
  key(&ui,&edit,SDLK_LEFT,WORD_MOD);CHECK(edit.cursor==0 && edit.select_start==edit.select_end);
  key(&ui,&edit,SDLK_RIGHT,WORD_MOD|SDL_KMOD_SHIFT);key(&ui,&edit,SDLK_RIGHT,WORD_MOD);CHECK(edit.cursor==right[0] && edit.select_start==edit.select_end);
  cursor(&ui,&edit,1);Cluster point={.byte=strlen("Éther éla")};CHECK(sb_ui_edit_geometry(&ui,&edit,&ui.normal->handle,70,true,cluster,&point));CHECK(point.found);
  SDL_Event click={0};click.type=SDL_EVENT_MOUSE_BUTTON_DOWN;click.button.button=SDL_BUTTON_LEFT;click.button.down=true;click.button.clicks=2;click.button.windowID=SDL_GetWindowID(ui.window);click.button.x=field.x+point.x/ui.density-edit.scrollbar.x;click.button.y=edit.caret_bounds.y+edit.caret_bounds.h/2;frame(&ui,&edit,&click);CHECK(edit.select_start==6 && edit.select_end==11 && edit.cursor==11);
  click.type=SDL_EVENT_MOUSE_BUTTON_UP;click.button.down=false;frame(&ui,&edit,&click);CHECK(edit.select_start==6 && edit.select_end==11);
  CHECK(ui.ctx->input.mouse.pos.x==click.button.x && ui.ctx->input.mouse.pos.y==click.button.y);
  CHECK((size_t)nk_str_len_char(&edit.string)==strlen(source) && !memcmp(nk_str_get_const(&edit.string),source,strlen(source)));
  key(&ui,&edit,SDLK_BACKSPACE,0);const char *removed="Éther  can't 3,456.7 👩‍👩‍👧‍👦 τέλος";
  CHECK((size_t)nk_str_len_char(&edit.string)==strlen(removed) && !memcmp(nk_str_get_const(&edit.string),removed,strlen(removed)));
  key(&ui,&edit,SDLK_Z,EDIT_MOD);CHECK((size_t)nk_str_len_char(&edit.string)==strlen(source) && !memcmp(nk_str_get_const(&edit.string),source,strlen(source)));
  cursor(&ui,&edit,6);key(&ui,&edit,SDLK_RIGHT,WORD_MOD);CHECK(edit.cursor==right[1]);
  int before=edit.cursor;SDL_Event other={0};other.type=SDL_EVENT_KEY_DOWN;other.key.key=SDLK_RIGHT;other.key.mod=WORD_MOD;frame(&ui,&edit,&other);CHECK(edit.cursor==before);
  click.type=SDL_EVENT_MOUSE_BUTTON_DOWN;click.button.down=true;frame(&ui,&edit,&click);click.type=SDL_EVENT_MOUSE_BUTTON_UP;click.button.down=false;frame(&ui,&edit,&click);CHECK(edit.select_start==6 && edit.select_end==11);
  if(scale==2)CHECK(sb_ui_capture(&ui,argv[2]).code==SB_OK);nk_textedit_free(&edit);
 }
 flags=NK_EDIT_EDITOR;struct nk_text_edit edit;nk_textedit_init_default(&edit);edit.mode=NK_TEXT_EDIT_MODE_INSERT;const char *multi="alpha\nÉther élan\nomega";CHECK(nk_textedit_paste(&edit,multi,(int)strlen(multi)));cursor(&ui,&edit,9);
 key(&ui,&edit,SDLK_LEFT,WORD_MOD);CHECK(edit.cursor==6);key(&ui,&edit,SDLK_LEFT,WORD_MOD);CHECK(edit.cursor==0);
#ifdef __APPLE__
 key(&ui,&edit,SDLK_RIGHT,WORD_MOD);CHECK(edit.cursor==5);key(&ui,&edit,SDLK_RIGHT,WORD_MOD);CHECK(edit.cursor==11);
#else
 key(&ui,&edit,SDLK_RIGHT,WORD_MOD);CHECK(edit.cursor==6);key(&ui,&edit,SDLK_RIGHT,WORD_MOD);CHECK(edit.cursor==12);
#endif
#ifdef __APPLE__
 cursor(&ui,&edit,9);
 key(&ui,&edit,SDLK_RIGHT,SDL_KMOD_GUI);CHECK(edit.cursor==17);key(&ui,&edit,SDLK_LEFT,SDL_KMOD_GUI);CHECK(edit.cursor==6);
 cursor(&ui,&edit,9);key(&ui,&edit,SDLK_RIGHT,SDL_KMOD_GUI|SDL_KMOD_SHIFT);CHECK(edit.select_start==9 && edit.select_end==17);
#endif
 CHECK((size_t)nk_str_len_char(&edit.string)==strlen(multi) && !memcmp(nk_str_get_const(&edit.string),multi,strlen(multi)));nk_textedit_free(&edit);
 sb_ui_shutdown(&ui);printf("%u Unicode word/selection/double-click/platform-key assertions passed.\n",checks);return 0;}
