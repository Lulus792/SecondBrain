#include "desktop.h"
#include "document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Exercise the identical budget path with a small, practical fixture. */
#define SB_READER_SOURCE_BYTES (64u*1024u)
#include "../app/reader_source.inc"
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"READER SOURCE %d: %s\n",__LINE__,#x);return 1;}}while(0)
static void release(SBDocument *doc,SBReferences *refs,bool borrowed){if(!borrowed){sb_references_free(refs);sb_document_free(doc);}}
int main(void){
    SBDesktop d={0};SBDocument doc={0};SBReferences refs={0};bool borrowed=false;
    char source[]="# Title\n\n> [Link][key]\n\n[key]: first.md\n";
    CHECK(reader_source_get(&d,source,strlen(source),true,&doc,&refs,&borrowed).code==SB_OK);
    CHECK(borrowed && d.reader_source && doc.source!=source && refs.count==1);
    CHECK(d.reader_source_bytes>strlen(source) && d.reader_source_bytes<=SB_READER_SOURCE_BYTES);
    const char *owned=doc.source;size_t builds=d.reader_source_builds;
    CHECK(reader_source_get(&d,source,strlen(source),true,&doc,&refs,&borrowed).code==SB_OK);
    CHECK(borrowed && doc.source==owned && d.reader_source_builds==builds && d.reader_source_hits==1);
    memcpy(strstr(source,"first.md"),"other.md",8);
    CHECK(!strstr(owned,"other.md"));
    CHECK(reader_source_get(&d,source,strlen(source),true,&doc,&refs,&borrowed).code==SB_OK);
    CHECK(borrowed && d.reader_source_builds==builds+1 && strstr(doc.source,"other.md"));
    CHECK(refs.count==1 && refs.items[0].source.destination_length==8);
    CHECK(!memcmp(refs.items[0].text+refs.items[0].source.destination,"other.md",8));
    /* A hidden reader must not replace its foreground owner's references. */
    owned=doc.source;
    const char *background="[key]: behind.md\n\n[key]\n";
    CHECK(reader_source_get(&d,background,strlen(background),false,&doc,&refs,&borrowed).code==SB_OK);
    CHECK(!borrowed && doc.source==background && d.reader_source->source==owned);
    release(&doc,&refs,borrowed);
    char *large=calloc(1,20000);CHECK(large);
    for(unsigned i=0;i<700;++i)strcat(large,"paragraph\n\n");
    CHECK(reader_source_get(&d,large,strlen(large),true,&doc,&refs,&borrowed).code==SB_OK);
    CHECK(!borrowed && !d.reader_source && d.reader_source_bytes==0 && doc.count==701 && doc.source==large);
    const char *content=NULL;size_t length=0;
    CHECK(sb_document_content(&doc.nodes[700],&content,&length).code==SB_OK);
    CHECK(length==9 && !memcmp(content,"paragraph",9));release(&doc,&refs,borrowed);free(large);
    const char invalid[]={ (char)0xff,0 };
    CHECK(reader_source_get(&d,invalid,1,true,&doc,&refs,&borrowed).code!=SB_OK);
    CHECK(!borrowed && !d.reader_source);release(&doc,&refs,borrowed);
    CHECK(reader_source_get(&d,"",0,true,&doc,&refs,&borrowed).code==SB_OK);
    CHECK(borrowed && doc.count==1);reader_source_free(&d);CHECK(!d.reader_source && !d.reader_source_bytes);
    printf("%u source ownership/reuse/replacement/reference/budget/fallback assertions passed.\n",checks);return 0;
}
