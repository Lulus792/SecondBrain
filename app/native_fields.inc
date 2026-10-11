/* Capture string widgets immediately: Nuklear reuses ctx->text_edit for
   every field, so its source and painted area are temporary. */
static const char *const native_field_ids[SB_NATIVE_FIELDS]={
    "search","form-name","form-id","form-repo","form-folder","backup-path","restore-id"
};
static SBFieldText *native_field(SBDesktop *d,const char *id){
    for(size_t i=0;i<SB_NATIVE_FIELDS;++i)if(!strcmp(id,native_field_ids[i]))return &d->native_fields[i];
    return NULL;
}
static void native_fields_clear(SBDesktop *d){
    for(size_t i=0;i<SB_NATIVE_FIELDS;++i){SBFieldText *field=&d->native_fields[i];
        sb_native_text_free(&field->text);free(field->source);memset(field,0,sizeof(*field));}
}
static nk_flags native_field_edit(SBDesktop *d,const char *id,char *source,int *length,int capacity){
    SBUi *ui=&d->ui;struct nk_text_edit *edit=&ui->ctx->text_edit;
    /* A clipped widget must not inherit the previous field's paint record. */
    ui->edit_paint.edit=NULL;
    nk_flags state=nk_edit_string(ui->ctx,NK_EDIT_FIELD,source,length,capacity,nk_filter_default);
    source[*length]=0;
    SBFieldText *field=native_field(d,id);
    if(field && ui->edit_paint.edit==edit){
        field->source=malloc((size_t)*length+1);
        if(field->source){memcpy(field->source,source,(size_t)*length+1);
            field->anchor=(size_t)edit->select_start;field->caret=(size_t)edit->select_end;
            sb_ui_field_native(ui,edit,&field->text);}
    }
    return state;
}
