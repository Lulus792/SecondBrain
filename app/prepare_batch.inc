/* Complete reader batches retain their plans independently of the small UI
   memo cache. Only the worker writes until done; binding/lookup use UI ownership. */
typedef struct {
    SBPrepareRequest request;
    SBStyledPlan plan;
    uint8_t **tokens;
    size_t token_count;
    uint64_t hash;
    size_t next;
} SBPreparedBlock;
struct SBPrepareBatch {
    SBTextSnapshot *fonts;
    SBPreparedBlock *blocks;
    size_t count,input_bytes,output_bytes;
    uint64_t context,epoch;
    SDL_Thread *thread;
    SDL_ThreadID owner;
    SDL_AtomicInt done,cancel;
    SBStatus status;
    SBUi *bound;
    size_t buckets[4096];
};
static bool batch_charge(size_t *used,size_t count,size_t unit,size_t limit){
    if(*used>limit || (unit && count>(limit-*used)/unit))return false;
    *used+=count*unit;return true;
}
static bool batch_request_valid(const SBPrepareRequest *r){
    return r && r->text.text && r->role<4 && r->text.count<=SB_INLINE_LIMIT &&
        (!r->text.count || r->text.spans) && isfinite(r->width) && r->width>0 &&
        isfinite(r->gap) && r->gap>=0 && isfinite(r->padding.x) && r->padding.x>=0 &&
        isfinite(r->padding.y) && r->padding.y>=0;
}
static bool batch_request_equal(const SBPrepareRequest *a,const SBPrepareRequest *b){
    if(a->role!=b->role || a->width!=b->width || a->gap!=b->gap ||
       a->padding.x!=b->padding.x || a->padding.y!=b->padding.y ||
       a->terminal_line!=b->terminal_line || a->text.count!=b->text.count ||
       strcmp(a->text.text,b->text.text))return false;
    for(size_t i=0;i<a->text.count;++i){SBTextSpan x=a->text.spans[i],y=b->text.spans[i];
        if(x.offset!=y.offset || x.length!=y.length || x.style!=y.style)return false;}
    return true;
}
static void batch_blocks_free(SBPrepareBatch *b){
    for(size_t i=0;i<b->count;++i){SBPreparedBlock *p=&b->blocks[i];
        for(size_t l=0;l<p->token_count;++l)free(p->tokens[l]);
        free(p->tokens);sb_ui_styled_plan_free(&p->plan);sb_styled_free(&p->request.text);}
    free(b->blocks);
}
static bool batch_encode(SBPrepareBatch *b,SBPreparedBlock *p,SBUi *cpu){
    SBStyledPlan *plan=&p->plan;
    if(!batch_charge(&b->output_bytes,plan->capacity,sizeof(*plan->lines),SB_PREPARE_OUTPUT_BYTES) ||
       !batch_charge(&b->output_bytes,plan->count,sizeof(*p->tokens),SB_PREPARE_OUTPUT_BYTES) ||
       !batch_charge(&b->output_bytes,plan->native.capacity,sizeof(*plan->native.runs),SB_PREPARE_OUTPUT_BYTES))return false;
    p->tokens=plan->count ? calloc(plan->count,sizeof(*p->tokens)) : NULL;
    if(plan->count && !p->tokens)return false;
    for(size_t l=0;l<plan->count;++l){
        if(SDL_GetAtomicInt(&b->cancel))return false;
        SBShapedLine *line=&plan->lines[l].shape;
        if(!batch_charge(&b->output_bytes,line->capacity,sizeof(*line->glyphs),SB_PREPARE_OUTPUT_BYTES) ||
           !batch_charge(&b->output_bytes,line->run_capacity,sizeof(*line->runs),SB_PREPARE_OUTPUT_BYTES) ||
           !batch_charge(&b->output_bytes,line->count,1,SB_PREPARE_OUTPUT_BYTES) ||
           !batch_charge(&b->output_bytes,plan->lines[l].box_count,sizeof(SBNativeCharBox),SB_PREPARE_OUTPUT_BYTES))return false;
        p->tokens[l]=line->count ? malloc(line->count) : NULL;p->token_count=l+1;
        if(line->count && !p->tokens[l])return false;
        for(size_t g=0;g<line->count;++g){unsigned token=sb_ui_font_token(cpu,line->glyphs[g].font);
            if(token>UINT8_MAX)return false;p->tokens[l][g]=(uint8_t)token;}
    }
    return true;
}
static int batch_worker(void *data){
    SBPrepareBatch *b=data;SBUi cpu={0};b->status=sb_ok();
    if(!SDL_GetAtomicInt(&b->cancel))b->status=sb_text_snapshot_open(b->fonts,&cpu);
    cpu.layout_cancel=&b->cancel;
    for(size_t i=0;i<b->count && b->status.code==SB_OK && !SDL_GetAtomicInt(&b->cancel);++i){
        SBPreparedBlock *p=&b->blocks[i];SBPrepareRequest *r=&p->request;
        size_t remaining=SB_PREPARE_OUTPUT_BYTES-b->output_bytes;cpu.layout_budget=&remaining;
        if(!sb_ui_styled_plan(&cpu,layout_font(&cpu,r->role),&r->text,r->width,r->padding,r->gap,r->terminal_line,&p->plan))
            b->status=sb_error(SB_INVALID,"Dokumentlayout konnte nicht vorbereitet werden.");
        else if(!batch_encode(b,p,&cpu))b->status=sb_error(SB_MEMORY,"Dokumentvorbereitung überschreitet das Speicherbudget.");
    }
    cpu.layout_budget=NULL;
    /* Also clear partially produced plans on cancellation or allocation error. */
    for(size_t i=0;i<b->count;++i)for(size_t l=0;l<b->blocks[i].plan.count;++l)
        for(size_t g=0;g<b->blocks[i].plan.lines[l].shape.count;++g)b->blocks[i].plan.lines[l].shape.glyphs[g].font=NULL;
    sb_ui_text_free(&cpu);SDL_SetAtomicInt(&b->done,1);return 0;
}
SBPrepareBatch *sb_prepare_batch_start(SBTextSnapshot *fonts,const SBPrepareRequest *requests,size_t count,uint64_t context){
    if(!fonts || !requests || !count || count>SB_PREPARE_REQUEST_LIMIT)return NULL;
    SBPrepareBatch *b=calloc(1,sizeof(*b));if(!b)return NULL;
    b->blocks=calloc(count,sizeof(*b->blocks));if(!b->blocks){free(b);return NULL;}
    b->input_bytes=sizeof(*b)+count*sizeof(*b->blocks);b->context=context;b->fonts=fonts;
    size_t offered=b->input_bytes;
    for(size_t i=0;i<count;++i){const SBPrepareRequest *r=&requests[i];
        if(!batch_request_valid(r))goto fail;
        size_t length=strlen(r->text.text);if(length>SB_TEXT_LIMIT ||
           !batch_charge(&offered,length+1,1,SB_PREPARE_INPUT_BYTES) ||
           !batch_charge(&offered,r->text.count,sizeof(*r->text.spans),SB_PREPARE_INPUT_BYTES))goto fail;
        uint64_t hash=sb_hash(r->text.text,length);size_t bucket=hash%4096;bool duplicate=false;
        for(size_t at=b->buckets[bucket];at;at=b->blocks[at-1].next){SBPreparedBlock *held=&b->blocks[at-1];
            if(held->hash==hash && batch_request_equal(r,&held->request)){duplicate=true;break;}}
        if(duplicate)continue;
        if(
           !batch_charge(&b->input_bytes,length+1,1,SB_PREPARE_INPUT_BYTES) ||
           !batch_charge(&b->input_bytes,r->text.count,sizeof(*r->text.spans),SB_PREPARE_INPUT_BYTES))goto fail;
        SBPreparedBlock *p=&b->blocks[b->count++];p->request=*r;p->request.text=(SBStyledText){0};
        p->request.text.text=malloc(length+1);
        p->request.text.spans=r->text.count ? malloc(r->text.count*sizeof(*r->text.spans)) : NULL;
        if(!p->request.text.text || (r->text.count && !p->request.text.spans))goto fail;
        memcpy(p->request.text.text,r->text.text,length+1);p->request.text.count=r->text.count;
        if(r->text.count)memcpy(p->request.text.spans,r->text.spans,r->text.count*sizeof(*r->text.spans));
        p->hash=hash;p->next=b->buckets[bucket];b->buckets[bucket]=b->count;
    }
    b->thread=SDL_CreateThread(batch_worker,"SecondBrain document",b);
    if(b->thread)return b;
fail:
    batch_blocks_free(b);free(b);return NULL; /* Caller still owns fonts. */
}
SBPrepareState sb_prepare_batch_state(SBPrepareBatch *b,SBStatus *status){
    if(!b){if(status)*status=sb_error(SB_INVALID,"Dokumentvorbereitung fehlt.");return SB_PREPARE_FAILED;}
    if(!SDL_GetAtomicInt(&b->done)){if(status)*status=sb_ok();return SB_PREPARE_PENDING;}
    if(status)*status=b->status;
    if(SDL_GetAtomicInt(&b->cancel))return SB_PREPARE_CANCELLED;
    return b->status.code==SB_OK ? SB_PREPARE_READY : SB_PREPARE_FAILED;
}
void sb_prepare_batch_cancel(SBPrepareBatch *b){if(b)SDL_SetAtomicInt(&b->cancel,1);}
SBStatus sb_prepare_batch_bind(SBPrepareBatch *b,SBUi *ui,uint64_t context){
    if(!b || !ui || sb_prepare_batch_state(b,NULL)!=SB_PREPARE_READY)return sb_error(SB_INVALID,"Dokumentvorbereitung ist nicht bereit.");
    if(b->context!=context || (b->bound && (b->bound!=ui || b->owner!=SDL_GetCurrentThreadID())))return sb_error(SB_CONFLICT,"Dokumentvorbereitung ist nicht mehr aktuell.");
    TTF_Font *fonts[256]={0};
    /* Validate every role, including empty blocks, before changing any glyph. */
    for(size_t i=0;i<b->count;++i){SBPreparedBlock *p=&b->blocks[i];unsigned base=p->request.role*64;
        if(!fonts[base])fonts[base]=sb_text_snapshot_bind(b->fonts,ui,base);
        if(!fonts[base])return sb_error(SB_CONFLICT,"Dokumentschrift ist nicht mehr aktuell.");
        for(size_t l=0;l<p->plan.count;++l)for(size_t g=0;g<p->plan.lines[l].shape.count;++g){unsigned token=p->tokens[l][g];
            if(!fonts[token])fonts[token]=sb_text_snapshot_bind(b->fonts,ui,token);
            if(!fonts[token])return sb_error(SB_CONFLICT,"Dokumentschrift ist nicht mehr aktuell.");}}
    for(size_t i=0;i<b->count;++i){SBPreparedBlock *p=&b->blocks[i];
        for(size_t l=0;l<p->plan.count;++l)for(size_t g=0;g<p->plan.lines[l].shape.count;++g)p->plan.lines[l].shape.glyphs[g].font=fonts[p->tokens[l][g]];}
    b->bound=ui;b->epoch=ui->font_epoch;b->owner=SDL_GetCurrentThreadID();return sb_ok();
}
const SBStyledPlan *sb_prepare_batch_lookup(SBPrepareBatch *b,SBUi *ui,const SBPrepareRequest *r){
    if(!b || b->bound!=ui || b->owner!=SDL_GetCurrentThreadID() || b->epoch!=ui->font_epoch ||
       SDL_GetAtomicInt(&b->cancel) || !batch_request_valid(r))return NULL;
    uint64_t hash=sb_hash(r->text.text,strlen(r->text.text));
    for(size_t at=b->buckets[hash%4096];at;at=b->blocks[at-1].next){size_t i=at-1;if(b->blocks[i].hash==hash && batch_request_equal(r,&b->blocks[i].request))return &b->blocks[i].plan;}
    return NULL;
}
size_t sb_prepare_batch_bytes(const SBPrepareBatch *b){return b && SDL_GetAtomicInt((SDL_AtomicInt *)&b->done) ? b->input_bytes+b->output_bytes : 0;}
bool sb_prepare_batch_try_free(SBPrepareBatch *b){
    if(b && SDL_GetThreadState(b->thread)!=SDL_THREAD_COMPLETE)return false;
    sb_prepare_batch_free(b);return true;
}
void sb_prepare_batch_free(SBPrepareBatch *b){if(!b)return;sb_prepare_batch_cancel(b);SDL_WaitThread(b->thread,NULL);batch_blocks_free(b);sb_text_snapshot_free(b->fonts);free(b);}
