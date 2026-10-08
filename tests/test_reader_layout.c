#include "text.h"
#include "platform.h"
#include "grapheme.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"READER %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define TAG(a,b,c,d) (((uint32_t)(a)<<24)|((uint32_t)(b)<<16)|((uint32_t)(c)<<8)|(uint32_t)(d))
static uint64_t pixels(SBUi *ui) {
    SDL_Surface *raw=SDL_RenderReadPixels(ui->renderer,NULL),*rgba=raw ? SDL_ConvertSurface(raw,SDL_PIXELFORMAT_RGBA32) : NULL;uint64_t hash=0;
    if(rgba)for(int y=0;y<rgba->h;++y)hash=hash*1099511628211ULL^sb_hash((char *)rgba->pixels+y*rgba->pitch,(size_t)rgba->w*4);
    SDL_DestroySurface(rgba);SDL_DestroySurface(raw);return hash;
}
static uint64_t paint(SBUi *ui,const SBStyledText *styled,const SBShapedLine *reference,float height,nk_flags alignment) {
    nk_input_begin(ui->ctx);nk_input_end(ui->ctx);
    if(nk_begin(ui->ctx,"Reader proof",nk_rect(0,0,1000,900),NK_WINDOW_NO_SCROLLBAR)) {
        nk_style_set_font(ui->ctx,&ui->body->handle);nk_layout_row_static(ui->ctx,height,800,1);
        if(reference){struct nk_rect bounds;nk_widget(&bounds,ui->ctx);struct nk_vec2 pad=ui->ctx->style.text.padding;float free_width=bounds.w-2*pad.x-reference->advance/ui->density;
            float shift=alignment&NK_TEXT_ALIGN_RIGHT ? fmaxf(0,free_width) : alignment&NK_TEXT_ALIGN_CENTERED ? fmaxf(0,free_width/2) : 0;
            sb_ui_shaped_draw(ui,reference,bounds.x+pad.x+shift,bounds.y+pad.y+reference->ascent/ui->density,ui->ctx->style.text.color);
        }else sb_ui_styled_aligned(ui,&ui->body->handle,styled,alignment);
    }nk_end(ui->ctx);sb_ui_draw(ui);uint64_t hash=pixels(ui);SDL_RenderPresent(ui->renderer);return hash;
}
typedef struct {SBTextParagraph *paragraph;SBShapeFontSpan font;float density,available;size_t next;bool valid;unsigned lines;} WrapProbe;
static bool check_wrap(void *user,const SBShapedLine *line,size_t offset,float y){
    WrapProbe *p=user;(void)y;
    if(offset || line->byte!=p->next || line->base_level!=sb_bidi_paragraph_level(p->paragraph)){p->valid=false;return false;}
    /* Independent exhaustive candidates, including ligature/joining changes
       at each possible grapheme boundary, rather than a wrap-estimate oracle. */
    SBGrapheme reader;SBGraphemeBoundary b;const char *text=sb_bidi_paragraph_text(p->paragraph);size_t length=sb_bidi_paragraph_length(p->paragraph),best=0,first=0;
    if(!sb_grapheme_init(&reader,text,length)){p->valid=false;return false;}
    while(sb_grapheme_next(&reader,&b))if(b.byte>p->next){SBShapedLine candidate={0};if(!first)first=b.byte;
        if(sb_shape_line(p->paragraph,p->next,b.byte-p->next,&p->font,1,&candidate).code!=SB_OK){p->valid=false;return false;}
        if(candidate.advance/p->density<=p->available)best=b.byte;sb_shape_line_free(&candidate);}
    if(!best)best=first;
    if(line->byte+line->length!=best){p->valid=false;return false;}
    p->next=best;++p->lines;return true;
}
int main(int argc,char **argv) {
    CHECK(argc==3);SBUi ui;CHECK(sb_ui_init(&ui,argv[1],1000,900,true).code==SB_OK);ui.space.dark=true;
    /* Nuklear must preserve fractional and large image coordinates; truncating
       them makes reader textures jump while the scroll offset moves smoothly. */
    struct nk_buffer commands;struct nk_command_buffer canvas;
    nk_buffer_init_default(&commands);canvas=(struct nk_command_buffer){.base=&commands,.use_clipping=NK_CLIPPING_OFF};
    struct nk_image image=nk_image_id(1);struct nk_rect rect=nk_rect(-40000.25f,21.75f,35.5f,18.25f);
    nk_draw_image(&canvas,rect,&image,nk_rgba(255,255,255,255));
    const struct nk_command_image *cmd=nk_buffer_memory(&commands);
    CHECK(cmd && cmd->x==rect.x && cmd->y==rect.y && cmd->w==rect.w && cmd->h==rect.h);
    nk_buffer_free(&commands);
    for(unsigned size=0;size<3;++size){CHECK(sb_ui_fonts(&ui,1+size*0.5f).code==SB_OK);
        const char *source="AB **ب**بب CD";SBInline reader;SBStyledText styled;
        CHECK(sb_inline_init(&reader,source,strlen(source)).code==SB_OK);CHECK(sb_inline_styled(&reader,0,strlen(source),&styled).code==SB_OK);
        CHECK(!strcmp(styled.text,"AB ببب CD"));
        const struct nk_user_font *base=&ui.body->handle,*bold=sb_ui_text_style(&ui,base,SB_TEXT_BOLD);
        TTF_Font *latin=sb_ui_cluster_font(base,"A",1,NULL),*arabic=sb_ui_cluster_font(base,"ب",2,NULL),*arabic_bold=sb_ui_cluster_font(bold,"ب",2,NULL);CHECK(latin && arabic && arabic_bold);
        SBTextParagraph *paragraph=NULL;SBShapedLine line={0};CHECK(sb_bidi_paragraph_create(styled.text,strlen(styled.text),SB_BIDI_AUTO_LTR,&paragraph).code==SB_OK);
        SBShapeFontSpan fonts[]={{0,3,latin,TAG('L','a','t','n')},{3,2,arabic_bold,TAG('A','r','a','b')},{5,5,arabic,TAG('A','r','a','b')},{10,2,latin,TAG('L','a','t','n')}};
        CHECK(sb_shape_line(paragraph,0,strlen(styled.text),fonts,4,&line).code==SB_OK);
        float height=fmaxf(base->height,(line.ascent+line.descent)/ui.density)+4*ui.scale+2*ui.ctx->style.text.padding.y;
        CHECK(fabsf(sb_ui_styled_height(&ui,base,&styled,800)-height)<0.01f);
        for(unsigned aligned=0;aligned<3;++aligned){nk_flags align=aligned==0 ? NK_TEXT_LEFT : aligned==1 ? NK_TEXT_CENTERED : NK_TEXT_RIGHT;
            uint64_t expected=paint(&ui,&styled,&line,height,align),actual=paint(&ui,&styled,NULL,height,align);CHECK(expected && expected==actual);
            ui.styled_cache_disabled=true;CHECK(paint(&ui,&styled,NULL,height,align)==expected);ui.styled_cache_disabled=false;
        }
        char path[SB_PATH_CAP],name[40];snprintf(name,sizeof(name),"reader-shaped-%u.bmp",size);CHECK(sb_path_join(path,sizeof(path),argv[2],name).code==SB_OK);CHECK(sb_ui_capture(&ui,path).code==SB_OK);
        sb_shape_line_free(&line);sb_bidi_paragraph_free(paragraph);sb_styled_free(&styled);sb_inline_free(&reader);
        const char *wrapped="אבג ab cd ef gh ij kl mn op qr st uv wx yz";SBTextSpan span={0,strlen(wrapped),0};SBStyledText text={.text=(char *)wrapped,.spans=&span,.count=1};
        float narrow=sb_ui_styled_height(&ui,base,&text,100),wide=sb_ui_styled_height(&ui,base,&text,800);CHECK(narrow>wide*2);
        float repeated=sb_ui_styled_height(&ui,base,&text,100);CHECK(repeated==narrow);
        const char *sequences[]={"officeaffinityéofficeaffinity", "ببببسلامببببسلامببببسلام"};
        for(unsigned script=0;script<2;++script){const char *sequence=sequences[script];SBTextSpan one={0,strlen(sequence),0};SBStyledText sample={.text=(char *)sequence,.spans=&one,.count=1};
            SBTextParagraph *bidi=NULL;CHECK(sb_bidi_paragraph_create(sequence,strlen(sequence),SB_BIDI_AUTO_LTR,&bidi).code==SB_OK);
            TTF_Font *font=sb_ui_cluster_font(base,sequence,strlen(sequence),NULL);CHECK(font!=NULL);
            for(unsigned w=0;w<3;++w){float width=30+35*w;WrapProbe probe={.paragraph=bidi,.font={0,strlen(sequence),font,script ? TAG('A','r','a','b') : TAG('L','a','t','n')},.density=ui.density,.available=width-2*ui.ctx->style.text.padding.x,.valid=true};
                CHECK(sb_ui_styled_geometry(&ui,base,&sample,width,check_wrap,&probe));CHECK(probe.valid && probe.next==strlen(sequence) && probe.lines>1);}
            sb_bidi_paragraph_free(bidi);
        }
    }
    sb_ui_shutdown(&ui);printf("%u reader glyph/context/cache assertions passed.\n",checks);return 0;
}
