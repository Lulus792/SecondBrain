#ifndef SB_MARKDOWN_H
#define SB_MARKDOWN_H
#include <stdbool.h>
#include <stddef.h>
/* Read-only source spans. Unsupported container/inline syntax stays literal. */
typedef enum { SB_MD_TEXT,SB_MD_HEADING,SB_MD_CODE,SB_MD_BLANK,SB_MD_FENCE,SB_MD_TABLE,SB_MD_RULE,SB_MD_REFERENCE } SBMarkdownKind;
typedef struct {
    SBMarkdownKind kind;
    size_t offset,content,length;
    unsigned level;
} SBMarkdownBlock;
typedef struct {
    const char *text;
    size_t length,cursor,fence_length;
    unsigned fence_indent;
    char fence;
    bool literal;
    size_t reference_budget;
    bool reference_limit;
} SBMarkdown;
void sb_markdown_init(SBMarkdown *reader,const char *text,size_t length,bool literal);
bool sb_markdown_boundary(const char *text,size_t length,size_t offset);
bool sb_markdown_next(SBMarkdown *reader,SBMarkdownBlock *block);
#endif
