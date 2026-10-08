#include "grapheme.h"
struct nk_text_edit;
static int sb_ui_grapheme_index(struct nk_text_edit *,int,int);
void sb_ui_grapheme_clamp(struct nk_text_edit *);
static int sb_ui_grapheme_text(struct nk_text_edit *,const char *,int);
#define NK_TEXTEDIT_GRAPHEME_INDEX sb_ui_grapheme_index
#define NK_TEXTEDIT_GRAPHEME_CLAMP sb_ui_grapheme_clamp
#define NK_TEXTEDIT_TEXT_CUSTOM sb_ui_grapheme_text
struct nk_draw_list;
struct nk_command_text;
void sb_ui_text_draw(struct nk_draw_list *, const struct nk_command_text *);
struct nk_context;
struct nk_rect;
struct nk_command_buffer;
struct nk_style_edit;
struct nk_user_font;
struct nk_input;
static void sb_ui_input_caret(struct nk_context *,struct nk_text_edit *);
static void sb_ui_composition_prepare(struct nk_context *,struct nk_text_edit *,unsigned int,unsigned int);
static struct nk_text_edit *sb_ui_composition_display(struct nk_text_edit *);
static void sb_ui_composition_mark(struct nk_command_buffer *,struct nk_text_edit *,const struct nk_style_edit *,const struct nk_user_font *,struct nk_rect,float);
#define NK_TEXTEDIT_PREPARE_CUSTOM sb_ui_composition_prepare
#define NK_TEXTEDIT_DISPLAY_CUSTOM sb_ui_composition_display
#define NK_TEXTEDIT_MARKED_CUSTOM sb_ui_composition_mark
#define NK_TEXTEDIT_CARET_CUSTOM sb_ui_input_caret
static int sb_ui_edit_locate(struct nk_text_edit *,float,float,const struct nk_user_font *,float);
static int sb_ui_edit_key(struct nk_text_edit *,int,int,const struct nk_user_font *,float);
static int sb_ui_edit_render(struct nk_command_buffer *,struct nk_text_edit *,const struct nk_style_edit *,const struct nk_user_font *,struct nk_rect,struct nk_rect,struct nk_rect,unsigned int,struct nk_input *,unsigned int,float,int,int);
#define NK_TEXTEDIT_LOCATE_CUSTOM sb_ui_edit_locate
#define NK_TEXTEDIT_KEY_CUSTOM sb_ui_edit_key
#define NK_TEXTEDIT_RENDER_CUSTOM sb_ui_edit_render
#define NK_DRAW_TEXT_CUSTOM sb_ui_text_draw
#define NK_IMPLEMENTATION
#define NK_SDL3_RENDERER_IMPLEMENTATION
#include "ui.h"
#include "text.h"
#include "styled_text.inc"
#include "edit_geometry.inc"
#define SDL_MAIN_HANDLED
#include <SDL3/SDL_main.h>
#include "platform.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

static struct nk_window *sb_ui_input_owner(struct nk_context *ctx) {
    struct nk_window *owner=ctx->active;
    return owner && owner->popup.active && owner->popup.win ? owner->popup.win : owner;
}
static void sb_ui_input_caret(struct nk_context *ctx,struct nk_text_edit *edit) {
    struct nk_rect caret=edit->caret_bounds,clip=edit->caret_clip;
    SBUi *ui=nk_sdl_get_userdata(ctx).ptr;
    if (!ui || ctx->current!=sb_ui_input_owner(ctx) ||
        clip.w<1 || clip.h<1 || caret.h<=0) return;
    float height=fminf(caret.h,clip.h);
    ui->input_area=(SDL_Rect){(int)floorf(NK_CLAMP(clip.x,caret.x,clip.x+clip.w-1)),
        (int)floorf(NK_CLAMP(clip.y,caret.y,clip.y+clip.h-height)),
        1,(int)ceilf(height)};
    ui->input_area_pending=true;
    ui->input_window=ctx->current;
    ui->input_id=ctx->current->edit.name;
    ui->input_hash=sb_hash(nk_str_get_const(&edit->string),(size_t)nk_str_len_char(&edit->string));
    ui->input_cursor=edit->cursor;ui->input_start=edit->select_start;ui->input_end=edit->select_end;
    ui->input_mode=edit->mode;ui->input_scrollbar=edit->scrollbar;
}
void sb_ui_focus_input(SBUi *ui) {
    struct nk_window *window=ui->input_window;
    if(!ui->input_area_pending || !window || (window->popup.active && window->popup.win))return;
    struct nk_window *root=window->parent ? window->parent : window;
    nk_window_set_focus(ui->ctx,root->name_string);
    /* Nuklear's collector forgets edits when the visible widget count changes.
       The app has already resolved this exact field's stable focus target. */
    if(!window->edit.active || window->edit.name!=ui->input_id) {
        window->edit.active=nk_true;window->edit.name=ui->input_id;
        window->edit.cursor=ui->input_cursor;window->edit.sel_start=ui->input_start;window->edit.sel_end=ui->input_end;
        window->edit.mode=ui->input_mode;
        window->edit.scrollbar.x=(nk_uint)ui->input_scrollbar.x;window->edit.scrollbar.y=(nk_uint)ui->input_scrollbar.y;
    }
}

