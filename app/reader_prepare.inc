/* The reader owns at most one batch. Retargeting cancels it without joining;
   the next frame starts only the latest document after the old worker exits. */
struct SBReaderPreparation {
    SBPrepareBatch *batch;
    char *source;
    float width,scale,density;
    uint64_t epoch,context;
    bool code,bound;
};
typedef struct {SBPrepareRequest *items;size_t count,capacity;bool failed;} SBReaderRequests;
static void reader_requests_free(SBReaderRequests *r){for(size_t i=0;i<r->count;++i)sb_styled_free(&r->items[i].text);free(r->items);}
static bool reader_request_add(SBReaderRequests *r,SBStyledText *text,unsigned role,float width,SBUi *ui,bool literal){
    if(r->count==SB_PREPARE_REQUEST_LIMIT){r->failed=true;return false;}
    if(r->count==r->capacity){size_t cap=r->capacity ? r->capacity*2 : 32;SBPrepareRequest *items=realloc(r->items,cap*sizeof(*items));if(!items){r->failed=true;return false;}r->items=items;r->capacity=cap;}
    struct nk_vec2 padding=ui->ctx->style.text.padding;
    r->items[r->count++]=(SBPrepareRequest){.text=*text,.role=role,.width=width,.padding=padding,.gap=literal ? 2*padding.y : 4*ui->scale,.terminal_line=literal};
    *text=(SBStyledText){0};return true;
}
static bool reader_request_text(SBReaderRequests *r,SBUi *ui,const char *text,size_t length,unsigned role,float width,const SBReferences *references,bool literal){
    SBStyledText styled={0};SBInline in={0};SBStatus status=sb_ok();
    if(literal){styled.text=malloc(length+1);styled.spans=length ? malloc(sizeof(*styled.spans)) : NULL;
        if(!styled.text || (length && !styled.spans))status=sb_error(SB_MEMORY,"Kein Speicher für Dokumentvorbereitung.");
        else{memcpy(styled.text,text,length);styled.text[length]=0;styled.count=length ? 1 : 0;if(length)styled.spans[0]=(SBTextSpan){0,length,0};}}
    else{status=sb_inline_init_references(&in,text,length,references);if(status.code==SB_OK)status=sb_inline_styled(&in,0,length,&styled);}
    bool ok=status.code==SB_OK && reader_request_add(r,&styled,role,width,ui,literal);
    sb_styled_free(&styled);sb_inline_free(&in);if(!ok)r->failed=true;
    /* Height measurement uses panel width; nk_widget truncates paint width.
       Retain both exact plans rather than silently changing existing wraps. */
    float painted=(float)(int)width;
    if(ok && painted>0 && painted!=width)ok=reader_request_text(r,ui,text,length,role,painted,references,literal);
    return ok;
}
static bool reader_request_table(SBReaderRequests *r,SBUi *ui,const SBDocumentNode *node,float width,const SBReferences *references){
    SBTable table;if(!sb_table_parse(node->view.text,node->view.length,node->content,&table))return false;
    bool stacked=width/table.columns<130*ui->scale;
    float column=stacked ? width : width/table.columns;
    float draw_width=column-16*ui->scale,measure_width=fmaxf(40,draw_width-4*ui->scale);
    size_t cursor=table.offset;SBTableRow row;bool header=true;
    while(sb_table_next(&table,&cursor,&row)){
        for(size_t c=0;c<table.columns;++c){char *raw=NULL;
            if(sb_table_cell_text(&table,&row.cells[c],&raw).code!=SB_OK){r->failed=true;return false;}
            bool ok=reader_request_text(r,ui,raw,strlen(raw),1,measure_width,references,false) &&
                reader_request_text(r,ui,raw,strlen(raw),1,draw_width,references,false);
            if(ok && stacked && header)ok=reader_request_text(r,ui,raw,strlen(raw),0,measure_width,references,false) &&
                reader_request_text(r,ui,raw,strlen(raw),0,draw_width,references,false);
            free(raw);if(!ok)return false;
        }header=false;
    }return true;
}
static bool reader_requests_collect(SBDesktop *d,const char *text,bool code,const SBDocument *document,const SBReferences *references,float width,SBReaderRequests *out){
    if(code){SBMarkdown markdown;SBMarkdownBlock block;sb_markdown_init(&markdown,text,strlen(text),true);
        while(sb_markdown_next(&markdown,&block))if(block.kind==SB_MD_CODE && block.length &&
            !reader_request_text(out,&d->ui,text+block.content,block.length,3,width,references,true))return false;
        return true;}
    SBDocumentLayout *layout=calloc(document->count,sizeof(*layout));if(!layout)return false;
    bool ok=true;float s=d->ui.scale;
    for(size_t i=1;i<document->count && ok;++i){const SBDocumentNode *node=&document->nodes[i];SBDocumentLayout *entry=&layout[i];
        entry->inset=layout[node->parent].inset;
        if(node->kind==SB_DOC_QUOTE)entry->inset+=24*s;
        if(node->kind==SB_DOC_ITEM)entry->inset+=layout[node->parent].marker_width+8*s;
        float available=width-fminf(entry->inset,fmaxf(0,width-100*s));
        if(node->kind==SB_DOC_LIST){entry->marker_width=16*s;
            if(node->ordered){unsigned number=node->start;const struct nk_user_font *font=&d->ui.body->handle;
                for(size_t child=node->first;child!=SIZE_MAX;child=document->nodes[child].next){char label[32];snprintf(label,sizeof(label),"%u%c",number++,node->marker);entry->marker_width=fmaxf(entry->marker_width,font->width(font->userdata,font->height,label,(int)strlen(label)));}}continue;}
        if(node->kind==SB_DOC_ITEM || node->kind==SB_DOC_QUOTE || node->kind==SB_DOC_RULE)continue;
        const char *content=NULL;size_t length=0;if(sb_document_content(node,&content,&length).code!=SB_OK){ok=false;break;}
        if(!length)continue;
        if(node->kind==SB_DOC_TABLE){ok=reader_request_table(out,&d->ui,node,available,references);continue;}
        if(node->kind==SB_DOC_CODE || node->kind==SB_DOC_RAW){for(size_t at=0;at<length && ok;){size_t end=at;while(end<length && content[end]!='\n')++end;
                if(end>at)ok=reader_request_text(out,&d->ui,content+at,end-at,3,available,references,true);at=end<length ? end+1 : end;}}
        else ok=reader_request_text(out,&d->ui,content,length,node->kind==SB_DOC_HEADING ? 2 : 1,available,references,false);
    }
    free(layout);return ok && !out->failed;
}
static const SBStyledPlan *reader_prepared_lookup(void *data,const struct nk_user_font *font,const SBStyledText *text,float width,struct nk_vec2 padding,float gap,bool terminal){
    SBDesktop *d=data;unsigned role;
    if(font==&d->ui.normal->handle)role=0;else if(font==&d->ui.body->handle)role=1;else if(font==&d->ui.heading->handle)role=2;else if(font==&d->ui.code->handle)role=3;else return NULL;
    SBPrepareRequest r={.text=*text,.role=role,.width=width,.padding=padding,.gap=gap,.terminal_line=terminal};
    const SBStyledPlan *plan=sb_prepare_batch_lookup(d->reader_preparation->batch,&d->ui,&r);
    if(plan)++d->reader_plan_hits;else ++d->reader_plan_misses;
    return plan;
}
static void reader_preparation_free(SBDesktop *d){
    if(!d->reader_preparation)return;
    sb_prepare_batch_free(d->reader_preparation->batch);free(d->reader_preparation->source);free(d->reader_preparation);d->reader_preparation=NULL;
}
static bool reader_preparing(const SBDesktop *d){return d->reader_preparation && !d->reader_preparation->bound;}
static void reader_preparation_retire(SBDesktop *d){
    SBReaderPreparation *p=d->reader_preparation;if(!p || d->reader_preparation_used)return;
    sb_prepare_batch_cancel(p->batch);
    if(sb_prepare_batch_try_free(p->batch)){free(p->source);free(p);d->reader_preparation=NULL;}
}
static bool reader_preparation_ready(SBDesktop *d,const char *text,bool code,const SBDocument *document,const SBReferences *references,float width){
    /* Short documents retain immediate display; no artificial wait or timer. */
    if(strlen(text)<4096)return true;
    /* A modal reader and its obscured document must not continually replace
       each other's single worker. Only the foreground reader owns preparation. */
    if(d->form!=SB_FORM_NONE && (!modal_reader(d) || (text!=d->context && text!=d->notice)))return false;
    d->reader_preparation_used=true;SBReaderPreparation *p=d->reader_preparation;
    uint64_t context=accessible_context(d);
    bool same=p && p->context==context && p->epoch==d->ui.font_epoch && p->scale==d->ui.scale && p->density==d->ui.density && p->width==width && p->code==code && !strcmp(p->source,text);
    if(p && sb_prepare_batch_state(p->batch,NULL)==SB_PREPARE_CANCELLED)same=false;
    if(p && !same){sb_prepare_batch_cancel(p->batch);
        if(!sb_prepare_batch_try_free(p->batch)){p->bound=false;return false;}
        free(p->source);free(p);d->reader_preparation=p=NULL;
    }
    if(!p){SBReaderRequests requests={0};
        bool collected=reader_requests_collect(d,text,code,document,references,width,&requests);
        if(!collected || !requests.count){reader_requests_free(&requests);return true;}
        p=calloc(1,sizeof(*p));SBTextSnapshot *fonts=sb_ui_text_snapshot(&d->ui);
        if(p){size_t length=strlen(text);p->source=malloc(length+1);if(p->source)memcpy(p->source,text,length+1);}
        if(p && p->source && fonts)p->batch=sb_prepare_batch_start(fonts,requests.items,requests.count,context);
        reader_requests_free(&requests);
        if(!p || !p->batch){sb_text_snapshot_free(fonts);if(p){free(p->source);free(p);}return true;}
        p->context=context;p->epoch=d->ui.font_epoch;p->width=width;p->scale=d->ui.scale;p->density=d->ui.density;p->code=code;d->reader_preparation=p;
        if(!d->reduced_motion && d->form==SB_FORM_NONE && !d->model.source){sb_ui_transition_tick(&d->ui,1,true);d->ui.capture_pending=true;}
    }
    SBStatus status;SBPrepareState state=sb_prepare_batch_state(p->batch,&status);
    if(state==SB_PREPARE_PENDING)return false;
    if(state!=SB_PREPARE_READY){d->message=status;p->bound=true;return true;}
    if(!p->bound){status=sb_prepare_batch_bind(p->batch,&d->ui,context);if(status.code!=SB_OK){d->message=status;p->bound=true;sb_prepare_batch_cancel(p->batch);return true;}p->bound=true;
        if(!d->reduced_motion && d->form==SB_FORM_NONE && !d->model.source)sb_ui_transition_begin(&d->ui);}
    d->ui.prepared_lookup=reader_prepared_lookup;d->ui.prepared_data=d;return true;
}
