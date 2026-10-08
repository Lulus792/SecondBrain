#include "scripts.h"
#include "grapheme.h"
#include "bidi.h"
#include <stdlib.h>
#include <string.h>
typedef struct {uint32_t lo,hi,script;} ScriptRange;
typedef struct {uint32_t lo,hi;unsigned short start,count;} ExtensionRange;
typedef struct {uint32_t cp,pair;unsigned char kind;} ScriptBracket;
#include "script_data.inc"
#define COUNT(a) (sizeof(a)/sizeof(*(a)))
SBScriptProperty sb_script_property(uint32_t cp) {
    SBScriptProperty p={.primary=SB_SCRIPT_UNKNOWN};size_t lo=0,hi=COUNT(script_ranges);
    while(lo<hi){size_t mid=lo+(hi-lo)/2;if(script_ranges[mid].hi<cp)lo=mid+1;else hi=mid;}
    if(lo<COUNT(script_ranges) && script_ranges[lo].lo<=cp)p.primary=script_ranges[lo].script;
    lo=0;hi=COUNT(extension_ranges);
    while(lo<hi){size_t mid=lo+(hi-lo)/2;if(extension_ranges[mid].hi<cp)lo=mid+1;else hi=mid;}
    if(lo<COUNT(extension_ranges) && extension_ranges[lo].lo<=cp){p.extensions=script_extensions+extension_ranges[lo].start;p.count=extension_ranges[lo].count;}
    lo=0;hi=COUNT(script_brackets);
    while(lo<hi){size_t mid=lo+(hi-lo)/2;if(script_brackets[mid].cp<cp)lo=mid+1;else hi=mid;}
    if(lo<COUNT(script_brackets) && script_brackets[lo].cp==cp){p.paired=script_brackets[lo].pair;p.bracket=script_brackets[lo].kind;}
    return p;
}
static bool specific(uint32_t script){return script!=SB_SCRIPT_COMMON && script!=SB_SCRIPT_INHERITED && script!=SB_SCRIPT_UNKNOWN;}
static bool compatible(SBScriptProperty p,uint32_t script){if(!p.count)return !specific(p.primary) || p.primary==script;for(size_t i=0;i<p.count;++i)if(p.extensions[i]==script)return true;return false;}
static uint32_t decode(const char *text,size_t *at) {
    const unsigned char *p=(const unsigned char *)text+*at;unsigned n=*p<0x80 ? 1 : *p<0xe0 ? 2 : *p<0xf0 ? 3 : 4;
    uint32_t cp=n==1 ? *p : *p&((1u<<(7-n))-1);for(unsigned i=1;i<n;++i)cp=(cp<<6)|(p[i]&63);*at+=n;return cp;
}
typedef struct {size_t byte,end;uint32_t cp,script,next;SBScriptProperty property;} ScriptCluster;
typedef struct {size_t key,previous;uint32_t script;} ScriptPair;
static size_t bracket_key(uint32_t cp){if(cp==0x232a)cp=0x3009;size_t lo=0,hi=COUNT(script_brackets);while(lo<hi){size_t mid=lo+(hi-lo)/2;if(script_brackets[mid].cp<cp)lo=mid+1;else hi=mid;}return lo<COUNT(script_brackets) && script_brackets[lo].cp==cp ? lo : COUNT(script_brackets);}
void sb_script_plan_free(SBScriptPlan *p){if(p){free(p->spans);memset(p,0,sizeof(*p));}}
SBStatus sb_script_plan(const char *text,size_t length,SBScriptPlan *out) {
    SBGrapheme reader;SBGraphemeBoundary b;
    if(!out || out->spans || out->count || !sb_grapheme_init(&reader,text,length))return sb_error(SB_INVALID,"Ungültiger Scriptabsatz.");
    ScriptCluster *clusters=NULL;size_t count=0,capacity=0,start=0;sb_grapheme_next(&reader,&b);
    while(sb_grapheme_next(&reader,&b)) {
        if(count==capacity){size_t next=capacity ? capacity*2 : 16;ScriptCluster *grown=realloc(clusters,next*sizeof(*grown));if(!grown){free(clusters);return sb_error(SB_MEMORY,"Scriptlayout benötigt mehr Speicher.");}clusters=grown;capacity=next;}
        size_t at=start;uint32_t cp=decode(text,&at);SBScriptProperty property=sb_script_property(cp);uint32_t script=specific(property.primary) ? property.primary : SB_SCRIPT_COMMON;
        while(at<b.byte){SBScriptProperty mark=sb_script_property(decode(text,&at));if(!specific(script) && specific(mark.primary))script=mark.primary;}
        clusters[count++]=(ScriptCluster){start,b.byte,cp,script,0,property};start=b.byte;
    }
    if(!count){free(clusters);return sb_ok();}
    uint32_t following=SB_SCRIPT_COMMON;
    for(size_t i=count;i>0;--i){if(sb_bidi_separator(clusters[i-1].cp))following=SB_SCRIPT_COMMON;clusters[i-1].next=following;if(specific(clusters[i-1].script))following=clusters[i-1].script;}
    ScriptPair *stack=malloc(count*sizeof(*stack));SBScriptSpan *spans=malloc(count*sizeof(*spans));
    if(!stack || !spans){free(stack);free(spans);free(clusters);return sb_error(SB_MEMORY,"Scriptlayout benötigt mehr Speicher.");}
    size_t depth=0,used=0,heads[COUNT(script_brackets)];for(size_t i=0;i<COUNT(heads);++i)heads[i]=SIZE_MAX;uint32_t previous=SB_SCRIPT_COMMON;
    for(size_t i=0;i<count;++i) {
        ScriptCluster c=clusters[i];uint32_t script=c.script;
        if(!specific(script) || c.property.count) {
            if(specific(previous) && compatible(c.property,previous))script=previous;
            else if(specific(c.next) && compatible(c.property,c.next))script=c.next;
            else if(!specific(script) && c.property.count)script=c.property.extensions[0];
        }
        /* Enclosing punctuation follows its enclosing script even in
           technical text outside its customary Script_Extensions usage. */
        if(c.property.bracket && specific(previous))script=previous;
        if(c.property.bracket==2){size_t key=bracket_key(c.cp);if(key<COUNT(heads) && heads[key]!=SIZE_MAX){size_t found=heads[key];script=stack[found].script;while(depth>found){ScriptPair popped=stack[--depth];heads[popped.key]=popped.previous;}}}
        if(c.property.bracket==1){size_t key=bracket_key(c.property.paired);if(key<COUNT(heads)){stack[depth]=(ScriptPair){key,heads[key],script};heads[key]=depth++;}}
        if(used && spans[used-1].script==script)spans[used-1].length+=c.end-c.byte;
        else spans[used++]=(SBScriptSpan){c.byte,c.end-c.byte,script};
        if(specific(script))previous=script;
        if(sb_bidi_separator(c.cp)){previous=SB_SCRIPT_COMMON;while(depth){ScriptPair popped=stack[--depth];heads[popped.key]=popped.previous;}}
    }
    free(stack);free(clusters);out->spans=spans;out->count=used;return sb_ok();
}
