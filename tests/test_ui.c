#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned checks;
static const char *expected_line;
static bool complete_line;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr, "FAIL %d: %s\n", __LINE__, #x); return 1; } } while (0)
static bool equal(struct nk_text_edit *edit, const char *text) {
    int length = nk_str_len_char(&edit->string);
    return (size_t)length == strlen(text) && !memcmp(nk_str_get_const(&edit->string), text, (size_t)length);
}
static bool inspect_line(void *user,const char *text,size_t length,size_t offset,const SBShapedLine *line,const SBCaretPlan *carets,float y){
    (void)user;(void)offset;(void)y;if(length==strlen(expected_line) && !memcmp(text,expected_line,length) && line->count && carets->count)complete_line=true;return true;
}
static void editor_frame(SBUi *ui,struct nk_text_edit *edit,enum nk_keys key,bool down,bool shift,const char *text) {
    nk_input_begin(ui->ctx);
    nk_input_key(ui->ctx,NK_KEY_SHIFT,shift); if (key!=NK_KEY_NONE) nk_input_key(ui->ctx,key,down);
    if (text) { SDL_Event event={0}; event.type=SDL_EVENT_TEXT_INPUT; event.text.text=text; event.text.windowID=SDL_GetWindowID(ui->window); sb_ui_event(ui,&event); }
    nk_input_end(ui->ctx);
    if (nk_begin(ui->ctx,"Grapheme",nk_rect(0,0,1000,720),NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(ui->ctx,400,1); nk_edit_focus(ui->ctx,NK_EDIT_BOX);
        nk_edit_buffer(ui->ctx,NK_EDIT_BOX,edit,nk_filter_default);
    }
    nk_end(ui->ctx);
    if (expected_line) {
        complete_line=false;
        sb_ui_edit_geometry(ui,edit,ui->ctx->style.font,ui->ctx->style.font->height+ui->ctx->style.edit.row_padding,false,inspect_line,NULL);
    }
    sb_ui_draw(ui); SDL_RenderPresent(ui->renderer);
}
static void editor_key(SBUi *ui,struct nk_text_edit *edit,enum nk_keys key,bool shift) {
    editor_frame(ui,edit,key,true,shift,NULL); editor_frame(ui,edit,key,false,false,NULL);
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

    const char *clusters[]={"e\xcc\x81","👩‍💻","🇩🇪","👍🏽","क्ष","각"};
    for (size_t i=0;i<sizeof(clusters)/sizeof(*clusters);++i) {
        nk_textedit_init_default(&edit); edit.mode=NK_TEXT_EDIT_MODE_INSERT;
        editor_frame(&ui,&edit,NK_KEY_NONE,false,false,NULL);
        CHECK(nk_textedit_paste(&edit,clusters[i],(int)strlen(clusters[i])));
        int scalars=edit.string.len; CHECK(scalars>1);
        editor_key(&ui,&edit,NK_KEY_LEFT,false); CHECK(edit.cursor==0);
        editor_key(&ui,&edit,NK_KEY_RIGHT,false); CHECK(edit.cursor==scalars);
        editor_key(&ui,&edit,NK_KEY_LEFT,true); CHECK(edit.select_start==scalars && edit.select_end==0);
        editor_key(&ui,&edit,NK_KEY_BACKSPACE,false); CHECK(equal(&edit,""));
        nk_textedit_undo(&edit); CHECK(equal(&edit,clusters[i]));
        edit.cursor=edit.select_start=edit.select_end=0;
        editor_key(&ui,&edit,NK_KEY_DEL,false); CHECK(equal(&edit,""));
        nk_textedit_undo(&edit); CHECK(equal(&edit,clusters[i]));
        edit.select_start=1; edit.select_end=2; sb_ui_grapheme_clamp(&edit);
        CHECK(edit.select_start==0 && edit.select_end==scalars);
        nk_textedit_free(&edit);
    }
    nk_textedit_init_default(&edit); edit.mode=NK_TEXT_EDIT_MODE_INSERT;
    editor_frame(&ui,&edit,NK_KEY_NONE,false,false,NULL);
    editor_frame(&ui,&edit,NK_KEY_NONE,false,false,"e\xcc\x81"); CHECK(equal(&edit,"e\xcc\x81"));
    nk_textedit_undo(&edit); CHECK(equal(&edit,"")); nk_textedit_redo(&edit); CHECK(equal(&edit,"e\xcc\x81"));
    edit.mode=NK_TEXT_EDIT_MODE_REPLACE; edit.cursor=edit.select_start=edit.select_end=0;
    nk_textedit_text(&edit,"Q",1); CHECK(equal(&edit,"Q")); nk_textedit_undo(&edit); CHECK(equal(&edit,"e\xcc\x81"));
    nk_textedit_free(&edit);
    nk_textedit_init_default(&edit); edit.mode=NK_TEXT_EDIT_MODE_INSERT;
    editor_frame(&ui,&edit,NK_KEY_NONE,false,false,NULL);
    const char *lines[]={"Hangul: 각","Verbindung: क्ष","Emoji: 👩‍💻 🇩🇪 👍🏽","Ligatur: لا","Akzent: e\xcc\x81"};
    for (size_t i=0;i<sizeof(lines)/sizeof(*lines);++i) {
        nk_textedit_select_all(&edit); CHECK(nk_textedit_paste(&edit,lines[i],(int)strlen(lines[i])));
        expected_line=lines[i]; editor_frame(&ui,&edit,NK_KEY_NONE,false,false,NULL); CHECK(complete_line);
    }
    expected_line=NULL; nk_textedit_free(&edit);

    /* Real SDL input-method anchor follows the rendered insertion point,
       including selection, multiline scrolling and font scaling. */
    nk_textedit_init_default(&edit); edit.mode=NK_TEXT_EDIT_MODE_INSERT;
    editor_frame(&ui,&edit,NK_KEY_NONE,false,false,NULL);
    SDL_Rect anchor={0}; int offset=-1;
    CHECK(SDL_GetTextInputArea(ui.window,&anchor,&offset));
    CHECK(anchor.w==1 && anchor.h>0 && offset==0);
    int left=anchor.x,top=anchor.y;
    editor_frame(&ui,&edit,NK_KEY_NONE,false,false,"Hello");
    CHECK(SDL_GetTextInputArea(ui.window,&anchor,&offset));
    CHECK(anchor.x>left && anchor.y==top);
    int right=anchor.x;
    editor_key(&ui,&edit,NK_KEY_LEFT,false);
    CHECK(SDL_GetTextInputArea(ui.window,&anchor,&offset) && anchor.x<right);
    editor_key(&ui,&edit,NK_KEY_LEFT,true);
    CHECK(SDL_GetTextInputArea(ui.window,&anchor,&offset) && anchor.x<right);
    CHECK(nk_textedit_paste(&edit,"\nNext",5));
    editor_frame(&ui,&edit,NK_KEY_NONE,false,false,NULL);
    CHECK(SDL_GetTextInputArea(ui.window,&anchor,&offset) && anchor.y>top);
    edit.scrollbar.y=10;
    editor_frame(&ui,&edit,NK_KEY_NONE,false,false,NULL);
    CHECK(SDL_GetTextInputArea(ui.window,&anchor,&offset));
    CHECK(anchor.x>=0 && anchor.y>=0 && anchor.x<1000 && anchor.y+anchor.h<=720);
    CHECK(sb_ui_fonts(&ui,1.5f).code==SB_OK);
    editor_frame(&ui,&edit,NK_KEY_NONE,false,false,NULL);
    CHECK(SDL_GetTextInputArea(ui.window,&anchor,&offset));
    CHECK(anchor.h>20 && anchor.h<60); /* Window points, never backing pixels. */
    CHECK(sb_ui_fonts(&ui,1).code==SB_OK);
    nk_textedit_free(&edit);

    nk_input_begin(ui.ctx); nk_input_end(ui.ctx);
    nk_begin(ui.ctx,"Grapheme",nk_rect(0,0,1000,720),NK_WINDOW_NO_SCROLLBAR);
    nk_edit_unfocus(ui.ctx); nk_layout_row_dynamic(ui.ctx,40,1);
    char field[32]="Name";
    nk_edit_focus(ui.ctx,NK_EDIT_FIELD);
    nk_edit_string_zero_terminated(ui.ctx,NK_EDIT_FIELD,field,sizeof(field),nk_filter_default);
    nk_end(ui.ctx); sb_ui_draw(&ui); SDL_RenderPresent(ui.renderer);
    CHECK(SDL_GetTextInputArea(ui.window,&anchor,&offset) && anchor.w==1 && anchor.h>0);
    /* Opening a popup after painting its parent must discard that parent anchor. */
    nk_input_begin(ui.ctx); nk_input_end(ui.ctx);
    nk_begin(ui.ctx,"Grapheme",nk_rect(0,0,1000,720),NK_WINDOW_NO_SCROLLBAR);
    nk_layout_row_dynamic(ui.ctx,40,1); nk_edit_focus(ui.ctx,NK_EDIT_FIELD);
    nk_edit_string_zero_terminated(ui.ctx,NK_EDIT_FIELD,field,sizeof(field),nk_filter_default);
    CHECK(nk_popup_begin(ui.ctx,NK_POPUP_STATIC,"Eingabe-Popup",NK_WINDOW_NO_SCROLLBAR,nk_rect(100,100,300,150)));
    nk_layout_row_dynamic(ui.ctx,30,1); nk_label(ui.ctx,"Auswahl",NK_TEXT_LEFT);
    nk_popup_end(ui.ctx); nk_end(ui.ctx); nk_sdl_update_TextInput(ui.ctx);
    sb_ui_draw(&ui); SDL_RenderPresent(ui.renderer);
    CHECK(SDL_GetTextInputArea(ui.window,&anchor,&offset) && anchor.w==0 && anchor.h==0);
    CHECK(!SDL_TextInputActive(ui.window));
    nk_input_begin(ui.ctx); nk_input_end(ui.ctx);
    nk_begin(ui.ctx,"Grapheme",nk_rect(0,0,1000,720),NK_WINDOW_NO_SCROLLBAR);
    CHECK(nk_popup_begin(ui.ctx,NK_POPUP_STATIC,"Eingabe-Popup",NK_WINDOW_NO_SCROLLBAR,nk_rect(100,100,300,150)));
    nk_popup_close(ui.ctx); nk_popup_end(ui.ctx);
    nk_layout_row_dynamic(ui.ctx,40,1); nk_edit_focus(ui.ctx,NK_EDIT_FIELD);
    nk_edit_string_zero_terminated(ui.ctx,NK_EDIT_FIELD,field,sizeof(field),nk_filter_default);
    nk_end(ui.ctx); nk_sdl_update_TextInput(ui.ctx);
    sb_ui_draw(&ui); SDL_RenderPresent(ui.renderer);
    CHECK(SDL_GetTextInputArea(ui.window,&anchor,&offset) && anchor.w==1 && anchor.h>0);
    CHECK(SDL_TextInputActive(ui.window));
    nk_input_begin(ui.ctx); nk_input_end(ui.ctx);
    nk_begin(ui.ctx,"Grapheme",nk_rect(0,0,1000,720),NK_WINDOW_NO_SCROLLBAR);
    nk_edit_unfocus(ui.ctx); nk_end(ui.ctx); nk_sdl_update_TextInput(ui.ctx);
    sb_ui_draw(&ui); SDL_RenderPresent(ui.renderer);
    CHECK(SDL_GetTextInputArea(ui.window,&anchor,&offset));
    CHECK(anchor.x==0 && anchor.y==0 && anchor.w==0 && anchor.h==0);

    char tiny[4]={0}; nk_textedit_init_fixed(&edit,tiny,sizeof(tiny)); edit.mode=NK_TEXT_EDIT_MODE_INSERT;
    CHECK(nk_textedit_paste(&edit,"e\xcc\x81",3)); edit.select_start=1;edit.select_end=2;
    CHECK(!nk_textedit_paste(&edit,"long",4)); CHECK(equal(&edit,"e\xcc\x81") && edit.select_start==1 && edit.select_end==2);
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