static int sb_ui_grapheme_index(struct nk_text_edit *edit,int index,int direction) {
    SBGraphemePosition p; index=NK_CLAMP(0,index,edit->string.len);
    if (!sb_grapheme_position(nk_str_get_const(&edit->string),(size_t)nk_str_len_char(&edit->string),(size_t)index,&p)) return index;
    return (int)(direction==-1 ? p.previous : direction==1 ? p.next : direction==-2 ? p.floor : p.ceil);
}
void sb_ui_grapheme_clamp(struct nk_text_edit *edit) {
    if (!edit) return;
    if (edit->select_start==edit->select_end) {
        edit->cursor=sb_ui_grapheme_index(edit,edit->cursor,2);
        edit->select_start=edit->select_end=edit->cursor;
    } else {
        bool forward=edit->select_start<edit->select_end;
        int lo=sb_ui_grapheme_index(edit,NK_MIN(edit->select_start,edit->select_end),-2);
        int hi=sb_ui_grapheme_index(edit,NK_MAX(edit->select_start,edit->select_end),2);
        edit->select_start=forward ? lo : hi; edit->select_end=forward ? hi : lo;
        edit->cursor=edit->select_end;
    }
}

static int sb_ui_grapheme_text(struct nk_text_edit *edit,const char *text,int length) {
    if (!edit || !text || length<=0 || (size_t)length>SB_TEXT_LIMIT || edit->mode==NK_TEXT_EDIT_MODE_VIEW || !sb_utf8_valid(text,(size_t)length)) return 0;
    char *filtered=malloc((size_t)length); if (!filtered) return 0;
    int used=0;
    for (int at=0;at<length;) {
        nk_rune rune; int bytes=nk_utf_decode(text+at,&rune,length-at);
        if (rune!=127 && !(rune=='\n' && edit->single_line) && (!edit->filter || edit->filter(edit,rune))) {
            memcpy(filtered+used,text+at,(size_t)bytes); used+=bytes;
        }
        at+=bytes;
    }
    if (!used) { free(filtered); return 0; }
    int cursor=edit->cursor,start=edit->select_start,end=edit->select_end;
    sb_ui_grapheme_clamp(edit);
    if (edit->mode==NK_TEXT_EDIT_MODE_REPLACE && edit->select_start==edit->select_end) {
        SBGrapheme input,existing; SBGraphemeBoundary b; size_t clusters=0;
        sb_grapheme_init(&input,filtered,(size_t)used);
        while (sb_grapheme_next(&input,&b)) if (b.characters) ++clusters;
        edit->select_start=edit->cursor;
        sb_grapheme_init(&existing,nk_str_get_const(&edit->string),(size_t)nk_str_len_char(&edit->string));
        while (clusters && sb_grapheme_next(&existing,&b)) if (b.characters>(size_t)edit->cursor) { edit->select_end=(int)b.characters; --clusters; }
    }
    int inserted=nk_textedit_paste(edit,filtered,used);
    if (!inserted) { edit->cursor=cursor; edit->select_start=start; edit->select_end=end; }
    if(inserted)edit->visual_valid=0;
    free(filtered);return inserted;
}

#include "composition.inc"

