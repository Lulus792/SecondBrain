/* One read-only help excursion. Form values remain owned by SBDesktop. */
struct SBHelpReturn {
    SBForm form,measured_form;
    SBScroll body,reader;
    SBFieldSelection selections[SB_NATIVE_FIELDS];
    SBStatus message;
    char focus[100];
    int form_focus,active_field;
    float height,width,scale;
    unsigned generation;
    bool keyboard;
};
static void help_return_free(SBDesktop *d) {
    if (!d->help_return) return;
    for (size_t i=0;i<SB_NATIVE_FIELDS;++i) free(d->help_return->selections[i].source);
    free(d->help_return);d->help_return=NULL;
}
static void help_open(SBDesktop *d) {
    if (d->form==SB_FORM_HELP || d->model.guard || d->backup) return;
    help_return_free(d);
    if (d->form!=SB_FORM_NONE) {
        SBHelpReturn *saved=calloc(1,sizeof(*saved));
        if (!saved) {d->message=sb_error(SB_MEMORY,"Hilfe konnte nicht geöffnet werden.");return;}
        saved->form=d->form;saved->measured_form=d->measured_form;
        saved->body=d->scrolling[3];saved->reader=d->scrolling[2];saved->message=d->message;
        saved->body.dragging=false;saved->reader.dragging=false;
        saved->form_focus=d->form_focus;saved->active_field=d->active_form_field;
        saved->generation=d->model.generation;saved->keyboard=d->keyboard;
        saved->height=d->modal_content_height;saved->width=d->modal_content_width;saved->scale=d->modal_content_scale;
        snprintf(saved->focus,sizeof(saved->focus),"%s",d->focus);
        for (size_t i=0;i<SB_NATIVE_FIELDS;++i) {
            saved->selections[i]=d->field_selections[i];memset(&d->field_selections[i],0,sizeof(d->field_selections[i]));
        }
        d->help_return=saved;
    }
    sb_ui_input_barrier(&d->ui);d->message=sb_ok();d->form=SB_FORM_HELP;
}
static bool help_close(SBDesktop *d) {
    SBHelpReturn *saved=d->help_return;
    if (d->form!=SB_FORM_HELP || !saved) return false;
    if (saved->generation!=d->model.generation) {help_return_free(d);return false;}
    field_selections_free(d);memset(d->field_selections,0,sizeof(d->field_selections));
    for (size_t i=0;i<SB_NATIVE_FIELDS;++i) {
        d->field_selections[i]=saved->selections[i];saved->selections[i].source=NULL;
        d->field_selections[i].pending=d->field_selections[i].source!=NULL;
    }
    d->form=saved->form;d->focus_form=saved->form;d->focus_guard=false;
    d->scrolling[3]=saved->body;d->scrolling[2]=saved->reader;d->help_restore_scroll=true;
    d->message=saved->message;d->form_focus=saved->form_focus;d->active_form_field=saved->active_field;
    d->measured_form=saved->measured_form;d->modal_content_height=saved->height;
    d->modal_content_width=saved->width;d->modal_content_scale=saved->scale;
    snprintf(d->focus,sizeof(d->focus),"%s",saved->focus);
    d->focus_changed=true;d->focus_scroll_frames=0;d->keyboard=saved->keyboard;
    help_return_free(d);return true;
}
