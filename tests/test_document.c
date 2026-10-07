#include "document.h"
#include "inline.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"DOCUMENT FAIL %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static void validate(const SBDocument *d) {
    CHECK(d->ready && d->count && d->nodes[0].kind==SB_DOC_ROOT && d->nodes[0].parent==SIZE_MAX);
    for(size_t i=0;i<d->count;++i){const SBDocumentNode *n=&d->nodes[i];CHECK(n->offset<=n->end && n->end<=d->length);
        if(i){CHECK(n->parent<i);const SBDocumentNode *parent=&d->nodes[n->parent];CHECK(n->offset>=parent->offset && n->end<=parent->end);if(parent->kind==SB_DOC_LIST)CHECK(n->kind==SB_DOC_ITEM);if(n->kind==SB_DOC_ITEM)CHECK(parent->kind==SB_DOC_LIST);}
        size_t previous=SIZE_MAX,children=0;
        for(size_t child=n->first;child!=SIZE_MAX;child=d->nodes[child].next){CHECK(child>i && child<d->count && ++children<=d->count && d->nodes[child].parent==i);previous=child;}
        CHECK(previous==n->last);
        if(n->view.text){CHECK(n->content<=n->view.length && strlen(n->view.text)==n->view.length);size_t covered=0;
            for(size_t s=0;s<n->view.count;++s){const SBProjectionSpan *span=&n->view.spans[s];CHECK(span->view==covered && span->length && span->length<=n->view.length-covered && span->source<=d->length && span->source_length<=d->length-span->source);
                if(span->copy)CHECK(span->length==span->source_length && !memcmp(n->view.text+span->view,d->source+span->source,span->length));covered+=span->length;
            }CHECK(covered==n->view.length);
            for(size_t at=0;at<=n->view.length;++at){size_t original;CHECK(sb_projection_source(&n->view,at,&original).code==SB_OK && original<=d->length);}
        }
    }
}
int main(void){
    const char *source="> - [Straße]: <f&ouml;&ouml;.md>\r\n>\r\n>   # [Titel][STRASSE]\r\n>\r\n>   [Ziel][Straße]\r\n>\r\n>   ```\r\n>   [fake]: wrong.md\r\n>   ```\r\n\r\n[STRASSE]: second.md\r\n";
    char *copy=malloc(strlen(source)+1);CHECK(copy);strcpy(copy,source);SBDocument d;CHECK(sb_document_init(&d,source,strlen(source)).code==SB_OK);validate(&d);
    SBReferences refs;CHECK(sb_document_references(&d,&refs).code==SB_OK && refs.count==1 && sb_reference_find(&refs,"fake",4)==SIZE_MAX);
    unsigned headings=0,links=0;
    for(size_t i=0;i<d.count;++i){SBDocumentNode *n=&d.nodes[i];if(n->kind!=SB_DOC_PARAGRAPH && n->kind!=SB_DOC_HEADING)continue;const char *text;size_t length;CHECK(sb_document_content(n,&text,&length).code==SB_OK);
        SBInline reader;CHECK(sb_inline_init_references(&reader,text,length,&refs).code==SB_OK);SBInlineToken token;
        while(sb_inline_next(&reader,&token))if(token.kind==SB_INLINE_LINK){char target[80];CHECK(sb_inline_destination(&reader,&token,target,sizeof(target)).code==SB_OK && !strcmp(target,"föö.md"));++links;}
        if(n->kind==SB_DOC_HEADING){++headings;CHECK(n->level==1 && n->offset>0 && strstr(source+n->offset,"# [Titel]")==source+n->offset);}
        sb_inline_free(&reader);
    }CHECK(headings==1 && links==2 && !strcmp(source,copy));free(copy);sb_references_free(&refs);sb_document_free(&d);
    CHECK(sb_document_init(&d,"- foo\n> bar\n",12).code==SB_OK);validate(&d);CHECK(d.nodes[d.nodes[d.nodes[0].first].next].kind==SB_DOC_QUOTE);sb_document_free(&d);
    const char *atx="# [foo]: /wrong\n\n[foo]\n";
    CHECK(sb_document_init(&d,atx,strlen(atx)).code==SB_OK);validate(&d);CHECK(sb_document_references(&d,&refs).code==SB_OK && !refs.count);
    const char *caption;size_t caption_length;CHECK(sb_document_content(&d.nodes[d.nodes[0].first],&caption,&caption_length).code==SB_OK && caption_length==strlen("[foo]: /wrong") && !strncmp(caption,"[foo]: /wrong",caption_length));
    sb_references_free(&refs);sb_document_free(&d);
    CHECK(sb_document_init(&d,NULL,0).code==SB_OK);validate(&d);sb_document_free(&d);
    CHECK(sb_document_init(&d,"a\0b",3).code==SB_INVALID && !d.ready);CHECK(sb_document_references(&d,&refs).code==SB_INVALID);sb_references_free(&refs);sb_document_free(&d);
    CHECK(sb_document_init(&d,"\xff",1).code==SB_INVALID);sb_document_free(&d);
    char nested[SB_DOCUMENT_DEPTH+3];memset(nested,'>',SB_DOCUMENT_DEPTH+1);nested[SB_DOCUMENT_DEPTH+1]='x';nested[SB_DOCUMENT_DEPTH+2]=0;
    CHECK(sb_document_init(&d,nested,strlen(nested)).code==SB_LIMIT && !d.ready);CHECK(sb_document_references(&d,&refs).code==SB_INVALID);sb_references_free(&refs);sb_document_free(&d);
    size_t dense_length=SB_DOCUMENT_NODES*4;char *dense=malloc(dense_length);CHECK(dense);
    for(size_t i=0;i<SB_DOCUMENT_NODES;++i)memcpy(dense+i*4,"# x\n",4);
    CHECK(sb_document_init(&d,dense,dense_length).code==SB_LIMIT && !d.ready && d.count==SB_DOCUMENT_NODES);
    CHECK(sb_document_references(&d,&refs).code==SB_INVALID);sb_references_free(&refs);sb_document_free(&d);
    CHECK(dense[0]=='#' && dense[dense_length-1]=='\n');free(dense);
    char *maximum=malloc(SB_TEXT_LIMIT);CHECK(maximum);memset(maximum,'x',SB_TEXT_LIMIT);
    CHECK(sb_document_init(&d,maximum,SB_TEXT_LIMIT).code==SB_OK && d.ready && d.count==2);
    const char *maximum_text;size_t maximum_length;CHECK(sb_document_content(&d.nodes[1],&maximum_text,&maximum_length).code==SB_OK && maximum_length==SB_TEXT_LIMIT && !memcmp(maximum_text,maximum,maximum_length));
    CHECK(sb_document_references(&d,&refs).code==SB_OK && !refs.count);sb_references_free(&refs);sb_document_free(&d);free(maximum);
    uint32_t seed=0x326;
    for(unsigned run=0;run<4000;++run){char input[260],before[260];size_t length=run%259;
        for(size_t i=0;i<length;++i){seed=seed*1664525u+1013904223u;static const char alphabet[]=" >-+*012.)#`~_[]():abc \t\r\n";input[i]=alphabet[(seed>>16)%(sizeof(alphabet)-1)];}memcpy(before,input,length);
        SBStatus status=sb_document_init(&d,input,length);CHECK(status.code==SB_OK || status.code==SB_LIMIT);if(status.code==SB_OK){validate(&d);CHECK(sb_document_references(&d,&refs).code==SB_OK);sb_references_free(&refs);}CHECK(!memcmp(input,before,length));sb_document_free(&d);
    }
    printf("%u document assertions passed: tree/source-map invariants, scoped definitions, Unicode targets, limits and 4000 bounded inputs.\n",checks);return 0;
}
