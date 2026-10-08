#include "text.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"EDIT GEOMETRY %d: %s (%s)\n",__LINE__,#x,SDL_GetError());return 1;}}while(0)
#define TAG(a,b,c,d) (((uint32_t)(a)<<24)|((uint32_t)(b)<<16)|((uint32_t)(c)<<8)|(uint32_t)(d))
static uint64_t pixels(SBUi *ui){SDL_Surface *raw=SDL_RenderReadPixels(ui->renderer,NULL),*s=raw ? SDL_ConvertSurface(raw,SDL_PIXELFORMAT_RGBA32) : NULL;uint64_t hash=0;if(s)for(int y=0;y<s->h;++y)hash=hash*1099511628211ULL^sb_hash((char *)s->pixels+y*s->pitch,(size_t)s->w*4);SDL_DestroySurface(s);SDL_DestroySurface(raw);return hash;}
static struct nk_rect field;
static bool exact_caret;
static void begin(SBUi *ui){nk_input_begin(ui->ctx);nk_input_end(ui->ctx);nk_begin(ui->ctx,"Exact field",nk_rect(0,0,1000,500),NK_WINDOW_NO_SCROLLBAR);nk_style_set_font(ui->ctx,&ui->normal->handle);nk_layout_row_static(ui->ctx,70,850,1);field=nk_widget_bounds(ui->ctx);}
static uint64_t actual(SBUi *ui,struct nk_text_edit *edit){begin(ui);nk_edit_focus(ui->ctx,NK_EDIT_FIELD);nk_edit_buffer(ui->ctx,NK_EDIT_FIELD,edit,nk_filter_default);nk_end(ui->ctx);exact_caret=false;const struct nk_command *command;nk_foreach(command,ui->ctx)if(command->type==NK_COMMAND_RECT_FILLED){const struct nk_command_rect_filled *rect=(const struct nk_command_rect_filled *)command;if(rect->x==edit->caret_bounds.x && rect->y==edit->caret_bounds.y && rect->w==edit->caret_bounds.w && rect->h==edit->caret_bounds.h && rect->color.a)exact_caret=true;}sb_ui_draw(ui);uint64_t hash=pixels(ui);SDL_RenderPresent(ui->renderer);return hash;}
static uint64_t reference(SBUi *ui,const SBShapedLine *line,const SBCaretPlan *carets,size_t a,size_t b){
    begin(ui);struct nk_rect r;nk_widget(&r,ui->ctx);struct nk_command_buffer *canvas=nk_window_get_canvas(ui->ctx);struct nk_rect old=canvas->clip;float left=fmaxf(old.x,r.x),top=fmaxf(old.y,r.y);struct nk_rect clip=nk_rect(left,top,fminf(old.x+old.w,r.x+r.w)-left,fminf(old.y+old.h,r.y+r.h)-top);nk_fill_rect(canvas,r,0,ui->ctx->style.edit.active.data.color);nk_push_scissor(canvas,clip);
    SBSelectionPlan selection={0};if(a<b)sb_caret_selection(carets,a,b,&selection);
    /* The selected Hebrew word is one rectangle, followed logically by the
       first digit in a separate visual area. Internal letter seams aren't UI. */
    if(selection.count==3){float left=fminf(selection.spans[0].x,selection.spans[1].x),right=fmaxf(selection.spans[0].x+selection.spans[0].width,selection.spans[1].x+selection.spans[1].width);nk_fill_rect(canvas,nk_rect(r.x+left/ui->density,r.y,(right-left)/ui->density,r.h),0,ui->ctx->style.edit.selected_hover);SBSelectionSpan s=selection.spans[2];nk_fill_rect(canvas,nk_rect(r.x+s.x/ui->density,r.y,s.width/ui->density,r.h),0,ui->ctx->style.edit.selected_hover);}
    float baseline=r.y+(r.h-(line->ascent+line->descent)/ui->density)/2+line->ascent/ui->density;
    sb_ui_shaped_draw(ui,line,r.x,baseline,ui->ctx->style.edit.text_active);sb_caret_selection_free(&selection);nk_push_scissor(canvas,old);nk_end(ui->ctx);sb_ui_draw(ui);uint64_t hash=pixels(ui);SDL_RenderPresent(ui->renderer);return hash;
}
typedef struct {const SBShapedLine *reference;bool valid;unsigned lines;} Probe;
static bool inspect(void *user,const char *source,size_t length,size_t offset,const SBShapedLine *line,const SBCaretPlan *carets,float y){Probe *p=user;(void)source;(void)length;if(offset || y || !carets->count || line->count!=p->reference->count){p->valid=false;return false;}
    for(size_t i=0;i<line->count;++i){SBShapeGlyph a=line->glyphs[i],b=p->reference->glyphs[i];if(a.font!=b.font || a.index!=b.index || a.byte!=b.byte || a.x!=b.x || a.advance!=b.advance){p->valid=false;return false;}}++p->lines;return true;}
