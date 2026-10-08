#include "text.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"PLAIN GEOMETRY %d: %s (%s)\n",__LINE__,#x,SDL_GetError());return 1;}}while(0)
#define TAG(a,b,c,d) (((uint32_t)(a)<<24)|((uint32_t)(b)<<16)|((uint32_t)(c)<<8)|(uint32_t)(d))
static uint64_t pixels(SBUi *ui){SDL_Surface *raw=SDL_RenderReadPixels(ui->renderer,NULL),*s=raw ? SDL_ConvertSurface(raw,SDL_PIXELFORMAT_RGBA32) : NULL;uint64_t hash=0;if(s)for(int y=0;y<s->h;++y)hash=hash*1099511628211ULL^sb_hash((char *)s->pixels+y*s->pitch,(size_t)s->w*4);SDL_DestroySurface(s);SDL_DestroySurface(raw);return hash;}
static uint64_t draw(SBUi *ui,const char *source,const SBShapedLine *reference,unsigned mode){
    nk_input_begin(ui->ctx);nk_input_end(ui->ctx);nk_begin(ui->ctx,"Plain text",nk_rect(0,0,1000,400),NK_WINDOW_NO_SCROLLBAR);
    struct nk_command_buffer *canvas=nk_window_get_canvas(ui->ctx);struct nk_rect clip=nk_rect(40,60,850,200);
    nk_push_scissor(canvas,clip);const struct nk_user_font *font=&ui->normal->handle;
    float x=40,y=70,width=reference->advance/ui->density;
    if(mode==1)x+=850-width;else if(mode==2)x+=(850-width)/2;
    if(source)nk_draw_text(canvas,nk_rect(x,y,width,font->height),source,(int)strlen(source),font,nk_rgba(0,0,0,0),nk_rgb(225,235,245));
    else sb_ui_shaped_draw(ui,reference,x,y+reference->ascent/ui->density,nk_rgb(225,235,245));
    nk_end(ui->ctx);sb_ui_draw(ui);uint64_t hash=pixels(ui);SDL_RenderPresent(ui->renderer);return hash;
}
int main(int argc,char **argv){CHECK(argc==3);SBUi ui;CHECK(sb_ui_init(&ui,argv[1],1000,400,true).code==SB_OK);ui.space.dark=true;
    for(unsigned scale=0;scale<3;++scale){CHECK(sb_ui_fonts(&ui,1+0.5f*scale).code==SB_OK);
        const char *parts[]={"AV ffi é ","אב","12","גד"," ","ببب"," ","👩‍💻"};char source[200]={0};SBShapeFontSpan fonts[8];size_t length=0;
        const struct nk_user_font *base=&ui.normal->handle;TTF_Font *latin=sb_ui_cluster_font(base,"A",1,NULL),*hebrew=sb_ui_cluster_font(base,"א",2,NULL),*arabic=sb_ui_cluster_font(base,"ب",2,NULL),*emoji=sb_ui_cluster_font(base,"👩‍💻",11,NULL);CHECK(latin && hebrew && arabic && emoji);
        for(unsigned i=0;i<8;++i){size_t n=strlen(parts[i]);TTF_Font *font=i==0 || i==2 ? latin : i<5 ? hebrew : i<7 ? arabic : emoji;uint32_t script=i==0 ? TAG('L','a','t','n') : i<5 ? TAG('H','e','b','r') : TAG('A','r','a','b');fonts[i]=(SBShapeFontSpan){length,n,font,script};memcpy(source+length,parts[i],n);length+=n;}source[length]=0;
        SBTextParagraph *bidi=NULL;SBShapedLine line={0};CHECK(sb_bidi_paragraph_create(source,length,SB_BIDI_AUTO_LTR,&bidi).code==SB_OK);CHECK(sb_shape_line(bidi,0,length,fonts,8,&line).code==SB_OK);
        float actual=base->width(base->userdata,base->height,source,(int)length);CHECK(fabsf(actual-line.advance/ui.density)<0.001f);
        for(unsigned mode=0;mode<3;++mode){uint64_t expected=draw(&ui,NULL,&line,mode),observed=draw(&ui,source,&line,mode);CHECK(expected && expected==observed);CHECK(draw(&ui,source,&line,mode)==observed);if(mode==2)CHECK(sb_ui_capture(&ui,argv[2]).code==SB_OK);}
        /* A second strong script must not inherit the first font run's script. */
        const char *greek="AV Ω";SBShapeFontSpan separate[]={{0,3,latin,TAG('L','a','t','n')},{3,2,latin,TAG('G','r','e','k')}};SBTextParagraph *other=NULL;SBShapedLine greek_line={0};CHECK(sb_bidi_paragraph_create(greek,5,SB_BIDI_AUTO_LTR,&other).code==SB_OK);CHECK(sb_shape_line(other,0,5,separate,2,&greek_line).code==SB_OK);CHECK(fabsf(base->width(base->userdata,base->height,greek,5)-greek_line.advance/ui.density)<0.001f);CHECK(draw(&ui,NULL,&greek_line,0)==draw(&ui,greek,&greek_line,0));
        sb_shape_line_free(&greek_line);sb_bidi_paragraph_free(other);sb_shape_line_free(&line);sb_bidi_paragraph_free(bidi);
    }
    const struct nk_user_font *font=&ui.normal->handle;
    const char *units[]={"é", "👩‍💻", "🇩🇪", "क्ष"};
    for(unsigned i=0;i<4;++i){size_t length=strlen(units[i]);float whole=font->width(font->userdata,font->height,units[i],(int)length),measured=0;
        CHECK(whole>0);CHECK(sb_ui_text_fit(font,units[i],length,whole-0.1f,&measured)==0 && measured==0);
        CHECK(sb_ui_text_fit(font,units[i],length,whole,&measured)==length && measured==whole);
        nk_input_begin(ui.ctx);nk_input_end(ui.ctx);nk_begin(ui.ctx,"Truncation",nk_rect(0,0,1000,400),NK_WINDOW_NO_SCROLLBAR);
        nk_draw_text(nk_window_get_canvas(ui.ctx),nk_rect(40,60,whole-0.1f,100),units[i],(int)length,font,nk_rgba(0,0,0,0),nk_rgb(255,255,255));nk_end(ui.ctx);
        bool cut=false;const struct nk_command *command;nk_foreach(command,ui.ctx)if(command->type==NK_COMMAND_TEXT){const struct nk_command_text *text=(const struct nk_command_text *)command;cut|=text->length<(int)length;}
        CHECK(!cut);sb_ui_draw(&ui);SDL_RenderPresent(ui.renderer);
    }
    CHECK(font->width(font->userdata,font->height,"אב\nA",6)>0);
    sb_ui_shutdown(&ui);printf("%u plain UI glyph/width/pixel assertions passed at three font sizes.\n",checks);return 0;
}
