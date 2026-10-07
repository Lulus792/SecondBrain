typedef struct { const char *name; uint32_t first,second; } EntityName;
#include "entity_names.inc"
static size_t entity_utf8(uint32_t cp,char *out) {
    if (cp<0x80) { out[0]=(char)cp; return 1; }
    if (cp<0x800) { out[0]=(char)(0xc0|(cp>>6)); out[1]=(char)(0x80|(cp&63)); return 2; }
    if (cp<0x10000) { out[0]=(char)(0xe0|(cp>>12)); out[1]=(char)(0x80|((cp>>6)&63)); out[2]=(char)(0x80|(cp&63)); return 3; }
    out[0]=(char)(0xf0|(cp>>18)); out[1]=(char)(0x80|((cp>>12)&63)); out[2]=(char)(0x80|((cp>>6)&63)); out[3]=(char)(0x80|(cp&63)); return 4;
}
/* Bounded by the longest pinned name and the CommonMark numeric grammar. */
static size_t entity_decode(const char *text,size_t length,char out[8],size_t *bytes) {
    if (!length || text[0]!='&') return 0;
    if (length>=2 && text[1]=='#') {
        unsigned base=10,max_digits=7; size_t p=2,start; uint32_t value=0;
        if (p<length && (text[p]=='x' || text[p]=='X')) { base=16; max_digits=6; ++p; }
        start=p;
        while (p<length && p-start<max_digits) {
            unsigned char c=(unsigned char)text[p]; unsigned digit=c>='0' && c<='9' ? c-'0' : base==16 && c>='a' && c<='f' ? c-'a'+10 : base==16 && c>='A' && c<='F' ? c-'A'+10 : 16;
            if (digit>=base) break;
            value=value*base+digit; ++p;
        }
        if (p==start || p>=length || text[p]!=';') return 0;
        if (!value || value>0x10ffff || (value>=0xd800 && value<=0xdfff)) value=0xfffd;
        *bytes=entity_utf8(value,out); return p+1;
    }
    size_t p=1;
    while (p<length && p<32 && (ascii_letter(text[p]) || (text[p]>='0' && text[p]<='9'))) ++p;
    if (p>=length || text[p]!=';' || p==1) return 0;
    char name[34]; memcpy(name,text,p+1); name[p+1]=0;
    size_t lo=0,hi=sizeof(entity_names)/sizeof(*entity_names);
    while (lo<hi) { size_t mid=lo+(hi-lo)/2; if (strcmp(entity_names[mid].name,name)<0) lo=mid+1; else hi=mid; }
    if (lo==sizeof(entity_names)/sizeof(*entity_names) || strcmp(entity_names[lo].name,name)) return 0;
    *bytes=entity_utf8(entity_names[lo].first,out);
    if (entity_names[lo].second) *bytes+=entity_utf8(entity_names[lo].second,out+*bytes);
    return p+1;
}
static bool entity_opaque(const SBInline *r,size_t p) {
    size_t lo=0,hi=r->opaque_count;
    while (lo<hi) { size_t mid=lo+(hi-lo)/2; if (r->opaque[mid].offset<=p) lo=mid+1; else hi=mid; }
    return lo && p<r->opaque[lo-1].offset+r->opaque[lo-1].length;
}
