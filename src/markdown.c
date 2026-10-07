#include "markdown.h"
#include "table.h"
#include <string.h>

typedef struct { size_t start,end,next,content; unsigned indent; } Line;
static bool space(char c) { return c==' ' || c=='\t'; }
static Line line_at(const SBMarkdown *r,size_t start) {
    Line l={.start=start,.end=start,.content=start};
    while (l.end<r->length && r->text[l.end]!='\n' && r->text[l.end]!='\r') ++l.end;
    l.next=l.end;
    if (l.next<r->length) {
        char ending=r->text[l.next++];
        if (ending=='\r' && l.next<r->length && r->text[l.next]=='\n') ++l.next;
    }
    while (l.content<l.end && space(r->text[l.content])) {
        l.indent=r->text[l.content]=='\t' ? (l.indent/4+1)*4 : l.indent+1;
        ++l.content;
    }
    return l;
}
static bool blank(Line l) { return l.content==l.end; }
static bool heading(const SBMarkdown *r,Line l,SBMarkdownBlock *b) {
    if (l.indent>3) return false;
    size_t p=l.content,count=0;
    while (p<l.end && r->text[p]=='#') { ++p; ++count; }
    if (!count || count>6 || (p<l.end && !space(r->text[p]))) return false;
    while (p<l.end && space(r->text[p])) ++p;
    size_t end=l.end;
    while (end>p && space(r->text[end-1])) --end;
    size_t hashes=end;
    while (hashes>p && r->text[hashes-1]=='#') --hashes;
    /* A closing sequence needs whitespace before it; opening whitespace may
       also precede an otherwise empty heading. Escaped hashes remain text. */
    if (hashes<end && ((hashes>p && space(r->text[hashes-1])) || hashes==p)) {
        end=hashes;
        while (end>p && space(r->text[end-1])) --end;
    }
    b->kind=SB_MD_HEADING; b->level=(unsigned)count; b->content=p; b->length=end-p;
    return true;
}
static size_t fence(const SBMarkdown *r,Line l,char *marker,size_t *tail) {
    if (l.indent>3 || l.content==l.end) return 0;
    char c=r->text[l.content]; if (c!='`' && c!='~') return 0;
    size_t p=l.content;
    while (p<l.end && r->text[p]==c) ++p;
    size_t count=p-l.content; if (count<3) return 0;
    *marker=c; *tail=p; return count;
}
static unsigned underline(const SBMarkdown *r,Line l) {
    if (l.indent>3 || blank(l)) return 0;
    char c=r->text[l.content]; if (c!='=' && c!='-') return 0;
    size_t p=l.content;
    while (p<l.end && r->text[p]==c) ++p;
    while (p<l.end && space(r->text[p])) ++p;
    return p==l.end ? (c=='=' ? 1 : 2) : 0;
}
static bool rule(const SBMarkdown *r,Line l) {
    if (l.indent>3 || blank(l)) return false;
    char c=r->text[l.content]; if (c!='-' && c!='*' && c!='_') return false;
    size_t count=0;
    for (size_t p=l.content;p<l.end;++p) {
        if (r->text[p]==c) ++count;
        else if (!space(r->text[p])) return false;
    }
    return count>=3;
}
static bool container(const SBMarkdown *r,Line l) {
    if (l.indent>3 || blank(l)) return false;
    const char *t=r->text; size_t p=l.content;
    if (t[p]=='>' || t[p]=='|') return true;
    if (t[p]=='-' || t[p]=='*' || t[p]=='+') return p+1==l.end || space(t[p+1]);
    size_t digits=0;
    while (p<l.end && t[p]>='0' && t[p]<='9' && digits<10) { ++p; ++digits; }
    return digits>0 && digits<=9 && p<l.end && (t[p]=='.' || t[p]==')') &&
        (p+1==l.end || space(t[p+1]));
}
static bool opening(const SBMarkdown *r,Line l,char *marker,size_t *count) {
    size_t tail=0; *count=fence(r,l,marker,&tail);
    if (!*count) return false;
    return *marker!='`' || !memchr(r->text+tail,'`',l.end-tail);
}
bool sb_markdown_boundary(const char *text,size_t length,size_t offset) {
    if (!text || offset>=length) return true;
    SBMarkdown r={.text=text,.length=length}; Line l=line_at(&r,offset);
    SBMarkdownBlock block={0}; char marker=0; size_t count=0;
    return blank(l) || l.indent>=4 || heading(&r,l,&block) || opening(&r,l,&marker,&count) || rule(&r,l) ||
        (container(&r,l) && text[l.content]!='|');
}
void sb_markdown_init(SBMarkdown *r,const char *text,size_t length,bool literal) {
    if (!r) return;
    *r=(SBMarkdown){.text=text ? text : "",.length=text ? length : 0,.literal=literal};
}
bool sb_markdown_next(SBMarkdown *r,SBMarkdownBlock *b) {
    if (!r || !b || r->cursor>=r->length) return false;
    Line l=line_at(r,r->cursor);
    *b=(SBMarkdownBlock){.kind=SB_MD_TEXT,.offset=l.start,.content=l.start,.length=l.end-l.start};
    r->cursor=l.next;
    if (r->literal) { b->kind=blank(l) && l.end==l.start ? SB_MD_BLANK : SB_MD_CODE; return true; }
    if (r->fence) {
        char marker=0; size_t tail=0,count=fence(r,l,&marker,&tail);
        if (count>=r->fence_length && marker==r->fence) {
            while (tail<l.end && space(r->text[tail])) ++tail;
            if (tail==l.end) { r->fence=0; b->kind=SB_MD_FENCE; return true; }
        }
        b->kind=SB_MD_CODE;
        unsigned removed=0;
        while (b->content<l.end && r->text[b->content]==' ' && removed<r->fence_indent) { ++removed; ++b->content; }
        b->length=l.end-b->content; return true;
    }
    if (blank(l)) { b->kind=SB_MD_BLANK; return true; }
    char marker=0; size_t count=0;
    if (opening(r,l,&marker,&count)) {
        r->fence=marker; r->fence_length=count; r->fence_indent=l.indent;
        b->kind=SB_MD_FENCE; return true;
    }
    if (heading(r,l,b)) return true;
    SBTable table;
    if (sb_table_parse(r->text,r->length,l.start,&table)) { b->kind=SB_MD_TABLE; b->length=table.end-l.start; r->cursor=table.end; return true; }
    if (l.indent>=4) {
        b->kind=SB_MD_CODE; size_t p=l.start; unsigned removed=0;
        while (p<l.end && space(r->text[p]) && removed<4) {
            removed=r->text[p]=='\t' ? (removed/4+1)*4 : removed+1; ++p;
        }
        b->content=p; b->length=l.end-p; return true;
    }
    if (rule(r,l)) { b->kind=SB_MD_RULE; return true; }
    if (container(r,l)) return true;
    b->content=l.content;
    size_t end=l.end;
    while (r->cursor<r->length) {
        Line next=line_at(r,r->cursor); SBMarkdownBlock probe={0};
        if (blank(next)) break;
        if (sb_table_parse(r->text,r->length,next.start,&table)) break;
        unsigned level=underline(r,next);
        if (level) {
            b->kind=SB_MD_HEADING; b->level=level; r->cursor=next.next;
            break;
        }
        if (heading(r,next,&probe) || opening(r,next,&marker,&count) || rule(r,next) || container(r,next)) break;
        end=next.end; r->cursor=next.next;
    }
    if (b->kind==SB_MD_HEADING) while (end>b->content && space(r->text[end-1])) --end;
    b->length=end-b->content;
    return true;
}
