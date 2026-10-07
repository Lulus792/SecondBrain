#include "document.h"
#include "inline.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void emit(const SBDocument *d,size_t index,const SBReferences *references) {
    const SBDocumentNode *n=&d->nodes[index];
    if(n->kind==SB_DOC_PARAGRAPH && n->content==n->view.length)return;
    const char *names[]={"root","quote","list","item","p","heading","code","rule","raw","table"};
    printf("+ %s %u %u %u %u\n",names[n->kind],n->level,n->ordered,n->start,n->tight);
    if(n->view.text){
        char *plain=NULL;const char *text=NULL;size_t length=0;if(sb_document_content(n,&text,&length).code!=SB_OK)exit(5);
        if(n->kind==SB_DOC_PARAGRAPH || n->kind==SB_DOC_HEADING){SBInline reader;SBStatus s=sb_inline_init_references(&reader,text,length,references);if(s.code==SB_OK)s=sb_inline_text(&reader,0,length,&plain);sb_inline_free(&reader);if(s.code!=SB_OK)exit(5);text=plain;length=strlen(plain);while(length && (text[length-1]==' '||text[length-1]=='\t'||text[length-1]=='\n'))--length;}
        printf("T ");for(size_t i=0;i<length;++i)printf("%02x",(unsigned char)text[i]);puts("");free(plain);
    }
    for(size_t child=n->first;child!=SIZE_MAX;child=d->nodes[child].next)emit(d,child,references);
    puts("-");
}
int main(int argc,char **argv){if(argc!=2)return 2;char *source=NULL;size_t length=0;if(sb_fs_read(argv[1],&source,&length).code!=SB_OK)return 3;
 SBDocument d;SBStatus s=sb_document_init(&d,source,length);if(s.code!=SB_OK){fprintf(stderr,"%s\n",s.message);sb_document_free(&d);free(source);return 4;}
 SBReferences references;s=sb_document_references(&d,&references);if(s.code!=SB_OK){fprintf(stderr,"%s\n",s.message);sb_references_free(&references);sb_document_free(&d);free(source);return 4;}
 emit(&d,0,&references);sb_references_free(&references);sb_document_free(&d);free(source);return 0;
}
