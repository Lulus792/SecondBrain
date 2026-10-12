#include "grapheme.h"
#include "word.h"
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
struct nk_text;
static int sb_ui_plain_wrap(struct nk_command_buffer *,struct nk_rect,const char *,int,const struct nk_text *,const struct nk_user_font *);
#define NK_WIDGET_TEXT_WRAP_CUSTOM sb_ui_plain_wrap
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
static int sb_ui_edit_select_word(struct nk_text_edit *,const struct nk_user_font *,float,float,float);
static int sb_ui_edit_render(struct nk_command_buffer *,struct nk_text_edit *,const struct nk_style_edit *,const struct nk_user_font *,struct nk_rect,struct nk_rect,struct nk_rect,unsigned int,struct nk_input *,unsigned int,float,int,int);
#define NK_TEXTEDIT_LOCATE_CUSTOM sb_ui_edit_locate
#define NK_TEXTEDIT_KEY_CUSTOM sb_ui_edit_key
#define NK_TEXTEDIT_SELECT_WORD_CUSTOM sb_ui_edit_select_word
#define NK_TEXTEDIT_RENDER_CUSTOM sb_ui_edit_render
static int sb_ui_plain_clamp(const struct nk_user_font *,const char *,int,float,int *,float *,unsigned int *,int);
#define NK_TEXT_CLAMP_CUSTOM sb_ui_plain_clamp
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
static bool edit_native_build(SBUi *ui,const SBEditPlan *p,SBNativeText *out){
    SBNativeCharBox *boxes=NULL;size_t count=0,capacity=0;bool ok=true;
    for(size_t i=0;i<p->count && ok;++i){SBEditLine *l=&p->lines[i];
        float x=0,y=i*p->row;
        size_t extra=l->carets.cluster_count+(l->next>l->last || !l->length ? 1 : 0);
        if(count+extra>capacity){size_t next=capacity ? capacity*2 : 32;if(next<count+extra)next=count+extra;SBNativeCharBox *grown=realloc(boxes,next*sizeof(*grown));if(!grown){ok=false;break;}boxes=grown;capacity=next;}
        for(size_t j=0;j<l->carets.cluster_count;++j){SBCaretCluster c=l->carets.clusters[j];boxes[count++]=(SBNativeCharBox){l->byte+c.byte,c.length,x+c.left/ui->density,y,(c.right-c.left)/ui->density,p->row,c.level};}
        if(l->next>l->last || !l->length){SBCaret c;size_t n=p->bytes[l->next]-p->bytes[l->last];
            if(!sb_caret_find(&l->carets,l->length,SB_CARET_AFTER,l->shape.base_level,&c)){ok=false;break;}
            boxes[count++]=(SBNativeCharBox){l->byte+l->length,n,x+c.x/ui->density,y,0,p->row,l->shape.base_level};}
    }
    if(ok)ok=sb_native_text_geometry(p->text,p->length,NULL,0,boxes,count,out).code==SB_OK;
    free(boxes);return ok;
}
static bool edit_native_copy(const SBNativeText *source,SBNativeText *out){
    SBNativeText copy=*source;copy.capacity=copy.count;copy.runs=malloc(copy.count*sizeof(*copy.runs));if(!copy.runs)return false;
    memcpy(copy.runs,source->runs,copy.count*sizeof(*copy.runs));*out=copy;return true;
}
static bool edit_native_export(SBUi *ui,struct nk_text_edit *edit,SBNativeText *out,bool retain){
    if(!ui || !edit || !out || ui->edit_paint.edit!=edit)return false;
    SBEditPlan *p=edit_plan(edit,ui->edit_paint.font,ui->edit_paint.row);if(!p || p->hash!=ui->edit_paint.hash)return false;
    SBEditNativeCache *memo=ui->edit_native_cache;
    bool reuse=memo && memo->hash==p->hash && memo->length==p->length && memo->font==p->font && memo->row==p->row && memo->density==ui->density && memo->single==p->single && !memcmp(memo->source,p->text,p->length);
    if(reuse){if(!edit_native_copy(&memo->text,out))return false;}
    else{
        if(!edit_native_build(ui,p,out))return false;
        /* The source and actual retained run capacity both count toward the
           budget. Cache allocation failure never rejects a complete export. */
        if(retain && p->length+1+sizeof(SBEditNativeCache)<SB_EDIT_NATIVE_BUDGET && out->count<=(SB_EDIT_NATIVE_BUDGET-p->length-1-sizeof(SBEditNativeCache))/sizeof(*out->runs)){
            SBEditNativeCache *next=calloc(1,sizeof(*next));
            if(next){next->source=malloc(p->length+1);
                if(next->source && edit_native_copy(out,&next->text)){
                    memcpy(next->source,p->text,p->length+1);next->length=p->length;next->hash=p->hash;next->font=p->font;next->row=p->row;next->density=ui->density;next->single=p->single;
                    edit_native_cache_clear(ui);ui->edit_native_cache=next;
                }else{free(next->source);sb_native_text_free(&next->text);free(next);}}
        }
    }
    float x=ui->edit_paint.area.x-ui->edit_paint.scroll.x;
    for(size_t i=0;i<out->count;++i)if(out->runs[i].geometry){out->runs[i].x+=x;out->runs[i].y=ui->edit_paint.area.y+out->runs[i].y-ui->edit_paint.scroll.y;}
    return true;
}
bool sb_ui_edit_native(SBUi *ui,struct nk_text_edit *edit,SBNativeText *out){return edit_native_export(ui,edit,out,true);}
bool sb_ui_field_native(SBUi *ui,struct nk_text_edit *edit,SBNativeText *out){return edit_native_export(ui,edit,out,false);}


