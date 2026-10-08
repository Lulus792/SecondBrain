#include "text.h"
#include "grapheme.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"WRAP %d: %s (%s)\n",__LINE__,#x,SDL_GetError());return 1;}}while(0)
#define TAG(a,b,c,d) (((uint32_t)(a)<<24)|((uint32_t)(b)<<16)|((uint32_t)(c)<<8)|(uint32_t)(d))
typedef struct {SBShapedLine shape;size_t offset;float y;} RefLine;
typedef struct {RefLine lines[256];size_t count;float height;} RefPlan;
static bool blank(const char *s,size_t a,size_t b){return b==a+1 && (s[a]==' ' || s[a]=='\t');}
static void release(RefPlan *p){for(size_t i=0;i<p->count;++i)sb_shape_line_free(&p->lines[i].shape);memset(p,0,sizeof(*p));}
static bool append(RefPlan *p,SBShapedLine *line,size_t offset,const struct nk_user_font *font,float density,float gap){
    if(p->count==256)return false;p->lines[p->count++]=(RefLine){*line,offset,p->height};
    p->height+=fmaxf(font->height,(line->ascent+line->descent)/density)+gap;memset(line,0,sizeof(*line));return true;
}
/* Oracle tries EVERY grapheme endpoint against the original paragraph. It has
   no production wrap estimates, cached prefixes, script resolver or layout calls. */
