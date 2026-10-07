#ifndef SB_REFERENCES_H
#define SB_REFERENCES_H
#include "sb.h"
#define SB_REFERENCE_LIMIT 65536u
#define SB_REFERENCE_LABEL_CAP 11989u
/* Source ranges remain borrowed; normalized names belong to the environment. */
typedef struct {
    size_t offset,end,label,label_length,destination,destination_length;
} SBReferenceDefinition;
typedef struct {
    char *name;
    SBReferenceDefinition source;
} SBReference;
typedef struct {
    const char *text;
    size_t length,count,capacity,name_bytes;
    SBReference *items;
} SBReferences;
SBStatus sb_reference_label(const char *text,size_t length,char *out,size_t capacity);
bool sb_reference_parse(const char *text,size_t length,size_t offset,SBReferenceDefinition *out,size_t *budget);
SBStatus sb_references_init(SBReferences *references,const char *text,size_t length);
void sb_references_free(SBReferences *references);
size_t sb_reference_find(const SBReferences *references,const char *text,size_t length);
#endif
