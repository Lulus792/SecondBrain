/* Display-only IME state; persisted text remains owned by the original editor. */
typedef struct {SDL_Event event;char *text;bool boundary;} SBCompositionEvent;
struct SBComposition {
    char *preedit,*commit,*render_text,*baseline;
    size_t render_capacity,baseline_capacity,baseline_length;
    struct nk_text_edit *visual;
    struct nk_window *window;
    nk_hash id;
    uint64_t hash;
    int cursor,start,end,marked_start,marked_end,selection_start,selection_length;
    bool active,ignore_text,seen,had_preedit;
    SBCompositionEvent *deferred;
    size_t deferred_count;
    bool ready,replaying,wait_for_frame;
    bool baseline_valid,display_pending;
};
bool sb_ui_composition_active(const SBUi *ui){return ui->composition && ui->composition->active;}
bool sb_ui_composition_committing(const SBUi *ui){return ui->composition && ui->composition->commit;}
bool sb_ui_composition_working(const SBUi *ui){return ui->composition && (ui->composition->commit || ui->composition->deferred_count);}
static size_t composition_byte(const char *text,size_t length,int characters) {
    size_t at=0;nk_rune rune;
    while(at<length && characters-->0)at+=(size_t)nk_utf_decode(text+at,&rune,(int)(length-at));
    return at;
}
static void composition_drop(SBUi *ui,bool native) {
    SBComposition *c=ui->composition;if(!c)return;
    bool pending=c->active || c->commit;
    free(c->preedit);free(c->commit);c->preedit=c->commit=NULL;c->active=false;
    if(native && pending && ui->window)SDL_ClearComposition(ui->window);
}
void sb_ui_composition_cancel(SBUi *ui) {
    if(ui->composition && (ui->composition->active || ui->composition->commit)) {
        composition_drop(ui,true);ui->composition->ignore_text=true;
    }
}
static SBComposition *composition_begin(SBUi *ui) {
    if(!ui->input_area_applied)return NULL;
    if(!ui->composition)ui->composition=calloc(1,sizeof(*ui->composition));
    SBComposition *c=ui->composition;
    if(!c){ui->input_status=sb_error(SB_IO,"Texteingabe: nicht genug Arbeitsspeicher.");return NULL;}
    if(!c->active && !c->commit) {
        c->window=ui->input_window;c->id=ui->input_id;c->hash=ui->input_hash;
        c->cursor=ui->input_cursor;c->start=ui->input_start;c->end=ui->input_end;
        c->baseline_valid=false;
    }
    return c;
}
bool sb_ui_composition_event(SBUi *ui,const SDL_Event *event) {
    SBComposition *c=ui->composition;
    bool key=event->type==SDL_EVENT_KEY_DOWN || event->type==SDL_EVENT_KEY_UP;
    if(c && !c->replaying && (c->deferred_count || c->wait_for_frame || c->commit) &&
       (key || event->type==SDL_EVENT_TEXT_INPUT || event->type==SDL_EVENT_TEXT_EDITING || event->type==SDL_EVENT_MOUSE_BUTTON_DOWN ||
        event->type==SDL_EVENT_MOUSE_BUTTON_UP || event->type==SDL_EVENT_WINDOW_FOCUS_LOST ||
        event->type==SDL_EVENT_QUIT || event->type==SDL_EVENT_WINDOW_CLOSE_REQUESTED)) {
        if(key && c->had_preedit && !c->deferred_count && (event->key.key==SDLK_RETURN || event->key.key==SDLK_KP_ENTER))return true;
        /* Confirmed chunks before a focus action still form one transaction. */
        if(!c->deferred_count && !c->wait_for_frame &&
           (event->type==SDL_EVENT_TEXT_INPUT || event->type==SDL_EVENT_TEXT_EDITING))goto text_event;
        SBCompositionEvent item={0};item.event=*event;
        item.boundary=event->type!=SDL_EVENT_KEY_UP && event->type!=SDL_EVENT_MOUSE_BUTTON_UP && event->type!=SDL_EVENT_TEXT_EDITING;
        if(event->type==SDL_EVENT_TEXT_INPUT || event->type==SDL_EVENT_TEXT_EDITING){
            const char *text=event->type==SDL_EVENT_TEXT_INPUT ? event->text.text : event->edit.text;
            if(!text)text="";size_t bytes=strlen(text);
            if(bytes>SB_TEXT_LIMIT){ui->input_status=sb_error(SB_LIMIT,"Vorgemerkte Texteingabe ist zu lang.");return true;}
            item.text=malloc(bytes+1);if(!item.text){ui->input_status=sb_error(SB_IO,"Texteingabe: nicht genug Arbeitsspeicher.");return true;}
            memcpy(item.text,text,bytes+1);
            if(event->type==SDL_EVENT_TEXT_INPUT)item.event.text.text=item.text;else item.event.edit.text=item.text;
        }
        SBCompositionEvent *queue=realloc(c->deferred,(c->deferred_count+1)*sizeof(*queue));
        if(!queue){free(item.text);ui->input_status=sb_error(SB_IO,"Texteingabe: Aktion konnte nicht vorgemerkt werden.");return true;}
        c->deferred=queue;c->deferred[c->deferred_count++]=item;return true;
    }
    if(event->type==SDL_EVENT_WINDOW_FOCUS_LOST || event->type==SDL_EVENT_MOUSE_BUTTON_DOWN) {
        sb_ui_composition_cancel(ui);return false;
    }
    if(event->type==SDL_EVENT_KEY_DOWN || event->type==SDL_EVENT_KEY_UP) {
        if(!c || !c->active)return false;
        if(event->key.key==SDLK_TAB){sb_ui_composition_cancel(ui);return false;}
        if(event->type==SDL_EVENT_KEY_DOWN && event->key.key==SDLK_ESCAPE)sb_ui_composition_cancel(ui);
        return true;
    }
text_event:;
    bool editing=event->type==SDL_EVENT_TEXT_EDITING;
    if(!editing && event->type!=SDL_EVENT_TEXT_INPUT)return false;
    SDL_WindowID id=editing ? event->edit.windowID : event->text.windowID;
    if(id && id!=SDL_GetWindowID(ui->window))return true;
    const char *text=editing ? event->edit.text : event->text.text;
    if(!text)text="";
    size_t length=strlen(text);
    if(length>SB_TEXT_LIMIT || !sb_utf8_valid(text,length)) {
        ui->input_status=sb_error(SB_INVALID,"Texteingabe ist zu lang oder enthält ungültiges UTF-8.");
        sb_ui_composition_cancel(ui);return true;
    }
    if(c && c->ignore_text)return true;
    if(!length) {if(editing && c){free(c->preedit);c->preedit=NULL;c->active=false;}return true;}
    if(!editing && length<NK_INPUT_MAX && (!c || (!c->active && !c->commit && !c->had_preedit)))return false;
    c=composition_begin(ui);
    if(!c)return editing; /* Keep direct low-level input usable before a first widget. */
    if(editing) {
        char *copy=malloc(length+1);
        if(!copy){ui->input_status=sb_error(SB_IO,"Texteingabe: nicht genug Arbeitsspeicher.");sb_ui_composition_cancel(ui);return true;}
        memcpy(copy,text,length+1);free(c->preedit);c->preedit=copy;c->active=true;c->had_preedit=true;
        int characters=nk_utf_len(text,(int)length);
        c->selection_start=event->edit.start<0 ? characters : NK_MIN(event->edit.start,characters);
        c->selection_length=event->edit.length<0 ? 0 : NK_MIN(event->edit.length,characters-c->selection_start);
    } else {
        size_t previous=c->commit ? strlen(c->commit) : 0;
        if(previous>SB_TEXT_LIMIT-length){ui->input_status=sb_error(SB_LIMIT,"Texteingabe überschreitet die Textgrenze.");sb_ui_composition_cancel(ui);return true;}
        char *copy=realloc(c->commit,previous+length+1);
        if(!copy){ui->input_status=sb_error(SB_IO,"Texteingabe: nicht genug Arbeitsspeicher.");sb_ui_composition_cancel(ui);return true;}
        memcpy(copy+previous,text,length+1);c->commit=copy;free(c->preedit);c->preedit=NULL;c->active=false;
    }
    ui->caret_epoch=SDL_GetTicksNS();return true;
}
static void sb_ui_composition_prepare(struct nk_context *ctx,struct nk_text_edit *edit,nk_hash id,nk_flags flags) {
    SBUi *ui=nk_sdl_get_userdata(ctx).ptr;
    edit->display_userdata=nk_handle_ptr(NULL);
    if(!ui || !edit->active || (flags&(NK_EDIT_READ_ONLY|NK_EDIT_NO_CURSOR)))return;
    SBComposition *c=ui->composition;
    if(!c || (!c->active && !c->commit) || ctx->current!=c->window || id!=c->id)return;
    uint64_t hash=sb_hash(nk_str_get_const(&edit->string),(size_t)nk_str_len_char(&edit->string));
    const char *source=nk_str_get_const(&edit->string);size_t bytes=(size_t)nk_str_len_char(&edit->string);
    bool changed=c->baseline_valid && (bytes!=c->baseline_length || (bytes && memcmp(c->baseline,source,bytes)));
    if(hash!=c->hash || changed || edit->cursor!=c->cursor || edit->select_start!=c->start || edit->select_end!=c->end) {
        ui->input_status=sb_error(SB_CONFLICT,"Das Textfeld wurde während der Eingabe geändert. Bitte erneut eingeben.");
        sb_ui_composition_cancel(ui);return;
    }
    c->seen=true;
    if(c->commit) {
        if(!sb_ui_grapheme_text(edit,c->commit,(int)strlen(c->commit))) {
            ui->input_status=sb_error(SB_LIMIT,"Texteingabe passt nicht in das Feld. Der bisherige Inhalt bleibt erhalten.");
            composition_drop(ui,false);return;
        }
        free(c->commit);c->commit=NULL;
        c->hash=sb_hash(nk_str_get_const(&edit->string),(size_t)nk_str_len_char(&edit->string));
        c->cursor=edit->cursor;c->start=edit->select_start;c->end=edit->select_end;
    }
    if(c->active){
        source=nk_str_get_const(&edit->string);bytes=(size_t)nk_str_len_char(&edit->string);
        if(c->baseline_capacity<bytes+1){char *copy=realloc(c->baseline,bytes+1);if(!copy){ui->input_status=sb_error(SB_IO,"Texteingabe: nicht genug Arbeitsspeicher.");sb_ui_composition_cancel(ui);return;}c->baseline=copy;c->baseline_capacity=bytes+1;}
        if(bytes)memcpy(c->baseline,source,bytes);c->baseline[bytes]=0;c->baseline_length=bytes;c->baseline_valid=true;
        edit->display_userdata=nk_handle_ptr(ui);c->display_pending=true;
    }
}
static struct nk_text_edit *sb_ui_composition_display(struct nk_text_edit *edit) {
    SBUi *ui=edit->display_userdata.ptr;
    if(!ui || !edit->active)return edit;
    SBComposition *c=ui->composition;
    if(!c || !c->active || !c->preedit || !c->display_pending)return edit;
    c->display_pending=false;
    const char *source=nk_str_get_const(&edit->string);
    size_t bytes=(size_t)nk_str_len_char(&edit->string),marked=strlen(c->preedit);
    int first=NK_MIN(edit->select_start,edit->select_end),last=NK_MAX(edit->select_start,edit->select_end);
    first=sb_ui_grapheme_index(edit,first,-2);last=sb_ui_grapheme_index(edit,last,2);
    size_t begin=composition_byte(source,bytes,first),end=composition_byte(source,bytes,last);
    size_t retained=bytes-(end-begin);
    if(retained>SB_TEXT_LIMIT || marked>SB_TEXT_LIMIT-retained){ui->input_status=sb_error(SB_LIMIT,"Die vorläufige Texteingabe ist zu lang.");sb_ui_composition_cancel(ui);return edit;}
    size_t length=retained+marked;
    if(c->render_capacity<length+1) {
        char *copy=realloc(c->render_text,length+1);
        if(!copy){ui->input_status=sb_error(SB_IO,"Texteingabe: nicht genug Arbeitsspeicher.");sb_ui_composition_cancel(ui);return edit;}
        c->render_text=copy;c->render_capacity=length+1;
    }
    if(!c->visual)c->visual=malloc(sizeof(*c->visual));
    if(!c->visual){ui->input_status=sb_error(SB_IO,"Texteingabe: nicht genug Arbeitsspeicher.");sb_ui_composition_cancel(ui);return edit;}
    if(begin)memcpy(c->render_text,source,begin);
    memcpy(c->render_text+begin,c->preedit,marked);
    if(bytes>end)memcpy(c->render_text+begin+marked,source+end,bytes-end);
    c->render_text[length]=0;
    *c->visual=*edit;
    nk_str_init_fixed(&c->visual->string,c->render_text,length+1);
    c->visual->string.buffer.allocated=length;c->visual->string.len=nk_utf_len(c->render_text,(int)length);
    c->visual->mode=NK_TEXT_EDIT_MODE_INSERT;
    c->marked_start=first;c->marked_end=first+nk_utf_len(c->preedit,(int)marked);
    c->visual->select_start=first+c->selection_start;
    c->visual->select_end=c->visual->select_start+c->selection_length;
    c->visual->cursor=c->visual->select_start;
    return c->visual;
}
static void sb_ui_composition_mark(struct nk_command_buffer *out,struct nk_text_edit *edit,const struct nk_style_edit *style,const struct nk_user_font *font,struct nk_rect area,float row_height) {
    SBUi *ui=edit->display_userdata.ptr;SBComposition *c=ui ? ui->composition : NULL;
    if(!c || !c->active || edit!=c->visual)return;
    const char *text=c->render_text;size_t bytes=strlen(text),at=0,line_begin=0;int index=0,line_index=0,line=0;
    while(at<=bytes) {
        if(at==bytes || text[at]=='\n') {
            int first=NK_MAX(line_index,c->marked_start),last=NK_MIN(index,c->marked_end);
            if(first<last) {
                size_t a=composition_byte(text+line_begin,at-line_begin,first-line_index);
                size_t b=composition_byte(text+line_begin,at-line_begin,last-line_index);
                float x=area.x-edit->scrollbar.x;
                float left=x+font->width(font->userdata,font->height,text+line_begin,(int)a);
                float right=x+font->width(font->userdata,font->height,text+line_begin,(int)b);
                float y=area.y+line*row_height-edit->scrollbar.y+(row_height+font->height)/2+1;
                left=fmaxf(left,out->clip.x);right=fminf(right,out->clip.x+out->clip.w);
                if(right>left && y>=out->clip.y && y<=out->clip.y+out->clip.h)
                    nk_stroke_line(out,left,y,right,y,1,style->text_normal);
            }
            if(at==bytes)break;
            ++at;++index;++line;line_begin=at;line_index=index;
        } else {nk_rune rune;at+=(size_t)nk_utf_decode(text+at,&rune,(int)(bytes-at));++index;}
    }
}
static void sb_ui_composition_frame_end(SBUi *ui) {
    SBComposition *c=ui->composition;if(!c)return;
    if((c->active || c->commit) && (!c->seen || !ui->input_area_applied))sb_ui_composition_cancel(ui);
    c->seen=false;c->ignore_text=false;
    if(!c->active)c->had_preedit=false;
    c->wait_for_frame=false;c->ready=c->deferred_count!=0;
}
void sb_ui_composition_replay(SBUi *ui,void (*dispatch)(void *,const SDL_Event *),void *user) {
    SBComposition *c=ui->composition;if(!c || !c->ready || c->replaying)return;
    c->ready=false;c->replaying=true;
    size_t used=0;
    while(used<c->deferred_count){
        SBCompositionEvent item=c->deferred[used++];dispatch(user,&item.event);free(item.text);
        if(item.boundary){c->wait_for_frame=true;break;}
    }
    memmove(c->deferred,c->deferred+used,(c->deferred_count-used)*sizeof(*c->deferred));c->deferred_count-=used;c->replaying=false;
}
static void sb_ui_composition_free(SBUi *ui) {
    SBComposition *c=ui->composition;if(!c)return;
    for(size_t i=0;i<c->deferred_count;++i)free(c->deferred[i].text);
    free(c->preedit);free(c->commit);free(c->render_text);free(c->baseline);free(c->visual);free(c->deferred);free(c);ui->composition=NULL;
}