bool sb_ui_edit_geometry(SBUi *ui,struct nk_text_edit *edit,const struct nk_user_font *font,float row,bool display,SBEditGeometryVisitor visitor,void *user){
    if(!ui || !edit || !visitor || sb_ui_font_owner(font)!=ui)return false;
    if(display && ui->composition && ui->composition->active && ui->composition->visual && edit->display_userdata.ptr==ui)edit=ui->composition->visual;
    SBEditPlan *plan=edit_plan(edit,font,row);if(!plan)return false;
    for(size_t i=0;i<plan->count;++i){SBEditLine *line=&plan->lines[i];if(!visitor(user,plan->text+line->byte,line->length,line->byte,&line->shape,&line->carets,i*plan->row))return false;}
    return true;
}


float sb_ui_wrap_height(struct nk_context *ctx,const struct nk_user_font *font,const char *text,size_t length,float width) {
    struct nk_vec2 padding=ctx->style.text.padding;
    float available=width-2*padding.x; if (available<1) available=1;
    int done=0,lines=0; nk_rune separator=' ';
    while ((size_t)done<length) {
        int glyphs=0; float measured=0;
        int fitting=nk_text_clamp(font,text+done,(int)length-done,available,&glyphs,&measured,&separator,1);
        if (fitting<=0) break;
        done+=fitting; ++lines;
    }
    if (!lines) lines=1;
    return lines*(font->height+2*padding.y)+3*padding.y+2;
}
void sb_ui_text_aligned(struct nk_context *ctx,const char *text,size_t length,nk_flags alignment) {
    struct nk_rect bounds; nk_widget(&bounds,ctx);
    const struct nk_user_font *font=ctx->style.font; struct nk_vec2 pad=ctx->style.text.padding;
    float width=bounds.w-2*pad.x,y=bounds.y+pad.y; int done=0; nk_rune separator=' ';
    if (width<1) return;
    while ((size_t)done<length) {
        int glyphs=0; float measured=0;
        int fitting=nk_text_clamp(font,text+done,(int)length-done,width,&glyphs,&measured,&separator,1);
        if (fitting<=0) break;
        measured=font->width(font->userdata,font->height,text+done,fitting);
        float x=bounds.x+pad.x;
        if (alignment&NK_TEXT_ALIGN_RIGHT) x+=NK_MAX(0,width-measured);
        else if (alignment&NK_TEXT_ALIGN_CENTERED) x+=NK_MAX(0,(width-measured)/2);
        nk_draw_text(nk_window_get_canvas(ctx),nk_rect(x,y,width,font->height),text+done,fitting,font,nk_rgba(0,0,0,0),ctx->style.text.color);
        done+=fitting; y+=font->height+2*pad.y;
    }
}
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
    ui->ctx->style.edit.cursor_size=1.5f*ui->scale;
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
    SDL_SetHint(SDL_HINT_IME_IMPLEMENTED_UI,"composition");
    if (!SDL_Init(SDL_INIT_VIDEO)) return sb_error(SB_IO, "Fenstersystem: %s", SDL_GetError());
    ui->window = SDL_CreateWindow("SecondBrain", width, height,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | (testing ? SDL_WINDOW_HIDDEN : 0));
    if (!ui->window) { SDL_Quit(); return sb_error(SB_IO, "Fenster: %s", SDL_GetError()); }
    SDL_SetWindowMinimumSize(ui->window, 780, 520);
    ui->renderer = SDL_CreateRenderer(ui->window, testing ? "software" : NULL);
    if (!ui->renderer) { sb_ui_shutdown(ui); return sb_error(SB_IO, "Darstellung: %s", SDL_GetError()); }
    ui->text_cursor=SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);ui->caret_epoch=SDL_GetTicksNS();
    ui->ctx = nk_sdl_init(ui->window, ui->renderer, nk_sdl_allocator());
    nk_sdl_set_userdata(ui->ctx,nk_handle_ptr(ui));
    ui->ctx->clip.paste = paste;
    result = sb_ui_fonts(ui, 1);
    if (result.code != SB_OK) { sb_ui_shutdown(ui); return result; }
    sb_ui_theme(ui, true);
    return sb_ok();
}

