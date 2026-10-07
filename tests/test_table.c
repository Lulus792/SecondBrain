#include "table.h"
#include "markdown.h"
#include "inline.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"TABLE FAIL %d: %s\n",__LINE__,#x); exit(1); } } while (0)
static void cell(const SBTable *t,SBTableRow *row,size_t col,const char *expected) {
    char *raw=NULL,*plain=NULL; SBInline r;
    CHECK(sb_table_cell_text(t,&row->cells[col],&raw).code==SB_OK);
    CHECK(sb_inline_init(&r,raw,strlen(raw)).code==SB_OK);
    CHECK(sb_inline_text(&r,0,strlen(raw),&plain).code==SB_OK && !strcmp(plain,expected));
    sb_inline_free(&r);free(raw);free(plain);
}
int main(void) {
    const char *source="| Aktion | Taste | Zahl |\r\n| :--- | :---: | ---: |\r\n| Speichern | `Ctrl+S` | 7 |\r\nNur eine Zelle\r\n| Lesen | F6 | 8 | ignoriert |\r\n\r\nDanach.\r\n";
    SBTable t; CHECK(sb_table_parse(source,strlen(source),0,&t));
    CHECK(t.columns==3 && t.rows==4 && t.end<strlen(source));
    CHECK(t.alignment[0]==SB_TABLE_LEFT && t.alignment[1]==SB_TABLE_CENTER && t.alignment[2]==SB_TABLE_RIGHT);
    size_t cursor=0; SBTableRow row;
    CHECK(sb_table_next(&t,&cursor,&row));cell(&t,&row,0,"Aktion");cell(&t,&row,1,"Taste");
    CHECK(sb_table_next(&t,&cursor,&row));cell(&t,&row,0,"Speichern");cell(&t,&row,1,"Ctrl+S");
    CHECK(sb_table_next(&t,&cursor,&row));cell(&t,&row,0,"Nur eine Zelle");cell(&t,&row,1,"");cell(&t,&row,2,"");
    CHECK(sb_table_next(&t,&cursor,&row));cell(&t,&row,2,"8");CHECK(row.count==3);CHECK(!sb_table_next(&t,&cursor,&row));
    const char *pipes="| Links \\| rechts |\n| --- |\n| `\\|` **\\|** |\n# Stopp\n";
    CHECK(sb_table_parse(pipes,strlen(pipes),0,&t));CHECK(t.columns==1 && t.rows==2);
    cursor=0;CHECK(sb_table_next(&t,&cursor,&row));cell(&t,&row,0,"Links | rechts");
    CHECK(sb_table_next(&t,&cursor,&row));cell(&t,&row,0,"| |");
    CHECK(!sb_table_parse("a | b\n---\nx",strlen("a | b\n---\nx"),0,&t));
    CHECK(!sb_table_parse("    a | b\n--- | ---\n",strlen("    a | b\n--- | ---\n"),0,&t));
    CHECK(!sb_table_parse("   ",3,0,&t));
    const char *empty="| | |\n| - | - |\n| | |\n";
    CHECK(sb_table_parse(empty,strlen(empty),0,&t) && t.columns==2 && t.rows==2);
    cursor=0;CHECK(sb_table_next(&t,&cursor,&row));cell(&t,&row,0,"");cell(&t,&row,1,"");
    SBMarkdown md;SBMarkdownBlock block; const char *mixed="Vorher\nA | B\n- | -\nx | y\n\n```\na | b\n- | -\n```\n";
    sb_markdown_init(&md,mixed,strlen(mixed),false);
    CHECK(sb_markdown_next(&md,&block) && block.kind==SB_MD_TEXT && block.length==6);
    CHECK(sb_markdown_next(&md,&block) && block.kind==SB_MD_TABLE);
    CHECK(sb_markdown_next(&md,&block) && block.kind==SB_MD_BLANK);
    CHECK(sb_markdown_next(&md,&block) && block.kind==SB_MD_FENCE);
    CHECK(sb_markdown_next(&md,&block) && block.kind==SB_MD_CODE);
    const char *indented_delimiter="A | B\n    - | -\n";
    CHECK(!sb_table_parse(indented_delimiter,strlen(indented_delimiter),0,&t));
    char columns[1400]; size_t used=0;
    for (unsigned line=0;line<2;++line) {
        for (unsigned c=0;c<SB_TABLE_COLUMNS;++c) { columns[used++]=line ? '-' : 'A'; columns[used++]='|'; }
        columns[used++]='\n';
    }
    CHECK(sb_table_parse(columns,used,0,&t) && t.columns==SB_TABLE_COLUMNS);
    used=0;
    for (unsigned line=0;line<2;++line) {
        for (unsigned c=0;c<=SB_TABLE_COLUMNS;++c) { columns[used++]=line ? '-' : 'A'; columns[used++]='|'; }
        columns[used++]='\n';
    }
    CHECK(!sb_table_parse(columns,used,0,&t));
    size_t capacity=8+(size_t)SB_TABLE_CELLS*2;
    char *many=malloc(capacity); CHECK(many); memcpy(many,"V |\n- |\n",8);
    for (size_t i=8;i<capacity;i+=2) { many[i]='x'; many[i+1]='\n'; }
    CHECK(sb_table_parse(many,capacity-2,0,&t) && t.rows==SB_TABLE_CELLS);
    CHECK(!sb_table_parse(many,capacity,0,&t)); free(many);
    uint32_t seed=0x916;
    for (unsigned run=0;run<5000;++run) {
        char bytes[258],original[258]; size_t length=run%257;
        for (size_t i=0;i<length;++i) { seed=seed*1664525u+1013904223u; static const char alphabet[]="|\\-:abc \t\r\n`#[]";bytes[i]=alphabet[(seed>>16)%(sizeof(alphabet)-1)]; }
        memcpy(original,bytes,length);
        for (size_t offset=0;offset<length;++offset) if (!offset || bytes[offset-1]=='\n') {
            if (sb_table_parse(bytes,length,offset,&t)) {
                CHECK(t.offset==offset && t.end<=length && t.columns>0 && t.columns<=SB_TABLE_COLUMNS && t.rows*t.columns<=SB_TABLE_CELLS);
                size_t previous=offset;cursor=offset;size_t rows=0;
                while (sb_table_next(&t,&cursor,&row)) {
                    CHECK(cursor>previous && cursor<=t.end && row.count==t.columns);previous=cursor;++rows;
                    for (size_t j=0;j<row.count;++j) CHECK(row.cells[j].offset<=length && row.cells[j].length<=length-row.cells[j].offset);
                }
                CHECK(rows==t.rows && cursor==t.end);
            }
        }
        CHECK(!memcmp(original,bytes,length));
    }
    printf("%u table assertions passed, including 5000 bounded inputs.\n",checks); return 0;
}
