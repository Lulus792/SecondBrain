#include "carets.h"
#include "ttf_shape.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
static size_t boundary_at(const uint32_t *a,size_t count,size_t byte){size_t lo=0,hi=count;while(lo<hi){size_t m=lo+(hi-lo)/2;if(a[m]<byte)lo=m+1;else hi=m;}return lo;}
static void *grow(void *old,size_t wanted,size_t *capacity,size_t size){
    if(wanted<=*capacity)return old;
    if(wanted>SIZE_MAX/size)return NULL;
    size_t next=*capacity ? *capacity : 16;
    while(next<wanted){if(next>SIZE_MAX/size/2){next=wanted;break;}next*=2;}
    void *p=realloc(old,next*size);if(p)*capacity=next;return p;
}
void sb_caret_plan_free(SBCaretPlan *p){if(p){free(p->logical);free(p->stops);free(p->clusters);memset(p,0,sizeof(*p));}}
void sb_caret_selection_free(SBSelectionPlan *p){if(p){free(p->spans);memset(p,0,sizeof(*p));}}
static bool add_stop(SBCaretPlan *p,size_t byte,float x,unsigned char level,unsigned char affinity){
    SBCaret *a=grow(p->stops,p->count+1,&p->capacity,sizeof(*a));if(!a)return false;p->stops=a;p->stops[p->count++]=(SBCaret){byte,x,level,affinity};return true;
}
static bool add_cluster(SBCaretPlan *p,size_t byte,size_t length,float a,float b,unsigned char level,unsigned char metric){
    SBCaretCluster *c=grow(p->clusters,p->cluster_count+1,&p->cluster_capacity,sizeof(*c));if(!c)return false;p->clusters=c;
    c[p->cluster_count++]=(SBCaretCluster){byte,length,fminf(a,b),fmaxf(a,b),level,metric};return true;
}
static int stop_compare(const void *left,const void *right){const SBCaret *a=left,*b=right;
    if(a->x!=b->x)return a->x<b->x ? -1 : 1;
    if(a->byte!=b->byte)return a->byte<b->byte ? -1 : 1;
    if(a->level!=b->level)return a->level<b->level ? -1 : 1;
    return (int)a->affinity-(int)b->affinity;
}
static int float_compare(const void *left,const void *right){float a=*(const float *)left,b=*(const float *)right;return a<b ? -1 : a>b;}
static int cluster_compare(const void *left,const void *right){const SBCaretCluster *a=left,*b=right;return a->byte<b->byte ? -1 : a->byte>b->byte;}
static int index_compare(const void *left,const void *right){const SBCaretIndex *a=left,*b=right;return a->byte<b->byte ? -1 : a->byte>b->byte ? 1 : a->stop<b->stop ? -1 : a->stop>b->stop;}
static bool index_stops(SBCaretPlan *plan){
    plan->logical=malloc(plan->count*sizeof(*plan->logical));if(!plan->logical)return false;
    for(size_t i=0;i<plan->count;++i)plan->logical[i]=(SBCaretIndex){plan->stops[i].byte,i};
    qsort(plan->logical,plan->count,sizeof(*plan->logical),index_compare);return true;
}
static size_t visual_at(const SBCaretPlan *plan,float x,bool after){size_t lo=0,hi=plan->count;while(lo<hi){size_t m=lo+(hi-lo)/2;if(plan->stops[m].x<x || (after && plan->stops[m].x==x))lo=m+1;else hi=m;}return lo;}
static SBStatus map_cluster(SBCaretPlan *plan,const uint32_t *boundaries,size_t count,
    size_t begin,size_t end,float left,float right,unsigned char level,const SBShapeGlyph *glyph,size_t glyphs){
    size_t first=boundary_at(boundaries,count,begin),last=boundary_at(boundaries,count,end);
    if(first>=count || last>=count || boundaries[first]!=begin || boundaries[last]!=end || last<=first)
        return sb_error(SB_INVALID,"Glyphencluster trennt ein Graphem.");
    size_t pieces=last-first;float *positions=NULL;size_t defined=0;
    if(pieces>1 && glyphs==1){
        if(!sb_ttf_ligature_carets(glyph->font,glyph->index,(level&1)!=0,NULL,0,&defined))return sb_error(SB_IO,"Ligaturpositionen: %s",SDL_GetError());
        if(defined==pieces-1){
            positions=malloc(defined*sizeof(*positions));if(!positions)return sb_error(SB_MEMORY,"Ligaturpositionen benötigen mehr Speicher.");
            if(!sb_ttf_ligature_carets(glyph->font,glyph->index,(level&1)!=0,positions,defined,&defined)){free(positions);return sb_error(SB_IO,"Ligaturpositionen: %s",SDL_GetError());}
            if(defined!=pieces-1){free(positions);positions=NULL;}
            else{for(size_t i=0;i<defined;++i)positions[i]+=glyph->x;
                qsort(positions,defined,sizeof(*positions),float_compare);
                for(size_t i=0;i<defined;++i)if(!isfinite(positions[i]) || positions[i]<=left || positions[i]>=right || (i && positions[i]<=positions[i-1])){free(positions);positions=NULL;break;}}
        }
    }
    float previous=level&1 ? right : left;
    bool ok=add_stop(plan,begin,previous,level,SB_CARET_BEFORE);
    for(size_t i=1;i<=pieces && ok;++i){
        float at;
        if(i==pieces)at=level&1 ? left : right;
        else if(positions)at=positions[level&1 ? pieces-i-1 : i-1];
        else at=level&1 ? right-(right-left)*(float)((double)i/pieces) : left+(right-left)*(float)((double)i/pieces);
        ok=add_cluster(plan,boundaries[first+i-1],boundaries[first+i]-boundaries[first+i-1],previous,at,level,pieces==1 ? SB_CARET_ADVANCE : positions ? SB_CARET_GDEF : SB_CARET_PROPORTIONAL) &&
            add_stop(plan,boundaries[first+i],at,level,i==pieces ? SB_CARET_AFTER : SB_CARET_BOTH);previous=at;
    }
    free(positions);return ok ? sb_ok() : sb_error(SB_MEMORY,"Cursorplan benötigt mehr Speicher.");
}
SBStatus sb_caret_plan(const SBShapeParagraph *paragraph,const SBShapedLine *line,SBCaretPlan *out){
    size_t count=0;const uint32_t *boundaries=sb_shape_paragraph_boundaries(paragraph,&count);
    const SBTextParagraph *bidi=sb_shape_paragraph_bidi(paragraph);size_t length=sb_bidi_paragraph_length(bidi);
    if(!bidi || !line || !out || out->logical || out->stops || out->count || out->capacity || out->clusters || out->cluster_count || out->cluster_capacity ||
       line->byte>length || line->length>length-line->byte || !isfinite(line->advance) || line->advance<0 ||
       (line->count && !line->glyphs) || (line->run_count && !line->runs))
        return sb_error(SB_INVALID,"Ungültige Cursorgeometrie.");
    if(!length && !line->length && !line->count && !line->run_count){SBCaretPlan empty={0};
        if(!add_stop(&empty,0,0,sb_bidi_paragraph_level(bidi),SB_CARET_BOTH))return sb_error(SB_MEMORY,"Leerer Cursorplan benötigt mehr Speicher.");
        if(!index_stops(&empty)){sb_caret_plan_free(&empty);return sb_error(SB_MEMORY,"Cursorindex benötigt mehr Speicher.");}
        *out=empty;return sb_ok();}
    if(!line->length || !line->run_count)return sb_error(SB_INVALID,"Unvollständige Cursorzeile.");
    SBCaretPlan plan={.byte=line->byte,.length=line->length};SBStatus status=sb_ok();size_t consumed=0;
    for(size_t r=0;r<line->run_count && status.code==SB_OK;++r){const SBShapeRun *run=&line->runs[r];
        if(!run->length || run->byte<line->byte || run->byte-line->byte>line->length || run->length>line->length-(run->byte-line->byte) ||
           run->glyph_begin!=consumed || run->glyph_count>line->count-consumed || !isfinite(run->x) || !isfinite(run->advance) || run->advance<0){status=sb_error(SB_INVALID,"Ungültiger Cursor-Schriftlauf.");break;}
        size_t end=run->byte+run->length,limit=run->glyph_begin+run->glyph_count;
        float pen=run->x;
        if(!run->glyph_count)status=map_cluster(&plan,boundaries,count,run->byte,end,pen,pen+run->advance,run->level,NULL,0);
        for(size_t at=run->glyph_begin;at<limit && status.code==SB_OK;){size_t next=at+1;
            const SBShapeGlyph *g=&line->glyphs[at];float advance=g->advance;
            while(next<limit && line->glyphs[next].byte==g->byte){advance+=line->glyphs[next].advance;++next;}
            if(g->byte<run->byte || g->byte>=end || !isfinite(g->x) || !isfinite(advance) || !isfinite(pen+advance) || advance<0 || !g->font){status=sb_error(SB_INVALID,"Ungültiger Cursor-Glyphencluster.");break;}
            size_t cluster_end=run->level&1 ? (at==run->glyph_begin ? end : line->glyphs[at-1].byte) : (next==limit ? end : line->glyphs[next].byte);
            status=map_cluster(&plan,boundaries,count,g->byte,cluster_end,pen,pen+advance,run->level,g,next-at);pen+=advance;at=next;
        }
        consumed=limit;
    }
    if(status.code==SB_OK && consumed!=line->count)status=sb_error(SB_INVALID,"Unvollständige Cursor-Glyphenläufe.");
    if(status.code==SB_OK){
        qsort(plan.stops,plan.count,sizeof(*plan.stops),stop_compare);size_t used=0;
        for(size_t i=0;i<plan.count;++i){SBCaret c=plan.stops[i];if(used && c.byte==plan.stops[used-1].byte && c.x==plan.stops[used-1].x && c.level==plan.stops[used-1].level)plan.stops[used-1].affinity|=c.affinity;else plan.stops[used++]=c;}plan.count=used;
        qsort(plan.clusters,plan.cluster_count,sizeof(*plan.clusters),cluster_compare);
        size_t byte=plan.byte;
        for(size_t i=0;i<plan.cluster_count;++i){if(plan.clusters[i].byte!=byte){status=sb_error(SB_INVALID,"Cursorcluster decken die Zeile nicht ab.");break;}byte+=plan.clusters[i].length;}
        if(status.code==SB_OK && byte!=line->byte+line->length)status=sb_error(SB_INVALID,"Unvollständige Cursorcluster.");
    }
    if(status.code==SB_OK && !index_stops(&plan))status=sb_error(SB_MEMORY,"Cursorindex benötigt mehr Speicher.");
    if(status.code!=SB_OK){sb_caret_plan_free(&plan);return status;}*out=plan;return sb_ok();
}
bool sb_caret_find(const SBCaretPlan *plan,size_t byte,SBCaretAffinity affinity,unsigned char base_level,SBCaret *out){
    if(!plan || !out || !plan->logical || affinity<SB_CARET_BEFORE || affinity>SB_CARET_BOTH)return false;
    size_t lo=0,hi=plan->count;while(lo<hi){size_t m=lo+(hi-lo)/2;if(plan->logical[m].byte<byte)lo=m+1;else hi=m;}
    const SBCaret *best=NULL;
    for(size_t i=lo;i<plan->count && plan->logical[i].byte==byte;++i){const SBCaret *c=&plan->stops[plan->logical[i].stop];if(!(c->affinity&affinity))continue;
        if(!best || ((c->level&1)==(base_level&1) && (best->level&1)!=(base_level&1)) || ((c->level&1)==(best->level&1) && c->level<best->level))best=c;}
    if(!best)return false;*out=*best;return true;
}
bool sb_caret_nearest(const SBCaretPlan *plan,float x,unsigned char base_level,SBCaret *out){
    if(!plan || !out || !plan->count || !isfinite(x))return false;
    size_t at=visual_at(plan,x,false);const SBCaret *best=NULL;float distance=INFINITY;
    size_t choices[2]={at<plan->count ? at : plan->count-1,at ? at-1 : 0};
    for(unsigned side=0;side<2;++side){float px=plan->stops[choices[side]].x;size_t begin=visual_at(plan,px,false),end=visual_at(plan,px,true);
        for(size_t i=begin;i<end;++i){const SBCaret *c=&plan->stops[i];float d=fabsf(c->x-x);
            if(!best || d<distance || (d==distance && (c->level&1)==(base_level&1) && (best->level&1)!=(base_level&1))){best=c;distance=d;}}}
    *out=*best;return true;
}
bool sb_caret_step(const SBCaretPlan *plan,const SBCaret *from,int direction,SBCaret *out){
    if(!plan || !from || !out || !plan->logical || (direction!=-1 && direction!=1))return false;
    size_t lo=0,hi=plan->count;while(lo<hi){size_t m=lo+(hi-lo)/2;if(plan->logical[m].byte<from->byte)lo=m+1;else hi=m;}
    bool found=false;for(size_t i=lo;i<plan->count && plan->logical[i].byte==from->byte;++i){const SBCaret *c=&plan->stops[plan->logical[i].stop];if(c->x==from->x && c->level==from->level && (c->affinity&from->affinity)){found=true;break;}}
    if(!found)return false;
    /* One press moves to a different visual position, skipping zero advances. */
    size_t next=visual_at(plan,from->x,direction>0);
    if(direction>0){if(next==plan->count)return false;*out=plan->stops[next];}
    else{if(!next)return false;*out=plan->stops[next-1];}
    return true;
}
SBStatus sb_caret_selection(const SBCaretPlan *plan,size_t begin,size_t end,SBSelectionPlan *out){
    if(!plan || !out || out->spans || out->count || begin>end || begin<plan->byte || end-plan->byte>plan->length)return sb_error(SB_INVALID,"Ungültige visuelle Auswahl.");
    SBCaret a,b;if(!sb_caret_find(plan,begin,SB_CARET_BOTH,0,&a) || !sb_caret_find(plan,end,SB_CARET_BOTH,0,&b))return sb_error(SB_INVALID,"Auswahl trennt ein Graphem.");
    if(begin==end)return sb_ok();
    size_t first=0,last=plan->cluster_count;
    while(first<last){size_t m=first+(last-first)/2;if(plan->clusters[m].byte<begin)first=m+1;else last=m;}
    size_t limit=first;last=plan->cluster_count;
    while(limit<last){size_t m=limit+(last-limit)/2;if(plan->clusters[m].byte<end)limit=m+1;else last=m;}
    SBSelectionSpan *spans=malloc((limit-first)*sizeof(*spans));if(!spans)return sb_error(SB_MEMORY,"Auswahlplan benötigt mehr Speicher.");
    size_t used=0;
    for(size_t i=first;i<limit;++i){const SBCaretCluster *c=&plan->clusters[i];if(c->byte>=begin && c->byte+c->length<=end)spans[used++]=(SBSelectionSpan){c->left,c->right-c->left,c->byte,c->length,c->level};}
    *out=(SBSelectionPlan){spans,used};return sb_ok();
}
