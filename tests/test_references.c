#include "references.h"
#include "inline.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"REFERENCE FAIL %d: %s\n",__LINE__,#x);exit(1);}}while(0)
int main(void) {
    char name[SB_REFERENCE_LABEL_CAP];
    CHECK(sb_reference_label("  STRAẞE\r\n\t IİΣςKﬃ  ",strlen("  STRAẞE\r\n\t IİΣςKﬃ  "),name,sizeof(name)).code==SB_OK);
    CHECK(!strcmp(name,"strasse ii̇σσkffi"));
    CHECK(sb_reference_label("A\\[B",4,name,sizeof(name)).code==SB_OK && !strcmp(name,"a\\[b"));
    CHECK(sb_reference_label("[B",2,name,sizeof(name)).code==SB_INVALID);
    CHECK(sb_reference_label(" \r\n\t",4,name,sizeof(name)).code==SB_INVALID);
    CHECK(sb_reference_label("\xff",1,name,sizeof(name)).code==SB_INVALID);
    char exact[3],short_name[2];
    CHECK(sb_reference_label("ß",2,exact,sizeof(exact)).code==SB_OK && !strcmp(exact,"ss"));
    CHECK(sb_reference_label("ß",2,short_name,sizeof(short_name)).code==SB_LIMIT && !*short_name);
    char *long_name=malloc(1000*2);CHECK(long_name);
    for(size_t i=0;i<1000;++i)memcpy(long_name+i*2,"İ",2);
    CHECK(sb_reference_label(long_name,999*2,name,sizeof(name)).code==SB_OK && strlen(name)==999*3);
    CHECK(sb_reference_label(long_name,1000*2,name,sizeof(name)).code==SB_INVALID);free(long_name);
    const char *source="[Straße]: <f&ouml;&ouml;.md> 'Titel'\n[STRASSE]: wrong.md\n\n# [Titel][strasse]\n\n[Ziel][Straße] [Straße][] [Straße] ![Bild][Straße] [Unbekannt][x]\n\n```\n[x]: hidden.md\n```\n\n    [y]: hidden.md\n";
    char *original=malloc(strlen(source)+1);CHECK(original);strcpy(original,source);
    SBReferences refs;CHECK(sb_references_init(&refs,source,strlen(source)).code==SB_OK && refs.count==1);
    size_t index=sb_reference_find(&refs,"  STRASSE  ",11);CHECK(index==0);
    CHECK(sb_reference_find(&refs,"x",1)==SIZE_MAX && sb_reference_find(&refs,"y",1)==SIZE_MAX);
    const char *paragraph=strstr(source,"[Ziel]");size_t length=(size_t)(strstr(paragraph,"\n\n")-paragraph);
    SBInline reader;CHECK(sb_inline_init_references(&reader,paragraph,length,&refs).code==SB_OK);
    char *plain=NULL;CHECK(sb_inline_text(&reader,0,length,&plain).code==SB_OK && !strcmp(plain,"Ziel Straße Straße Bild [Unbekannt][x]"));free(plain);
    unsigned links=0,images=0;SBInlineToken token;
    while(sb_inline_next(&reader,&token))if(token.kind==SB_INLINE_LINK || token.kind==SB_INLINE_IMAGE){
        char destination[80];CHECK(token.has_reference && sb_inline_destination(&reader,&token,destination,sizeof(destination)).code==SB_OK && !strcmp(destination,"föö.md"));
        if(token.kind==SB_INLINE_LINK)++links;else ++images;
    }
    CHECK(links==3 && images==1);sb_inline_free(&reader);
    CHECK(!strcmp(source,original));free(original);sb_references_free(&refs);
    const char *not_definition="Text\n[x]: /url\n\n[x]\n";
    CHECK(sb_references_init(&refs,not_definition,strlen(not_definition)).code==SB_OK && refs.count==0);sb_references_free(&refs);
    size_t budget=1;SBReferenceDefinition definition;
    CHECK(!sb_reference_parse("[x]: /url\n",10,0,&definition,&budget) && budget==0);
    CHECK(sb_references_init(&refs,NULL,1).code==SB_INVALID);sb_references_free(&refs);
    size_t lines=SB_REFERENCE_LIMIT+1,bytes=lines*10;char *dense=malloc(bytes);CHECK(dense);
    for(size_t i=0;i<lines;++i)memcpy(dense+i*10,"[x]: /url\n",10);
    CHECK(sb_references_init(&refs,dense,bytes).code==SB_LIMIT && refs.count==SB_REFERENCE_LIMIT);sb_references_free(&refs);free(dense);
    uint32_t seed=0x3102;
    for(unsigned run=0;run<3000;++run){
        char input[258],copy[258];size_t n=run%257;
        for(size_t i=0;i<n;++i){seed=seed*1664525u+1013904223u;const char *alphabet="[]()<>`\\#*_ \t\r\n:ab012";input[i]=alphabet[(seed>>16)%strlen(alphabet)];}
        memcpy(copy,input,n);CHECK(sb_references_init(&refs,input,n).code==SB_OK);
        for(size_t i=0;i<refs.count;++i){SBReferenceDefinition *s=&refs.items[i].source;CHECK(s->offset<s->end && s->end<=n && s->label<=n && s->label_length<=n-s->label && s->destination<=n && s->destination_length<=n-s->destination);}
        CHECK(!memcmp(input,copy,n));sb_references_free(&refs);
    }
    printf("%u reference assertions passed, including Unicode expansion, source preservation, limits and 3000 bounded cases.\n",checks);return 0;
}
