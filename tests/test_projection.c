#include "projection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"PROJECTION FAIL %d: %s\n",__LINE__,#x);exit(1);}}while(0)
int main(void) {
    const char *source=">\tAlpha\r\n  β\rEnd";SBProjection v;
    CHECK(sb_projection_init(&v,source,strlen(source)).code==SB_OK);
    SBProjectionCursor cursor;size_t used,origin;
    CHECK(sb_projection_cursor(&cursor,1,7,1).code==SB_OK);
    CHECK(sb_projection_indent(source,strlen(source),&cursor,1,&used).code==SB_OK && used==1 && cursor.pending==2 && cursor.column==2);
    CHECK(sb_projection_remainder(&v,&cursor).code==SB_OK && !strcmp(v.text,"  Alpha"));
    CHECK(sb_projection_newline(&v,7,2).code==SB_OK);
    CHECK(sb_projection_copy(&v,11,2).code==SB_OK);
    CHECK(sb_projection_newline(&v,13,1).code==SB_OK);
    CHECK(sb_projection_copy(&v,14,3).code==SB_OK);
    CHECK(sb_projection_newline(&v,17,0).code==SB_OK);
    CHECK(!strcmp(v.text,"  Alpha\nβ\nEnd\n"));
    const size_t origins[]={1,1,2,3,4,5,6,7,11,12,13,14,15,16,17,17};
    for(size_t i=0;i<=v.length;++i){CHECK(sb_projection_source(&v,i,&origin).code==SB_OK && origin==origins[i]);}
    size_t before=v.length,count=v.count;
    CHECK(sb_projection_newline(&v,7,1).code==SB_OK); // standalone CR is permitted
    CHECK(sb_projection_newline(&v,2,1).code==SB_INVALID);
    CHECK(sb_projection_copy(&v,18,1).code==SB_INVALID);
    CHECK(v.length==before+1 && v.count==count+1);sb_projection_free(&v);
    CHECK(sb_projection_init(&v,NULL,0).code==SB_OK && !strcmp(v.text,""));
    CHECK(sb_projection_source(&v,0,&origin).code==SB_OK && origin==0);
    CHECK(sb_projection_newline(&v,0,0).code==SB_OK && !strcmp(v.text,"\n"));sb_projection_free(&v);
    CHECK(sb_projection_cursor(&cursor,0,0,0).code==SB_OK);
    CHECK(sb_projection_indent(NULL,0,&cursor,0,&used).code==SB_OK && !used);
    CHECK(sb_projection_remainder(&v,&cursor).code==SB_INVALID);
    CHECK(sb_projection_source(&v,0,&origin).code==SB_INVALID);
    CHECK(sb_projection_init(&v,NULL,1).code==SB_INVALID);sb_projection_free(&v);
    CHECK(sb_projection_cursor(&cursor,0,1,SIZE_MAX).code==SB_OK);
    SBProjectionCursor saved=cursor;CHECK(sb_projection_indent(" ",1,&cursor,1,&used).code==SB_LIMIT && !memcmp(&saved,&cursor,sizeof(cursor)));
    CHECK(sb_projection_cursor(&cursor,0,3,0).code==SB_OK);
    CHECK(sb_projection_indent("  x",3,&cursor,7,&used).code==SB_OK && used==2 && cursor.byte==2);
    const char *tab_remainders[4][6]={
        {"\tx","   x","  x"," x","x","x"},
        {"\tx","  x"," x","x","x","x"},
        {"\tx"," x","x","x","x","x"},
        {"\tx","x","x","x","x","x"}
    };
    const size_t tab_widths[]={4,3,2,1};
    for(size_t column=0;column<4;++column)for(size_t take=0;take<6;++take){
        CHECK(sb_projection_init(&v,"\tx",2).code==SB_OK);
        CHECK(sb_projection_cursor(&cursor,0,2,column).code==SB_OK);
        CHECK(sb_projection_indent("\tx",2,&cursor,take,&used).code==SB_OK && used==(take<tab_widths[column] ? take : tab_widths[column]));
        CHECK(sb_projection_remainder(&v,&cursor).code==SB_OK && !strcmp(v.text,tab_remainders[column][take]));
        sb_projection_free(&v);
    }
    CHECK(sb_projection_init(&v,"abcdef",6).code==SB_OK);
    CHECK(sb_projection_copy(&v,0,3).code==SB_OK && sb_projection_copy(&v,3,3).code==SB_OK && v.count==1 && !strcmp(v.text,"abcdef"));sb_projection_free(&v);
    uint32_t seed=0x1706;
    for(unsigned run=0;run<3000;++run){
        char raw[80],original[80];size_t n=run%79;for(size_t i=0;i<n;++i){seed=seed*1664525u+1013904223u;raw[i]=" \tab"[(seed>>16)%4];}
        memcpy(original,raw,n);
        CHECK(sb_projection_init(&v,raw,n).code==SB_OK);
        CHECK(sb_projection_cursor(&cursor,0,n,run%4).code==SB_OK);
        size_t want=run%24;CHECK(sb_projection_indent(raw,n,&cursor,want,&used).code==SB_OK);
        CHECK(used<=want && cursor.byte<=n && cursor.pending<=3);
        CHECK(sb_projection_remainder(&v,&cursor).code==SB_OK);
        CHECK(v.length==cursor.pending+n-cursor.byte);
        for(size_t i=0;i<v.length;++i){CHECK(sb_projection_source(&v,i,&origin).code==SB_OK);CHECK(origin<n && (i<cursor.pending ? raw[origin]=='\t' && v.text[i]==' ' : v.text[i]==raw[origin]));}
        CHECK(!memcmp(original,raw,n));
        sb_projection_free(&v);
    }
    CHECK(sb_projection_init(&v,"\t",1).code==SB_OK);
    for(size_t i=0;i<SB_PROJECTION_LIMIT;++i)CHECK(sb_projection_spaces(&v,1,0).code==SB_OK);
    before=v.length;CHECK(sb_projection_spaces(&v,1,0).code==SB_LIMIT && v.length==before && v.count==SB_PROJECTION_LIMIT);sb_projection_free(&v);
    char *large=malloc(SB_TEXT_LIMIT);CHECK(large);memset(large,'a',SB_TEXT_LIMIT);large[0]='\t';
    CHECK(sb_projection_init(&v,large,SB_TEXT_LIMIT).code==SB_OK);
    CHECK(sb_projection_copy(&v,0,SB_TEXT_LIMIT-2).code==SB_OK);
    cursor=(SBProjectionCursor){.byte=1,.end=2,.pending=3,.anchor=0};
    before=v.length;count=v.count;
    CHECK(sb_projection_remainder(&v,&cursor).code==SB_LIMIT && v.length==before && v.count==count && v.text[before]==0 && v.spans[0].length==before);
    CHECK(sb_projection_copy(&v,SB_TEXT_LIMIT-2,2).code==SB_OK && v.length==SB_TEXT_LIMIT);
    CHECK(sb_projection_newline(&v,SB_TEXT_LIMIT,0).code==SB_OK && v.length==SB_TEXT_LIMIT+1);
    CHECK(sb_projection_copy(&v,0,1).code==SB_LIMIT && v.length==SB_TEXT_LIMIT+1 && v.text[v.length]==0);
    CHECK(large[0]=='\t' && large[SB_TEXT_LIMIT-1]=='a');sb_projection_free(&v);free(large);
    printf("%u projection assertions passed: partial tabs, source maps, newline forms, rollback, bounds and 3000 generated views.\n",checks);return 0;
}
