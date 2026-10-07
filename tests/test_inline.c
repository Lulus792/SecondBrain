#include "inline.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"INLINE FAIL %d: %s\n",__LINE__,#x); exit(1); } } while (0)
static void example(const char *source,const char *plain,unsigned links,const char *destinations[]) {
    SBInline r; CHECK(sb_inline_init(&r,source,strlen(source)).code==SB_OK);
    char *text=NULL; CHECK(sb_inline_text(&r,0,strlen(source),&text).code==SB_OK); if (strcmp(text,plain)) fprintf(stderr,"Source: %s\nActual: %s\nExpected: %s\n",source,text,plain); CHECK(!strcmp(text,plain)); free(text);
    unsigned found=0; SBInlineToken t; size_t cursor=0;
    while (sb_inline_next(&r,&t)) {
        CHECK(r.cursor>cursor && t.offset==cursor && t.length<=r.length-cursor); cursor=r.cursor;
        if (t.kind==SB_INLINE_LINK || t.kind==SB_INLINE_AUTOLINK) {
            CHECK(found<links); char path[SB_PATH_CAP];
            CHECK(sb_inline_destination(&r,&t,path,sizeof(path)).code==SB_OK);
            CHECK(!strcmp(path,destinations[found++]));
        }
    }
    CHECK(cursor==strlen(source) && found==links); sb_inline_free(&r);
}
static void styled(const char *source,const char *plain,const unsigned char *styles) {
    SBInline reader; CHECK(sb_inline_init(&reader,source,strlen(source)).code==SB_OK);
    SBStyledText out; CHECK(sb_inline_styled(&reader,0,strlen(source),&out).code==SB_OK);
    CHECK(!strcmp(out.text,plain)); size_t end=0;
    for (size_t i=0;i<out.count;++i) {
        SBTextSpan span=out.spans[i]; CHECK(span.offset==end && span.length && span.length<=strlen(plain)-end);
        for (size_t p=span.offset;p<span.offset+span.length;++p) CHECK(span.style==styles[p]);
        end+=span.length;
    }
    CHECK(end==strlen(plain)); sb_styled_free(&out); sb_inline_free(&reader);
}
int main(void) {
    const char *one[]={"../STATE.md"},*two[]={"../STATE.md","../DECISIONS.md"};
    example("`[Nur Code](../PROJECT.md)` [Stand](../STATE.md)","[Nur Code](../PROJECT.md) Stand",1,one);
    example("\\[Maskiert](../PROJECT.md) [Stand](../STATE.md)","[Maskiert](../PROJECT.md) Stand",1,one);
    example("![Bild](../PROJECT.md) [Stand](../STATE.md)","Bild Stand",1,one);
    example("`` [Code ` und Link](../PROJECT.md) `` [Stand](../STATE.md)","[Code ` und Link](../PROJECT.md) Stand",1,one);
    example("\\``[Nur Code](../PROJECT.md)` [Stand](../STATE.md)","`[Nur Code](../PROJECT.md) Stand",1,one);
    example("`unbeendet [Stand](../STATE.md)","`unbeendet Stand",1,one);
    example("[Titel [mit Klammern]](../STATE.md) [Entscheidung](../DECISIONS.md \"Titel\")","Titel [mit Klammern] Entscheidung",2,two);
    example("[Außen [Stand](../STATE.md)](../PROJECT.md)","[Außen Stand](../PROJECT.md)",1,one);
    example("[![Bild](bild.png)](../STATE.md)","Bild",1,one);
    example("![Stand [innen](../PROJECT.md)](bild.png) [Stand](../STATE.md)","Stand innen Stand",1,one);
    example("[Code `]` im Label](../STATE.md)","Code ] im Label",1,one);
    const char *punct[]={"../name(teil).md","../mit Leerzeichen.md","../a)b.md"};
    example("[Eins](../name(teil).md 'Text') [Zwei](<../mit Leerzeichen.md>) [Drei](../a\\)b.md)","Eins Zwei Drei",3,punct);
    example("**betont** und **unfertig", "betont und **unfertig",0,NULL);
    styled("*ab* __cd__", "ab cd",(const unsigned char[]){1,1,0,2,2});
    styled("***ab***", "ab",(const unsigned char[]){3,3});
    styled("**a *b* c**", "a b c",(const unsigned char[]){2,2,3,2,2});
    styled("*a **b** c*", "a b c",(const unsigned char[]){1,1,3,1,1});
    styled("a_b_c", "a_b_c",(const unsigned char[]){0,0,0,0,0});
    styled("a*b*c", "abc",(const unsigned char[]){0,1,0});
    styled("**`ab`**", "ab",(const unsigned char[]){6,6});
    styled("`*ab*`", "*ab*",(const unsigned char[]){4,4,4,4});
    styled("\\*ab*", "*ab*",(const unsigned char[]){0,0,0,0});
    styled("*[a*](x)", "*a*",(const unsigned char[]){0,0,0});
    styled("*[ab](x)*", "ab",(const unsigned char[]){1,1});
    styled("[**ab**](x)", "ab",(const unsigned char[]){2,2});
    example("*unfertig und __auch", "*unfertig und __auch",0,NULL);
    example("ä_wort_ü", "ä_wort_ü",0,NULL);
    example("«_Wort_»", "«Wort»",0,NULL);
    example("*\u00a0Wort*", "*\u00a0Wort*",0,NULL);
    styled("*<img title=\"*\"/>", "*<img title=\"*\"/>",(const unsigned char[17]){0});
    const char *url[]={"https://foo.bar/?q=**"};
    example("**a<https://foo.bar/?q=**>","**ahttps://foo.bar/?q=**",1,url);
    example("<a title=\"[Kein](../STATE.md)\">", "<a title=\"[Kein](../STATE.md)\">",0,NULL);
    example("``\r\n code \\* **literal** \r\n``"," code \\* **literal** ",0,NULL);
    example("`   `", "   ",0,NULL);
    example("eins\r\nzwei\rdrei\nvier", "eins zwei drei vier",0,NULL);
    example("[ungültig](ziel Leerraum) [Stand](../STATE.md)","[ungültig](ziel Leerraum) Stand",1,one);
    example("&amp; &lt; &nGt; &nLt; &#x1f600; &#128;", "& < ≫⃒ ≪⃒ 😀 \xc2\x80",0,NULL);
    example("&unknown; &amp &#; &#x; &#12345678; &#x1234567;", "&unknown; &amp &#; &#x; &#12345678; &#x1234567;",0,NULL);
    example("&#0; &#xD800; &#1114112; &#x10FFFF;", "� � � \xf4\x8f\xbf\xbf",0,NULL);
    example("\\&amp; `&amp;` &#42;ab&#42;", "&amp; &amp; *ab*",0,NULL);
    styled("**&nGt;**", "≫⃒",(const unsigned char[]){2,2,2,2,2,2});
    const char *entity_paths[]={"../föö.md","../a&amp;.md"};
    example("[Titel &amp; Text](../f&ouml;&ouml;.md) [Maskiert](../a\\&amp;.md)","Titel & Text Maskiert",2,entity_paths);
    const char *literal_url[]={"https://example.org/?q=&amp;"};
    example("<https://example.org/?q=&amp;>","https://example.org/?q=&amp;",1,literal_url);
    example("<a title=\"&amp;\">", "<a title=\"&amp;\">",0,NULL);
    /* The shortest expanding named references must not overrun source-sized storage. */
    example("&nGt;&nGt;&nGt;", "≫⃒≫⃒≫⃒",0,NULL);
    SBInline r; CHECK(sb_inline_init(&r,NULL,1).code==SB_INVALID); sb_inline_free(&r);
    CHECK(sb_inline_init(&r,"",0).code==SB_OK); char *text=NULL;
    CHECK(sb_inline_text(&r,1,0,&text).code==SB_INVALID && !text); sb_inline_free(&r);
    char *dense=malloc(2*(SB_INLINE_LIMIT+1)+1); CHECK(dense);
    for (size_t i=0;i<SB_INLINE_LIMIT+1;++i) { dense[2*i]='`'; dense[2*i+1]='a'; }
    CHECK(sb_inline_init(&r,dense,2*(SB_INLINE_LIMIT+1)).code==SB_LIMIT); sb_inline_free(&r); free(dense);
    const char *short_source="[a](&nGt;)"; SBInlineToken short_link;
    CHECK(sb_inline_init(&r,short_source,strlen(short_source)).code==SB_OK);
    CHECK(sb_inline_next(&r,&short_link) && short_link.kind==SB_INLINE_LINK);
    char exact[7],small[6];
    CHECK(sb_inline_destination(&r,&short_link,exact,sizeof(exact)).code==SB_OK && !strcmp(exact,"≫⃒"));
    CHECK(sb_inline_destination(&r,&short_link,small,sizeof(small)).code==SB_LIMIT && !*small);
    sb_inline_free(&r);
    size_t repeats=SB_TEXT_LIMIT/6+1,expanded_source_length=repeats*5;
    char *expanded=malloc(expanded_source_length); CHECK(expanded);
    for (size_t i=0;i<repeats;++i) memcpy(expanded+i*5,"&nGt;",5);
    CHECK(sb_inline_init(&r,expanded,expanded_source_length).code==SB_OK);
    text=NULL; CHECK(sb_inline_text(&r,0,expanded_source_length,&text).code==SB_LIMIT && !text);
    CHECK(!memcmp(expanded,"&nGt;",5) && !memcmp(expanded+expanded_source_length-5,"&nGt;",5));
    sb_inline_free(&r); free(expanded);
    /* Capacity follows displayed bytes, not longer CRLF source bytes. */
    size_t prefix_refs=(SB_TEXT_LIMIT/2)/5,prefix_length=prefix_refs*5;
    size_t crlf_count=(SB_TEXT_LIMIT-prefix_length-2)/2;
    size_t mixed_length=prefix_length+2+crlf_count*2;
    char *mixed=malloc(mixed_length); CHECK(mixed);
    for (size_t i=0;i<prefix_refs;++i) memcpy(mixed+i*5,"&nGt;",5);
    mixed[prefix_length]='`';
    for (size_t i=0;i<crlf_count;++i) memcpy(mixed+prefix_length+1+i*2,"\r\n",2);
    mixed[mixed_length-1]='`';CHECK(sb_inline_init(&r,mixed,mixed_length).code==SB_OK);
    text=NULL;CHECK(sb_inline_text(&r,0,mixed_length,&text).code==SB_OK);
    CHECK(strlen(text)==prefix_refs*6+crlf_count && strlen(text)<SB_TEXT_LIMIT);
    free(text);sb_inline_free(&r);free(mixed);
    uint32_t seed=0x3192;
    for (unsigned run=0;run<5000;++run) {
        char bytes[258],original[258]; size_t length=run%257;
        for (size_t i=0;i<length;++i) {
            seed=seed*1664525u+1013904223u;
            static const char alphabet[]="[]()<>`\\!*\"'abc&;#012x \t\r\n";
            bytes[i]=alphabet[(seed>>16)%(sizeof(alphabet)-1)];
        }
        memcpy(original,bytes,length); CHECK(sb_inline_init(&r,bytes,length).code==SB_OK);
        SBInlineToken t; size_t previous=0;
        while (sb_inline_next(&r,&t)) {
            CHECK(r.cursor>previous && r.cursor<=length && t.offset==previous && t.length==r.cursor-previous);
            CHECK(t.content<=length && t.content_length<=length-t.content);
            CHECK(t.destination<=length && t.destination_length<=length-t.destination);
            previous=r.cursor;
        }
        CHECK(previous==length); CHECK(sb_inline_text(&r,0,length,&text).code==SB_OK && strlen(text)<=length); free(text); text=NULL;
        CHECK(!memcmp(original,bytes,length)); sb_inline_free(&r);
    }
    printf("%u inline assertions passed, including 5000 bounded source cases.\n",checks); return 0;
}
