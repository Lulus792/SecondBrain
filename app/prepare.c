#include "prepare.h"
#include "scripts.h"
#include "grapheme.h"
#include <stdlib.h>
#include <string.h>
struct SBPrepareJob {
    SBFontSnapshot *font;char *source;size_t length;uint64_t context;
    SDL_Thread *thread;SDL_AtomicInt done,cancel;SBStatus status;
    SBShapedLine line;uint8_t *tokens;bool taken;
};
static int prepare_worker(void *data){
    SBPrepareJob *j=data;SBFontInstance *fonts=NULL;SBTextParagraph *bidi=NULL;SBShapeParagraph *prepared=NULL;SBScriptPlan scripts={0};SBShapeFontSpan *spans=NULL;size_t count=0,capacity=0;
    if(SDL_GetAtomicInt(&j->cancel)){SDL_SetAtomicInt(&j->done,1);return 0;}
    if(!sb_utf8_valid(j->source,j->length)){j->status=sb_error(SB_INVALID,"Ungültiger vorbereiteter Text.");SDL_SetAtomicInt(&j->done,1);return 0;}
    j->status=sb_font_snapshot_open(j->font,&fonts);
    if(j->status.code==SB_OK && !SDL_GetAtomicInt(&j->cancel))j->status=sb_bidi_paragraph_create(j->source,j->length,SB_BIDI_AUTO_LTR,&bidi);
    if(j->status.code==SB_OK && !SDL_GetAtomicInt(&j->cancel))j->status=sb_script_plan(j->source,j->length,&scripts);
    if(j->status.code==SB_OK && !SDL_GetAtomicInt(&j->cancel)){
        SBGrapheme reader;SBGraphemeBoundary boundary;sb_grapheme_init(&reader,j->source,j->length);sb_grapheme_next(&reader,&boundary);size_t byte=0,script=0;TTF_Font *last=NULL;
        while(sb_grapheme_next(&reader,&boundary)){
            if(SDL_GetAtomicInt(&j->cancel))break;
            while(script+1<scripts.count && scripts.spans[script].byte+scripts.spans[script].length<=byte)++script;
            TTF_Font *font=sb_font_instance_select(fonts,j->source+byte,boundary.byte-byte,last);uint32_t tag=scripts.count ? scripts.spans[script].script : SB_SCRIPT_COMMON;
            if(!font){j->status=sb_error(SB_IO,"Schrift konnte nicht gewählt werden.");break;}
            if(count && spans[count-1].font==font && spans[count-1].script==tag)spans[count-1].length+=boundary.byte-byte;
            else{if(count==capacity){size_t next=capacity ? capacity*2 : 16;SBShapeFontSpan *grown=realloc(spans,next*sizeof(*grown));if(!grown){j->status=sb_error(SB_MEMORY,"Kein Speicher für vorbereitete Schriftbereiche.");break;}spans=grown;capacity=next;}spans[count++]=(SBShapeFontSpan){byte,boundary.byte-byte,font,tag};}
            byte=boundary.byte;last=font;
        }
    }
    if(j->status.code==SB_OK && !SDL_GetAtomicInt(&j->cancel))j->status=sb_shape_paragraph_create(bidi,spans,count,&prepared);
    if(j->status.code==SB_OK && !SDL_GetAtomicInt(&j->cancel))j->status=sb_shape_paragraph_line(prepared,0,j->length,&j->line);
    if(j->status.code==SB_OK && !SDL_GetAtomicInt(&j->cancel)){
        j->tokens=j->line.count ? malloc(j->line.count) : NULL;
        if(j->line.count && !j->tokens)j->status=sb_error(SB_MEMORY,"Kein Speicher für vorbereitete Glyphen.");
        else for(size_t i=0;i<j->line.count;++i){unsigned token=sb_font_instance_token(fonts,j->line.glyphs[i].font);if(token>=8){j->status=sb_error(SB_INVALID,"Ungültige vorbereitete Schriftkennung.");break;}j->tokens[i]=(uint8_t)token;}
    }
    /* No worker font address escapes its lifetime. Binding happens on UI. */
    for(size_t i=0;i<j->line.count;++i)j->line.glyphs[i].font=NULL;
    sb_shape_paragraph_free(prepared);sb_bidi_paragraph_free(bidi);sb_script_plan_free(&scripts);free(spans);sb_font_instance_free(fonts);
    SDL_SetAtomicInt(&j->done,1);return 0;
}
SBPrepareJob *sb_prepare_start(SBFontSnapshot *font,const char *source,size_t length,uint64_t context){
    if(!font || !source || length>SB_TEXT_LIMIT)return NULL;
    SBPrepareJob *j=calloc(1,sizeof(*j));if(!j)return NULL;j->source=malloc(length+1);if(!j->source){free(j);return NULL;}
    memcpy(j->source,source,length);j->source[length]=0;j->length=length;j->context=context;j->font=font;j->status=sb_ok();
    j->thread=SDL_CreateThread(prepare_worker,"SecondBrain text",j);if(!j->thread){free(j->source);free(j);return NULL;}return j;
}
SBPrepareState sb_prepare_state(SBPrepareJob *j,SBStatus *status){
    if(!j){if(status)*status=sb_error(SB_INVALID,"Vorbereitung fehlt.");return SB_PREPARE_FAILED;}
    if(!SDL_GetAtomicInt(&j->done)){if(status)*status=sb_ok();return SB_PREPARE_PENDING;}
    if(status)*status=j->status;
    if(SDL_GetAtomicInt(&j->cancel))return SB_PREPARE_CANCELLED;
    return j->status.code==SB_OK ? SB_PREPARE_READY : SB_PREPARE_FAILED;
}
void sb_prepare_cancel(SBPrepareJob *j){if(j)SDL_SetAtomicInt(&j->cancel,1);}
SBStatus sb_prepare_take(SBPrepareJob *j,SBUi *ui,const char *source,size_t length,uint64_t context,SBShapedLine *out){
    if(!j || !ui || !out || out->glyphs || out->runs || out->count || out->run_count || out->capacity || out->run_capacity || j->taken)return sb_error(SB_INVALID,"Ungültige Vorbereitungsübernahme.");
    if(sb_prepare_state(j,NULL)!=SB_PREPARE_READY)return sb_error(SB_INVALID,"Vorbereitung ist nicht bereit.");
    if(!source || length!=j->length || context!=j->context || memcmp(source,j->source,length))return sb_error(SB_CONFLICT,"Vorbereiteter Inhalt ist nicht mehr aktuell.");
    TTF_Font *fonts[8];for(unsigned i=0;i<8;++i){fonts[i]=sb_font_snapshot_bind(j->font,ui,i);if(!fonts[i])return sb_error(SB_CONFLICT,"Vorbereitete Schrift ist nicht mehr aktuell.");}
    for(size_t i=0;i<j->line.count;++i)j->line.glyphs[i].font=fonts[j->tokens[i]];
    *out=j->line;j->line=(SBShapedLine){0};j->taken=true;return sb_ok();
}
void sb_prepare_free(SBPrepareJob *j){if(!j)return;sb_prepare_cancel(j);SDL_WaitThread(j->thread,NULL);sb_shape_line_free(&j->line);free(j->tokens);free(j->source);sb_font_snapshot_free(j->font);free(j);}
