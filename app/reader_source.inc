/* One owned foreground document; geometry still follows the current layout. */
#ifndef SB_READER_SOURCE_BYTES
#define SB_READER_SOURCE_BYTES (16u*1024u*1024u)
#endif
typedef struct { SBInline reader; SBStyledText styled; } SBReaderInline;
struct SBReaderSource {
    char *source;
    size_t length,bytes;
    SBDocument document;
    SBReferences references;
    SBReaderInline **inline_nodes;
};
static void reader_source_free(SBDesktop *d){
    SBReaderSource *p=d->reader_source;if(!p)return;
    if(p->inline_nodes){for(size_t i=0;i<p->document.count;++i){SBReaderInline *entry=p->inline_nodes[i];
        if(entry){sb_inline_free(&entry->reader);sb_styled_free(&entry->styled);free(entry);}}
        free(p->inline_nodes);}
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
    /* Outputs may still describe the preceding borrowed plan. Do not carry
       those aliases into cleanup if the replacement fails before references. */
    *borrowed=false;*parsed=(SBDocument){0};*references=(SBReferences){0};SBReaderSource *p=d->reader_source;
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
static SBStatus reader_inline_get(SBDesktop *d,size_t node,const char *text,size_t length,const SBReferences *references,SBInline *reader,SBStyledText *styled,bool *borrowed){
    *borrowed=false;SBReaderSource *p=d->reader_source;
    const char *content=NULL;size_t content_length=0;
    bool owned=p && d->reader_source_used && node<p->document.count &&
        references->items==p->references.items && references->count==p->references.count &&
        sb_document_content(&p->document.nodes[node],&content,&content_length).code==SB_OK &&
        content==text && content_length==length;
    SBReaderInline *entry=owned && p->inline_nodes ? p->inline_nodes[node] : NULL;
    if(entry){*reader=entry->reader;reader->cursor=0;*styled=entry->styled;*borrowed=true;++d->reader_inline_hits;return sb_ok();}
    SBStatus status=sb_inline_init_references(reader,text,length,owned ? &p->references : references);++d->reader_inline_builds;
    if(status.code==SB_OK)status=sb_inline_styled(reader,0,length,styled);
    if(status.code!=SB_OK || !owned)return status;
    size_t raw=sb_inline_storage_bytes(reader),chars=strlen(styled->text)+1,bytes=sizeof(SBReaderInline);
    if(raw>SB_READER_SOURCE_BYTES-bytes)return status;bytes+=raw;
    if(chars>SB_READER_SOURCE_BYTES-bytes)return status;bytes+=chars;
    if(styled->capacity>(SB_READER_SOURCE_BYTES-bytes)/sizeof(*styled->spans))return status;
    bytes+=styled->capacity*sizeof(*styled->spans);
    size_t slots=p->inline_nodes ? 0 : p->document.count*sizeof(*p->inline_nodes);
    if(slots>SB_READER_SOURCE_BYTES-p->bytes || bytes>SB_READER_SOURCE_BYTES-p->bytes-slots)return status;
    if(!p->inline_nodes){p->inline_nodes=calloc(p->document.count,sizeof(*p->inline_nodes));if(!p->inline_nodes)return status;p->bytes+=slots;}
    entry=malloc(sizeof(*entry));char *compact=malloc(chars);
    if(!entry || !compact){free(entry);free(compact);d->reader_source_bytes=p->bytes;return status;}
    /* The styled writer may reserve more characters than it emits. Retain an
       exact copy so the shared budget counts the actual held allocation. */
    memcpy(compact,styled->text,chars);free(styled->text);styled->text=compact;
    *entry=(SBReaderInline){.reader=*reader,.styled=*styled};p->inline_nodes[node]=entry;
    p->bytes+=bytes;d->reader_source_bytes=p->bytes;*borrowed=true;
    return status;
}
