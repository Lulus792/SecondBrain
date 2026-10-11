/* Capture string widgets immediately: Nuklear reuses ctx->text_edit for
   every field, so its source and painted area are temporary. */
static const char *const native_field_ids[SB_NATIVE_FIELDS]={
    "search","form-name","form-id","form-repo","form-folder","backup-path","restore-id"
};
static SBFieldText *native_field(SBDesktop *d,const char *id){
    for(size_t i=0;i<SB_NATIVE_FIELDS;++i)if(!strcmp(id,native_field_ids[i]))return &d->native_fields[i];
    return NULL;
}
static SBFieldSelection *field_selection(SBDesktop *d,const char *id){
    SBFieldText *field=native_field(d,id);return field ? &d->field_selections[field-d->native_fields] : NULL;
}
static bool field_selection_matches(SBDesktop *d,const SBFieldSelection *selection,const char *source){
    return selection && selection->source && selection->context==accessible_context(d) && !strcmp(selection->source,source);
}
static bool field_selection_store(SBDesktop *d,SBFieldSelection *selection,const char *source,size_t anchor,size_t caret,bool pending){
    if(!field_selection_matches(d,selection,source)){
        char *copy=malloc(strlen(source)+1);if(!copy)return false;strcpy(copy,source);free(selection->source);selection->source=copy;
    }
    selection->context=accessible_context(d);selection->anchor=anchor;selection->caret=caret;selection->pending=pending;return true;
}
static void field_selections_free(SBDesktop *d){
    for(size_t i=0;i<SB_NATIVE_FIELDS;++i)free(d->field_selections[i].source);
}
static size_t field_byte(const char *source,size_t scalar){
    const char *p=source;while(scalar-- && *p)SDL_StepUTF8(&p,NULL);return (size_t)(p-source);
}
static bool field_range(const char *source,size_t *anchor,size_t *caret){
    size_t total=(size_t)nk_utf_len(source,(int)strlen(source));if(*anchor>total || *caret>total)return false;
    SBGraphemePosition a,b;if(!sb_grapheme_position(source,strlen(source),*anchor,&a) || !sb_grapheme_position(source,strlen(source),*caret,&b))return false;
    if(*anchor==*caret)*anchor=*caret=a.floor;
    else if(*anchor<*caret){*anchor=a.floor;*caret=b.ceil;}
    else{*anchor=a.ceil;*caret=b.floor;}return true;
}
static SBStatus native_field_action(SBDesktop *d,const SBAccessibleAction *action,char *source,size_t capacity){
    SBFieldSelection *selection=field_selection(d,action->id);SBFieldText *snapshot=native_field(d,action->id);
    if(!selection || !source)return sb_error(SB_INVALID,"Textfeld ist nicht verfügbar.");
    bool current=field_selection_matches(d,selection,source);
    if(action->action!=ACCESSKIT_ACTION_SET_VALUE && snapshot->source && strcmp(snapshot->source,source) && !(current && selection->pending))
        return sb_error(SB_CONFLICT,"Das Textfeld wurde geändert. Bitte erneut auswählen.");
    size_t anchor=current ? selection->anchor : 0,caret=current ? selection->caret : 0;
    if(action->action==ACCESSKIT_ACTION_SET_TEXT_SELECTION){
        anchor=action->anchor;caret=action->caret;if(!field_range(source,&anchor,&caret))return sb_error(SB_INVALID,"Textauswahl liegt außerhalb des Feldes.");
        return field_selection_store(d,selection,source,anchor,caret,true) ? sb_ok() : sb_error(SB_MEMORY,"Textauswahl benötigt mehr Speicher.");
    }
    const char *value=action->value;size_t length=value ? strlen(value) : 0;
    if(!value || !sb_utf8_valid(value,length) || strchr(value,'\n') || strchr(value,'\r'))return sb_error(SB_INVALID,"Eingabe ist ungültig oder zu lang.");
    size_t bytes=strlen(source),first=0,last=bytes;
    if(action->action==ACCESSKIT_ACTION_REPLACE_SELECTED_TEXT){
        if(!field_range(source,&anchor,&caret))return sb_error(SB_INVALID,"Textauswahl liegt außerhalb des Feldes.");
        first=field_byte(source,anchor<caret ? anchor : caret);last=field_byte(source,anchor>caret ? anchor : caret);
    }
    if(length>=capacity || bytes-(last-first)>=capacity-length)return sb_error(SB_LIMIT,"Eingabe ist zu lang. Der bisherige Inhalt bleibt erhalten.");
    size_t next_length=first+length+bytes-last;char *next=malloc(next_length+1);if(!next)return sb_error(SB_MEMORY,"Texteingabe benötigt mehr Speicher.");
    memcpy(next,source,first);memcpy(next+first,value,length);memcpy(next+first+length,source+last,bytes-last+1);
    caret=(size_t)nk_utf_len(next,(int)(first+length));SBGraphemePosition position;
    if(!sb_grapheme_position(next,next_length,caret,&position)){free(next);return sb_error(SB_INVALID,"Eingabe ist ungültig.");}
    anchor=caret=position.ceil;
    bool stored=field_selection_store(d,selection,next,anchor,caret,true);if(stored)memcpy(source,next,next_length+1);free(next);
    return stored ? sb_ok() : sb_error(SB_MEMORY,"Texteingabe benötigt mehr Speicher.");
}
static void native_fields_clear(SBDesktop *d){
    for(size_t i=0;i<SB_NATIVE_FIELDS;++i){SBFieldText *field=&d->native_fields[i];
        sb_native_text_free(&field->text);free(field->source);memset(field,0,sizeof(*field));
        SBFieldSelection *selection=&d->field_selections[i];
        if(selection->source && selection->context!=accessible_context(d)){free(selection->source);memset(selection,0,sizeof(*selection));}}
}
static nk_flags native_field_edit(SBDesktop *d,const char *id,char *source,int *length,int capacity){
    SBUi *ui=&d->ui;struct nk_text_edit *edit=&ui->ctx->text_edit;
    SBFieldSelection *selection=field_selection(d,id);
    if(selection && selection->pending && !field_selection_matches(d,selection,source))selection->pending=false;
    if(selection && selection->pending && !strcmp(d->focus,id)){
        nk_edit_focus(ui->ctx,NK_EDIT_ALWAYS_INSERT_MODE);struct nk_window *window=ui->ctx->current;
        window->edit.cursor=(int)selection->caret;window->edit.sel_start=(int)selection->anchor;window->edit.sel_end=(int)selection->caret;window->edit.visual_valid=0;
    }
    /* A clipped widget must not inherit the previous field's paint record. */
    ui->edit_paint.edit=NULL;
    nk_flags state=nk_edit_string(ui->ctx,NK_EDIT_FIELD,source,length,capacity,nk_filter_default);
    source[*length]=0;
    if(selection && (state&NK_EDIT_ACTIVE))field_selection_store(d,selection,source,(size_t)edit->select_start,(size_t)edit->select_end,false);
    SBFieldText *field=native_field(d,id);
    if(field && ui->edit_paint.edit==edit){
        field->source=malloc((size_t)*length+1);
        if(field->source){memcpy(field->source,source,(size_t)*length+1);
            field->anchor=(size_t)edit->select_start;field->caret=(size_t)edit->select_end;
            sb_ui_field_native(ui,edit,&field->text);}
    }
    return state;
}