static void key(SBUi *ui,struct nk_text_edit *edit,enum nk_keys key,bool shift){nk_input_begin(ui->ctx);nk_input_key(ui->ctx,NK_KEY_SHIFT,shift);nk_input_key(ui->ctx,key,true);nk_input_end(ui->ctx);nk_begin(ui->ctx,"Exact field",nk_rect(0,0,1000,500),NK_WINDOW_NO_SCROLLBAR);nk_layout_row_static(ui->ctx,70,850,1);nk_edit_focus(ui->ctx,NK_EDIT_FIELD);nk_edit_buffer(ui->ctx,NK_EDIT_FIELD,edit,nk_filter_default);nk_end(ui->ctx);sb_ui_draw(ui);SDL_RenderPresent(ui->renderer);nk_input_begin(ui->ctx);nk_input_key(ui->ctx,key,false);nk_input_key(ui->ctx,NK_KEY_SHIFT,false);nk_input_end(ui->ctx);}
static void mouse(SBUi *ui,struct nk_text_edit *edit,float x,float y,bool down){nk_input_begin(ui->ctx);nk_input_motion(ui->ctx,x,y);nk_input_button(ui->ctx,NK_BUTTON_LEFT,x,y,down);nk_input_end(ui->ctx);nk_begin(ui->ctx,"Exact field",nk_rect(0,0,1000,500),NK_WINDOW_NO_SCROLLBAR);nk_layout_row_static(ui->ctx,70,850,1);nk_edit_focus(ui->ctx,NK_EDIT_FIELD);nk_edit_buffer(ui->ctx,NK_EDIT_FIELD,edit,nk_filter_default);nk_end(ui->ctx);sb_ui_draw(ui);SDL_RenderPresent(ui->renderer);}
static void multi(SBUi *ui,struct nk_text_edit *edit,enum nk_keys key,bool down){nk_input_begin(ui->ctx);if(key!=NK_KEY_NONE)nk_input_key(ui->ctx,key,down);nk_input_end(ui->ctx);nk_begin(ui->ctx,"Exact field",nk_rect(0,0,1000,500),NK_WINDOW_NO_SCROLLBAR);nk_layout_row_static(ui->ctx,350,850,1);field=nk_widget_bounds(ui->ctx);nk_edit_focus(ui->ctx,NK_EDIT_BOX);nk_edit_buffer(ui->ctx,NK_EDIT_BOX,edit,nk_filter_default);nk_end(ui->ctx);sb_ui_draw(ui);SDL_RenderPresent(ui->renderer);}
static int vertical(SBUi *ui){
    struct nk_text_edit edit;nk_textedit_init_default(&edit);edit.mode=NK_TEXT_EDIT_MODE_INSERT;const char *source="AV ffi\r\nאבג\nx";CHECK(nk_textedit_paste(&edit,source,(int)strlen(source)));edit.cursor=edit.select_start=edit.select_end=1;multi(ui,&edit,NK_KEY_NONE,false);float x=edit.caret_bounds.x-field.x,y=edit.caret_bounds.y;
    const struct nk_user_font *base=&ui->normal->handle;TTF_Font *font=sb_ui_cluster_font(base,"א",2,NULL);SBTextParagraph *bidi=NULL;SBShapeParagraph *p=NULL;SBShapedLine line={0};SBCaretPlan carets={0};SBShapeFontSpan span={0,6,font,TAG('H','e','b','r')};CHECK(sb_bidi_paragraph_create("אבג",6,SB_BIDI_AUTO_LTR,&bidi).code==SB_OK);CHECK(sb_shape_paragraph_create(bidi,&span,1,&p).code==SB_OK);CHECK(sb_shape_paragraph_line(p,0,6,&line).code==SB_OK);CHECK(sb_caret_plan(p,&line,&carets).code==SB_OK);SBCaret expected;CHECK(sb_caret_nearest(&carets,x*ui->density,1,&expected));
    multi(ui,&edit,NK_KEY_DOWN,true);multi(ui,&edit,NK_KEY_DOWN,false);CHECK(edit.cursor==8+(int)(expected.byte/2));CHECK(edit.caret_bounds.y>y && fabsf(edit.caret_bounds.x-field.x-expected.x/ui->density)<0.01f);
    multi(ui,&edit,NK_KEY_UP,true);multi(ui,&edit,NK_KEY_UP,false);CHECK(edit.cursor==1 && fabsf(edit.caret_bounds.x-field.x-x)<0.01f);
    /* A whole CRLF is crossed in one key press; source bytes remain intact. */
    edit.cursor=edit.select_start=edit.select_end=6;edit.visual_valid=0;multi(ui,&edit,NK_KEY_RIGHT,true);multi(ui,&edit,NK_KEY_RIGHT,false);CHECK(edit.cursor==8);CHECK((size_t)nk_str_len_char(&edit.string)==strlen(source) && !memcmp(nk_str_get_const(&edit.string),source,strlen(source)));
    sb_caret_plan_free(&carets);sb_shape_line_free(&line);sb_shape_paragraph_free(p);sb_bidi_paragraph_free(bidi);nk_textedit_free(&edit);return 0;
}
static int native_scroll(SBUi *ui){
    char source[1000]={0};for(unsigned i=0;i<30;++i)strcat(source,"AV ffi\r\nאבג\n\n");
    struct nk_text_edit edit;nk_textedit_init_default(&edit);edit.mode=NK_TEXT_EDIT_MODE_INSERT;CHECK(nk_textedit_paste(&edit,source,(int)strlen(source)));edit.cursor=edit.select_start=edit.select_end=1;
    multi(ui,&edit,NK_KEY_NONE,false);SBNativeText before={0},after={0};CHECK(sb_ui_edit_native(ui,&edit,&before));
    size_t bytes=0;bool crlf=false,rtl=false;for(size_t i=0;i<before.count;++i){SBNativeTextRun *r=&before.runs[i];CHECK(r->geometry && r->span.offset==bytes);crlf|=r->span.length>=2 && !memcmp(source+r->span.offset+r->span.length-2,"\r\n",2);rtl|=(r->level&1)!=0;bytes+=r->span.length;}
    CHECK(bytes==strlen(source) && crlf && rtl && !before.runs[before.count-1].characters);
    float old_x=edit.scrollbar.x,old_y=edit.scrollbar.y;edit.scrollbar=nk_vec2(37.25f,73.5f);multi(ui,&edit,NK_KEY_NONE,false);CHECK(sb_ui_edit_native(ui,&edit,&after));CHECK(before.count==after.count && edit.scrollbar.x>0 && edit.scrollbar.y>0);
    for(size_t i=0;i<before.count;++i){SBNativeTextRun a=before.runs[i],b=after.runs[i];CHECK(a.span.offset==b.span.offset && a.span.length==b.span.length && a.characters==b.characters);
        CHECK(fabsf(b.x-a.x+edit.scrollbar.x-old_x)<0.01f && fabsf(b.y-a.y+edit.scrollbar.y-old_y)<0.01f);
        CHECK(a.width==b.width && a.height==b.height && a.level==b.level && !memcmp(a.positions,b.positions,a.characters*sizeof(float)) && !memcmp(a.widths,b.widths,a.characters*sizeof(float)));}
    CHECK(after.runs[0].y<field.y); /* Offscreen source remains available to native text navigation. */
    sb_native_text_free(&before);sb_native_text_free(&after);nk_textedit_free(&edit);return 0;
}
typedef struct {SBCaret before,after;bool valid;} AffinityProbe;
static bool inspect_affinity(void *user,const char *source,size_t length,size_t offset,const SBShapedLine *line,const SBCaretPlan *carets,float y){AffinityProbe *p=user;(void)source;(void)length;(void)offset;(void)y;p->valid=sb_caret_find(carets,3,SB_CARET_BEFORE,line->base_level,&p->before) && sb_caret_find(carets,3,SB_CARET_AFTER,line->base_level,&p->after);return p->valid;}
static int font_affinity(SBUi *ui){const char *source="AB אב CD";struct nk_text_edit edit;nk_textedit_init_default(&edit);edit.mode=NK_TEXT_EDIT_MODE_INSERT;CHECK(nk_textedit_paste(&edit,source,(int)strlen(source)));edit.cursor=edit.select_start=edit.select_end=3;actual(ui,&edit);AffinityProbe old={0};CHECK(sb_ui_edit_geometry(ui,&edit,&ui->normal->handle,70,false,inspect_affinity,&old));CHECK(old.valid && old.before.x>old.after.x);edit.visual_valid=1;edit.visual_cursor=3;edit.visual_hash=sb_hash(source,strlen(source));edit.visual_affinity=old.before.affinity;edit.visual_level=old.before.level;edit.visual_x=old.before.x;actual(ui,&edit);CHECK(sb_ui_fonts(ui,1.5f).code==SB_OK);actual(ui,&edit);AffinityProbe resized={0};CHECK(sb_ui_edit_geometry(ui,&edit,&ui->normal->handle,70,false,inspect_affinity,&resized));CHECK(resized.valid && fabsf(edit.caret_bounds.x-field.x-resized.before.x/ui->density)<0.01f && resized.before.x>resized.after.x);CHECK(edit.cursor==3);nk_textedit_free(&edit);return 0;}
int main(int argc,char **argv){CHECK(argc==3);SBUi ui;CHECK(sb_ui_init(&ui,argv[1],1000,500,true).code==SB_OK);ui.space.dark=true;
    for(unsigned scale=0;scale<3;++scale){CHECK(sb_ui_fonts(&ui,1+0.5f*scale).code==SB_OK);struct nk_style_edit *style=&ui.ctx->style.edit;style->border=style->rounding=0;style->padding=nk_vec2(0,0);style->active=style->hover=style->normal=nk_style_item_color(nk_rgb(8,16,28));style->text_active=style->text_hover=style->text_normal=nk_rgb(225,235,245);style->selected_text_hover=style->selected_text_normal=style->text_active;style->selected_hover=style->selected_normal=nk_rgb(35,65,90);style->cursor_hover=style->cursor_normal=nk_rgba(0,0,0,0);
        const char *parts[]={"AV ffi é ","אב","12","גד"," ","ببب"," ","👩‍💻"};char source[200]={0};SBShapeFontSpan fonts[8];size_t length=0;
        const struct nk_user_font *base=&ui.normal->handle;TTF_Font *latin=sb_ui_cluster_font(base,"A",1,NULL),*hebrew=sb_ui_cluster_font(base,"א",2,NULL),*arabic=sb_ui_cluster_font(base,"ب",2,NULL),*emoji=sb_ui_cluster_font(base,"👩‍💻",11,NULL);CHECK(latin && hebrew && arabic && emoji);
        for(unsigned i=0;i<8;++i){size_t n=strlen(parts[i]);TTF_Font *font=i==0 || i==2 ? latin : i<5 ? hebrew : i<7 ? arabic : emoji;uint32_t script=i==0 ? TAG('L','a','t','n') : i<5 ? TAG('H','e','b','r') : TAG('A','r','a','b');fonts[i]=(SBShapeFontSpan){length,n,font,script};memcpy(source+length,parts[i],n);length+=n;}source[length]=0;
        SBTextParagraph *bidi=NULL;SBShapeParagraph *paragraph=NULL;SBShapedLine line={0};SBCaretPlan carets={0};CHECK(sb_bidi_paragraph_create(source,length,SB_BIDI_AUTO_LTR,&bidi).code==SB_OK);CHECK(sb_shape_paragraph_create(bidi,fonts,8,&paragraph).code==SB_OK);CHECK(sb_shape_paragraph_line(paragraph,0,length,&line).code==SB_OK);CHECK(sb_caret_plan(paragraph,&line,&carets).code==SB_OK);
        struct nk_text_edit edit;nk_textedit_init_default(&edit);edit.mode=NK_TEXT_EDIT_MODE_INSERT;CHECK(nk_textedit_paste(&edit,source,(int)length));actual(&ui,&edit);
        SBNativeText native={0};CHECK(sb_ui_edit_native(&ui,&edit,&native));size_t units=0;
        for(size_t r=0;r<native.count;++r){SBNativeTextRun *run=&native.runs[r];CHECK(run->geometry);size_t byte=run->span.offset;
            for(size_t j=0;j<run->characters;++j){const SBCaretCluster *expected=NULL;for(size_t k=0;k<carets.cluster_count;++k)if(carets.clusters[k].byte==byte)expected=&carets.clusters[k];CHECK(expected!=NULL);
                float x=(run->level&1) ? run->x+run->width-run->positions[j]-run->widths[j] : run->x+run->positions[j];
                CHECK(fabsf(x-field.x-expected->left/ui.density)<0.01f && fabsf(run->widths[j]-(expected->right-expected->left)/ui.density)<0.01f);
                CHECK((run->level&1)==(expected->level&1));byte+=run->lengths[j];++units;}}
        CHECK(units==carets.cluster_count);sb_native_text_free(&native);
        Probe probe={.reference=&line,.valid=true};CHECK(sb_ui_edit_geometry(&ui,&edit,base,70,false,inspect,&probe));CHECK(probe.valid && probe.lines==1);
        for(unsigned selected=0;selected<2;++selected){size_t a=selected ? strlen(parts[0]) : 0,b=selected ? a+5 : 0;edit.select_start=(int)nk_utf_len(source,(int)a);edit.select_end=edit.cursor=(int)nk_utf_len(source,(int)b);uint64_t expected=reference(&ui,&line,&carets,a,b),observed=actual(&ui,&edit);CHECK(expected && expected==observed);}
        edit.cursor=edit.select_start=edit.select_end=4;actual(&ui,&edit);SBCaret expected;CHECK(sb_caret_find(&carets,4,SB_CARET_BOTH,0,&expected));CHECK(fabsf(edit.caret_bounds.x-field.x-expected.x/ui.density)<0.01f);
        style->cursor_hover=style->cursor_normal=nk_rgb(120,200,250);actual(&ui,&edit);CHECK(exact_caret);style->cursor_hover=style->cursor_normal=nk_rgba(0,0,0,0);
        float mouse_x=field.x+expected.x/ui.density+0.1f,mouse_y=field.y+35.25f;
        mouse(&ui,&edit,mouse_x,mouse_y,true);CHECK(edit.cursor==4 && ui.ctx->input.mouse.pos.x==mouse_x && ui.ctx->input.mouse.pos.y==mouse_y);mouse(&ui,&edit,mouse_x,mouse_y,false);
        key(&ui,&edit,NK_KEY_RIGHT,false);CHECK(edit.cursor==5);CHECK(sb_caret_find(&carets,5,SB_CARET_BOTH,0,&expected));CHECK(fabsf(edit.caret_bounds.x-field.x-expected.x/ui.density)<0.01f);
        /* Hebrew letters: one physical left press increases the logical index. */
        int hebrew_start=nk_utf_len(parts[0],(int)strlen(parts[0]));edit.cursor=edit.select_start=edit.select_end=hebrew_start;edit.visual_valid=0;actual(&ui,&edit);
        size_t byte=strlen(parts[0]);CHECK(sb_caret_find(&carets,byte,SB_CARET_BEFORE,0,&expected));edit.visual_valid=1;edit.visual_cursor=edit.cursor;edit.visual_hash=sb_hash(source,length);edit.visual_x=expected.x;edit.visual_level=expected.level;edit.visual_affinity=expected.affinity;
        key(&ui,&edit,NK_KEY_LEFT,false);CHECK(edit.cursor==hebrew_start+1);CHECK((size_t)nk_str_len_char(&edit.string)==length && !memcmp(nk_str_get_const(&edit.string),source,length));
        actual(&ui,&edit);if(scale==2)CHECK(sb_ui_capture(&ui,argv[2]).code==SB_OK);
        nk_textedit_free(&edit);sb_caret_plan_free(&carets);sb_shape_line_free(&line);sb_shape_paragraph_free(paragraph);sb_bidi_paragraph_free(bidi);
    }CHECK(!vertical(&ui));CHECK(!native_scroll(&ui));CHECK(!font_affinity(&ui));sb_ui_shutdown(&ui);printf("%u integrated field glyph/pixel/ligature/bidi/native-scroll assertions passed.\n",checks);return 0;
}
