#include "prepare.h"
#include "scripts.h"
#include "grapheme.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
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

struct SBPrepareLayout {
    SBTextSnapshot *fonts;SBStyledText text;size_t length;unsigned role;float width,gap;struct nk_vec2 padding;bool terminal_line,taken;uint64_t context;
    SDL_Thread *thread;SDL_AtomicInt done,cancel;SBStatus status;SBStyledPlan plan;uint8_t **tokens;size_t token_count;
};
static const struct nk_user_font *layout_font(SBUi *ui,unsigned role){return role==0 ? &ui->normal->handle : role==1 ? &ui->body->handle : role==2 ? &ui->heading->handle : &ui->code->handle;}
static int prepare_layout_worker(void *data){
    SBPrepareLayout *j=data;SBUi cpu={0};j->status=sb_ok();
    if(!SDL_GetAtomicInt(&j->cancel))j->status=sb_text_snapshot_open(j->fonts,&cpu);
    cpu.layout_cancel=&j->cancel;
    if(j->status.code==SB_OK && !SDL_GetAtomicInt(&j->cancel) && !sb_ui_styled_plan(&cpu,layout_font(&cpu,j->role),&j->text,j->width,j->padding,j->gap,j->terminal_line,&j->plan))j->status=sb_error(SB_INVALID,"Textlayout konnte nicht vorbereitet werden.");
    if(j->status.code==SB_OK && !SDL_GetAtomicInt(&j->cancel)){
        j->tokens=j->plan.count ? calloc(j->plan.count,sizeof(*j->tokens)) : NULL;
        if(j->plan.count && !j->tokens)j->status=sb_error(SB_MEMORY,"Kein Speicher für Layoutkennungen.");
        else for(size_t i=0;i<j->plan.count && j->status.code==SB_OK;++i){SBShapedLine *line=&j->plan.lines[i].shape;j->tokens[i]=line->count ? malloc(line->count) : NULL;j->token_count=i+1;
            if(line->count && !j->tokens[i]){j->status=sb_error(SB_MEMORY,"Kein Speicher für Glyphenkennungen.");break;}
            for(size_t g=0;g<line->count;++g){unsigned token=sb_ui_font_token(&cpu,line->glyphs[g].font);if(token>UINT8_MAX){j->status=sb_error(SB_INVALID,"Ungültige Layoutschrift.");break;}j->tokens[i][g]=(uint8_t)token;}}
    }
    for(size_t i=0;i<j->plan.count;++i)for(size_t g=0;g<j->plan.lines[i].shape.count;++g)j->plan.lines[i].shape.glyphs[g].font=NULL;
    sb_ui_text_free(&cpu);SDL_SetAtomicInt(&j->done,1);return 0;
}
SBPrepareLayout *sb_prepare_layout_start(SBTextSnapshot *fonts,const SBStyledText *text,unsigned role,float width,struct nk_vec2 padding,float gap,bool terminal_line,uint64_t context){
    if(!fonts || !text || !text->text || role>=4 || text->count>SB_INLINE_LIMIT || !isfinite(width) || width<=0 || !isfinite(padding.x) || !isfinite(padding.y) || padding.x<0 || padding.y<0 || !isfinite(gap) || gap<0)return NULL;
    size_t length=strlen(text->text);if(length>SB_TEXT_LIMIT || (text->count && !text->spans))return NULL;
    SBPrepareLayout *j=calloc(1,sizeof(*j));if(!j)return NULL;j->text.text=malloc(length+1);j->text.spans=text->count ? malloc(text->count*sizeof(*text->spans)) : NULL;
    if(!j->text.text || (text->count && !j->text.spans)){sb_styled_free(&j->text);free(j);return NULL;}
    memcpy(j->text.text,text->text,length+1);j->text.count=text->count;if(text->count)memcpy(j->text.spans,text->spans,text->count*sizeof(*text->spans));
    j->length=length;j->fonts=fonts;j->role=role;j->width=width;j->padding=padding;j->gap=gap;j->terminal_line=terminal_line;j->context=context;
    j->thread=SDL_CreateThread(prepare_layout_worker,"SecondBrain layout",j);if(!j->thread){sb_styled_free(&j->text);free(j);return NULL;}return j;
}
SBPrepareState sb_prepare_layout_state(SBPrepareLayout *j,SBStatus *status){
    if(!j){if(status)*status=sb_error(SB_INVALID,"Layoutvorbereitung fehlt.");return SB_PREPARE_FAILED;}
    if(!SDL_GetAtomicInt(&j->done)){if(status)*status=sb_ok();return SB_PREPARE_PENDING;}
    if(status)*status=j->status;if(SDL_GetAtomicInt(&j->cancel))return SB_PREPARE_CANCELLED;
    return j->status.code==SB_OK ? SB_PREPARE_READY : SB_PREPARE_FAILED;
}
void sb_prepare_layout_cancel(SBPrepareLayout *j){if(j)SDL_SetAtomicInt(&j->cancel,1);}
SBStatus sb_prepare_layout_take(SBPrepareLayout *j,SBUi *ui,const SBStyledText *text,unsigned role,float width,struct nk_vec2 padding,float gap,bool terminal_line,uint64_t context,SBStyledPlan *out){
    if(!j || !ui || !out || out->lines || out->count || out->capacity || j->taken || sb_prepare_layout_state(j,NULL)!=SB_PREPARE_READY)return sb_error(SB_INVALID,"Layout ist nicht übernehmbar.");
    if(!text || !text->text || role!=j->role || width!=j->width || padding.x!=j->padding.x || padding.y!=j->padding.y || gap!=j->gap || terminal_line!=j->terminal_line || context!=j->context || text->count!=j->text.count || strlen(text->text)!=j->length || memcmp(text->text,j->text.text,j->length))return sb_error(SB_CONFLICT,"Layout gehört nicht mehr zur aktuellen Ansicht.");
    if(text->count && !text->spans)return sb_error(SB_INVALID,"Textstile fehlen.");
    for(size_t i=0;i<text->count;++i){SBTextSpan a=text->spans[i],b=j->text.spans[i];if(a.offset!=b.offset || a.length!=b.length || a.style!=b.style)return sb_error(SB_CONFLICT,"Textstile haben sich geändert.");}
    TTF_Font *fonts[256]={0};for(size_t i=0;i<j->plan.count;++i)for(size_t g=0;g<j->plan.lines[i].shape.count;++g){unsigned token=j->tokens[i][g];if(!fonts[token]){fonts[token]=sb_text_snapshot_bind(j->fonts,ui,token);if(!fonts[token])return sb_error(SB_CONFLICT,"Layoutschrift ist nicht mehr aktuell.");}}
    /* Empty layouts must also validate the snapshot generation. */
    if(!sb_text_snapshot_bind(j->fonts,ui,j->role*64))return sb_error(SB_CONFLICT,"Layoutschrift ist nicht mehr aktuell.");
    for(size_t i=0;i<j->plan.count;++i)for(size_t g=0;g<j->plan.lines[i].shape.count;++g)j->plan.lines[i].shape.glyphs[g].font=fonts[j->tokens[i][g]];
    *out=j->plan;j->plan=(SBStyledPlan){0};j->taken=true;return sb_ok();
}
void sb_prepare_layout_free(SBPrepareLayout *j){if(!j)return;sb_prepare_layout_cancel(j);SDL_WaitThread(j->thread,NULL);if(j->tokens){for(size_t i=0;i<j->token_count;++i)free(j->tokens[i]);free(j->tokens);}sb_ui_styled_plan_free(&j->plan);sb_styled_free(&j->text);sb_text_snapshot_free(j->fonts);free(j);}
