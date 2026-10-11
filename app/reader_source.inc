/* One owned foreground document; geometry still follows the current layout. */
#ifndef SB_READER_SOURCE_BYTES
#define SB_READER_SOURCE_BYTES (16u*1024u*1024u)
#endif
struct SBReaderSource {
    char *source;
    size_t length,bytes;
    SBDocument document;
    SBReferences references;
};
static void reader_source_free(SBDesktop *d){
    SBReaderSource *p=d->reader_source;if(!p)return;
    sb_references_free(&p->references);sb_document_free(&p->document);free(p->source);free(p);
    d->reader_source=NULL;d->reader_source_bytes=0;
}
static bool reader_source_size(SBReaderSource *p){
    size_t bytes=sizeof(*p)+p->length+1;
    #define SOURCE_ADD(n) do{size_t add=(n);if(add>SB_READER_SOURCE_BYTES-bytes)return false;bytes+=add;}while(0)
    if(bytes>SB_READER_SOURCE_BYTES)return false;
    SOURCE_ADD(p->document.capacity*sizeof(*p->document.nodes));
    for(size_t i=0;i<p->document.count;++i){const SBProjection *v=&p->document.nodes[i].view;
        SOURCE_ADD(v->capacity);SOURCE_ADD(v->span_capacity*sizeof(*v->spans));}
    SOURCE_ADD(p->references.capacity*sizeof(*p->references.items));SOURCE_ADD(p->references.name_bytes);
    #undef SOURCE_ADD
    p->bytes=bytes;return true;
}
static SBStatus reader_source_get(SBDesktop *d,const char *text,size_t length,bool foreground,SBDocument *parsed,SBReferences *references,bool *borrowed){
    *borrowed=false;SBReaderSource *p=d->reader_source;
    if(foreground && p && p->length==length && !memcmp(p->source,text,length)){
        *parsed=p->document;*references=p->references;*borrowed=true;d->reader_source_used=true;++d->reader_source_hits;return sb_ok();
    }
    if(foreground)reader_source_free(d);
    p=foreground ? calloc(1,sizeof(*p)) : NULL;
    if(p){p->source=malloc(length+1);if(p->source){memcpy(p->source,text,length);p->source[length]=0;p->length=length;}else{free(p);p=NULL;}}
    const char *source=p ? p->source : text;
    SBStatus status=sb_document_init(parsed,source,length);++d->reader_source_builds;
    if(status.code==SB_OK)status=sb_document_references(parsed,references);
    if(p){p->document=*parsed;p->references=*references;
        if(status.code==SB_OK && reader_source_size(p)){d->reader_source=p;d->reader_source_bytes=p->bytes;d->reader_source_used=true;*borrowed=true;}
        else{
            /* Scratch parsing must borrow the caller, not a freed copy. */
            sb_references_free(references);sb_document_free(parsed);free(p->source);free(p);
            if(status.code==SB_OK){status=sb_document_init(parsed,text,length);if(status.code==SB_OK)status=sb_document_references(parsed,references);}
        }
    }
    return status;
}
