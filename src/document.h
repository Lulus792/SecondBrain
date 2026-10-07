#ifndef SB_DOCUMENT_H
#define SB_DOCUMENT_H
#include "projection.h"
#include "markdown.h"
#include "references.h"
#define SB_DOCUMENT_NODES 65536u
#define SB_DOCUMENT_DEPTH 64u
typedef enum { SB_DOC_ROOT,SB_DOC_QUOTE,SB_DOC_LIST,SB_DOC_ITEM,SB_DOC_PARAGRAPH,SB_DOC_HEADING,SB_DOC_CODE,SB_DOC_RULE,SB_DOC_RAW,SB_DOC_TABLE } SBDocumentKind;
typedef struct {
    SBDocumentKind kind;
    size_t parent,first,last,next,offset,end,line,end_line,content;
    unsigned level,start,indent,padding;
    char marker;
    bool ordered,tight,blank,has_blank,setext;
    SBProjection view;
} SBDocumentNode;
typedef struct {
    const char *source;size_t length,count,capacity;
    SBDocumentNode *nodes;
    bool ready;
} SBDocument;
/* Fresh storage; free also after errors. Source remains borrowed. */
SBStatus sb_document_init(SBDocument *document,const char *source,size_t length);
/* Environment borrows the completed document's views; free it before document. */
SBStatus sb_document_references(const SBDocument *document,SBReferences *references);
SBStatus sb_document_content(const SBDocumentNode *node,const char **text,size_t *length);
void sb_document_free(SBDocument *document);
#endif