/* Fit actual shaped advances and never split a displayed grapheme. The
   separator policy remains the existing logical word-wrap policy. */
static int sb_ui_plain_clamp(const struct nk_user_font *font,const char *value,int length,float space,int *glyphs,float *measured,unsigned int *separators,int count){
    if(!sb_ui_font_owner(font))return -1;
    if(length<=0 || space<=0){*glyphs=0;*measured=0;return 0;}
    size_t fitting=sb_ui_text_fit(font,value,(size_t)length,space,measured);
    if(count>0 && fitting<(size_t)length){
        const char *at=value,*end=value+fitting;size_t last=0;
        while(at<end){uint32_t cp=SDL_StepUTF8(&at,NULL);for(int i=0;i<count;++i)if(cp==separators[i])last=(size_t)(at-value);}
        if(last){SBGrapheme reader;SBGraphemeBoundary b;size_t whole=0;if(sb_grapheme_init(&reader,value,(size_t)length))while(sb_grapheme_next(&reader,&b) && b.byte<=last)whole=b.byte;fitting=whole;}
        /* Wrapping must consume at least one whole unit even in a tiny area. */
        if(!fitting){SBGrapheme reader;SBGraphemeBoundary b;if(sb_grapheme_init(&reader,value,(size_t)length)){sb_grapheme_next(&reader,&b);if(sb_grapheme_next(&reader,&b))fitting=b.byte;}}
        *measured=font->width(font->userdata,font->height,value,(int)fitting);
    }
    *glyphs=nk_utf_len(value,(int)fitting);return (int)fitting;
}
/* The same immutable paragraph plan measures and paints plain wrapped text.
   Callers may pass a nonterminated source slice; the copy is strictly bounded. */
