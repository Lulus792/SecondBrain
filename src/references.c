#include "references.h"
#include "markdown.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
typedef struct { uint32_t scalar; unsigned count; uint32_t mapping[3]; } CaseFold;
#include "casefold_data.inc"
static bool horizontal(char c) { return c==' ' || c=='\t'; }
static bool space(char c) { return horizontal(c) || c=='\r' || c=='\n'; }
static bool punct(unsigned char c) { return (c>=33 && c<=47)||(c>=58 && c<=64)||(c>=91 && c<=96)||(c>=123 && c<=126); }
static bool spend(size_t *budget) { if (!*budget) return false; --*budget; return true; }
static size_t scalar_read(const char *text,size_t *p) {
    unsigned char c=(unsigned char)text[(*p)++];
    if (c<128) return c;
    unsigned n=c<224 ? 1 : c<240 ? 2 : 3;
    uint32_t cp=c&((1u<<(6-n))-1);
    while(n--) cp=(cp<<6)|((unsigned char)text[(*p)++]&63);
    return cp;
}
static unsigned scalar_write(uint32_t cp,char out[4]) {
    if(cp<128){out[0]=(char)cp;return 1;}
    if(cp<2048){out[0]=(char)(192|(cp>>6));out[1]=(char)(128|(cp&63));return 2;}
    if(cp<65536){out[0]=(char)(224|(cp>>12));out[1]=(char)(128|((cp>>6)&63));out[2]=(char)(128|(cp&63));return 3;}
    out[0]=(char)(240|(cp>>18));out[1]=(char)(128|((cp>>12)&63));out[2]=(char)(128|((cp>>6)&63));out[3]=(char)(128|(cp&63));return 4;
}
SBStatus sb_reference_label(const char *text,size_t length,char *out,size_t capacity) {
    if (!out || !capacity) return sb_error(SB_INVALID,"Referenznamen-Puffer fehlt.");
    *out=0;
    if (!text || !length || length>999*4 || !sb_utf8_valid(text,length)) return sb_error(SB_INVALID,"Referenzname ist ungültig.");
    size_t used=0,characters=0; bool escaped=false,pending=false;
    for(size_t p=0;p<length;) {
        size_t start=p; uint32_t cp=(uint32_t)scalar_read(text,&p);
        if(++characters>999 || !cp || ((cp=='[' || cp==']') && !escaped)) { *out=0;return sb_error(SB_INVALID,"Referenzname ist ungültig."); }
        escaped=cp=='\\' && !escaped;
        if(p==start+1 && space((char)cp)) { pending=used!=0; continue; }
        if(pending) { if(used+1>=capacity) { *out=0;return sb_error(SB_LIMIT,"Referenzname ist zu lang."); } out[used++]=' ';pending=false; }
        size_t lo=0,hi=sizeof(case_folds)/sizeof(*case_folds);
        while(lo<hi){size_t mid=lo+(hi-lo)/2;if(case_folds[mid].scalar<cp)lo=mid+1;else hi=mid;}
        const CaseFold *fold=lo<sizeof(case_folds)/sizeof(*case_folds)&&case_folds[lo].scalar==cp ? &case_folds[lo] : NULL;
        unsigned count=fold ? fold->count : 1;
        for(unsigned i=0;i<count;++i) {
            char bytes[4];unsigned n=scalar_write(fold ? fold->mapping[i] : cp,bytes);
            if(n>=capacity-used) { *out=0;return sb_error(SB_LIMIT,"Referenzname ist zu lang."); }
            memcpy(out+used,bytes,n);used+=n;
        }
    }
    if(!used) return sb_error(SB_INVALID,"Referenzname ist leer.");
    out[used]=0;return sb_ok();
}
static bool label_end(const char *t,size_t n,size_t *p,size_t *budget) {
    if(*p>=n || t[(*p)++]!='[') return false;
    size_t start=*p;unsigned endings=0;
    while(*p<n && *p-start<=999*4) {
        if(!spend(budget))return false;
        char c=t[*p];
        if(c==']') { char name[SB_REFERENCE_LABEL_CAP]; if(sb_reference_label(t+start,*p-start,name,sizeof(name)).code!=SB_OK)return false; ++*p;return true; }
        if(c=='[')return false;
        if(c=='\\' && *p+1<n && punct((unsigned char)t[*p+1])) {*p+=2;endings=0;continue;}
        if(c=='\r'||c=='\n') {if(++endings>1)return false;if(c=='\r' && *p+1<n && t[*p+1]=='\n')++*p;}
        else if(!horizontal(c))endings=0;
        ++*p;
    }
    return false;
}
static bool spaces(const char *t,size_t n,size_t *p,size_t *budget) {
    unsigned endings=0;
    while(*p<n && space(t[*p])) {
        if(!spend(budget))return false;
        if(t[*p]=='\r'||t[*p]=='\n'){if(++endings>1)return false;if(t[*p]=='\r' && *p+1<n && t[*p+1]=='\n')++*p;}
        ++*p;
    }
    return true;
}
static size_t next_line(const char *t,size_t n,size_t p) {
    if(p<n && t[p]=='\r'){++p;if(p<n && t[p]=='\n')++p;}
    else if(p<n && t[p]=='\n')++p;
    return p;
}
static bool line_end(const char *t,size_t n,size_t *p,size_t *budget) {
    while(*p<n && horizontal(t[*p])){if(!spend(budget))return false;++*p;}
    if(*p<n && t[*p]!='\r' && t[*p]!='\n')return false;
    *p=next_line(t,n,*p);return true;
}
static bool title(const char *t,size_t n,size_t *p,size_t *budget) {
    if(*p>=n || (t[*p]!='\'' && t[*p]!='"' && t[*p]!='('))return false;
    char opening=t[(*p)++],closing=opening=='(' ? ')' : opening;unsigned endings=0;
    while(*p<n && t[*p]!=closing) {
        if(!spend(budget))return false;
        char c=t[*p];
        if(c=='\\' && *p+1<n && punct((unsigned char)t[*p+1])){*p+=2;endings=0;continue;}
        if(opening=='(' && c=='(')return false;
        if(c=='\r'||c=='\n'){if(++endings>1)return false;if(c=='\r' && *p+1<n && t[*p+1]=='\n')++*p;}
        else if(!horizontal(c))endings=0;
        ++*p;
    }
    if(*p==n)return false;
    ++*p;return line_end(t,n,p,budget);
}
bool sb_reference_parse(const char *t,size_t n,size_t offset,SBReferenceDefinition *out,size_t *budget) {
    if(!t || !out || !budget || offset>=n)return false;
    size_t p=offset;unsigned indent=0;
    while(p<n && horizontal(t[p])){if(!spend(budget))return false;indent=t[p]=='\t' ? (indent/4+1)*4 : indent+1;++p;}
    if(indent>3)return false;
    size_t label=p+1;
    if(!label_end(t,n,&p,budget) || p>=n || t[p++]!=':')return false;
    size_t label_length=p-label-2;
    if(!spaces(t,n,&p,budget) || p==n)return false;
    size_t destination=p,destination_length=0;
    if(t[p]=='<') {
        destination=++p;
        while(p<n && t[p]!='>') {
            if(!spend(budget) || t[p]=='\n' || t[p]=='\r' || t[p]=='<')return false;
            if(t[p]=='\\' && p+1<n && punct((unsigned char)t[p+1]))++p;
            ++p;
        }
        if(p==n)return false;
        destination_length=p-destination;++p;
    } else {
        unsigned depth=0;
        while(p<n && !space(t[p])) {
            if(!spend(budget) || (unsigned char)t[p]<32 || t[p]==127)return false;
            if(t[p]=='\\' && p+1<n && punct((unsigned char)t[p+1])){p+=2;continue;}
            if(t[p]=='(' && ++depth>32)return false;
            if(t[p]==')'){if(!depth)return false;--depth;}
            ++p;
        }
        if(depth || p==destination)return false;
        destination_length=p-destination;
    }
    size_t q=p;
    bool destination_line=line_end(t,n,&q,budget);
    bool separator=p<n && space(t[p]);
    size_t end=q;
    if(separator && spaces(t,n,&p,budget)) {
        size_t candidate=p;
        if(title(t,n,&candidate,budget))end=candidate;
        else if(!destination_line)return false;
    } else if(!destination_line)return false;
    if(!*budget)return false;
    *out=(SBReferenceDefinition){offset,end,label,label_length,destination,destination_length};return true;
}
static int compare(const void *a,const void *b) {
    const SBReference *x=a,*y=b;int c=strcmp(x->name,y->name);
    return c ? c : x->source.offset<y->source.offset ? -1 : x->source.offset>y->source.offset;
}
void sb_references_free(SBReferences *r) {
    if(!r)return;
    for(size_t i=0;i<r->count;++i)free(r->items[i].name);
    free(r->items);*r=(SBReferences){0};
}
SBStatus sb_references_init(SBReferences *r,const char *text,size_t length) {
    if(!r)return sb_error(SB_INVALID,"Referenzumgebung fehlt.");
    *r=(SBReferences){.text=text,.length=length};
    if((!text && length) || length>SB_TEXT_LIMIT)return sb_error(SB_INVALID,"Referenztext ist ungültig oder zu groß.");
    SBMarkdown reader;SBMarkdownBlock block;sb_markdown_init(&reader,text,length,false);
    while(sb_markdown_next(&reader,&block)) {
        if(reader.reference_limit)return sb_error(SB_LIMIT,"Markdown-Definitionen sind zu komplex.");
        if(block.kind!=SB_MD_REFERENCE)continue;
        SBReferenceDefinition source;
        if(!sb_reference_parse(text,length,block.offset,&source,&reader.reference_budget))return sb_error(SB_LIMIT,"Markdown-Definitionen sind zu komplex.");
        char name[SB_REFERENCE_LABEL_CAP];SBStatus status=sb_reference_label(text+source.label,source.label_length,name,sizeof(name));
        if(status.code!=SB_OK)return status;
        size_t bytes=strlen(name)+1;
        if(r->count==SB_REFERENCE_LIMIT || bytes>SB_TEXT_LIMIT-r->name_bytes)return sb_error(SB_LIMIT,"Das Dokument enthält zu viele Referenzdefinitionen.");
        if(r->count==r->capacity){size_t capacity=r->capacity ? r->capacity*2 : 32;void *items=realloc(r->items,capacity*sizeof(*r->items));if(!items)return sb_error(SB_MEMORY,"Kein Speicher für Referenzdefinitionen.");r->items=items;r->capacity=capacity;}
        char *copy=malloc(bytes);if(!copy)return sb_error(SB_MEMORY,"Kein Speicher für Referenznamen.");
        memcpy(copy,name,bytes);r->items[r->count++]=(SBReference){copy,source};r->name_bytes+=bytes;
    }
    if(r->count>1)qsort(r->items,r->count,sizeof(*r->items),compare);
    size_t used=0;
    for(size_t i=0;i<r->count;++i){
        if(used && !strcmp(r->items[used-1].name,r->items[i].name)){r->name_bytes-=strlen(r->items[i].name)+1;free(r->items[i].name);}
        else r->items[used++]=r->items[i];
    }
    r->count=used;return sb_ok();
}
size_t sb_reference_find(const SBReferences *r,const char *text,size_t length) {
    if(!r || !r->count)return SIZE_MAX;
    char name[SB_REFERENCE_LABEL_CAP];if(sb_reference_label(text,length,name,sizeof(name)).code!=SB_OK)return SIZE_MAX;
    size_t lo=0,hi=r->count;
    while(lo<hi){size_t mid=lo+(hi-lo)/2;if(strcmp(r->items[mid].name,name)<0)lo=mid+1;else hi=mid;}
    return lo<r->count && !strcmp(r->items[lo].name,name) ? lo : SIZE_MAX;
}
