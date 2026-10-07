#include "table.h"
#include "markdown.h"
#include <stdlib.h>
#include <string.h>
static bool space(char c) { return c==' ' || c=='\t'; }
static void line(const char *text,size_t length,size_t start,size_t *end,size_t *next) {
    *end=start;
    while (*end<length && text[*end]!='\r' && text[*end]!='\n') ++*end;
    *next=*end;
    if (*next<length) { char c=text[(*next)++]; if (c=='\r' && *next<length && text[*next]=='\n') ++*next; }
}
static bool split(const char *text,size_t length,size_t start,SBTableRow *row,bool *pipe) {
    size_t end,next; line(text,length,start,&end,&next);
    *row=(SBTableRow){.offset=start,.next=next}; *pipe=false;
    size_t begin=start; while (begin<end && space(text[begin])) ++begin;
    while (end>begin && space(text[end-1])) --end;
    if (begin<end && text[begin]=='|') { *pipe=true; ++begin; }
    size_t cell=begin;
    for (size_t p=begin;p<=end;++p) {
        if (p<end && text[p]=='\\' && p+1<end) { ++p; continue; }
        if (p<end && text[p]!='|') continue;
        if (p<end) *pipe=true;
        size_t left=cell,right=p;
        while (left<right && space(text[left])) ++left;
        while (right>left && space(text[right-1])) --right;
        if (row->count<SB_TABLE_COLUMNS) row->cells[row->count]=(SBTableCell){left,right-left};
        ++row->count;
        cell=p+1;
        if (cell==end && p<end) break; /* optional trailing pipe */
    }
    return true;
}
bool sb_table_parse(const char *text,size_t length,size_t offset,SBTable *table) {
    if (!text || !table || offset>=length || length>SB_TEXT_LIMIT) return false;
    *table=(SBTable){0}; size_t indent=offset; while (indent<length && text[indent]==' ') ++indent;
    if (indent>=length || indent-offset>3 || text[indent]=='\t') return false;
    SBTableRow header,delimiter; bool header_pipe=false,delimiter_pipe=false;
    if (!split(text,length,offset,&header,&header_pipe) || !header_pipe || header.count>SB_TABLE_COLUMNS || header.next==length ||
        !split(text,length,header.next,&delimiter,&delimiter_pipe) || header.count!=delimiter.count) return false;
    size_t delimiter_indent=header.next;
    while (delimiter_indent<length && text[delimiter_indent]==' ') ++delimiter_indent;
    if (delimiter_indent-header.next>3 || (delimiter_indent<length && text[delimiter_indent]=='\t')) return false;
    SBTable parsed={.text=text,.length=length,.offset=offset,.body=delimiter.next,.end=delimiter.next,.columns=header.count,.rows=1};
    for (size_t i=0;i<delimiter.count;++i) {
        SBTableCell cell=delimiter.cells[i]; size_t p=cell.offset,end=p+cell.length;
        bool left=p<end && text[p]==':'; if (left) ++p;
        size_t start=p; while (p<end && text[p]=='-') ++p;
        if (p==start) return false;
        bool right=p<end && text[p]==':'; if (right) ++p;
        if (p!=end) return false;
        parsed.alignment[i]=left && right ? SB_TABLE_CENTER : right ? SB_TABLE_RIGHT : SB_TABLE_LEFT;
    }
    while (parsed.end<length && !sb_markdown_boundary(text,length,parsed.end)) {
        SBTableRow row; bool pipe=false;
        if (!split(text,length,parsed.end,&row,&pipe)) return false;
        if (++parsed.rows>SB_TABLE_CELLS/parsed.columns) return false;
        parsed.end=row.next;
    }
    *table=parsed; return true;
}
bool sb_table_next(const SBTable *table,size_t *cursor,SBTableRow *row) {
    if (!table || !cursor || !row || !table->columns || *cursor<table->offset || *cursor>=table->end) return false;
    bool pipe=false;
    if (!split(table->text,table->length,*cursor,row,&pipe)) return false;
    if (*cursor==table->offset) row->next=table->body;
    if (row->count>table->columns) row->count=table->columns;
    while (row->count<table->columns) row->cells[row->count++]=(SBTableCell){row->next,0};
    *cursor=row->next; return true;
}
SBStatus sb_table_cell_text(const SBTable *table,const SBTableCell *cell,char **text) {
    if (text) *text=NULL;
    if (!table || !cell || !text || cell->offset>table->length || cell->length>table->length-cell->offset)
        return sb_error(SB_INVALID,"Tabellenzelle ist ungültig.");
    char *out=malloc(cell->length+1); if (!out) return sb_error(SB_MEMORY,"Tabelle benötigt mehr Speicher.");
    size_t used=0;
    for (size_t i=0;i<cell->length;++i) {
        char c=table->text[cell->offset+i];
        if (c=='\\' && i+1<cell->length) {
            char next=table->text[cell->offset+i+1];
            if (next=='|') { out[used++]='|'; ++i; continue; }
            out[used++]=c; out[used++]=next; ++i; continue;
        }
        out[used++]=c;
    }
    out[used]=0; *text=out; return sb_ok();
}