typedef struct {SBStyledText text;SBTextSpan span;} SBWrappedSource;
static bool wrapped_source(const char *value,size_t length,SBWrappedSource *source){
    memset(source,0,sizeof(*source));if(!value || length==SIZE_MAX)return false;
    source->text.text=malloc(length+1);if(!source->text.text)return false;
    memcpy(source->text.text,value,length);source->text.text[length]=0;
    source->span=(SBTextSpan){0,length,0};source->text.spans=length ? &source->span : NULL;source->text.count=length ? 1 : 0;return true;
}
static const StyledPlan *wrapped_plan(SBUi *ui,const struct nk_user_font *font,const SBWrappedSource *source,float width,struct nk_vec2 padding,StyledPlan *fresh){
    const StyledPlan *memo=styled_lookup(ui,font,&source->text,width,padding,2*padding.y,true);
    if(memo)return memo;
    return styled_build(ui,font,&source->text,width,padding,2*padding.y,true,fresh) ? fresh : NULL;
}
static bool wrapped_paint(SBUi *ui,struct nk_command_buffer *canvas,struct nk_rect bounds,const char *value,size_t length,const struct nk_user_font *font,struct nk_vec2 padding,struct nk_color color,nk_flags alignment){
    SBWrappedSource source;StyledPlan fresh={0};if(!wrapped_source(value,length,&source))return false;
    const StyledPlan *plan=wrapped_plan(ui,font,&source,bounds.w,padding,&fresh);
    if(!plan){free(source.text.text);return false;}
    struct nk_rect old=canvas->clip;
    float x=fmaxf(old.x,bounds.x),y=fmaxf(old.y,bounds.y),right=fminf(old.x+old.w,bounds.x+bounds.w),bottom=fminf(old.y+old.h,bounds.y+bounds.h);
    bool ok=true;
    if(right>x && bottom>y){
        nk_push_scissor(canvas,nk_rect(x,y,right-x,bottom-y));float available=fmaxf(1,bounds.w-2*padding.x);
        for(size_t i=0;i<plan->count;++i){const StyledLine *line=&plan->lines[i];float measured=line->shape.advance/ui->density,shift=0;
            if(alignment&NK_TEXT_ALIGN_RIGHT)shift=fmaxf(0,available-measured);else if(alignment&NK_TEXT_ALIGN_CENTERED)shift=fmaxf(0,(available-measured)/2);
            if(!sb_ui_shaped_draw_canvas(ui,canvas,&line->shape,bounds.x+padding.x+shift,bounds.y+padding.y+line->y+line->shape.ascent/ui->density,color))ok=false;
        }
        nk_push_scissor(canvas,old);
    }
    styled_plan_free(&fresh);free(source.text.text);return ok;
}
static int sb_ui_plain_wrap(struct nk_command_buffer *canvas,struct nk_rect bounds,const char *value,int length,const struct nk_text *style,const struct nk_user_font *font){
    SBUi *ui=sb_ui_font_owner(font);if(!ui || !canvas || !style || !value || length<0)return 0;
    /* A resource failure is reported instead of repainting a second layout on
       top of any glyphs that have already been submitted. */
    if(!wrapped_paint(ui,canvas,bounds,value,(size_t)length,font,style->padding,style->text,NK_TEXT_LEFT))ui->input_status=sb_error(SB_MEMORY,"Textlayout konnte nicht vorbereitet werden.");
    return 1;
}
float sb_ui_wrap_height(struct nk_context *ctx,const struct nk_user_font *font,const char *text,size_t length,float width){
    SBUi *ui=sb_ui_font_owner(font);if(!ui || ui->ctx!=ctx)return 0;
    SBWrappedSource source;StyledPlan fresh={0};if(!wrapped_source(text,length,&source))return 0;
    const StyledPlan *plan=wrapped_plan(ui,font,&source,width,ctx->style.text.padding,&fresh);float height=plan ? plan->height : font->height+4*ctx->style.text.padding.y;
    if(!plan)ui->input_status=sb_error(SB_MEMORY,"Textlayout konnte nicht vorbereitet werden.");
    styled_plan_free(&fresh);free(source.text.text);return height;
}
void sb_ui_text_aligned(struct nk_context *ctx,const char *text,size_t length,nk_flags alignment){
    struct nk_rect bounds;if(nk_widget(&bounds,ctx)==NK_WIDGET_INVALID)return;
    const struct nk_user_font *font=ctx->style.font;SBUi *ui=sb_ui_font_owner(font);if(!ui || ui->ctx!=ctx)return;
    if(!wrapped_paint(ui,nk_window_get_canvas(ctx),bounds,text,length,font,ctx->style.text.padding,ctx->style.text.color,alignment))ui->input_status=sb_error(SB_MEMORY,"Textlayout konnte nicht vorbereitet werden.");
}
bool sb_ui_wrap_geometry(SBUi *ui,const struct nk_user_font *font,const char *text,size_t length,float width,SBStyledGeometryVisitor visitor,void *user){
    if(!ui || sb_ui_font_owner(font)!=ui || !visitor)return false;
    SBWrappedSource source;StyledPlan fresh={0};if(!wrapped_source(text,length,&source))return false;
    const StyledPlan *plan=wrapped_plan(ui,font,&source,width,ui->ctx->style.text.padding,&fresh);bool ok=plan!=NULL;
    if(plan)for(size_t i=0;i<plan->count && ok;++i)ok=visitor(user,&plan->lines[i].shape,plan->lines[i].source_offset,plan->lines[i].y);
    styled_plan_free(&fresh);free(source.text.text);return ok;
}
bool sb_ui_wrap_native(SBUi *ui,const struct nk_user_font *font,const char *text,size_t length,const SBTextSpan *styles,size_t style_count,struct nk_rect bounds,SBNativeText *out){
    if(!ui || sb_ui_font_owner(font)!=ui || !ui->ctx || !out || style_count>1 || (style_count && (!styles || styles[0].offset || styles[0].length!=length)) || !isfinite(bounds.x) || !isfinite(bounds.y) || !isfinite(bounds.w) || !isfinite(bounds.h) || bounds.w<=0 || bounds.h<0)return false;
    SBWrappedSource source;StyledPlan fresh={0};if(!wrapped_source(text,length,&source))return false;
    const StyledPlan *plan=wrapped_plan(ui,font,&source,bounds.w,ui->ctx->style.text.padding,&fresh);bool ok=false;
    if(plan){SBStyledText formatted=source.text.text ? (SBStyledText){.text=source.text.text,.spans=(SBTextSpan *)styles,.count=style_count} : source.text;
        if(!styles)formatted=source.text;
        ok=styled_native_export(ui,font,&formatted,plan,bounds,ui->ctx->style.text.padding,NK_TEXT_LEFT,out);}
    styled_plan_free(&fresh);free(source.text.text);return ok;
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

struct nk_color sb_ui_palette_color(const SBUi *ui,unsigned role){SDL_Color c=ui->palette.color[role];return nk_rgb(c.r,c.g,c.b);}
void sb_ui_set_palette(SBUi *ui,SBPalette palette){
    if(!sb_palette_equal(&ui->palette,&palette)){SDL_DestroyTexture(ui->outgoing_texture);ui->outgoing_texture=NULL;ui->transitioning=false;ui->space.cached=false;ui->space.sky_ready=false;}
    ui->palette=ui->space.palette=palette;
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
    bool custom=ui->contrast && ui->palette.valid;
    if(custom){bg=sb_ui_palette_color(ui,SB_P_WINDOW);text=sb_ui_palette_color(ui,SB_P_TEXT);
        for(int i=0;i<NK_COLOR_COUNT;++i)colors[i]=bg;
        colors[NK_COLOR_TEXT]=text;colors[NK_COLOR_BORDER]=text;colors[NK_COLOR_EDIT_CURSOR]=text;
        colors[NK_COLOR_BUTTON]=sb_ui_palette_color(ui,SB_P_BUTTON);
        colors[NK_COLOR_BUTTON_HOVER]=colors[NK_COLOR_BUTTON_ACTIVE]=sb_ui_palette_color(ui,SB_P_HIGHLIGHT);
        colors[NK_COLOR_TOGGLE]=sb_ui_palette_color(ui,SB_P_BUTTON);colors[NK_COLOR_TOGGLE_CURSOR]=text;
        colors[NK_COLOR_SELECT_ACTIVE]=sb_ui_palette_color(ui,SB_P_HIGHLIGHT);
        colors[NK_COLOR_SCROLLBAR_CURSOR]=text;colors[NK_COLOR_SCROLLBAR_CURSOR_HOVER]=colors[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE]=text;
        colors[NK_COLOR_SLIDER_CURSOR]=colors[NK_COLOR_SLIDER_CURSOR_HOVER]=colors[NK_COLOR_SLIDER_CURSOR_ACTIVE]=text;
        colors[NK_COLOR_CHART_COLOR]=text;colors[NK_COLOR_CHART_COLOR_HIGHLIGHT]=text;
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
    if(custom){struct nk_color selected=sb_ui_palette_color(ui,SB_P_HIGHLIGHT),ink=sb_ui_palette_color(ui,SB_P_HIGHLIGHT_TEXT);
        ui->ctx->style.edit.selected_normal=ui->ctx->style.edit.selected_hover=selected;
        ui->ctx->style.edit.selected_text_normal=ui->ctx->style.edit.selected_text_hover=ink;
        ui->ctx->style.button.text_normal=ui->ctx->style.button.border_color=sb_ui_palette_color(ui,SB_P_BUTTON_TEXT);
        ui->ctx->style.button.text_hover=ui->ctx->style.button.text_active=ink;
        ui->ctx->style.selectable.hover=ui->ctx->style.selectable.pressed=nk_style_item_color(selected);
        ui->ctx->style.selectable.text_hover=ui->ctx->style.selectable.text_pressed=ink;
        ui->ctx->style.selectable.text_normal_active=ui->ctx->style.selectable.text_hover_active=ui->ctx->style.selectable.text_pressed_active=ink;
    }
    ui->dark = dark;
}

static SBStatus ui_init(SBUi *ui, const char *font_path, int width, int height, bool testing, bool defer_show) {
    SBStatus result;
    memset(ui, 0, sizeof(*ui));
    if (strlen(font_path) >= sizeof(ui->font_path)) return sb_error(SB_LIMIT, "Schriftpfad zu lang.");
    strcpy(ui->font_path, font_path);
    ui->testing = testing;
    SDL_SetMainReady();
    SDL_SetHint(SDL_HINT_IME_IMPLEMENTED_UI,"composition");
    if (!SDL_Init(SDL_INIT_VIDEO)) return sb_error(SB_IO, "Fenstersystem: %s", SDL_GetError());
    ui->window = SDL_CreateWindow("SecondBrain", width, height,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | ((testing || defer_show) ? SDL_WINDOW_HIDDEN : 0));
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

SBStatus sb_ui_init(SBUi *ui,const char *font_path,int width,int height,bool testing) {
    return ui_init(ui,font_path,width,height,testing,false);
}
SBStatus sb_ui_init_hidden(SBUi *ui,const char *font_path,int width,int height,bool testing) {
    return ui_init(ui,font_path,width,height,testing,true);
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
        if(copy.key.key==SDLK_LEFT || copy.key.key==SDLK_RIGHT){
            if(SDL_GetWindowFromEvent(&copy)!=ui->window)return;
            bool left=copy.key.key==SDLK_LEFT,down=copy.type==SDL_EVENT_KEY_DOWN;
            enum nk_keys plain=left ? NK_KEY_LEFT : NK_KEY_RIGHT,word=left ? NK_KEY_TEXT_WORD_LEFT : NK_KEY_TEXT_WORD_RIGHT,line=left ? NK_KEY_TEXT_LINE_START : NK_KEY_TEXT_LINE_END;
            nk_input_key(ui->ctx,NK_KEY_SHIFT,(copy.key.mod&SDL_KMOD_SHIFT)!=0);
            if(!down){nk_input_key(ui->ctx,plain,false);nk_input_key(ui->ctx,word,false);nk_input_key(ui->ctx,line,false);}
            else{
#ifdef __APPLE__
                enum nk_keys selected=(copy.key.mod&SDL_KMOD_GUI) ? line : (copy.key.mod&(SDL_KMOD_ALT|SDL_KMOD_CTRL)) ? word : plain;
#else
                enum nk_keys selected=(copy.key.mod&SDL_KMOD_CTRL) ? word : plain;
#endif
                nk_input_key(ui->ctx,plain,selected==plain);nk_input_key(ui->ctx,word,selected==word);nk_input_key(ui->ctx,line,selected==line);
            }return;
        }
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
    struct nk_color background=ui->palette.valid && ui->contrast ? sb_ui_palette_color(ui,SB_P_WINDOW) : ui->dark ? nk_rgb(22,34,51) : nk_rgb(246,249,253);
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
        nk_fill_rect(canvas,badge,4*s,ui->palette.valid && ui->contrast ? sb_ui_palette_color(ui,SB_P_WINDOW) : ui->dark ? nk_rgba(170,190,220,35) : nk_rgba(60,85,120,22));
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
typedef struct {struct nk_vec2 position;float alpha;} SBMaterialVertex;
static void material_triangle(SBUi *ui,struct nk_draw_list *list,SBMaterialVertex a,SBMaterialVertex b,SBMaterialVertex c,unsigned depth){
    float left=fminf(a.position.x,fminf(b.position.x,c.position.x)),right=fmaxf(a.position.x,fmaxf(b.position.x,c.position.x));
    float top=fminf(a.position.y,fminf(b.position.y,c.position.y)),bottom=fmaxf(a.position.y,fmaxf(b.position.y,c.position.y));
    /* SDL's pinned software triangle path stores source-coordinate * area in
       a signed int. Bound backing-pixel triangles, including rounding slack. */
    double area=((right-left)*ui->material_pixel_x+2)*((bottom-top)*ui->material_pixel_y+2)*4;
    if(depth<16 && area*fmax(ui->space.width,ui->space.height)>536870912.0){
        float ab=(a.position.x-b.position.x)*(a.position.x-b.position.x)+(a.position.y-b.position.y)*(a.position.y-b.position.y);
        float bc=(b.position.x-c.position.x)*(b.position.x-c.position.x)+(b.position.y-c.position.y)*(b.position.y-c.position.y);
        float ca=(c.position.x-a.position.x)*(c.position.x-a.position.x)+(c.position.y-a.position.y)*(c.position.y-a.position.y);
        if(bc>ab && bc>=ca){SBMaterialVertex temp=a;a=b;b=c;c=temp;}
        else if(ca>ab && ca>=bc){SBMaterialVertex temp=a;a=c;c=b;b=temp;}
        SBMaterialVertex middle={nk_vec2((a.position.x+b.position.x)/2,(a.position.y+b.position.y)/2),(a.alpha+b.alpha)/2};
        material_triangle(ui,list,a,middle,c,depth+1);material_triangle(ui,list,middle,b,c,depth+1);return;
    }
    nk_draw_index base=(nk_draw_index)list->vertex_count;void *vertices=nk_draw_list_alloc_vertices(list,3);nk_draw_index *indices=nk_draw_list_alloc_elements(list,3);
    if(!vertices || !indices)return;SBMaterialVertex points[]={a,b,c};
    for(unsigned i=0;i<3;++i){struct nk_vec2 p=points[i].position;struct nk_colorf color={1,1,1,points[i].alpha};
        vertices=nk_draw_vertex(vertices,&list->config,p,nk_vec2(p.x/ui->material_width,p.y/ui->material_height),color);indices[i]=(nk_draw_index)(base+i);}
}
static void material_panel_draw(void *canvas,short x,short y,unsigned short w,unsigned short h,nk_handle data){
    (void)x;(void)y;(void)w;(void)h;
    SBUi *ui=data.ptr;struct nk_draw_list *list=canvas;struct nk_rect b=ui->material_bounds;
    bool texture=ui->material_ready && ui->space.texture && ui->material_width>0 && ui->material_height>0 &&
        SDL_SetTextureBlendMode(ui->space.texture,SDL_BLENDMODE_BLEND);
    if(!texture){nk_draw_list_fill_rect(list,b,ui->contrast && ui->palette.valid ? sb_ui_palette_color(ui,SB_P_WINDOW) : ui->contrast ? ui->dark ? nk_rgb(0,0,0) : nk_rgb(255,255,255) : ui->dark ? nk_rgb(17,29,47) : nk_rgb(237,245,255),ui->material_rounding);return;}
    /* Rounded shape with opaque interior and an antialiased outer ring. */
    struct nk_vec2 points[32],normals[32];
    nk_draw_list_path_rect_to(list,nk_vec2(b.x,b.y),nk_vec2(b.x+b.w,b.y+b.h),ui->material_rounding);
    unsigned count=list->path_count;if(count<3 || count>32){nk_draw_list_path_clear(list);return;}
    memcpy(points,nk_buffer_memory(list->buffer),count*sizeof(*points));nk_draw_list_path_clear(list);

    nk_draw_list_push_image(list,nk_handle_ptr(ui->space.texture));
    for(unsigned i=0;i<count;++i){unsigned next=(i+1)%count;float dx=points[next].x-points[i].x,dy=points[next].y-points[i].y,length=sqrtf(dx*dx+dy*dy);normals[i]=length>0 ? nk_vec2(dy/length,-dx/length) : nk_vec2(0,0);}
    SBMaterialVertex inner[32],outer[32],center={nk_vec2(b.x+b.w/2,b.y+b.h/2),1};
    for(unsigned i=0;i<count;++i){unsigned previous=(i+count-1)%count;struct nk_vec2 m=nk_vec2((normals[previous].x+normals[i].x)/2,(normals[previous].y+normals[i].y)/2);
        float length=m.x*m.x+m.y*m.y,factor=length>0.000001f ? fminf(100,1/length)*0.5f : 0;m.x*=factor;m.y*=factor;
        inner[i]=(SBMaterialVertex){nk_vec2(points[i].x-m.x,points[i].y-m.y),1};outer[i]=(SBMaterialVertex){nk_vec2(points[i].x+m.x,points[i].y+m.y),0};}
    for(unsigned i=0;i<count;++i){unsigned next=(i+1)%count;
        material_triangle(ui,list,center,inner[i],inner[next],0);
        material_triangle(ui,list,inner[i],outer[i],inner[next],0);
        material_triangle(ui,list,outer[i],outer[next],inner[next],0);}

}
void sb_ui_material_panel(SBUi *ui,struct nk_rect bounds,float rounding){
    if(!ui || !ui->ctx || !ui->ctx->current || !isfinite(bounds.x) || !isfinite(bounds.y) || !isfinite(bounds.w) || !isfinite(bounds.h) || !isfinite(rounding) || bounds.w<=0 || bounds.h<=0)return;
    ui->material_bounds=bounds;ui->material_rounding=fminf(fmaxf(0,rounding),fminf(bounds.w,bounds.h)/2);
    struct nk_command_buffer *canvas=nk_window_get_canvas(ui->ctx);struct nk_rect clip=canvas->clip;
    nk_push_scissor(canvas,bounds);nk_push_custom(canvas,bounds,material_panel_draw,nk_handle_ptr(ui));nk_push_scissor(canvas,clip);
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
    int pixels_w=0,pixels_h=0;bool output_known=SDL_GetRenderOutputSize(ui->renderer,&pixels_w,&pixels_h);
    /* The retained card belongs to its capture layout and backing resolution.
       Reflow must never crop it using another window's dimensions. */
    if(ui->outgoing_texture && (width!=ui->snapshot_logical_width || height!=ui->snapshot_logical_height ||
       (output_known && (pixels_w!=ui->snapshot_width || pixels_h!=ui->snapshot_height)))){
        SDL_DestroyTexture(ui->outgoing_texture);ui->outgoing_texture=NULL;ui->transitioning=false;
    }
    if(ui->capture_pending && output_known) {
        snapshot=SDL_CreateTexture(ui->renderer,SDL_PIXELFORMAT_RGBA32,SDL_TEXTUREACCESS_TARGET,pixels_w,pixels_h);
        if(snapshot && !SDL_SetRenderTarget(ui->renderer,snapshot)) {SDL_DestroyTexture(snapshot);snapshot=NULL;}
    }
    SDL_SetRenderLogicalPresentation(ui->renderer,width,height,SDL_LOGICAL_PRESENTATION_STRETCH);
    SDL_SetRenderDrawColor(ui->renderer,ui->dark ? 28 : 255,ui->dark ? 29 : 255,ui->dark ? 33 : 255,255);SDL_RenderClear(ui->renderer);
    ui->material_width=width;ui->material_height=height;
    int material_pixels_w=width,material_pixels_h=height;SDL_GetCurrentRenderOutputSize(ui->renderer,&material_pixels_w,&material_pixels_h);
    ui->material_pixel_x=fmaxf(1,(float)material_pixels_w/fmaxf(1,width));ui->material_pixel_y=fmaxf(1,(float)material_pixels_h/fmaxf(1,height));
    ui->material_ready=sb_space_draw(&ui->space,ui->renderer,width,height);
    if(!ui->material_ready)sb_space_fallback(&ui->space,ui->renderer);
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
            ui->snapshot_width=pixels_w;ui->snapshot_height=pixels_h;
            ui->snapshot_logical_width=width;ui->snapshot_logical_height=height;ui->outgoing_bounds=ui->card_bounds;
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
