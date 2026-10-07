#ifndef SB_TABLE_H
#define SB_TABLE_H
#include "sb.h"
#define SB_TABLE_COLUMNS 64u
#define SB_TABLE_CELLS 65536u
typedef enum { SB_TABLE_LEFT,SB_TABLE_CENTER,SB_TABLE_RIGHT } SBTableAlignment;
typedef struct { size_t offset,length; } SBTableCell;
typedef struct { size_t offset,next,count; SBTableCell cells[SB_TABLE_COLUMNS]; } SBTableRow;
typedef struct {
    const char *text; size_t length,offset,body,end,columns,rows;
    SBTableAlignment alignment[SB_TABLE_COLUMNS];
} SBTable;
bool sb_table_parse(const char *text,size_t length,size_t offset,SBTable *table);
bool sb_table_next(const SBTable *table,size_t *cursor,SBTableRow *row);
SBStatus sb_table_cell_text(const SBTable *table,const SBTableCell *cell,char **text);
#endif
