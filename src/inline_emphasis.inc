/* Own C implementation of CommonMark delimiter matching; source spans stay immutable. */
typedef struct { uint32_t lo,hi; } UClassRange;
#include "inline_unicode.inc"
typedef struct SBInlineMark { size_t offset,length; unsigned style,active; bool opening; } Mark;
typedef struct { size_t offset,original,left,remaining,previous,next; char marker; bool opens,closes; } Delimiter;
static bool at(const SBInline *,size_t,size_t,SBInlineToken *);
static uint32_t scalar(const char *text,size_t p,size_t end) {
    if (p>=end) return 0;
    unsigned char first=(unsigned char)text[p++];
    if (first<0x80) return first;
    unsigned n=first>=0xc2 && first<=0xdf ? 1 : first>=0xe0 && first<=0xef ? 2 : first>=0xf0 && first<=0xf4 ? 3 : 0;
    if (!n || n>end-p) return UINT32_MAX;
    uint32_t cp=first&((1u<<(6-n))-1);
    while (n--) { unsigned char c=(unsigned char)text[p++]; if ((c&0xc0)!=0x80) return UINT32_MAX; cp=(cp<<6)|(c&63); }
    return cp;
}
static bool unicode_space(uint32_t cp) {
    return !cp || cp==9 || cp==10 || cp==12 || cp==13 || cp==32 || cp==0xa0 || cp==0x1680 ||
        (cp>=0x2000 && cp<=0x200a) || cp==0x202f || cp==0x205f || cp==0x3000;
}
static bool unicode_punct(uint32_t cp) {
    size_t lo=0,hi=sizeof(unicode_punctuation)/sizeof(*unicode_punctuation);
    while (lo<hi) { size_t mid=lo+(hi-lo)/2; if (unicode_punctuation[mid].hi<cp) lo=mid+1; else hi=mid; }
    return lo<sizeof(unicode_punctuation)/sizeof(*unicode_punctuation) && unicode_punctuation[lo].lo<=cp;
}
static void unlink_delimiter(Delimiter *d,size_t i) {
    if (d[i].previous!=SIZE_MAX) d[d[i].previous].next=d[i].next;
    if (d[i].next!=SIZE_MAX) d[d[i].next].previous=d[i].previous;
}
static SBStatus mark_add(SBInline *r,size_t offset,size_t length,unsigned style,bool opening) {
    void *storage=r->marks; SBStatus status=reserve(&storage,&r->mark_capacity,r->mark_count,sizeof(Mark)); r->marks=storage;
    if (status.code==SB_OK) r->marks[r->mark_count++]=(Mark){.offset=offset,.length=length,.style=style,.opening=opening};
    return status;
}
static SBStatus emphasis_scope(SBInline *r,size_t begin,size_t end,unsigned depth,size_t *budget) {
    if (depth>32) return sb_error(SB_LIMIT,"Markdown-Textstruktur ist zu tief.");
    Delimiter *d=NULL; size_t count=0,capacity=0; SBStatus status=sb_ok();
    for (size_t p=begin;p<end;) {
        if (!spend(budget)) { status=sb_error(SB_LIMIT,"Markdown-Hervorhebung ist zu komplex."); goto done; }
        SBInlineToken t; at(r,p,end,&t);
        if (t.kind==SB_INLINE_LINK || t.kind==SB_INLINE_IMAGE) {
            status=emphasis_scope(r,t.content,t.content+t.content_length,depth+1,budget);
            if (status.code!=SB_OK) goto done;
        }
        if (t.kind!=SB_INLINE_TEXT || (r->text[p]!='*' && r->text[p]!='_')) { p+=t.length; continue; }
        char marker=r->text[p]; size_t start=p;
        while (p<end && r->text[p]==marker) ++p;
        size_t previous=start;
        if (previous>begin) { --previous; while (previous>begin && ((unsigned char)r->text[previous]&0xc0)==0x80) --previous; }
        uint32_t before=start==begin ? 0 : scalar(r->text,previous,start),after=scalar(r->text,p,end);
        bool ws_before=unicode_space(before),ws_after=unicode_space(after),punct_before=unicode_punct(before),punct_after=unicode_punct(after);
        bool left=!ws_after && (!punct_after || ws_before || punct_before);
        bool right=!ws_before && (!punct_before || ws_after || punct_after);
        bool opens=left && (marker=='*' || !right || punct_before),closes=right && (marker=='*' || !left || punct_after);
        if (!opens && !closes) continue;
        void *storage=d; status=reserve(&storage,&capacity,count,sizeof(*d)); d=storage;
        if (status.code!=SB_OK) goto done;
        d[count]=(Delimiter){.offset=start,.original=p-start,.remaining=p-start,.previous=count ? count-1 : SIZE_MAX,
            .next=SIZE_MAX,.marker=marker,.opens=opens,.closes=closes};
        if (count) d[count-1].next=count;
        ++count;
    }
    size_t bottoms[12]; for (unsigned i=0;i<12;++i) bottoms[i]=SIZE_MAX;
    size_t closer=count ? 0 : SIZE_MAX;
    while (closer!=SIZE_MAX) {
        if (!d[closer].closes) { closer=d[closer].next; continue; }
        unsigned key=(d[closer].marker=='_' ? 6 : 0)+(d[closer].opens ? 3 : 0)+(unsigned)(d[closer].original%3);
        size_t opener=d[closer].previous;
        while (opener!=SIZE_MAX && (bottoms[key]==SIZE_MAX || opener>bottoms[key])) {
            if (!spend(budget)) { status=sb_error(SB_LIMIT,"Markdown-Hervorhebung ist zu komplex."); goto done; }
            bool odd=(d[closer].opens || d[opener].closes) && d[closer].original%3 &&
                (d[opener].original+d[closer].original)%3==0;
            if (d[opener].marker==d[closer].marker && d[opener].opens && !odd) break;
            opener=d[opener].previous;
        }
        if (opener==SIZE_MAX || (bottoms[key]!=SIZE_MAX && opener<=bottoms[key])) {
            bottoms[key]=d[closer].previous; size_t next=d[closer].next;
            if (!d[closer].opens) unlink_delimiter(d,closer);
            closer=next; continue;
        }
        size_t use=d[opener].remaining>=2 && d[closer].remaining>=2 ? 2 : 1;
        unsigned style=use==2 ? SB_TEXT_BOLD : SB_TEXT_ITALIC;
        size_t open=d[opener].offset+d[opener].left+d[opener].remaining-use,close=d[closer].offset+d[closer].left;
        status=mark_add(r,open,use,style,true); if (status.code!=SB_OK) goto done;
        status=mark_add(r,close,use,style,false); if (status.code!=SB_OK) goto done;
        d[opener].remaining-=use; d[closer].remaining-=use; d[closer].left+=use;
        d[opener].next=closer; d[closer].previous=opener;
        if (!d[opener].remaining) unlink_delimiter(d,opener);
        if (!d[closer].remaining) { size_t next=d[closer].next; unlink_delimiter(d,closer); closer=next; }
    }
done:
    free(d); return status;
}
static int mark_position(const void *a,const void *b) {
    const Mark *x=a,*y=b; return x->offset<y->offset ? -1 : x->offset>y->offset;
}
static SBStatus emphasis_init(SBInline *r) {
    size_t budget=r->length*32+64; SBStatus status=emphasis_scope(r,0,r->length,0,&budget);
    if (status.code!=SB_OK) return status;
    if (r->mark_count>1) qsort(r->marks,r->mark_count,sizeof(Mark),mark_position);
    unsigned italic=0,bold=0;
    for (size_t i=0;i<r->mark_count;++i) {
        Mark *m=&r->marks[i]; unsigned *count=m->style==SB_TEXT_ITALIC ? &italic : &bold;
        if (m->opening) ++*count; else if (*count) --*count;
        m->active=(italic ? SB_TEXT_ITALIC : 0)|(bold ? SB_TEXT_BOLD : 0);
    }
    r->marks_ready=true;
    return sb_ok();
}
static Mark *mark_at(const SBInline *r,size_t p) {
    size_t lo=0,hi=r->mark_count;
    while (lo<hi) { size_t mid=lo+(hi-lo)/2; if (r->marks[mid].offset<=p) lo=mid+1; else hi=mid; }
    return lo ? &r->marks[lo-1] : NULL;
}