static bool reference(SBUi *ui,const struct nk_user_font *font,const char *s,size_t length,unsigned kind,float width,RefPlan *p){
    size_t offset=0;struct nk_vec2 pad=ui->ctx->style.text.padding;bool trailing=false;
    while(offset<length){SBTextParagraph *bidi=NULL;if(sb_bidi_paragraph_create(s+offset,length-offset,SB_BIDI_AUTO_LTR,&bidi).code!=SB_OK)return false;
        size_t consumed=sb_bidi_paragraph_length(bidi),visible=consumed;const char *at=s+offset,*end=at+consumed;trailing=false;
        while(at<end){size_t start=(size_t)(at-(s+offset));uint32_t cp=SDL_StepUTF8(&at,NULL);if(sb_bidi_separator(cp)){visible=start;trailing=true;break;}}
        TTF_Font *normal=sb_ui_cluster_font(font,"A",1,NULL),*other=kind==1 ? sb_ui_cluster_font(font,"א",2,NULL) : kind==2 ? sb_ui_cluster_font(font,"ب",2,NULL) : kind==3 ? sb_ui_cluster_font(font,"👩‍💻",11,NULL) : normal;
        uint32_t script=kind==2 ? TAG('A','r','a','b') : kind==3 ? TAG('Z','y','y','y') : TAG('L','a','t','n');
        SBShapeFontSpan spans[2]={{0,consumed,other,script}};size_t span_count=1;
        if(kind==1 && offset==0){spans[0]=(SBShapeFontSpan){0,7,other,TAG('H','e','b','r')};spans[1]=(SBShapeFontSpan){7,consumed-7,normal,TAG('L','a','t','n')};span_count=2;}
        size_t boundaries[1024],count=0;SBGrapheme g;SBGraphemeBoundary b;
        if(!sb_grapheme_init(&g,s+offset,visible)){sb_bidi_paragraph_free(bidi);return false;}
        while(sb_grapheme_next(&g,&b)){if(count==1024){sb_bidi_paragraph_free(bidi);return false;}boundaries[count++]=b.byte;}
        size_t begin=0,terminal=count ? count-1 : 0;float available=fmaxf(1,width-2*pad.x);
        if(!terminal){SBShapedLine empty={.base_level=sb_bidi_paragraph_level(bidi)};if(!append(p,&empty,offset,font,ui->density,2*pad.y)){sb_bidi_paragraph_free(bidi);return false;}}
        while(begin<terminal){size_t best=begin+1;
            for(size_t candidate=begin+1;candidate<=terminal;++candidate){SBShapedLine line={0};bool ok=sb_shape_line(bidi,boundaries[begin],boundaries[candidate]-boundaries[begin],spans,span_count,&line).code==SB_OK;
                bool fits=ok && line.advance/ui->density<=available;sb_shape_line_free(&line);if(!ok){sb_bidi_paragraph_free(bidi);return false;}if(fits)best=candidate;}
            if(best<terminal && best>begin+1){size_t word=best;while(word>begin && !blank(s+offset,boundaries[word-1],boundaries[word]))--word;if(word>begin)best=word;}
            size_t ink=best;while(ink>begin+1 && blank(s+offset,boundaries[ink-1],boundaries[ink]))--ink;
            SBShapedLine line={0};bool ok=sb_shape_line(bidi,boundaries[begin],boundaries[ink]-boundaries[begin],spans,span_count,&line).code==SB_OK && append(p,&line,offset,font,ui->density,2*pad.y);
            sb_shape_line_free(&line);if(!ok){sb_bidi_paragraph_free(bidi);return false;}begin=best;
            while(begin<terminal && blank(s+offset,boundaries[begin],boundaries[begin+1]))++begin;
        }
        sb_bidi_paragraph_free(bidi);offset+=consumed;
    }
    if(!length || trailing){SBShapedLine empty={0};if(!append(p,&empty,length,font,ui->density,2*pad.y))return false;}
    p->height+=2*pad.y;return true;
}
typedef struct {const RefPlan *p;size_t index;bool valid,rtl_continuation;} Probe;
static bool inspect(void *user,const SBShapedLine *line,size_t offset,float y){
    Probe *p=user;if(p->index>=p->p->count){p->valid=false;return false;}const RefLine *expected=&p->p->lines[p->index++];
    if(expected->offset!=offset || expected->y!=y || expected->shape.byte!=line->byte || expected->shape.length!=line->length || expected->shape.base_level!=line->base_level || expected->shape.count!=line->count || expected->shape.advance!=line->advance){p->valid=false;return false;}
    for(size_t i=0;i<line->count;++i){SBShapeGlyph a=line->glyphs[i],b=expected->shape.glyphs[i];if(a.byte!=b.byte || a.font!=b.font || a.index!=b.index || a.x!=b.x || a.y!=b.y || a.advance!=b.advance || a.level!=b.level){p->valid=false;return false;}}
    if(line->byte>7 && line->base_level==1)p->rtl_continuation=true;return true;
}
static uint64_t pixels(SBUi *ui){SDL_Surface *raw=SDL_RenderReadPixels(ui->renderer,NULL),*s=raw ? SDL_ConvertSurface(raw,SDL_PIXELFORMAT_RGBA32) : NULL;uint64_t hash=0;if(s)for(int y=0;y<s->h;++y)hash=hash*1099511628211ULL^sb_hash((char *)s->pixels+y*s->pitch,(size_t)s->w*4);SDL_DestroySurface(s);SDL_DestroySurface(raw);return hash;}
static uint64_t paint(SBUi *ui,const struct nk_user_font *font,const char *s,size_t length,const RefPlan *reference,float width,float height,unsigned mode,bool expected){
    nk_input_begin(ui->ctx);nk_input_end(ui->ctx);nk_begin(ui->ctx,"Wrap proof",nk_rect(0,0,1000,900),NK_WINDOW_NO_SCROLLBAR);nk_style_set_font(ui->ctx,font);nk_layout_row_static(ui->ctx,height,(int)width,1);
    nk_flags align=mode<2 ? NK_TEXT_LEFT : mode==2 ? NK_TEXT_CENTERED : NK_TEXT_RIGHT;
    if(expected){struct nk_rect r;nk_widget(&r,ui->ctx);struct nk_command_buffer *canvas=nk_window_get_canvas(ui->ctx);struct nk_rect old=canvas->clip;float x=fmaxf(old.x,r.x),y=fmaxf(old.y,r.y);nk_push_scissor(canvas,nk_rect(x,y,fminf(old.x+old.w,r.x+r.w)-x,fminf(old.y+old.h,r.y+r.h)-y));struct nk_vec2 pad=ui->ctx->style.text.padding;
        for(size_t i=0;i<reference->count;++i){const RefLine *line=&reference->lines[i];float available=fmaxf(1,r.w-2*pad.x),shift=mode==2 ? fmaxf(0,(available-line->shape.advance/ui->density)/2) : mode==3 ? fmaxf(0,available-line->shape.advance/ui->density) : 0;
            sb_ui_shaped_draw(ui,&line->shape,r.x+pad.x+shift,r.y+pad.y+line->y+line->shape.ascent/ui->density,ui->ctx->style.text.color);}
        nk_push_scissor(canvas,old);
    }else if(mode==0)nk_text_wrap(ui->ctx,s,(int)length);else sb_ui_text_aligned(ui->ctx,s,length,align);
    nk_end(ui->ctx);sb_ui_draw(ui);uint64_t hash=pixels(ui);SDL_RenderPresent(ui->renderer);return hash;
}
int main(int argc,char **argv){CHECK(argc==3);SBUi ui;CHECK(sb_ui_init(&ui,argv[1],1000,900,true).code==SB_OK);ui.space.dark=true;
    const char *samples[]={"AV office affinity é office affinity é", "אבג ab cd ef gh ij kl mn op qr st uv wx yz", "ببببسلامببببسلامببببسلام", "👩‍💻👩‍💻👩‍💻", "AV ffi\r\n\r\né\n", ""};
    for(unsigned size=0;size<3;++size){CHECK(sb_ui_fonts(&ui,1+0.5f*size).code==SB_OK);ui.ctx->style.text.padding=nk_vec2(5.25f*ui.scale,3.5f*ui.scale);
        for(unsigned sample=0;sample<6;++sample){const struct nk_user_font *font=sample==4 ? &ui.code->handle : &ui.body->handle;size_t length=strlen(samples[sample]);
            for(unsigned w=0;w<2;++w){float width=w ? 260 : sample==3 ? 12 : 95;RefPlan reference_plan={0};CHECK(reference(&ui,font,samples[sample],length,sample<4 ? sample : 0,width,&reference_plan));
                float height=sb_ui_wrap_height(ui.ctx,font,samples[sample],length,width);CHECK(fabsf(height-reference_plan.height)<0.001f);
                /* Plain and rich text share bytes/fonts but have distinct gap
                   and terminal-line policies; their cache entries must differ. */
                SBTextSpan span={0,length,0};SBStyledText rich={.text=(char *)samples[sample],.spans=length ? &span : NULL,.count=length ? 1 : 0};
                size_t rich_count=reference_plan.count;const char *last=samples[sample]+length;
                if(length && sb_bidi_separator(SDL_StepBackUTF8(samples[sample],&last)))--rich_count;
                float rich_height=2*ui.ctx->style.text.padding.y;
                for(size_t i=0;i<rich_count;++i)rich_height+=fmaxf(font->height,(reference_plan.lines[i].shape.ascent+reference_plan.lines[i].shape.descent)/ui.density)+4*ui.scale;
                CHECK(fabsf(sb_ui_styled_height(&ui,font,&rich,width)-rich_height)<0.001f);
                CHECK(sb_ui_wrap_height(ui.ctx,font,samples[sample],length,width)==height);
                Probe probe={.p=&reference_plan,.valid=true};CHECK(sb_ui_wrap_geometry(&ui,font,samples[sample],length,width,inspect,&probe));CHECK(probe.valid && probe.index==reference_plan.count);
                if(sample==1 && !w)CHECK(probe.rtl_continuation);
                for(unsigned mode=0;mode<4;++mode){uint64_t expected=paint(&ui,font,samples[sample],length,&reference_plan,width,height,mode,true),actual=paint(&ui,font,samples[sample],length,&reference_plan,width,height,mode,false);CHECK(expected && expected==actual);CHECK(paint(&ui,font,samples[sample],length,&reference_plan,width,height,mode,false)==actual);}
                ui.styled_cache_disabled=true;Probe fresh={.p=&reference_plan,.valid=true};CHECK(sb_ui_wrap_geometry(&ui,font,samples[sample],length,width,inspect,&fresh));CHECK(fresh.valid && fresh.index==reference_plan.count);ui.styled_cache_disabled=false;
                if(sample==1 && !w && size==2)CHECK(sb_ui_capture(&ui,argv[2]).code==SB_OK);
                /* An intentionally short widget must clip ink at its own bounds. */
                CHECK(paint(&ui,font,samples[sample],length,&reference_plan,width,font->height,0,true)==paint(&ui,font,samples[sample],length,&reference_plan,width,font->height,0,false));
                release(&reference_plan);
            }
        }
    }
    /* No terminating NUL may be read from a raw source slice. */
    char slice[6]={'A','V',' ','f','f','i'};RefPlan raw={0};CHECK(reference(&ui,&ui.code->handle,slice,sizeof(slice),0,100,&raw));CHECK(sb_ui_wrap_height(ui.ctx,&ui.code->handle,slice,sizeof(slice),100)==raw.height);release(&raw);
    sb_ui_shutdown(&ui);printf("%u wrapped text paragraph/glyph/height/pixel/cache assertions passed.\n",checks);return 0;
}
