#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"IME %d: %s\n",__LINE__,#x);return 1;}}while(0)
static bool equal(struct nk_text_edit *edit,const char *s){int n=nk_str_len_char(&edit->string);return (size_t)n==strlen(s) && (!n || !memcmp(nk_str_get_const(&edit->string),s,(size_t)n));}
static SDL_Event editing(SBUi *ui,const char *s,int start,int length){SDL_Event e={0};e.type=SDL_EVENT_TEXT_EDITING;e.edit.windowID=SDL_GetWindowID(ui->window);e.edit.text=s;e.edit.start=start;e.edit.length=length;return e;}
static SDL_Event input(SBUi *ui,const char *s){SDL_Event e={0};e.type=SDL_EVENT_TEXT_INPUT;e.text.windowID=SDL_GetWindowID(ui->window);e.text.text=s;return e;}
static SDL_Event key(SBUi *ui,SDL_Keycode k){SDL_Event e={0};e.type=SDL_EVENT_KEY_DOWN;e.key.windowID=SDL_GetWindowID(ui->window);e.key.key=k;e.key.down=true;return e;}
static bool drawn,captured;
static const char *capture_path;
typedef struct {const char *expected;bool matched;} DisplayProbe;
static bool inspect_display(void *user,const char *text,size_t length,size_t offset,const SBShapedLine *line,const SBCaretPlan *carets,float y){
 DisplayProbe *p=user;(void)offset;(void)y;if(length==strlen(p->expected) && !memcmp(text,p->expected,length) && line->count && carets->count)p->matched=true;return true;
}
static void frame(SBUi *ui,struct nk_text_edit *edit,const SDL_Event *events,size_t count,const char *expected){
 nk_input_begin(ui->ctx);SDL_Event queued;while(SDL_PollEvent(&queued))sb_ui_event(ui,&queued);
 for(size_t i=0;i<count;++i)sb_ui_event(ui,&events[i]);nk_input_end(ui->ctx);
 nk_begin(ui->ctx,"IME",nk_rect(0,0,1000,640),NK_WINDOW_NO_SCROLLBAR);
 nk_layout_row_dynamic(ui->ctx,400,1);nk_edit_focus(ui->ctx,NK_EDIT_BOX);nk_edit_buffer(ui->ctx,NK_EDIT_BOX,edit,nk_filter_default);nk_end(ui->ctx);
 drawn=false;if(expected){DisplayProbe probe={.expected=expected};sb_ui_edit_geometry(ui,edit,ui->ctx->style.font,ui->ctx->style.font->height+ui->ctx->style.edit.row_padding,true,inspect_display,&probe);drawn=probe.matched;}
 nk_sdl_update_TextInput(ui->ctx);sb_ui_draw(ui);if(expected && capture_path){captured=sb_ui_capture(ui,capture_path).code==SB_OK;capture_path=NULL;}SDL_RenderPresent(ui->renderer);
}
int main(int argc,char **argv){
 if(argc!=3)return 2;SBUi ui;CHECK(sb_ui_init(&ui,argv[1],1000,640,true).code==SB_OK);struct nk_text_edit edit;nk_textedit_init_default(&edit);edit.mode=NK_TEXT_EDIT_MODE_INSERT;
 CHECK(nk_textedit_paste(&edit,"Before after",12));edit.cursor=edit.select_start=edit.select_end=7;frame(&ui,&edit,NULL,0,NULL);
 capture_path=argv[2];short undo=edit.undo.undo_point;SDL_Rect original,marked;int offset;CHECK(SDL_GetTextInputArea(ui.window,&original,&offset));
 SDL_Event e=editing(&ui,"日本語",3,0);frame(&ui,&edit,&e,1,"Before 日本語after");CHECK(drawn);CHECK(equal(&edit,"Before after"));CHECK(edit.cursor==7 && edit.undo.undo_point==undo);
 CHECK(SDL_GetTextInputArea(ui.window,&marked,&offset) && marked.x>original.x);
 SDL_Rect preedit_start,preedit_selected;e=editing(&ui,"日本語",1,0);frame(&ui,&edit,&e,1,NULL);CHECK(SDL_GetTextInputArea(ui.window,&preedit_start,&offset));
 e=editing(&ui,"日本語",1,1);frame(&ui,&edit,&e,1,NULL);CHECK(SDL_GetTextInputArea(ui.window,&preedit_selected,&offset));CHECK(preedit_start.x==preedit_selected.x && preedit_start.y==preedit_selected.y && equal(&edit,"Before after") && edit.undo.undo_point==undo);
 e=editing(&ui,"日本語",3,0);frame(&ui,&edit,&e,1,NULL);
 e=key(&ui,SDLK_LEFT);frame(&ui,&edit,&e,1,NULL);CHECK(edit.cursor==7 && equal(&edit,"Before after"));
 e=key(&ui,SDLK_RETURN);frame(&ui,&edit,&e,1,NULL);CHECK(equal(&edit,"Before after"));
 CHECK(captured);
 SDL_Event confirm[]={input(&ui,"日本語"),editing(&ui,"",0,0),key(&ui,SDLK_RETURN)};frame(&ui,&edit,confirm,3,NULL);
 CHECK(equal(&edit,"Before 日本語after"));CHECK(edit.undo.undo_point==undo+1);nk_textedit_undo(&edit);CHECK(equal(&edit,"Before after"));nk_textedit_redo(&edit);CHECK(equal(&edit,"Before 日本語after"));frame(&ui,&edit,NULL,0,NULL);
 edit.select_start=7;edit.select_end=10;edit.cursor=10;frame(&ui,&edit,NULL,0,NULL);undo=edit.undo.undo_point;
 e=editing(&ui,"漢字",2,0);frame(&ui,&edit,&e,1,"Before 漢字after");CHECK(drawn);CHECK(equal(&edit,"Before 日本語after"));
 e=key(&ui,SDLK_ESCAPE);frame(&ui,&edit,&e,1,NULL);CHECK(equal(&edit,"Before 日本語after") && edit.undo.undo_point==undo);
 e=editing(&ui,"かな",2,0);frame(&ui,&edit,&e,1,NULL);e=input(&ui,"仮名");frame(&ui,&edit,&e,1,NULL);CHECK(equal(&edit,"Before 仮名after"));nk_textedit_undo(&edit);CHECK(equal(&edit,"Before 日本語after"));frame(&ui,&edit,NULL,0,NULL);
 e=editing(&ui,"한글",1,1);frame(&ui,&edit,&e,1,NULL);CHECK(equal(&edit,"Before 日本語after"));e=editing(&ui,"",0,0);frame(&ui,&edit,&e,1,NULL);CHECK(equal(&edit,"Before 日本語after"));
 edit.cursor=edit.select_start=edit.select_end=edit.string.len;frame(&ui,&edit,NULL,0,NULL);
 SDL_Event next[]={editing(&ui,"あ",1,0),input(&ui,"あ"),editing(&ui,"い",1,0)};frame(&ui,&edit,next,3,"Before 日本語afterあい");CHECK(drawn);CHECK(equal(&edit,"Before 日本語afterあ"));e=input(&ui,"い");frame(&ui,&edit,&e,1,NULL);CHECK(equal(&edit,"Before 日本語afterあい"));
 e=editing(&ui,"e\xcc\x81",2,0);frame(&ui,&edit,&e,1,NULL);SDL_Event lost={0};lost.type=SDL_EVENT_WINDOW_FOCUS_LOST;lost.window.windowID=SDL_GetWindowID(ui.window);frame(&ui,&edit,&lost,1,NULL);CHECK(equal(&edit,"Before 日本語afterあい"));
 e=editing(&ui,"bad",3,0);frame(&ui,&edit,&e,1,NULL);e=input(&ui,"\xc0\x80");frame(&ui,&edit,&e,1,NULL);CHECK(equal(&edit,"Before 日本語afterあい") && ui.input_status.code==SB_INVALID);ui.input_status=sb_ok();
 e=editing(&ui,"old",3,0);frame(&ui,&edit,&e,1,NULL);CHECK(nk_textedit_paste(&edit,"external",8));e=input(&ui,"late");frame(&ui,&edit,&e,1,NULL);CHECK(equal(&edit,"Before 日本語afterあいexternal") && ui.input_status.code==SB_CONFLICT);ui.input_status=sb_ok();
 char *large=malloc(12001);CHECK(large);memset(large,'x',12000);large[12000]=0;e=editing(&ui,large,12000,0);frame(&ui,&edit,&e,1,NULL);CHECK(equal(&edit,"Before 日本語afterあいexternal"));e=input(&ui,large);frame(&ui,&edit,&e,1,NULL);CHECK(nk_str_len_char(&edit.string)==(int)strlen("Before 日本語afterあいexternal")+12000);nk_textedit_select_all(&edit);frame(&ui,&edit,NULL,0,NULL);CHECK(nk_str_len_char(&edit.string)>12000);nk_textedit_undo(&edit);CHECK(equal(&edit,"Before 日本語afterあいexternal"));free(large);
 nk_textedit_free(&edit);sb_ui_composition_cancel(&ui);
 char tiny[5]={0};nk_textedit_init_fixed(&edit,tiny,sizeof(tiny));edit.mode=NK_TEXT_EDIT_MODE_INSERT;CHECK(nk_textedit_paste(&edit,"abc",3));frame(&ui,&edit,NULL,0,NULL);e=editing(&ui,"long",4,0);frame(&ui,&edit,&e,1,"abclong");CHECK(drawn && equal(&edit,"abc"));e=input(&ui,"long");frame(&ui,&edit,&e,1,NULL);CHECK(equal(&edit,"abc") && ui.input_status.code==SB_LIMIT);
 nk_textedit_free(&edit);sb_ui_shutdown(&ui);printf("%u IME assertions passed: inline preedit, original protection, commit, undo, event order, cancellation, bounds.\n",checks);return 0;
}
