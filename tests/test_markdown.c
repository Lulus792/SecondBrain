#include "markdown.h"
#include "sb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"MARKDOWN FAIL %d: %s\n",__LINE__,#x); exit(1); } } while (0)
static void token(SBMarkdown *r,SBMarkdownKind kind,unsigned level,const char *value) {
    SBMarkdownBlock b; size_t before=r->cursor;
    CHECK(sb_markdown_next(r,&b)); CHECK(r->cursor>before);
    CHECK(b.kind==kind && b.level==level);
    CHECK(b.content<=r->length && b.length<=r->length-b.content);
    CHECK(b.length==strlen(value) && !memcmp(r->text+b.content,value,b.length));
}
static void done(SBMarkdown *r) { SBMarkdownBlock b; CHECK(!sb_markdown_next(r,&b)); }
int main(void) {
    SBMarkdown r;
#define START(s) sb_markdown_init(&r,s,strlen(s),false)
    START("  # Titel ü ##  \r\n##\tZweiter Abschnitt\n####### Rohtext\n#Ohne Leerzeichen\n");
    token(&r,SB_MD_HEADING,1,"Titel ü"); token(&r,SB_MD_HEADING,2,"Zweiter Abschnitt");
    token(&r,SB_MD_TEXT,0,"####### Rohtext\n#Ohne Leerzeichen"); done(&r);
    START("#\n# ###\n## Ende#\n## Ende \\#\n");
    token(&r,SB_MD_HEADING,1,""); token(&r,SB_MD_HEADING,1,"");
    token(&r,SB_MD_HEADING,2,"Ende#"); token(&r,SB_MD_HEADING,2,"Ende \\#"); done(&r);
    START("  ```c\n   # Code bleibt Code\n```x\n~~~\n ```` \t\n# Echt\n");
    token(&r,SB_MD_FENCE,0,"  ```c"); token(&r,SB_MD_CODE,0," # Code bleibt Code");
    token(&r,SB_MD_CODE,0,"```x"); token(&r,SB_MD_CODE,0,"~~~");
    token(&r,SB_MD_FENCE,0," ```` \t"); token(&r,SB_MD_HEADING,1,"Echt"); done(&r);
    START("~~~~\n```\n~~~\n~~~~~\nText\n");
    token(&r,SB_MD_FENCE,0,"~~~~"); token(&r,SB_MD_CODE,0,"```"); token(&r,SB_MD_CODE,0,"~~~");
    token(&r,SB_MD_FENCE,0,"~~~~~"); token(&r,SB_MD_TEXT,0,"Text"); done(&r);
    START("```info`keine Öffnung\nFortsetzung\n\n```\nUnbeendet\n\n# Kein Titel\n");
    token(&r,SB_MD_TEXT,0,"```info`keine Öffnung\nFortsetzung"); token(&r,SB_MD_BLANK,0,"");
    token(&r,SB_MD_FENCE,0,"```"); token(&r,SB_MD_CODE,0,"Unbeendet"); token(&r,SB_MD_CODE,0,"");
    token(&r,SB_MD_CODE,0,"# Kein Titel"); done(&r);
    START("    ```\n\t# eingerückter Code\n\nAbsatz\n    fortgesetzt\n");
    token(&r,SB_MD_CODE,0,"```"); token(&r,SB_MD_CODE,0,"# eingerückter Code");
    token(&r,SB_MD_BLANK,0,""); token(&r,SB_MD_TEXT,0,"Absatz\n    fortgesetzt"); done(&r);
    START("Titel\rZweite Zeile\r=== \t\r\n2026 ist Text\nund läuft weiter\n- Eintrag\n+ Eintrag\n7) Eintrag\n1234567890. Kein Marker\nFortsetzung\n");
    token(&r,SB_MD_HEADING,1,"Titel\rZweite Zeile");
    token(&r,SB_MD_TEXT,0,"2026 ist Text\nund läuft weiter");
    token(&r,SB_MD_TEXT,0,"- Eintrag"); token(&r,SB_MD_TEXT,0,"+ Eintrag"); token(&r,SB_MD_TEXT,0,"7) Eintrag");
    token(&r,SB_MD_TEXT,0,"1234567890. Kein Marker\nFortsetzung"); done(&r);
    START("Absatz\n    - kein neuer Block\n    > ebenfalls Fortsetzung\n");
    token(&r,SB_MD_TEXT,0,"Absatz\n    - kein neuer Block\n    > ebenfalls Fortsetzung"); done(&r);
    START("Absatz\n*\n2. fortgesetzt\n1. neuer Eintrag\n");
    token(&r,SB_MD_TEXT,0,"Absatz\n*\n2. fortgesetzt"); token(&r,SB_MD_TEXT,0,"1. neuer Eintrag"); done(&r);
    START("Abschnitt\n--\n\n---\n> Zitat\n| Spalte |\n**betont**\nweiter\n");
    token(&r,SB_MD_HEADING,2,"Abschnitt"); token(&r,SB_MD_BLANK,0,""); token(&r,SB_MD_RULE,0,"---");
    token(&r,SB_MD_TEXT,0,"> Zitat"); token(&r,SB_MD_TEXT,0,"| Spalte |");
    token(&r,SB_MD_TEXT,0,"**betont**\nweiter"); done(&r);
    START("***\n  _ _ _\n-\t-\t-\r\n    ---\n\\***\n**\n*-*\n\nAbsatz\n***\nDanach\n");
    token(&r,SB_MD_RULE,0,"***"); token(&r,SB_MD_RULE,0,"  _ _ _"); token(&r,SB_MD_RULE,0,"-\t-\t-");
    token(&r,SB_MD_CODE,0,"---"); token(&r,SB_MD_TEXT,0,"\\***\n**\n*-*");
    token(&r,SB_MD_BLANK,0,""); token(&r,SB_MD_TEXT,0,"Absatz"); token(&r,SB_MD_RULE,0,"***"); token(&r,SB_MD_TEXT,0,"Danach"); done(&r);
    START("Abschnitt\n---\n\n***\n\n```\n---\n***\n```\n");
    token(&r,SB_MD_HEADING,2,"Abschnitt"); token(&r,SB_MD_BLANK,0,""); token(&r,SB_MD_RULE,0,"***"); token(&r,SB_MD_BLANK,0,"");
    token(&r,SB_MD_FENCE,0,"```"); token(&r,SB_MD_CODE,0,"---"); token(&r,SB_MD_CODE,0,"***"); token(&r,SB_MD_FENCE,0,"```"); done(&r);
    const char *literal="# Keine Überschrift\r\n```\n  \n";
    sb_markdown_init(&r,literal,strlen(literal),true);
    token(&r,SB_MD_CODE,0,"# Keine Überschrift"); token(&r,SB_MD_CODE,0,"```"); token(&r,SB_MD_CODE,0,"  "); done(&r);
    sb_markdown_init(&r,NULL,100,false); done(&r);
    char title[40];
    CHECK(sb_markdown_title("\n  # Titel ü ##\n",title,sizeof(title)).code==SB_OK && !strcmp(title,"Titel ü"));
    CHECK(sb_markdown_title("Titel\r\nZweite Zeile\r\n===\n",title,sizeof(title)).code==SB_OK && !strcmp(title,"Titel Zweite Zeile"));
    CHECK(sb_markdown_title("####### Kein Titel",title,sizeof(title)).code==SB_OK && !*title);
    CHECK(sb_markdown_title("```\n# Nicht der Titel\n```",title,sizeof(title)).code==SB_OK && !*title);
    CHECK(sb_markdown_title("## ΩΩΩ",title,6).code==SB_OK && !strcmp(title,"ΩΩ"));
    CHECK(sb_markdown_title("# Ω",title,2).code==SB_OK && !*title);
    CHECK(sb_markdown_title("# Titel",title,0).code==SB_INVALID);
    CHECK(sb_markdown_title("# [Titel][x]\n\n> [x]: /ziel\n",title,sizeof(title)).code==SB_OK && !strcmp(title,"Titel"));
    CHECK(sb_markdown_title("> # Zitat\n\n# Später\n",title,sizeof(title)).code==SB_OK && !*title);
    CHECK(sb_markdown_title("[x]: /ziel\n\n# [Titel][x]\n",title,sizeof(title)).code==SB_OK && !strcmp(title,"Titel"));
    CHECK(sb_markdown_title(NULL,title,sizeof(title)).code==SB_INVALID);

    /* Deterministic arbitrary byte strings: every span is bounded, progress
       holds, the parser writes neither source bytes nor beyond their length. */
    uint32_t seed=0x6431u;
    for (unsigned run=0;run<10000;++run) {
        char bytes[258],original[258]; size_t length=run%257;
        for (size_t i=0;i<length;++i) {
            seed=seed*1664525u+1013904223u;
            static const char symbols[]="#`~ \t\r\n-=*+_>|01234567ü";
            bytes[i]=symbols[(seed>>16)%(sizeof(symbols)-1)];
        }
        memcpy(original,bytes,length); sb_markdown_init(&r,bytes,length,run%2!=0);
        SBMarkdownBlock b; size_t previous=0,tokens=0;
        while (sb_markdown_next(&r,&b)) {
            CHECK(r.cursor>previous && r.cursor<=length && ++tokens<=length);
            CHECK(b.offset>=previous && b.content>=b.offset && b.content<=length && b.length<=length-b.content);
            previous=r.cursor;
        }
        CHECK(!memcmp(bytes,original,length)); CHECK(previous==length);
    }
    printf("%u markdown block assertions passed, including 10000 bounded source cases.\n",checks);
    return 0;
}
