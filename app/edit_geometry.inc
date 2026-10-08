/* One unchanged source/layout for editor drawing, navigation and IME. */
#include "carets.h"
/* One original-source export, independent of edit-plan eviction and scrolling.
   A larger export remains complete but is not retained between frames. */
#define SB_EDIT_NATIVE_BUDGET (32u*1024u*1024u)
struct SBEditNativeCache {char *source;size_t length;uint64_t hash;const struct nk_user_font *font;float row,density;bool single;SBNativeText text;};
static void edit_native_cache_clear(SBUi *ui){if(ui->edit_native_cache){free(ui->edit_native_cache->source);sb_native_text_free(&ui->edit_native_cache->text);free(ui->edit_native_cache);ui->edit_native_cache=NULL;}}
typedef struct {SBShapedLine shape;SBCaretPlan carets;size_t byte,length;int first,last,next;} SBEditLine;
typedef struct {int first,last;bool significant;} SBEditWord;
typedef struct {char *text;uint32_t *bytes;size_t length,count,capacity,memory;int characters;SBEditLine *lines;SBEditWord *words;size_t word_count,word_capacity;const struct nk_user_font *font;float row,requested_row;bool single;uint64_t hash,stamp;} SBEditPlan;
#define SB_EDIT_WAYS 8
#define SB_EDIT_BUDGET (64u*1024u*1024u)
struct SBEditCache {SBEditPlan entries[SB_EDIT_WAYS],scratch;size_t bytes;uint64_t stamp;};
static void edit_plan_free(SBEditPlan *p){for(size_t i=0;i<p->count;++i){sb_shape_line_free(&p->lines[i].shape);sb_caret_plan_free(&p->lines[i].carets);}free(p->lines);free(p->text);free(p->bytes);free(p->words);memset(p,0,sizeof(*p));}
void sb_ui_edit_cache_clear(SBUi *ui){ui->edit_paint.edit=NULL;edit_native_cache_clear(ui);if(!ui->edit_cache)return;for(unsigned i=0;i<SB_EDIT_WAYS;++i)edit_plan_free(&ui->edit_cache->entries[i]);edit_plan_free(&ui->edit_cache->scratch);free(ui->edit_cache);ui->edit_cache=NULL;}
static void edit_cache_frame_end(SBUi *ui){if(ui->edit_cache)edit_plan_free(&ui->edit_cache->scratch);}
static int edit_character(const SBEditPlan *p,size_t byte){int lo=0,hi=p->characters;while(lo<hi){int m=lo+(hi-lo)/2;if(p->bytes[m]<byte)lo=m+1;else hi=m;}return lo;}
static void edit_tabs(SBShapedLine *shape,const char *source){
    float shift=0;
    for(size_t r=0;r<shape->run_count;++r){SBShapeRun *run=&shape->runs[r];run->x+=shift;float added=0;
        for(size_t i=run->glyph_begin;i<run->glyph_begin+run->glyph_count;++i){SBShapeGlyph *g=&shape->glyphs[i];g->x+=shift;
            if(source[g->byte]=='\t'){int w=0,h=0;if(TTF_GetStringSize(g->font,"    ",4,&w,&h)){float delta=(float)w-g->advance;g->advance=(float)w;g->width=g->height=0;shift+=delta;added+=delta;}}}
        run->advance+=added;
    }shape->advance+=shift;
}
static bool edit_build(SBUi *ui,SBEditPlan *p){
    if(p->length>SB_TEXT_LIMIT || !sb_utf8_valid(p->text,p->length))return false;
    p->characters=nk_utf_len(p->text,(int)p->length);p->bytes=malloc(((size_t)p->characters+1)*sizeof(*p->bytes));if(!p->bytes)return false;
    size_t at=0;for(int i=0;i<p->characters;++i){p->bytes[i]=(uint32_t)at;nk_rune cp;int n=nk_utf_decode(p->text+at,&cp,(int)(p->length-at));if(n<=0)return false;at+=(size_t)n;}p->bytes[p->characters]=(uint32_t)p->length;
    SBWord words;SBWordBoundary boundary;size_t first=0;
    if(!sb_word_init(&words,p->text,p->length))return false;
    while(sb_word_next(&words,&boundary)){
        if(boundary.characters>first){
            if(p->word_count==p->word_capacity){size_t next=p->word_capacity ? p->word_capacity*2 : 16;SBEditWord *grown=realloc(p->words,next*sizeof(*grown));if(!grown)return false;p->words=grown;p->word_capacity=next;}
            p->words[p->word_count++]=(SBEditWord){(int)first,(int)boundary.characters,boundary.significant};
        }first=boundary.characters;
    }
    size_t start=0;
    do{size_t end=start;
        while(end<p->length){const char *cp_at=p->text+end;uint32_t cp=SDL_StepUTF8(&cp_at,NULL);end=(size_t)(cp_at-p->text);if(sb_bidi_separator(cp)){if(cp=='\r' && end<p->length && p->text[end]=='\n')++end;break;}}
        size_t visible=end;
        if(visible>start){const char *last=p->text+visible-1;while(last>p->text+start && ((unsigned char)*last&0xc0)==0x80)--last;
            const char *step=last;uint32_t cp=SDL_StepUTF8(&step,NULL);if(sb_bidi_separator(cp)){visible=(size_t)(last-p->text);if(cp=='\n' && visible>start && p->text[visible-1]=='\r')--visible;}}
        if(p->count==p->capacity){size_t next=p->capacity ? p->capacity*2 : 16;SBEditLine *grown=realloc(p->lines,next*sizeof(*grown));if(!grown)return false;p->lines=grown;p->capacity=next;}
        SBEditLine *line=&p->lines[p->count++];*line=(SBEditLine){.byte=start,.length=visible-start,.first=edit_character(p,start),.last=edit_character(p,visible),.next=edit_character(p,end)};
        SBTextParagraph *bidi=NULL;SBShapeParagraph *prepared=NULL;SBShapeFontSpan *fonts=NULL;size_t font_count=0;
        SBTextSpan span={0,visible-start,0};SBStyledText text={.text=p->text+start,.spans=&span,.count=span.length ? 1 : 0};
        bool ok=sb_bidi_paragraph_create(text.text,span.length,SB_BIDI_AUTO_LTR,&bidi).code==SB_OK;
        if(ok && span.length)ok=styled_fonts(ui,p->font,&text,0,span.length,&fonts,&font_count);
        if(ok)ok=sb_shape_paragraph_create(bidi,fonts,font_count,&prepared).code==SB_OK;
        if(ok)ok=sb_shape_paragraph_line(prepared,0,span.length,&line->shape).code==SB_OK;
        if(ok){if(!p->single)p->row=fmaxf(p->row,(line->shape.ascent+line->shape.descent)/ui->density+4*ui->scale);edit_tabs(&line->shape,text.text);ok=sb_caret_plan(prepared,&line->shape,&line->carets).code==SB_OK;}
        free(fonts);sb_shape_paragraph_free(prepared);sb_bidi_paragraph_free(bidi);if(!ok)return false;
        start=end;
        if(p->single || (start==p->length && visible==end))break;
    }while(start<=p->length);
    p->memory=p->length+1+((size_t)p->characters+1)*sizeof(*p->bytes)+p->capacity*sizeof(*p->lines)+p->word_capacity*sizeof(*p->words);
    for(size_t i=0;i<p->count;++i){SBEditLine *l=&p->lines[i];p->memory+=l->shape.capacity*sizeof(SBShapeGlyph)+l->shape.run_capacity*sizeof(SBShapeRun)+l->carets.capacity*sizeof(SBCaret)+l->carets.count*sizeof(SBCaretIndex)+l->carets.cluster_capacity*sizeof(SBCaretCluster);}
    return true;
}
static SBEditPlan *edit_plan(struct nk_text_edit *edit,const struct nk_user_font *font,float row){
    SBUi *ui=sb_ui_font_owner(font);if(!ui || !isfinite(row) || row<=0)return NULL;
    size_t length=(size_t)nk_str_len_char(&edit->string);const char *source=length ? nk_str_get_const(&edit->string) : "";uint64_t hash=sb_hash(source,length);
    if(!ui->edit_cache)ui->edit_cache=calloc(1,sizeof(*ui->edit_cache));if(!ui->edit_cache)return NULL;SBEditCache *cache=ui->edit_cache;SBEditPlan *slot=&cache->entries[0];
    for(unsigned i=0;i<=SB_EDIT_WAYS;++i){SBEditPlan *p=i==SB_EDIT_WAYS ? &cache->scratch : &cache->entries[i];
        if(p->text && p->hash==hash && p->length==length && p->font==font && p->requested_row==row && p->single==(bool)edit->single_line && !memcmp(p->text,source,length)){p->stamp=++cache->stamp;return p;}
        if(i<SB_EDIT_WAYS && p->stamp<slot->stamp)slot=p;}
    SBEditPlan next={.length=length,.font=font,.row=row,.requested_row=row,.single=edit->single_line,.hash=hash};next.text=malloc(length+1);if(!next.text)return NULL;memcpy(next.text,source,length);next.text[length]=0;
    if(!edit_build(ui,&next)){edit_plan_free(&next);ui->input_status=sb_error(SB_MEMORY,"Textgeometrie konnte nicht vorbereitet werden. Der Inhalt bleibt erhalten.");return NULL;}
    if(next.memory>SB_EDIT_BUDGET){slot=&cache->scratch;edit_plan_free(slot);}
    else{if(cache->bytes+next.memory>SB_EDIT_BUDGET){for(unsigned i=0;i<SB_EDIT_WAYS;++i)edit_plan_free(&cache->entries[i]);cache->bytes=0;slot=&cache->entries[0];}cache->bytes-=slot->memory;edit_plan_free(slot);cache->bytes+=next.memory;}
    next.stamp=++cache->stamp;*slot=next;return slot;
}
static size_t edit_line_at(const SBEditPlan *p,int character){size_t lo=0,hi=p->count;while(lo<hi){size_t m=lo+(hi-lo)/2;if(p->lines[m].first<=character)lo=m+1;else hi=m;}return lo ? lo-1 : 0;}
static bool edit_point(SBEditPlan *p,struct nk_text_edit *edit,int character,size_t *row,SBCaret *point){
    character=NK_CLAMP(0,character,p->characters);*row=edit_line_at(p,character);SBEditLine *line=&p->lines[*row];size_t byte=p->bytes[character]-line->byte;if(byte>line->length)byte=line->length;
    if(edit->visual_valid && edit->visual_cursor==character && edit->visual_hash==p->hash && sb_caret_find(&line->carets,byte,(SBCaretAffinity)edit->visual_affinity,line->shape.base_level,point) && point->level==edit->visual_level)return true;
    return sb_caret_find(&line->carets,byte,SB_CARET_BOTH,line->shape.base_level,point);
}
static int edit_remember(SBEditPlan *p,struct nk_text_edit *edit,size_t row,SBCaret c){int character=edit_character(p,p->lines[row].byte+c.byte);edit->visual_valid=1;edit->visual_cursor=character;edit->visual_hash=p->hash;edit->visual_x=c.x;edit->visual_affinity=c.affinity;edit->visual_level=c.level;return character;}
static int sb_ui_edit_locate(struct nk_text_edit *edit,float x,float y,const struct nk_user_font *font,float row){
    SBEditPlan *p=edit_plan(edit,font,row);SBUi *ui=sb_ui_font_owner(font);if(!p || !ui)return -1;row=p->row;
    if(y<0){SBCaret c;if(sb_caret_find(&p->lines[0].carets,0,SB_CARET_BEFORE,p->lines[0].shape.base_level,&c))return edit_remember(p,edit,0,c);}
    if(y>=p->count*row){size_t last=p->count-1;SBCaret c;if(sb_caret_find(&p->lines[last].carets,p->lines[last].length,SB_CARET_AFTER,p->lines[last].shape.base_level,&c))return edit_remember(p,edit,last,c);}
    size_t line=(size_t)NK_CLAMP(0,floorf(y/row),(float)(p->count-1));SBCaret point;if(!sb_caret_nearest(&p->lines[line].carets,x*ui->density,p->lines[line].shape.base_level,&point))return -1;return edit_remember(p,edit,line,point);
}
static int sb_ui_edit_select_word(struct nk_text_edit *edit,const struct nk_user_font *font,float row,float x,float y){
    SBEditPlan *p=edit_plan(edit,font,row);SBUi *ui=sb_ui_font_owner(font);if(!p || !ui)return 0;
    if(!p->word_count){edit->cursor=edit->select_start=edit->select_end=0;return 1;}
    size_t line=(size_t)NK_CLAMP(0,floorf(y/p->row),(float)(p->count-1));SBEditLine *l=&p->lines[line];
    int character=NK_CLAMP(0,edit->cursor,p->characters);float point=x*ui->density;
    for(size_t i=0;i<l->carets.cluster_count;++i){SBCaretCluster c=l->carets.clusters[i];
        if(point>=fminf(c.left,c.right) && point<fmaxf(c.left,c.right)){character=edit_character(p,l->byte+c.byte);break;}}
    size_t lo=0,hi=p->word_count;while(lo<hi){size_t mid=lo+(hi-lo)/2;if(p->words[mid].first<=character)lo=mid+1;else hi=mid;}
    SBEditWord word=p->words[lo ? lo-1 : 0];
    edit->select_start=sb_ui_grapheme_index(edit,word.first,-2);edit->cursor=edit->select_end=sb_ui_grapheme_index(edit,word.last,2);
    edit->visual_valid=0;edit->has_preferred_x=0;return 1;
}
static int sb_ui_edit_key(struct nk_text_edit *edit,int key,int shift,const struct nk_user_font *font,float row){
    if(key!=NK_KEY_LEFT && key!=NK_KEY_RIGHT && key!=NK_KEY_UP && key!=NK_KEY_DOWN && key!=NK_KEY_TEXT_LINE_START && key!=NK_KEY_TEXT_LINE_END && key!=NK_KEY_TEXT_WORD_LEFT && key!=NK_KEY_TEXT_WORD_RIGHT)return 0;
    SBEditPlan *p=edit_plan(edit,font,row);SBUi *ui=sb_ui_font_owner(font);if(!p || !ui)return 0;
    if(key==NK_KEY_TEXT_WORD_LEFT || key==NK_KEY_TEXT_WORD_RIGHT){
        bool left=key==NK_KEY_TEXT_WORD_LEFT;int cursor=NK_CLAMP(0,edit->cursor,p->characters);
        if(!shift && edit->select_start!=edit->select_end)cursor=left ? NK_MIN(edit->select_start,edit->select_end) : NK_MAX(edit->select_start,edit->select_end);
        else{
            if(shift && edit->select_start==edit->select_end)edit->select_start=cursor;
            size_t lo=0,hi=p->word_count;
            if(left){while(lo<hi){size_t mid=lo+(hi-lo)/2;if(p->words[mid].first<cursor)lo=mid+1;else hi=mid;}while(lo && !p->words[lo-1].significant)--lo;cursor=lo ? p->words[lo-1].first : 0;}
            else{
#ifdef __APPLE__
                while(lo<hi){size_t mid=lo+(hi-lo)/2;if(p->words[mid].last<=cursor)lo=mid+1;else hi=mid;}while(lo<p->word_count && !p->words[lo].significant)++lo;cursor=lo<p->word_count ? p->words[lo].last : p->characters;
#else
                while(lo<hi){size_t mid=lo+(hi-lo)/2;if(p->words[mid].first<=cursor)lo=mid+1;else hi=mid;}while(lo<p->word_count && !p->words[lo].significant)++lo;cursor=lo<p->word_count ? p->words[lo].first : p->characters;
#endif
            }
        }
        edit->cursor=sb_ui_grapheme_index(edit,NK_CLAMP(0,cursor,p->characters),left ? -2 : 2);
        edit->visual_valid=0;edit->has_preferred_x=0;
        if(shift)edit->select_end=edit->cursor;else edit->select_start=edit->select_end=edit->cursor;
        return 1;
    }
    if(shift && edit->select_start==edit->select_end)edit->select_start=edit->cursor;
    if(!shift && edit->select_start!=edit->select_end && (key==NK_KEY_LEFT || key==NK_KEY_RIGHT)){
        int a=NK_CLAMP(0,NK_MIN(edit->select_start,edit->select_end),p->characters),b=NK_CLAMP(0,NK_MAX(edit->select_start,edit->select_end),p->characters);size_t ra=edit_line_at(p,a),rb=edit_line_at(p,b);SBCaret ca,cb;
        if(ra==rb && sb_caret_find(&p->lines[ra].carets,p->bytes[a]-p->lines[ra].byte,SB_CARET_BEFORE,p->lines[ra].shape.base_level,&ca) && sb_caret_find(&p->lines[rb].carets,p->bytes[b]-p->lines[rb].byte,SB_CARET_AFTER,p->lines[rb].shape.base_level,&cb))edit->cursor=edit_remember(p,edit,ra,(key==NK_KEY_LEFT)==(ca.x<=cb.x) ? ca : cb);
        else{edit->cursor=key==NK_KEY_LEFT ? a : b;edit->visual_valid=0;}
        edit->select_start=edit->select_end=edit->cursor;edit->has_preferred_x=0;return 1;
    }
    size_t line;SBCaret current,next;if(!edit_point(p,edit,edit->cursor,&line,&current))return 0;bool moved=false;
    if(edit->single_line && key==NK_KEY_UP)key=NK_KEY_LEFT;if(edit->single_line && key==NK_KEY_DOWN)key=NK_KEY_RIGHT;
    if(key==NK_KEY_LEFT || key==NK_KEY_RIGHT){int direction=key==NK_KEY_LEFT ? -1 : 1;moved=sb_caret_step(&p->lines[line].carets,&current,direction,&next);
        bool forward=(direction>0)!=((p->lines[line].shape.base_level&1)!=0);
        if(!moved && ((!forward && line) || (forward && line+1<p->count))){line=forward ? line+1 : line-1;SBEditLine *l=&p->lines[line];moved=sb_caret_find(&l->carets,forward ? 0 : l->length,forward ? SB_CARET_BEFORE : SB_CARET_AFTER,l->shape.base_level,&next);}edit->has_preferred_x=0;
    }else if(key==NK_KEY_UP || key==NK_KEY_DOWN){int direction=key==NK_KEY_UP ? -1 : 1;float x=edit->has_preferred_x ? edit->preferred_x*ui->density : current.x;
        if((direction<0 && line) || (direction>0 && line+1<p->count)){line=direction<0 ? line-1 : line+1;moved=sb_caret_nearest(&p->lines[line].carets,x,p->lines[line].shape.base_level,&next);}edit->preferred_x=x/ui->density;edit->has_preferred_x=1;
    }else{SBEditLine *l=&p->lines[line];size_t byte=key==NK_KEY_TEXT_LINE_START ? 0 : l->length;moved=sb_caret_find(&l->carets,byte,key==NK_KEY_TEXT_LINE_START ? SB_CARET_BEFORE : SB_CARET_AFTER,l->shape.base_level,&next);edit->has_preferred_x=0;}
    if(moved)edit->cursor=edit_remember(p,edit,line,next);
    if(shift)edit->select_end=edit->cursor;else edit->select_start=edit->select_end=edit->cursor;
    return 1;
}
static int edit_selection_compare(const void *left,const void *right){const SBSelectionSpan *a=left,*b=right;return a->x<b->x ? -1 : a->x>b->x;}
static void edit_selection_union(SBSelectionPlan *plan){
    if(!plan->count)return;qsort(plan->spans,plan->count,sizeof(*plan->spans),edit_selection_compare);size_t used=1;
    for(size_t i=1;i<plan->count;++i){SBSelectionSpan *last=&plan->spans[used-1],next=plan->spans[i];float right=last->x+last->width;
        if(next.x<=right+0.001f)last->width=fmaxf(right,next.x+next.width)-last->x;else plan->spans[used++]=next;}plan->count=used;
}
static void edit_fill(struct nk_command_buffer *out,struct nk_rect r,struct nk_color color){struct nk_rect c=out->clip;float x=fmaxf(r.x,c.x),y=fmaxf(r.y,c.y),right=fminf(r.x+r.w,c.x+c.w),bottom=fminf(r.y+r.h,c.y+c.h);if(right>x && bottom>y)nk_fill_rect(out,nk_rect(x,y,right-x,bottom-y),0,color);}
static int sb_ui_edit_render(struct nk_command_buffer *out,struct nk_text_edit *edit,const struct nk_style_edit *style,const struct nk_user_font *font,struct nk_rect area,struct nk_rect clip,struct nk_rect bounds,nk_flags flags,struct nk_input *in,nk_flags state,float row,int follow,int display){
    int requested_cursor=edit->cursor;sb_ui_grapheme_clamp(edit);if(display)edit->cursor=sb_ui_grapheme_index(edit,requested_cursor,2);
    SBEditPlan *p=edit_plan(edit,font,row);SBUi *ui=sb_ui_font_owner(font);if(!p || !ui)return 0;
    row=p->row;
    size_t cursor_line;SBCaret cursor;if(!edit_point(p,edit,edit->cursor,&cursor_line,&cursor))return 0;
    float cursor_x=cursor.x/ui->density,cursor_y=cursor_line*row;
    if(edit->active && follow){if(flags&NK_EDIT_NO_HORIZONTAL_SCROLL)edit->scrollbar.x=0;else{float margin=area.w*0.25f;if(cursor_x<edit->scrollbar.x)edit->scrollbar.x=fmaxf(0,cursor_x-margin);if(cursor_x>=edit->scrollbar.x+area.w)edit->scrollbar.x=fmaxf(0,cursor_x-area.w+margin);}
        if(flags&NK_EDIT_MULTILINE){if(cursor_y<edit->scrollbar.y)edit->scrollbar.y=cursor_y;if(cursor_y>edit->scrollbar.y+area.h-row)edit->scrollbar.y=fmaxf(0,cursor_y-area.h+row);}else edit->scrollbar.y=0;}
    if(edit->active && (flags&NK_EDIT_MULTILINE)){nk_flags ws;struct nk_rect scroll=area;scroll.x=bounds.x+bounds.w-style->border-style->scrollbar_size.x;scroll.w=style->scrollbar_size.x;bool hovered=in && nk_input_is_mouse_hovering_rect(in,area);edit->scrollbar.y=nk_do_scrollbarv(&ws,out,scroll,hovered,edit->scrollbar.y,p->count*row,scroll.h*0.1f,scroll.h*0.01f,&style->scrollbar,in,font);if(hovered && in->mouse.scroll_delta.y)in->mouse.scroll_delta.y=0;}
    nk_push_scissor(out,clip);
    struct nk_color text=state&NK_WIDGET_STATE_ACTIVED ? style->text_active : state&NK_WIDGET_STATE_HOVER ? style->text_hover : style->text_normal;
    struct nk_color selected=state&(NK_WIDGET_STATE_ACTIVED|NK_WIDGET_STATE_HOVER) ? style->selected_hover : style->selected_normal;
    struct nk_color selected_text=state&(NK_WIDGET_STATE_ACTIVED|NK_WIDGET_STATE_HOVER) ? style->selected_text_hover : style->selected_text_normal;
    struct nk_color caret=nk_rgb_factor(state&(NK_WIDGET_STATE_ACTIVED|NK_WIDGET_STATE_HOVER) ? style->cursor_hover : style->cursor_normal,style->color_factor);
    int first=NK_CLAMP(0,NK_MIN(edit->select_start,edit->select_end),p->characters),last=NK_CLAMP(0,NK_MAX(edit->select_start,edit->select_end),p->characters);
    size_t selection_begin=p->bytes[first],selection_end=p->bytes[last];
    size_t begin=(size_t)fmaxf(0,floorf((edit->scrollbar.y+clip.y-area.y)/row)),end=(size_t)fminf((float)p->count,ceilf((edit->scrollbar.y+clip.y+clip.h-area.y)/row));
    for(size_t i=begin;i<end;++i){SBEditLine *l=&p->lines[i];float x=area.x-edit->scrollbar.x,y=area.y+i*row-edit->scrollbar.y;float baseline=y+(row-(l->shape.ascent+l->shape.descent)/ui->density)/2+l->shape.ascent/ui->density;
        SBSelectionPlan selection={0};size_t a=selection_begin>l->byte ? selection_begin-l->byte : 0,b=selection_end>l->byte ? selection_end-l->byte : 0;a=SDL_min(a,l->length);b=SDL_min(b,l->length);
        if(edit->active && a<b && sb_caret_selection(&l->carets,a,b,&selection).code==SB_OK){edit_selection_union(&selection);}for(size_t j=0;j<selection.count;++j){SBSelectionSpan s=selection.spans[j];edit_fill(out,nk_rect(x+s.x/ui->density,y,s.width/ui->density,row),selected);}
        if(!selection.count)sb_ui_shaped_draw(ui,&l->shape,x,baseline,text);
        else{
            /* Each pixel is painted once. Partition clips change the color of
               the complete glyph line without re-shaping selected substrings. */
            float at=clip.x;
            for(size_t j=0;j<selection.count;++j){SBSelectionSpan s=selection.spans[j];float left=fmaxf(clip.x,x+s.x/ui->density),right=fminf(clip.x+clip.w,x+(s.x+s.width)/ui->density);if(right<=left)continue;
                if(left>at){nk_push_scissor(out,nk_rect(at,clip.y,left-at,clip.h));sb_ui_shaped_draw(ui,&l->shape,x,baseline,text);}
                nk_push_scissor(out,nk_rect(left,clip.y,right-left,clip.h));sb_ui_shaped_draw(ui,&l->shape,x,baseline,selected_text);at=right;
            }
            if(at<clip.x+clip.w){nk_push_scissor(out,nk_rect(at,clip.y,clip.x+clip.w-at,clip.h));sb_ui_shaped_draw(ui,&l->shape,x,baseline,text);}
        }
        nk_push_scissor(out,clip);sb_caret_selection_free(&selection);
    }
    edit->caret_bounds=nk_rect(area.x+cursor_x-edit->scrollbar.x,area.y+cursor_y-edit->scrollbar.y+(row-font->height)/2,style->cursor_size,font->height);edit->caret_clip=clip;
    if(!display){ui->edit_paint.edit=edit;ui->edit_paint.font=font;ui->edit_paint.hash=p->hash;ui->edit_paint.row=p->requested_row;ui->edit_paint.area=area;ui->edit_paint.clip=clip;ui->edit_paint.scroll=edit->scrollbar;}
    else ui->edit_paint.edit=NULL;
    if(edit->active && first==last && !(flags&NK_EDIT_NO_CURSOR)){
        struct nk_rect r=edit->caret_bounds;
        if(edit->mode==NK_TEXT_EDIT_MODE_REPLACE){SBEditLine *l=&p->lines[cursor_line];size_t byte=p->bytes[NK_CLAMP(0,edit->cursor,p->characters)]-l->byte;for(size_t j=0;j<l->carets.cluster_count;++j)if(l->carets.clusters[j].byte==byte){SBCaretCluster c=l->carets.clusters[j];r.x=area.x+c.left/ui->density-edit->scrollbar.x;r.w=fmaxf(style->cursor_size,(c.right-c.left)/ui->density);break;}}
        edit_fill(out,r,caret);
        if(edit->mode==NK_TEXT_EDIT_MODE_REPLACE && caret.a){struct nk_rect c;nk_unify(&c,&clip,r.x,r.y,r.x+r.w,r.y+r.h);if(c.w>0 && c.h>0){nk_push_scissor(out,c);SBEditLine *l=&p->lines[cursor_line];float y=area.y+cursor_y-edit->scrollbar.y;float baseline=y+(row-(l->shape.ascent+l->shape.descent)/ui->density)/2+l->shape.ascent/ui->density;sb_ui_shaped_draw(ui,&l->shape,area.x-edit->scrollbar.x,baseline,state&(NK_WIDGET_STATE_ACTIVED|NK_WIDGET_STATE_HOVER) ? style->cursor_text_hover : style->cursor_text_normal);nk_push_scissor(out,clip);}}
    }
    return 1;
}
static void sb_ui_edit_mark(struct nk_command_buffer *out,struct nk_text_edit *edit,const struct nk_style_edit *style,const struct nk_user_font *font,struct nk_rect area,float row,int first,int last){
    SBEditPlan *p=edit_plan(edit,font,row);SBUi *ui=sb_ui_font_owner(font);if(!p || !ui)return;row=p->row;
    first=sb_ui_grapheme_index(edit,first,-2);last=sb_ui_grapheme_index(edit,last,2);first=NK_CLAMP(0,first,p->characters);last=NK_CLAMP(first,last,p->characters);
    size_t begin=p->bytes[first],end=p->bytes[last];
    for(size_t i=0;i<p->count;++i){SBEditLine *l=&p->lines[i];if(begin>=l->byte+l->length || end<=l->byte)continue;
        size_t a=begin>l->byte ? begin-l->byte : 0,b=SDL_min(end-l->byte,l->length);SBSelectionPlan selection={0};
        if(sb_caret_selection(&l->carets,a,b,&selection).code!=SB_OK)continue;
        float y=area.y+i*row-edit->scrollbar.y+(row+font->height)/2+1;
        for(size_t j=0;j<selection.count;++j){SBSelectionSpan s=selection.spans[j];float left=fmaxf(out->clip.x,area.x-edit->scrollbar.x+s.x/ui->density),right=fminf(out->clip.x+out->clip.w,area.x-edit->scrollbar.x+(s.x+s.width)/ui->density);if(right>left && y>=out->clip.y && y<=out->clip.y+out->clip.h)nk_stroke_line(out,left,y,right,y,1,style->text_normal);}
        sb_caret_selection_free(&selection);
    }
}