void sb_ui_event(SBUi *ui, const SDL_Event *event) {
    if(sb_ui_composition_event(ui,event))return;
    if(event->type==SDL_EVENT_KEY_DOWN && ui->input_area_applied && composition_edit_key(&event->key))
        sb_ui_input_barrier(ui);
    if(event->type==SDL_EVENT_KEY_DOWN || event->type==SDL_EVENT_TEXT_INPUT || event->type==SDL_EVENT_MOUSE_BUTTON_DOWN)ui->caret_epoch=SDL_GetTicksNS();
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

static size_t hint_piece(SBUi *ui,const char *text,size_t length,float width) {
    float measured=0;size_t count=sb_ui_text_fit(&ui->normal->handle,text,length,width,&measured);
    if (!count && length) {SBGrapheme g;SBGraphemeBoundary boundary;sb_grapheme_init(&g,text,length);sb_grapheme_next(&g,&boundary);if(sb_grapheme_next(&g,&boundary))count=boundary.byte;}
    if (count<length) {size_t space=0;for(size_t i=0;i<count;++i)if(text[i]==' ')space=i+1;if(space)count=space;}
    return count;
}
float sb_ui_hint_height(SBUi *ui,const char *text,float width) {
    size_t length=strlen(text),at=0;unsigned lines=0;
    while(at<length){size_t n=hint_piece(ui,text+at,length-at,width);if(!n)break;at+=n;while(at<length && text[at]==' ')++at;++lines;}
    return (lines ? lines : 1)*(ui->normal->handle.height+4*ui->scale);
}
void sb_ui_hint_draw(SBUi *ui,struct nk_rect bounds,const char *text,float text_width,const char *shortcut) {
    struct nk_context *ctx=ui->ctx;struct nk_command_buffer *canvas=&ctx->overlay;float s=ui->scale;
    nk_command_buffer_init(canvas,&ctx->memory,NK_CLIPPING_OFF);nk_start_buffer(ctx,canvas);
    struct nk_color background=ui->dark ? nk_rgb(22,34,51) : nk_rgb(246,249,253);
    nk_fill_rect(canvas,bounds,7*s,background);nk_stroke_rect(canvas,bounds,7*s,ui->contrast ? 2 : 1,ui->contrast ? ctx->style.text.color : ui->dark ? nk_rgba(165,187,216,130) : nk_rgba(80,100,130,120));
    const struct nk_user_font *font=&ui->normal->handle;float y=bounds.y+6*s;size_t at=0,length=strlen(text);
    while(at<length && y+font->height<=bounds.y+bounds.h-4*s) {
        size_t n=hint_piece(ui,text+at,length-at,text_width);if(!n)break;
        nk_draw_text(canvas,nk_rect(bounds.x+10*s,y,text_width,font->height),text+at,(int)n,font,nk_rgba(0,0,0,0),ctx->style.text.color);
        at+=n;while(at<length && text[at]==' ')++at;y+=font->height+4*s;
    }
    if(shortcut) {
        float width=font->width(font->userdata,font->height,shortcut,(int)strlen(shortcut))+12*s;
        struct nk_rect badge=nk_rect(bounds.x+bounds.w-width-10*s,bounds.y+(bounds.h-font->height-4*s)/2,width,font->height+4*s);
        nk_fill_rect(canvas,badge,4*s,ui->dark ? nk_rgba(170,190,220,35) : nk_rgba(60,85,120,22));
        nk_draw_text(canvas,nk_rect(badge.x+6*s,badge.y+2*s,width-12*s,font->height),shortcut,(int)strlen(shortcut),font,nk_rgba(0,0,0,0),ctx->style.text.color);
    }
    nk_finish_buffer(ctx,canvas);
}
void sb_ui_transition_begin(SBUi *ui) {
    if (!ui->outgoing_texture) return;
    ui->transition=0;ui->transitioning=true;
}
void sb_ui_transition_tick(SBUi *ui,float seconds,bool reduced_motion) {
    if (!ui->transitioning) return;
    ui->transition=reduced_motion ? 1 : fminf(1,ui->transition+seconds/0.18f);
    if (ui->transition>=1) {SDL_DestroyTexture(ui->outgoing_texture);ui->outgoing_texture=NULL;ui->transitioning=false;}
}
void sb_ui_draw(SBUi *ui) {
    /* Nuklear uses window coordinates, independent of backing-pixel density. */
    struct nk_window *owner=sb_ui_input_owner(ui->ctx);
    if (ui->input_area_pending && owner && owner==ui->input_window && owner->edit.active) {
        SDL_Rect r=ui->input_area,last=ui->applied_input_area;
        if (!ui->input_area_applied || r.x!=last.x || r.y!=last.y || r.w!=last.w || r.h!=last.h) {
            if (SDL_SetTextInputArea(ui->window,&r,0)) {
                ui->applied_input_area=r;ui->input_area_applied=true;
            }
        }
    } else if (ui->input_area_applied && SDL_SetTextInputArea(ui->window,NULL,0))
        ui->input_area_applied=false;
    ui->input_area_pending=false;
    sb_ui_composition_frame_end(ui);
    int width,height;SDL_GetWindowSize(ui->window,&width,&height);
    /* Keep the outgoing frame on the GPU. Reading the window back to the CPU
       forces a synchronous GPU wait on every note change. */
    SDL_Texture *snapshot=NULL;
    int pixels_w=0,pixels_h=0;
    if(ui->capture_pending && SDL_GetRenderOutputSize(ui->renderer,&pixels_w,&pixels_h)) {
        snapshot=SDL_CreateTexture(ui->renderer,SDL_PIXELFORMAT_RGBA32,SDL_TEXTUREACCESS_TARGET,pixels_w,pixels_h);
        if(snapshot && !SDL_SetRenderTarget(ui->renderer,snapshot)) {SDL_DestroyTexture(snapshot);snapshot=NULL;}
    }
    SDL_SetRenderLogicalPresentation(ui->renderer,width,height,SDL_LOGICAL_PRESENTATION_STRETCH);
    SDL_SetRenderDrawColor(ui->renderer,ui->dark ? 28 : 255,ui->dark ? 29 : 255,ui->dark ? 33 : 255,255);SDL_RenderClear(ui->renderer);
    if (!sb_space_draw(&ui->space,ui->renderer,width,height)) {SDL_SetRenderDrawColor(ui->renderer,7,14,26,255);SDL_RenderClear(ui->renderer);}
    nk_sdl_render(ui->ctx,NK_ANTI_ALIASING_ON);sb_ui_text_frame_end(ui);edit_cache_frame_end(ui);
    if (ui->transitioning && ui->outgoing_texture) {
        SDL_FRect dest=ui->outgoing_bounds,source={dest.x*ui->snapshot_width/width,dest.y*ui->snapshot_height/height,dest.w*ui->snapshot_width/width,dest.h*ui->snapshot_height/height};
        float opacity=1-ui->transition;opacity=opacity*opacity;SDL_SetTextureAlphaModFloat(ui->outgoing_texture,opacity);
        SDL_RenderTexture(ui->renderer,ui->outgoing_texture,&source,&dest);
    }
    if (ui->capture_pending) {
        ui->capture_pending=false;
        if (snapshot) {
            SDL_SetRenderTarget(ui->renderer,NULL);
            SDL_SetRenderLogicalPresentation(ui->renderer,width,height,SDL_LOGICAL_PRESENTATION_STRETCH);
            SDL_SetTextureBlendMode(snapshot,SDL_BLENDMODE_NONE);
            SDL_RenderTexture(ui->renderer,snapshot,NULL,NULL);
            SDL_DestroyTexture(ui->outgoing_texture);ui->outgoing_texture=snapshot;
            ui->snapshot_width=pixels_w;ui->snapshot_height=pixels_h;ui->outgoing_bounds=ui->card_bounds;
            SDL_SetTextureBlendMode(snapshot,SDL_BLENDMODE_BLEND);
        } else {
            /* Never blend a stale note if allocation or target setup failed. */
            SDL_DestroyTexture(ui->outgoing_texture);ui->outgoing_texture=NULL;ui->transitioning=false;
        }
    }
}

void sb_ui_reset_editor(SBUi *ui) {
    sb_ui_composition_cancel(ui);
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
    sb_ui_composition_free(ui);
    SDL_DestroyTexture(ui->outgoing_texture);
    if(ui->text_cursor){SDL_SetCursor(SDL_GetDefaultCursor());SDL_DestroyCursor(ui->text_cursor);}
    sb_space_free(&ui->space);
    sb_ui_text_free(ui);
    if (ui->ctx) nk_sdl_shutdown(ui->ctx);
    if (ui->renderer) SDL_DestroyRenderer(ui->renderer);
    if (ui->window) SDL_DestroyWindow(ui->window);
    SDL_Quit();
    memset(ui, 0, sizeof(*ui));
}
