#include "shaped.h"
#include "ttf_shape.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
static unsigned checks;
#define CHECK(x) do {++checks;if(!(x)){fprintf(stderr,"SHAPED %d: %s (%s)\n",__LINE__,#x,SDL_GetError());return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
#define TAG(a,b,c,d) (((uint32_t)(a)<<24)|((uint32_t)(b)<<16)|((uint32_t)(c)<<8)|(uint32_t)(d))
static TTF_Font *open_font(const char *root,const char *name,float size){char path[4096];int n=snprintf(path,sizeof(path),"%s/%s",root,name);return n>0 && (size_t)n<sizeof(path) ? TTF_OpenFont(path,size) : NULL;}
static const SBTTFGlyph *cluster(const SBTTFShape *shape,Uint32 byte){for(size_t i=0;i<shape->count;++i)if(shape->glyphs[i].byte==byte)return &shape->glyphs[i];return NULL;}
static Uint32 observed_mirror;
static Uint32 mirror_probe(Uint32 cp){if(cp==0x221d)observed_mirror=sb_bidi_mirror(cp);return cp=='(' ? ']' : sb_bidi_mirror(cp);}
static int mirroring(TTF_Font *font) {
    SBTTFShape overridden={0},reference={0},supplementary={0};
    CHECK(sb_ttf_shape_range(font,"(",1,0,1,true,0,mirror_probe,&overridden));
    CHECK(sb_ttf_shape_range(font,"]",1,0,1,false,0,NULL,&reference));
    CHECK(overridden.count==1 && reference.count==1 && overridden.glyphs[0].index==reference.glyphs[0].index);
    observed_mirror=0;
    CHECK(sb_ttf_shape_range(font,"∝",3,0,3,true,0,mirror_probe,&supplementary));
    CHECK(observed_mirror==0x1db10);
    sb_ttf_shape_free(&overridden);sb_ttf_shape_free(&reference);sb_ttf_shape_free(&supplementary);return 0;
}
static int contextual(TTF_Font *arabic) {
    const char *text="ببب";SBTTFShape full={0},middle={0},isolated={0};
    TTF_Direction original_direction=TTF_GetFontDirection(arabic);Uint32 original_script=TTF_GetFontScript(arabic);
    CHECK(sb_ttf_shape_range(arabic,text,6,0,6,true,TAG('A','r','a','b'),NULL,&full));
    CHECK(sb_ttf_shape_range(arabic,text,6,2,2,true,TAG('A','r','a','b'),NULL,&middle));
    CHECK(sb_ttf_shape_range(arabic,text+2,2,0,2,true,TAG('A','r','a','b'),NULL,&isolated));
    CHECK(full.count==3 && middle.count==1 && isolated.count==1);
    const SBTTFGlyph *joined=cluster(&full,2);CHECK(joined);
    CHECK(joined->index==middle.glyphs[0].index && middle.glyphs[0].index!=isolated.glyphs[0].index);
    CHECK(fabsf(joined->advance-middle.glyphs[0].advance)<0.001f);
    CHECK(middle.glyphs[0].byte==2);
    CHECK(TTF_GetFontDirection(arabic)==original_direction && TTF_GetFontScript(arabic)==original_script);
    CHECK(!sb_ttf_shape_range(arabic,text,6,2,2,true,0,NULL,&middle));
    sb_ttf_shape_free(&full);sb_ttf_shape_free(&middle);sb_ttf_shape_free(&isolated);
    CHECK(!middle.glyphs && !middle.count);
    CHECK(!sb_ttf_shape_range(arabic,text,6,1,2,true,0,NULL,&middle));
    CHECK(!sb_ttf_shape_range(arabic,text,6,0,1,true,0,NULL,&middle));
    CHECK(!sb_ttf_shape_range(arabic,text,6,SIZE_MAX,1,true,0,NULL,&middle));
    return 0;
}
static int line_cases(TTF_Font *latin,TTF_Font *arabic,TTF_Font *hebrew,TTF_Font *bold) {
    const char *text="AB ببب CD";size_t length=strlen(text);
    SBTextParagraph *p=NULL;OK(sb_bidi_paragraph_create(text,length,SB_BIDI_AUTO_LTR,&p));
    SBShapeFontSpan spans[]={ {0,3,latin,TAG('L','a','t','n')},{3,2,arabic,TAG('A','r','a','b')},
        {5,2,bold,TAG('A','r','a','b')},{7,2,arabic,TAG('A','r','a','b')},{9,3,latin,TAG('L','a','t','n')}};
    SBShapedLine line={0};OK(sb_shape_line(p,0,length,spans,5,&line));
    CHECK(line.base_level==0 && line.run_count==5 && line.count==9 && line.advance>0 && line.ascent>0);
    const size_t order[]={0,7,5,3,9};
    for(size_t i=0;i<5;++i){CHECK(line.runs[i].byte==order[i]);CHECK(line.runs[i].glyph_count>0);}
    CHECK(line.glyphs[4].font==bold && line.glyphs[4].level==1);
    CHECK(!memcmp(sb_bidi_paragraph_text(p),text,length));
    SBTTFShape joined={0};CHECK(sb_ttf_shape_range(arabic,text,length,3,6,true,TAG('A','r','a','b'),NULL,&joined));
    for(size_t i=0;i<line.count;++i)if(line.glyphs[i].byte>=3 && line.glyphs[i].byte<9) {
        const SBTTFGlyph *expected=cluster(&joined,(Uint32)line.glyphs[i].byte);CHECK(expected && line.glyphs[i].index==expected->index);
    }
    sb_ttf_shape_free(&joined);CHECK(sb_shape_line(p,0,length,spans,5,&line).code==SB_INVALID);sb_shape_line_free(&line);
    /* A wrapped continuation retains its enclosing paragraph direction. */
    OK(sb_shape_line(p,3,6,spans,5,&line));CHECK(line.base_level==0 && line.run_count==3 && line.runs[0].byte==7);
    sb_shape_line_free(&line);sb_bidi_paragraph_free(p);
    text="אבג AB";length=strlen(text);OK(sb_bidi_paragraph_create(text,length,SB_BIDI_AUTO_LTR,&p));
    SBShapeFontSpan rtl_spans[]={{0,6,hebrew,TAG('H','e','b','r')},{6,3,latin,TAG('L','a','t','n')}};
    OK(sb_shape_line(p,7,2,rtl_spans,2,&line));CHECK(line.base_level==1 && line.run_count==1 && line.runs[0].level==2);
    sb_shape_line_free(&line);sb_bidi_paragraph_free(p);
    text="é";length=strlen(text);OK(sb_bidi_paragraph_create(text,length,SB_BIDI_LTR,&p));
    SBShapeFontSpan broken[]={{0,1,latin,0},{1,2,latin,0}},whole={0,length,latin,0};
    CHECK(sb_shape_line(p,0,length,broken,2,&line).code==SB_INVALID);
    CHECK(sb_shape_line(p,0,1,&whole,1,&line).code==SB_INVALID);
    CHECK(sb_shape_line(p,0,SIZE_MAX,&whole,1,&line).code==SB_INVALID);
    OK(sb_shape_line(p,0,length,&whole,1,&line));CHECK(line.count==1 && line.glyphs[0].byte==0);
    sb_shape_line_free(&line);sb_bidi_paragraph_free(p);return 0;
}
static int raster_metrics(TTF_Font *font,const char *text,bool rtl,uint32_t script) {
    SBTTFShape shape={0};
    CHECK(sb_ttf_shape_range(font,text,strlen(text),0,strlen(text),rtl,script,NULL,&shape));
    CHECK(shape.advance>0 && shape.count>0);
    for(size_t i=0;i<shape.count;++i) {
        SBTTFGlyph *g=&shape.glyphs[i];TTF_ImageType type;SDL_Surface *image=TTF_GetGlyphImageForIndex(font,g->index,&type);
        CHECK(image);CHECK(g->width>=0 && g->height>=0);
        if(g->width && g->height)CHECK(image->w==g->width && image->h==g->height);
        SDL_DestroySurface(image);
    }
    sb_ttf_shape_free(&shape);return 0;
}
static int raster_plan(const char *path,TTF_Font *latin,TTF_Font *arabic,TTF_Font *hebrew,TTF_Font *bold) {
    const char *parts[]={"Deutsch · ","مرحبا"," · ","עברית"," · AV ffi é"};
    char text[256]={0};SBShapeFontSpan spans[7];size_t count=0,byte=0;
    for(unsigned i=0;i<5;++i) {
        size_t length=strlen(parts[i]);strcat(text,parts[i]);
        if(i==1){spans[count++]=(SBShapeFontSpan){byte,4,arabic,TAG('A','r','a','b')};spans[count++]=(SBShapeFontSpan){byte+4,2,bold,TAG('A','r','a','b')};spans[count++]=(SBShapeFontSpan){byte+6,length-6,arabic,TAG('A','r','a','b')};}
        else spans[count++]=(SBShapeFontSpan){byte,length,i==3 ? hebrew : latin,i==3 ? TAG('H','e','b','r') : TAG('L','a','t','n')};
        byte+=length;
    }
    SBTextParagraph *p=NULL;SBShapedLine line={0};OK(sb_bidi_paragraph_create(text,byte,SB_BIDI_AUTO_LTR,&p));
    OK(sb_shape_line(p,0,byte,spans,count,&line));
    SDL_Surface *surface=SDL_CreateSurface((int)ceilf(line.advance)+40,line.ascent+line.descent+40,SDL_PIXELFORMAT_ARGB8888);CHECK(surface);
    CHECK(SDL_ClearSurface(surface,6.0f/255,16.0f/255,30.0f/255,1));
    for(size_t i=0;i<line.count;++i) {
        const SBShapeGlyph *g=&line.glyphs[i];TTF_ImageType type;SDL_Surface *image=TTF_GetGlyphImageForIndex(g->font,g->index,&type);CHECK(image);
        SDL_Rect destination={(int)floorf(20+g->x+g->left),(int)floorf(20+line.ascent+g->y-g->top),image->w,image->h};
        CHECK(destination.x>=0 && destination.x+image->w<=surface->w && destination.y>=0 && destination.y+image->h<=surface->h);
        CHECK(SDL_SetSurfaceBlendMode(image,SDL_BLENDMODE_BLEND) && SDL_BlitSurface(image,NULL,surface,&destination));SDL_DestroySurface(image);
    }
    CHECK(SDL_SaveBMP(surface,path));SDL_DestroySurface(surface);sb_shape_line_free(&line);sb_bidi_paragraph_free(p);return 0;
}
int main(int argc,char **argv) {
    CHECK(argc==3);CHECK(SDL_Init(0));CHECK(TTF_Init());
    for(unsigned size=0;size<3;++size) {
        float height=size==0 ? 18 : size==1 ? 27 : 36;
        TTF_Font *latin=open_font(argv[1],"NotoSans-Regular.ttf",height),*arabic=open_font(argv[1],"NotoSansArabic-Regular.ttf",height),
            *hebrew=open_font(argv[1],"NotoSansHebrew-Regular.ttf",height);CHECK(latin && arabic && hebrew);
        TTF_Font *bold=TTF_CopyFont(arabic);CHECK(bold);TTF_SetFontStyle(bold,TTF_STYLE_BOLD);
        if(contextual(arabic) || line_cases(latin,arabic,hebrew,bold) || mirroring(latin) ||
           raster_metrics(latin,"AV ffi é",false,TAG('L','a','t','n')) ||
           raster_metrics(arabic,"ببب",true,TAG('A','r','a','b')) ||
           raster_metrics(bold,"ببب",true,TAG('A','r','a','b')) ||
           (size==2 && raster_plan(argv[2],latin,arabic,hebrew,bold)))return 1;
        TTF_CloseFont(bold);TTF_CloseFont(hebrew);TTF_CloseFont(arabic);TTF_CloseFont(latin);
    }
    TTF_Quit();SDL_Quit();printf("%u contextual glyph/layout assertions passed.\n",checks);return 0;
}
