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
static bool same_styles(const SBStyledText *a,const SBStyledText *b){
    if(strcmp(a->text,b->text) || a->count!=b->count)return false;
    for(size_t i=0;i<a->count;++i)if(a->spans[i].offset!=b->spans[i].offset || a->spans[i].length!=b->spans[i].length || a->spans[i].style!=b->spans[i].style)return false;
    return true;
}
int main(void){
    SBDesktop d={0};SBDocument doc={0};SBReferences refs={0};bool borrowed=false;
    char source[]="# Title\n\n> [Link][key]\n\n[key]: first.md\n";
    CHECK(reader_source_get(&d,source,strlen(source),true,&doc,&refs,&borrowed).code==SB_OK);
    CHECK(borrowed && d.reader_source && doc.source!=source && refs.count==1);
    CHECK(d.reader_source_bytes>strlen(source) && d.reader_source_bytes<=SB_READER_SOURCE_BYTES);
    const char *owned=doc.source;size_t builds=d.reader_source_builds;
    CHECK(reader_source_get(&d,source,strlen(source),true,&doc,&refs,&borrowed).code==SB_OK);
    CHECK(borrowed && doc.source==owned && d.reader_source_builds==builds && d.reader_source_hits==1);
    size_t paragraph=SIZE_MAX;
    for(size_t i=0;i<doc.count;++i)if(doc.nodes[i].kind==SB_DOC_PARAGRAPH){paragraph=i;break;}
    CHECK(paragraph!=SIZE_MAX);
    const char *inline_text=NULL;size_t inline_length=0;
    CHECK(sb_document_content(&doc.nodes[paragraph],&inline_text,&inline_length).code==SB_OK);
    SBInline in={0};SBStyledText styled={0};bool inline_borrowed=false;
    CHECK(reader_inline_get(&d,paragraph,inline_text,inline_length,&refs,&in,&styled,&inline_borrowed).code==SB_OK);
    CHECK(inline_borrowed && !strcmp(styled.text,"Link") && in.references==&d.reader_source->references);
    CHECK(d.reader_source_bytes==d.reader_source->bytes && d.reader_source_bytes<=SB_READER_SOURCE_BYTES);
    const char *held_styled=styled.text;size_t inline_builds=d.reader_inline_builds;
    SBInlineToken token;CHECK(sb_inline_next(&in,&token) && token.kind==SB_INLINE_LINK);
    char destination[100];CHECK(sb_inline_destination(&in,&token,destination,sizeof(destination)).code==SB_OK && !strcmp(destination,"first.md"));
    CHECK(reader_inline_get(&d,paragraph,inline_text,inline_length,&refs,&in,&styled,&inline_borrowed).code==SB_OK);
    CHECK(inline_borrowed && styled.text==held_styled && in.cursor==0 && d.reader_inline_builds==inline_builds && d.reader_inline_hits==1);
    CHECK(sb_inline_next(&in,&token) && token.kind==SB_INLINE_LINK);
    SBInline reference={0};SBStyledText fresh={0};
    CHECK(sb_inline_init_references(&reference,inline_text,inline_length,&refs).code==SB_OK && sb_inline_styled(&reference,0,inline_length,&fresh).code==SB_OK);
    CHECK(same_styles(&styled,&fresh));
    CHECK(sb_inline_storage_bytes(&reference)>0);sb_inline_free(&reference);sb_styled_free(&fresh);
    CHECK(!sb_inline_storage_bytes(&reference));reference.run_capacity=SIZE_MAX;CHECK(sb_inline_storage_bytes(&reference)==SIZE_MAX);
    memcpy(strstr(source,"first.md"),"other.md",8);
    CHECK(!strstr(owned,"other.md"));
    CHECK(reader_source_get(&d,source,strlen(source),true,&doc,&refs,&borrowed).code==SB_OK);
    CHECK(borrowed && d.reader_source_builds==builds+1 && strstr(doc.source,"other.md"));
    CHECK(refs.count==1 && refs.items[0].source.destination_length==8);
    CHECK(!memcmp(refs.items[0].text+refs.items[0].source.destination,"other.md",8));
    CHECK(sb_document_content(&doc.nodes[paragraph],&inline_text,&inline_length).code==SB_OK);
    CHECK(reader_inline_get(&d,paragraph,inline_text,inline_length,&refs,&in,&styled,&inline_borrowed).code==SB_OK && inline_borrowed);
    CHECK(sb_inline_next(&in,&token) && sb_inline_destination(&in,&token,destination,sizeof(destination)).code==SB_OK && !strcmp(destination,"other.md"));
    /* A hidden reader must not replace its foreground owner's references. */
    owned=doc.source;
    const char *background="[key]: behind.md\n\n[key]\n";
    CHECK(reader_source_get(&d,background,strlen(background),false,&doc,&refs,&borrowed).code==SB_OK);
    CHECK(!borrowed && doc.source==background && d.reader_source->source==owned);
    const char *background_text=NULL;size_t background_length=0;
    for(size_t i=0;i<doc.count;++i)if(doc.nodes[i].kind==SB_DOC_PARAGRAPH && sb_document_content(&doc.nodes[i],&background_text,&background_length).code==SB_OK && background_length){paragraph=i;break;}
    CHECK(sb_document_content(&doc.nodes[paragraph],&background_text,&background_length).code==SB_OK);
    CHECK(reader_inline_get(&d,paragraph,background_text,background_length,&refs,&in,&styled,&inline_borrowed).code==SB_OK && !inline_borrowed);
    CHECK(in.references==&refs && sb_inline_next(&in,&token));
    CHECK(sb_inline_destination(&in,&token,destination,sizeof(destination)).code==SB_OK && !strcmp(destination,"behind.md"));sb_inline_free(&in);sb_styled_free(&styled);
    release(&doc,&refs,borrowed);
    char *large=calloc(1,20000);CHECK(large);
    for(unsigned i=0;i<700;++i)strcat(large,"paragraph\n\n");
    CHECK(reader_source_get(&d,large,strlen(large),true,&doc,&refs,&borrowed).code==SB_OK);
    CHECK(!borrowed && !d.reader_source && d.reader_source_bytes==0 && doc.count==701 && doc.source==large);
    const char *content=NULL;size_t length=0;
    CHECK(sb_document_content(&doc.nodes[700],&content,&length).code==SB_OK);
    CHECK(length==9 && !memcmp(content,"paragraph",9));release(&doc,&refs,borrowed);free(large);
    /* Keep the document, but force an inline allocation beyond its remaining
       budget. Rendering still gets complete caller-owned styles and links. */
    char *rich=malloc(30001);CHECK(rich);strcpy(rich,"# Title\n\n");
    for(unsigned i=0;i<1000;++i)strcat(rich,"*a* [b](target.md) ");
    CHECK(reader_source_get(&d,rich,strlen(rich),true,&doc,&refs,&borrowed).code==SB_OK && borrowed);
    paragraph=SIZE_MAX;for(size_t i=0;i<doc.count;++i)if(doc.nodes[i].kind==SB_DOC_PARAGRAPH){paragraph=i;break;}
    CHECK(paragraph!=SIZE_MAX && sb_document_content(&doc.nodes[paragraph],&inline_text,&inline_length).code==SB_OK);
    CHECK(reader_inline_get(&d,paragraph,inline_text,inline_length,&refs,&in,&styled,&inline_borrowed).code==SB_OK && !inline_borrowed);
    CHECK(d.reader_source && d.reader_source_bytes<=SB_READER_SOURCE_BYTES && strlen(styled.text)>3000);
    unsigned links=0;while(sb_inline_next(&in,&token))if(token.kind==SB_INLINE_LINK)++links;CHECK(links==1000);
    sb_inline_free(&in);sb_styled_free(&styled);free(rich);
    const char *decorated="# Styles\n\n> **bold** *italic* `a<b` &amp; [label][r] אבג\n\n[r]: target.md\n";
    CHECK(reader_source_get(&d,decorated,strlen(decorated),true,&doc,&refs,&borrowed).code==SB_OK && borrowed);
    paragraph=SIZE_MAX;for(size_t i=0;i<doc.count;++i)if(doc.nodes[i].kind==SB_DOC_PARAGRAPH && sb_document_content(&doc.nodes[i],&inline_text,&inline_length).code==SB_OK && inline_length){paragraph=i;break;}
    CHECK(paragraph!=SIZE_MAX);
    CHECK(reader_inline_get(&d,paragraph,inline_text,inline_length,&refs,&in,&styled,&inline_borrowed).code==SB_OK && inline_borrowed);
    CHECK(sb_inline_init_references(&reference,inline_text,inline_length,&refs).code==SB_OK && sb_inline_styled(&reference,0,inline_length,&fresh).code==SB_OK);
    CHECK(same_styles(&styled,&fresh));
    bool bold=false,italic=false,code=false;for(size_t i=0;i<styled.count;++i){bold|=(styled.spans[i].style&SB_TEXT_BOLD)!=0;italic|=(styled.spans[i].style&SB_TEXT_ITALIC)!=0;code|=(styled.spans[i].style&SB_TEXT_CODE)!=0;}
    CHECK(bold && italic && code && strstr(styled.text,"& label אבג"));
    sb_inline_free(&reference);sb_styled_free(&fresh);
    const char invalid[]={ (char)0xff,0 };
    CHECK(reader_source_get(&d,invalid,1,true,&doc,&refs,&borrowed).code!=SB_OK);
    CHECK(!borrowed && !d.reader_source && !refs.items);release(&doc,&refs,borrowed);
    CHECK(reader_source_get(&d,"",0,true,&doc,&refs,&borrowed).code==SB_OK);
    CHECK(borrowed && doc.count==1);reader_source_free(&d);CHECK(!d.reader_source && !d.reader_source_bytes);
    printf("%u source ownership/reuse/replacement/reference/budget/fallback assertions passed.\n",checks);return 0;
}
