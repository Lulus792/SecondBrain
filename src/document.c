#include "document.h"
#include "references.h"
#include "table.h"
#include "inline.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
typedef struct {
    SBDocument *doc;
    size_t stack[SB_DOCUMENT_DEPTH+1],depth,leaf,budget,line;
    char fence;
    size_t fence_length;
    unsigned fence_indent,html;
} Parser;
typedef struct { bool valid,ordered;char marker;unsigned start,indent,padding;SBProjectionCursor after; } Marker;
static bool horizontal(char c){return c==' '||c=='\t';}
static bool leaf(SBDocumentKind k){return k>=SB_DOC_PARAGRAPH;}
static bool spend(Parser *p,size_t amount){if(amount>p->budget){p->budget=0;return false;}p->budget-=amount;return true;}
static SBStatus add(Parser *p,SBDocumentKind kind,size_t parent,size_t offset,size_t *index) {
    SBDocument *d=p->doc;
    if(d->count==SB_DOCUMENT_NODES)return sb_error(SB_LIMIT,"Das Dokument enthält zu viele Blöcke.");
    if(d->count==d->capacity){size_t cap=d->capacity?d->capacity*2:64;void *nodes=realloc(d->nodes,cap*sizeof(*d->nodes));if(!nodes)return sb_error(SB_MEMORY,"Kein Speicher für Dokumentblöcke.");d->nodes=nodes;d->capacity=cap;}
    size_t i=d->count++;d->nodes[i]=(SBDocumentNode){.kind=kind,.parent=parent,.first=SIZE_MAX,.last=SIZE_MAX,.next=SIZE_MAX,.offset=offset,.end=offset,.line=p->line,.end_line=p->line,.tight=true};
    if(parent!=SIZE_MAX){SBDocumentNode *node=&d->nodes[parent];if(node->last!=SIZE_MAX)d->nodes[node->last].next=i;else node->first=i;node->last=i;}
    if(leaf(kind)){SBStatus status=sb_projection_init(&d->nodes[i].view,d->source,d->length);if(status.code!=SB_OK)return status;d->nodes[i].view.origin=offset;}
    *index=i;return sb_ok();
}
static SBStatus push(Parser *p,SBDocumentKind kind,size_t offset,size_t *index) {
    if(p->depth==SB_DOCUMENT_DEPTH)return sb_error(SB_LIMIT,"Markdown-Container sind zu tief verschachtelt.");
    SBStatus status=add(p,kind,p->stack[p->depth],offset,index);
    if(status.code==SB_OK)p->stack[++p->depth]=*index;
    return status;
}
static char peek(Parser *p,const SBProjectionCursor *c){return c->pending?' ':c->byte<c->end?p->doc->source[c->byte]:0;}
static SBStatus indent(Parser *p,SBProjectionCursor *c,size_t wanted,size_t *used){
    size_t before=c->byte;SBStatus status=sb_projection_indent(p->doc->source,p->doc->length,c,wanted,used);
    if(status.code==SB_OK && !spend(p,c->byte-before+1))return sb_error(SB_LIMIT,"Markdown-Container sind zu komplex.");return status;
}
static SBStatus leading(Parser *p,SBProjectionCursor c,SBProjectionCursor *body,size_t *width){
    SBStatus status=indent(p,&c,SIZE_MAX,width);*body=c;return status;
}
static void ascii(SBProjectionCursor *c,size_t n){c->byte+=n;c->column+=n;}
static bool same_list(const SBDocumentNode *n,Marker m){return n->kind==SB_DOC_LIST && n->ordered==m.ordered && n->marker==m.marker;}
static Marker marker(Parser *p,SBProjectionCursor c,SBProjectionCursor body,size_t width,bool interrupt) {
    Marker m={0};if(width>3 || body.pending || body.byte==body.end)return m;
    const char *t=p->doc->source;size_t q=body.byte;char ch=t[q];unsigned number=0;
    if(ch=='-'||ch=='+'||ch=='*'){++q;m.marker=ch;}
    else {
        size_t digits=0;while(q<body.end && t[q]>='0'&&t[q]<='9'&&digits<10){number=number*10+(unsigned)(t[q++]-'0');++digits;}
        if(!digits || digits>9 || q==body.end || (t[q]!='.'&&t[q]!=')') || (interrupt && number!=1))return m;
        m.ordered=true;m.start=number;m.marker=t[q++];
    }
    if(q<body.end && !horizontal(t[q]))return (Marker){0};
    size_t bytes=q-body.byte;ascii(&body,bytes);SBProjectionCursor after=body;size_t padding=0;
    if(indent(p,&after,5,&padding).code!=SB_OK)return (Marker){0};
    bool empty=peek(p,&after)==0;
    if(interrupt && empty)return (Marker){0};
    if(padding<1 || padding>=5 || empty){after=body;size_t one=0;if(horizontal(peek(p,&after)))indent(p,&after,1,&one);padding=1;}
    m.valid=true;m.indent=(unsigned)width;m.padding=(unsigned)(bytes+padding);m.after=after;(void)c;return m;
}
static bool lower_equal(const char *t,size_t n,const char *word){size_t i=0;for(;word[i];++i){if(i>=n)return false;char c=t[i];if(c>='A'&&c<='Z')c=(char)(c-'A'+'a');if(c!=word[i])return false;}return true;}
static unsigned html_start(Parser *p,SBProjectionCursor body,size_t width,bool interrupt) {
    if(width>3 || body.pending || peek(p,&body)!='<')return 0;
    const char *t=p->doc->source+body.byte;size_t n=body.end-body.byte;
    if(n>=4 && !memcmp(t,"<!--",4))return 2;
    if(n>=2 && t[1]=='?')return 3;
    if(n>=9 && !memcmp(t,"<![CDATA[",9))return 5;
    if(n>=3 && t[1]=='!' && t[2]>='A'&&t[2]<='Z')return 4;
    static const char *raw[]={"script","pre","style","textarea"};
    for(size_t i=0;i<4;++i){size_t z=strlen(raw[i]);if(n>z+1 && lower_equal(t+1,n-1,raw[i]) && (horizontal(t[z+1])||t[z+1]=='>'||t[z+1]==0))return 1;}
    static const char *blocks[]={"address","article","aside","base","basefont","blockquote","body","caption","center","col","colgroup","dd","details","dialog","dir","div","dl","dt","fieldset","figcaption","figure","footer","form","frame","frameset","h1","h2","h3","h4","h5","h6","head","header","hr","html","iframe","legend","li","link","main","menu","menuitem","nav","noframes","ol","optgroup","option","p","param","section","search","summary","table","tbody","td","tfoot","th","thead","title","tr","track","ul"};
    size_t start=n>1 && t[1]=='/' ? 2 : 1;
    for(size_t i=0;i<sizeof(blocks)/sizeof(*blocks);++i){size_t z=strlen(blocks[i]);if(n>start+z && lower_equal(t+start,n-start,blocks[i]) && (horizontal(t[start+z])||t[start+z]=='>'||t[start+z]=='/'))return 6;}
    /* Complete standalone tags form type 7; inline raw HTML stays literal. */
    if(!interrupt && n>2){
        size_t end=0;if(sb_inline_raw_tag(t,n,&end)){while(end<n&&horizontal(t[end]))++end;if(end==n)return 7;}
    }
    return 0;
}
static bool contains(const char *s,size_t n,const char *needle,bool insensitive){size_t z=strlen(needle);for(size_t i=0;i+z<=n;++i)if(insensitive?lower_equal(s+i,n-i,needle):!memcmp(s+i,needle,z))return true;return false;}
static bool html_close(Parser *p,SBProjectionCursor c){const char *s=p->doc->source+c.byte;size_t n=c.end-c.byte;
    if(p->html==1)return contains(s,n,"</script>",true)||contains(s,n,"</pre>",true)||contains(s,n,"</style>",true)||contains(s,n,"</textarea>",true);
    if(p->html==2)return contains(s,n,"-->",false);if(p->html==3)return contains(s,n,"?>",false);if(p->html==4)return contains(s,n,">",false);if(p->html==5)return contains(s,n,"]]>",false);return false;
}
static SBStatus append(Parser *p,size_t node,SBProjectionCursor c,size_t ending,size_t ending_length,bool newline) {
    SBProjection *v=&p->doc->nodes[node].view;size_t before=v->length;
    SBStatus status=sb_projection_remainder(v,&c);if(status.code==SB_OK && newline)status=sb_projection_newline(v,ending,ending_length);
    if(status.code==SB_OK && !spend(p,v->length-before+1))return sb_error(SB_LIMIT,"Markdown-Container sind zu komplex.");
    p->doc->nodes[node].end=ending+ending_length;p->doc->nodes[node].end_line=p->line;return status;
}
static size_t definitions(Parser *p,SBDocumentNode *n) {
    size_t offset=0;SBReferenceDefinition definition;
    while(offset<n->view.length && sb_reference_parse(n->view.text,n->view.length,offset,&definition,&p->budget))offset=definition.end;
    return offset;
}
static void close_leaf(Parser *p) {
    if(p->leaf==SIZE_MAX)return;SBDocumentNode *n=&p->doc->nodes[p->leaf];
    if(n->kind==SB_DOC_PARAGRAPH || (n->kind==SB_DOC_HEADING && n->setext)){
        n->content=definitions(p,n);
        if(n->content && n->content<n->view.length){sb_projection_source(&n->view,n->content,&n->offset);for(size_t i=0;i<n->content;++i)if(n->view.text[i]=='\n')++n->line;}
    }
    if(n->kind==SB_DOC_CODE && !p->fence){while(n->view.length && n->view.text[n->view.length-1]=='\n'){size_t previous=n->view.length-1;while(previous && n->view.text[previous-1]!='\n')--previous;bool blank=true;for(size_t i=previous;i+1<n->view.length;++i)if(!horizontal(n->view.text[i])){blank=false;break;}if(!blank)break;sb_projection_truncate(&n->view,previous);sb_projection_source(&n->view,previous,&n->end);if(n->end_line>n->line)--n->end_line;}}
    p->leaf=SIZE_MAX;p->fence=0;p->html=0;
}
static void close_to(Parser *p,size_t depth,size_t end,size_t end_line) {
    close_leaf(p);
    while(p->depth>depth){SBDocumentNode *n=&p->doc->nodes[p->stack[p->depth--]];n->end=end;n->end_line=end_line;
        if(n->last!=SIZE_MAX && (n->kind==SB_DOC_LIST || n->kind==SB_DOC_ITEM)){n->end_line=p->doc->nodes[n->last].end_line;n->end=p->doc->nodes[n->last].end;}
    }
}
static bool visible_paragraph(Parser *p){if(p->leaf==SIZE_MAX || p->doc->nodes[p->leaf].kind!=SB_DOC_PARAGRAPH)return false;SBDocumentNode *n=&p->doc->nodes[p->leaf];return definitions(p,n)<n->view.length;}
static SBStatus line(Parser *p,size_t start,size_t end,size_t next) {
    SBProjectionCursor c={.byte=start,.end=end};size_t matched=0;bool blank=false;
    for(size_t depth=1;depth<=p->depth;++depth){
        SBDocumentNode *n=&p->doc->nodes[p->stack[depth]];SBProjectionCursor body;size_t width=0;SBStatus status=leading(p,c,&body,&width);if(status.code!=SB_OK)return status;blank=peek(p,&body)==0;
        if(n->kind==SB_DOC_LIST){matched=depth;continue;}
        if(n->kind==SB_DOC_QUOTE){if(width>3 || peek(p,&body)!='>')break;c=body;ascii(&c,1);size_t used=0;if(horizontal(peek(p,&c))){status=indent(p,&c,1,&used);if(status.code!=SB_OK)return status;}}
        else if(n->kind==SB_DOC_ITEM){if(blank){if(n->first==SIZE_MAX)break;c=body;}else if(width>=n->indent+n->padding){size_t used=0;status=indent(p,&c,n->indent+n->padding,&used);if(status.code!=SB_OK)return status;}else break;}
        matched=depth;
    }
    SBProjectionCursor body;size_t width=0;SBStatus status=leading(p,c,&body,&width);if(status.code!=SB_OK)return status;blank=peek(p,&body)==0;
    SBMarkdownProbe probe;sb_markdown_probe(p->doc->source,end,body.byte,width>UINT_MAX?UINT_MAX:(unsigned)width,&probe);
    bool paragraph=p->leaf!=SIZE_MAX && p->doc->nodes[p->leaf].kind==SB_DOC_PARAGRAPH;
    bool sibling_list=p->doc->nodes[p->stack[matched]].kind==SB_DOC_LIST;
    Marker m=marker(p,c,body,width,paragraph && !sibling_list);unsigned html=html_start(p,body,width,paragraph);
    bool starts=width<4 && (peek(p,&body)=='>' || probe.kind==SB_MD_HEADING || probe.kind==SB_MD_FENCE || probe.kind==SB_MD_RULE || m.valid || html);
    if(matched<p->depth && paragraph && !blank && !starts){return append(p,p->leaf,body,end,next-end,true);}
    if(matched<p->depth)close_to(p,matched,start,p->line-1);
    if(p->leaf!=SIZE_MAX){
        SBDocumentNode *n=&p->doc->nodes[p->leaf];
        if(n->kind==SB_DOC_CODE && p->fence){
            size_t q=body.byte;while(q<end && p->doc->source[q]==p->fence)++q;size_t tail=q;while(tail<end&&horizontal(p->doc->source[tail]))++tail;
            if(width<=3 && q-body.byte>=p->fence_length && tail==end){n->end=next;n->end_line=p->line;close_leaf(p);return sb_ok();}
            size_t used=0;status=indent(p,&c,p->fence_indent,&used);if(status.code!=SB_OK)return status;return append(p,p->leaf,c,end,next-end,true);
        }
        if(n->kind==SB_DOC_CODE){if(width>=4 || blank){size_t used=0;if(width>=4){status=indent(p,&c,4,&used);if(status.code!=SB_OK)return status;}else c=body;return append(p,p->leaf,c,end,next-end,true);}close_leaf(p);}
        else if(n->kind==SB_DOC_RAW){if(blank && p->html>=6)close_leaf(p);else{status=append(p,p->leaf,c,end,next-end,true);if(html_close(p,c))close_leaf(p);return status;}}
        else if(n->kind==SB_DOC_PARAGRAPH){
            if(!blank && width<4 && probe.setext && visible_paragraph(p)){
                n->kind=SB_DOC_HEADING;n->setext=true;n->level=probe.setext;n->end=next;n->end_line=p->line;close_leaf(p);return sb_ok();
            }
            if(!blank && !starts){status=append(p,p->leaf,body,end,next-end,true);if(status.code!=SB_OK)return status;SBTable table;if(sb_table_parse(n->view.text,n->view.length,0,&table))n->kind=SB_DOC_TABLE;return sb_ok();}
            close_leaf(p);
        } else if(n->kind==SB_DOC_TABLE){if(!blank && !starts)return append(p,p->leaf,body,end,next-end,true);close_leaf(p);}
    }
    if(blank){if(p->depth){SBDocumentNode *n=&p->doc->nodes[p->stack[p->depth]];n->blank=true;n->has_blank=true;}return sb_ok();}
    for(;;){
        status=leading(p,c,&body,&width);if(status.code!=SB_OK)return status;
        sb_markdown_probe(p->doc->source,end,body.byte,width>UINT_MAX?UINT_MAX:(unsigned)width,&probe);
        m=marker(p,c,body,width,false);
        if(p->doc->nodes[p->stack[p->depth]].kind==SB_DOC_LIST && (probe.kind==SB_MD_RULE || !m.valid)){close_to(p,p->depth-1,start,p->line-1);continue;}
        if(width<=3 && peek(p,&body)=='>'){
            size_t index;status=push(p,SB_DOC_QUOTE,body.byte,&index);if(status.code!=SB_OK)return status;c=body;ascii(&c,1);size_t used=0;if(horizontal(peek(p,&c))){status=indent(p,&c,1,&used);if(status.code!=SB_OK)return status;}continue;
        }
        if(probe.kind==SB_MD_RULE){
            if(p->doc->nodes[p->stack[p->depth]].kind==SB_DOC_LIST)close_to(p,p->depth-1,start,p->line-1);
            size_t index;status=add(p,SB_DOC_RULE,p->stack[p->depth],body.byte,&index);if(status.code!=SB_OK)return status;p->doc->nodes[index].end=next;p->doc->nodes[index].end_line=p->line;return sb_ok();}
        if(m.valid){
            SBDocumentNode *tip=&p->doc->nodes[p->stack[p->depth]];
            if(tip->kind==SB_DOC_LIST && !same_list(tip,m)){if(!p->depth)return sb_error(SB_INVALID,"Listenwurzel ist ungültig.");close_to(p,p->depth-1,start,p->line-1);tip=&p->doc->nodes[p->stack[p->depth]];}
            if(!same_list(tip,m)){size_t index;status=push(p,SB_DOC_LIST,body.byte,&index);if(status.code!=SB_OK)return status;SBDocumentNode *list=&p->doc->nodes[index];list->ordered=m.ordered;list->start=m.start;list->marker=m.marker;}
            size_t index;status=push(p,SB_DOC_ITEM,body.byte,&index);if(status.code!=SB_OK)return status;SBDocumentNode *item=&p->doc->nodes[index];item->indent=m.indent;item->padding=m.padding;c=m.after;continue;
        }
        if(p->doc->nodes[p->stack[p->depth]].kind==SB_DOC_LIST){close_to(p,p->depth-1,start,p->line-1);continue;}
        if(peek(p,&body)==0)return sb_ok();
        SBDocumentKind kind=probe.kind==SB_MD_HEADING ? SB_DOC_HEADING : probe.kind==SB_MD_FENCE || width>=4 ? SB_DOC_CODE : SB_DOC_PARAGRAPH;
        html=html_start(p,body,width,false);if(html)kind=SB_DOC_RAW;
        size_t index;status=add(p,kind,p->stack[p->depth],body.byte,&index);if(status.code!=SB_OK)return status;p->leaf=index;
        if(kind==SB_DOC_HEADING){p->doc->nodes[index].level=probe.level;SBProjectionCursor heading={.byte=probe.content,.end=probe.content+probe.length};status=append(p,index,heading,end,next-end,false);close_leaf(p);return status;}
        if(kind==SB_DOC_CODE && probe.kind==SB_MD_FENCE){p->fence=probe.fence;p->fence_length=probe.fence_length;p->fence_indent=(unsigned)width;p->doc->nodes[index].end=next;p->doc->nodes[index].view.origin=next;return sb_ok();}
        if(kind==SB_DOC_CODE){size_t used=0;status=indent(p,&c,4,&used);if(status.code!=SB_OK)return status;}
        else if(kind==SB_DOC_RAW)p->html=html;
        else c=body;
        status=append(p,index,c,end,next-end,true);if(kind==SB_DOC_RAW && html_close(p,c))close_leaf(p);return status;
    }
}
void sb_document_free(SBDocument *d){if(!d)return;for(size_t i=0;i<d->count;++i)sb_projection_free(&d->nodes[i].view);free(d->nodes);*d=(SBDocument){0};}
SBStatus sb_document_content(const SBDocumentNode *n,const char **text,size_t *length) {
    if(text)*text=NULL;if(length)*length=0;
    if(!n || !text || !length || !leaf(n->kind) || !n->view.text || n->content>n->view.length)return sb_error(SB_INVALID,"Dokumentinhalt fehlt.");
    *text=n->view.text+n->content;*length=n->view.length-n->content;
    if(n->kind==SB_DOC_PARAGRAPH || n->kind==SB_DOC_HEADING)while(*length && ((*text)[*length-1]=='\n' || horizontal((*text)[*length-1])))--*length;
    return sb_ok();
}
SBStatus sb_document_references(const SBDocument *d,SBReferences *references) {
    if(!references)return sb_error(SB_INVALID,"Dokumentumgebung fehlt.");
    *references=(SBReferences){0};
    if(!d || !d->ready || !d->count)return sb_error(SB_INVALID,"Dokumentumgebung fehlt.");
    SBStatus status=sb_references_begin(references,d->source,d->length);if(status.code!=SB_OK)return status;
    size_t budget=d->length*64+1024;
    for(size_t i=0;i<d->count;++i){const SBDocumentNode *n=&d->nodes[i];if(n->kind!=SB_DOC_PARAGRAPH && n->kind!=SB_DOC_HEADING)continue;
        size_t offset=0;SBReferenceDefinition definition;
        while(offset<n->content && sb_reference_parse(n->view.text,n->view.length,offset,&definition,&budget)){
            size_t order=0;status=sb_projection_source(&n->view,definition.offset,&order);if(status.code==SB_OK)status=sb_references_add(references,n->view.text,n->view.length,&definition,order);if(status.code!=SB_OK)return status;offset=definition.end;
        }
        if(!budget)return sb_error(SB_LIMIT,"Dokumentdefinitionen sind zu komplex.");
    }
    sb_references_finish(references);return sb_ok();
}
SBStatus sb_document_init(SBDocument *d,const char *source,size_t length){
    if(!d)return sb_error(SB_INVALID,"Dokumentbaum fehlt.");*d=(SBDocument){.source=source,.length=length};
    if((!source&&length)||length>SB_TEXT_LIMIT || (length && !sb_text_valid(source,length)))return sb_error(SB_INVALID,"Dokumentquelle ist ungültig oder zu groß.");
    Parser p={.doc=d,.leaf=SIZE_MAX,.budget=length*128+1024,.line=1};size_t root;SBStatus status=add(&p,SB_DOC_ROOT,SIZE_MAX,0,&root);if(status.code!=SB_OK)return status;p.stack[0]=root;
    for(size_t start=0;start<length;){size_t end=start;while(end<length&&source[end]!='\n'&&source[end]!='\r')++end;size_t next=end;if(next<length){char ch=source[next++];if(ch=='\r'&&next<length&&source[next]=='\n')++next;}
        status=line(&p,start,end,next);if(status.code!=SB_OK)return status;if(!p.budget)return sb_error(SB_LIMIT,"Dokumentstruktur ist zu komplex.");start=next;++p.line;
    }
    close_to(&p,0,length,p.line?p.line-1:0);d->nodes[0].end=length;d->nodes[0].end_line=p.line>1?p.line-1:1;
    /* Blank separation is determined from sibling/source lines, rather than
       contaminating a parent list with blanks only inside a nested list. */
    for(size_t i=0;i<d->count;++i)if(d->nodes[i].kind==SB_DOC_LIST){SBDocumentNode *list=&d->nodes[i];for(size_t item=list->first;item!=SIZE_MAX;item=d->nodes[item].next){SBDocumentNode *it=&d->nodes[item];if(it->next!=SIZE_MAX && it->end_line+1<d->nodes[it->next].line)list->tight=false;for(size_t child=it->first;child!=SIZE_MAX;child=d->nodes[child].next){size_t sibling=d->nodes[child].next;if(sibling!=SIZE_MAX && d->nodes[child].end_line+1<d->nodes[sibling].line)list->tight=false;}}}
    if(!p.budget)return sb_error(SB_LIMIT,"Dokumentstruktur ist zu komplex.");d->ready=true;return sb_ok();
}
