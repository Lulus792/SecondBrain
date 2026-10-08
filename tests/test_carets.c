#include "carets.h"
#include "ttf_shape.h"
#include "grapheme.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"CARET %d: %s (%s)\n",__LINE__,#x,SDL_GetError());return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
#define TAG(a,b,c,d) (((uint32_t)(a)<<24)|((uint32_t)(b)<<16)|((uint32_t)(c)<<8)|(uint32_t)(d))
static TTF_Font *open_font(const char *root,const char *name,float size){char path[4096];snprintf(path,sizeof(path),"%s/%s",root,name);return TTF_OpenFont(path,size);}
static int verify(const char *text,SBShapeFontSpan *fonts,size_t count,size_t byte,size_t length,SBCaretPlan *out,SBShapedLine *line,SBTextParagraph **bidi){
    OK(sb_bidi_paragraph_create(text,strlen(text),SB_BIDI_AUTO_LTR,bidi));SBShapeParagraph *p=NULL;
    OK(sb_shape_paragraph_create(*bidi,fonts,count,&p));OK(sb_shape_paragraph_line(p,byte,length,line));OK(sb_caret_plan(p,line,out));
    CHECK(sb_caret_plan(p,line,out).code==SB_INVALID);
    const uint32_t *boundaries;size_t n=0;boundaries=sb_shape_paragraph_boundaries(p,&n);CHECK(boundaries && n>1);
    for(size_t i=0;i<out->count;++i){SBCaret c=out->stops[i];CHECK(isfinite(c.x) && c.x>=-0.01f && c.x<=line->advance+0.01f);CHECK(c.byte>=byte && c.byte<=byte+length);
        bool boundary=false;for(size_t j=0;j<n;++j)boundary|=c.byte==boundaries[j];CHECK(boundary);
        if(i)CHECK(c.x>=out->stops[i-1].x);
        SBCaret selected;CHECK(sb_caret_find(out,c.byte,(SBCaretAffinity)c.affinity,line->base_level,&selected));CHECK(selected.byte==c.byte);
        CHECK(sb_caret_nearest(out,c.x,line->base_level,&selected));CHECK(selected.x==c.x);
        SBCaret moved;if(sb_caret_step(out,&c,1,&moved))CHECK(moved.x>c.x);if(sb_caret_step(out,&c,-1,&moved))CHECK(moved.x<c.x);
    }
    size_t covered=byte;for(size_t i=0;i<out->cluster_count;++i){SBCaretCluster c=out->clusters[i];CHECK(c.byte==covered && c.length && c.right>=c.left);covered+=c.length;}CHECK(covered==byte+length);
    SBCaret edge;CHECK(sb_caret_nearest(out,-10000,line->base_level,&edge) && edge.x==out->stops[0].x);CHECK(sb_caret_nearest(out,10000,line->base_level,&edge) && edge.x==out->stops[out->count-1].x);
    SBSelectionPlan selection={0};OK(sb_caret_selection(out,byte,byte+length,&selection));size_t selected=0;float width=0;
    for(size_t i=0;i<selection.count;++i){selected+=selection.spans[i].length;width+=selection.spans[i].width;}CHECK(selected==length && fabsf(width-line->advance)<0.1f);
    sb_caret_selection_free(&selection);CHECK(!selection.spans && !selection.count);
    OK(sb_caret_selection(out,byte,byte,&selection));CHECK(!selection.count);CHECK(sb_caret_selection(out,byte+length,byte,&selection).code==SB_INVALID);
    CHECK(!sb_caret_nearest(out,NAN,0,&edge));CHECK(!sb_caret_find(out,byte,0,0,&edge));CHECK(!sb_caret_step(out,&edge,0,&edge));
    sb_shape_paragraph_free(p);return 0;
}
static int examples(TTF_Font *latin,TTF_Font *arabic,TTF_Font *hebrew,TTF_Font *emoji){
    SBCaretPlan carets={0};SBShapedLine line={0};SBTextParagraph *bidi=NULL;const char *text="AV office é";size_t length=strlen(text);SBShapeFontSpan one={0,length,latin,TAG('L','a','t','n')};
    CHECK(!verify(text,&one,1,0,length,&carets,&line,&bidi));
    /* FontTools inspection of the pinned Noto Sans original GDEF: ffi carets
       are 315/631 design units at 1000 UPM. These are not equal thirds. */
    size_t ffi_byte=(size_t)(strstr(text,"ffi")-text);const SBShapeGlyph *ligature=NULL;
    for(size_t i=0;i<line.count;++i)if(line.glyphs[i].byte==ffi_byte)ligature=&line.glyphs[i];CHECK(ligature);
    float points[2];size_t defined=0;CHECK(sb_ttf_ligature_carets(latin,ligature->index,false,points,2,&defined));CHECK(defined==2);
    float scale=TTF_GetFontSize(latin)/1000.0f;CHECK(fabsf(points[0]-315*scale)<0.04f && fabsf(points[1]-631*scale)<0.04f);
    SBCaret interior;CHECK(sb_caret_find(&carets,ffi_byte+1,SB_CARET_BOTH,0,&interior));CHECK(fabsf(interior.x-(ligature->x+points[0]))<0.001f);
    CHECK(sb_caret_find(&carets,ffi_byte+2,SB_CARET_BOTH,0,&interior));CHECK(fabsf(interior.x-(ligature->x+points[1]))<0.001f);
    unsigned gdef=0;for(size_t i=0;i<carets.cluster_count;++i)gdef+=carets.clusters[i].metric==SB_CARET_GDEF;CHECK(gdef>=3);
    size_t accent=(size_t)(strstr(text,"é")-text);CHECK(!sb_caret_find(&carets,accent+1,SB_CARET_BOTH,0,&interior));
    SBSelectionPlan bad={0};CHECK(sb_caret_selection(&carets,accent+1,length,&bad).code==SB_INVALID && !bad.spans);
    sb_caret_plan_free(&carets);sb_shape_line_free(&line);sb_bidi_paragraph_free(bidi);
    text="AB בבב CD";length=strlen(text);SBShapeFontSpan mixed[]={{0,3,latin,TAG('L','a','t','n')},{3,6,hebrew,TAG('H','e','b','r')},{9,3,latin,TAG('L','a','t','n')}};
    CHECK(!verify(text,mixed,3,0,length,&carets,&line,&bidi));SBCaret before,after;
    CHECK(sb_caret_find(&carets,3,SB_CARET_BEFORE,0,&before));CHECK(sb_caret_find(&carets,3,SB_CARET_AFTER,0,&after));CHECK(before.x>after.x && before.level==1 && after.level==0);
    /* Within RTL text, increasing the logical offset decreases its x. */
    CHECK(sb_caret_find(&carets,5,SB_CARET_BOTH,0,&interior) && interior.x<before.x && interior.x>after.x);
    sb_caret_plan_free(&carets);sb_shape_line_free(&line);sb_bidi_paragraph_free(bidi);
    text="ab א12ב cd";length=strlen(text);SBShapeFontSpan disconnected[]={{0,3,latin,TAG('L','a','t','n')},{3,2,hebrew,TAG('H','e','b','r')},{5,2,latin,TAG('L','a','t','n')},{7,2,hebrew,TAG('H','e','b','r')},{9,3,latin,TAG('L','a','t','n')}};
    CHECK(!verify(text,disconnected,5,0,length,&carets,&line,&bidi));SBSelectionPlan split={0};OK(sb_caret_selection(&carets,3,6,&split));CHECK(split.count==2);
    CHECK(split.spans[0].x>split.spans[1].x+split.spans[1].width || split.spans[1].x>split.spans[0].x+split.spans[0].width);
    sb_caret_selection_free(&split);sb_caret_plan_free(&carets);sb_shape_line_free(&line);sb_bidi_paragraph_free(bidi);
    text="a\xE2\x81\xA7" "אב" "\xE2\x81\xA9" "b";length=strlen(text);SBShapeFontSpan isolated[]={{0,4,latin,TAG('L','a','t','n')},{4,4,hebrew,TAG('H','e','b','r')},{8,4,latin,TAG('L','a','t','n')}};
    CHECK(!verify(text,isolated,3,0,length,&carets,&line,&bidi));
    sb_caret_plan_free(&carets);sb_shape_line_free(&line);sb_bidi_paragraph_free(bidi);
    text="ببب";length=strlen(text);one=(SBShapeFontSpan){0,length,arabic,TAG('A','r','a','b')};CHECK(!verify(text,&one,1,0,length,&carets,&line,&bidi));
    CHECK(sb_caret_find(&carets,0,SB_CARET_BEFORE,1,&before) && fabsf(before.x-line.advance)<0.001f);
    CHECK(sb_caret_find(&carets,length,SB_CARET_AFTER,1,&after) && after.x==0);
    sb_caret_plan_free(&carets);sb_shape_line_free(&line);sb_bidi_paragraph_free(bidi);
    text="👩‍💻🇩🇪👍🏽";length=strlen(text);one=(SBShapeFontSpan){0,length,emoji,0};CHECK(!verify(text,&one,1,0,length,&carets,&line,&bidi));CHECK(carets.cluster_count==3);
    for(size_t i=0;i<carets.count;++i)CHECK(carets.stops[i].byte==0 || carets.stops[i].byte==11 || carets.stops[i].byte==19 || carets.stops[i].byte==27);
    sb_caret_plan_free(&carets);sb_shape_line_free(&line);sb_bidi_paragraph_free(bidi);
    text="אבג office";length=strlen(text);SBShapeFontSpan wrap[]={{0,7,hebrew,TAG('H','e','b','r')},{7,6,latin,TAG('L','a','t','n')}};
    CHECK(!verify(text,wrap,2,7,6,&carets,&line,&bidi));CHECK(line.base_level==1 && carets.byte==7 && carets.length==6);
    sb_caret_plan_free(&carets);sb_shape_line_free(&line);sb_bidi_paragraph_free(bidi);return 0;
}
static int large_line(TTF_Font *font){
    size_t length=20000;char *text=malloc(length+1);CHECK(text);memset(text,'x',length);text[length]=0;
    SBTextParagraph *bidi=NULL;SBShapeParagraph *paragraph=NULL;SBShapedLine line={0};SBCaretPlan plan={0};SBShapeFontSpan span={0,length,font,TAG('L','a','t','n')};
    OK(sb_bidi_paragraph_create(text,length,SB_BIDI_LTR,&bidi));OK(sb_shape_paragraph_create(bidi,&span,1,&paragraph));OK(sb_shape_paragraph_line(paragraph,0,length,&line));OK(sb_caret_plan(paragraph,&line,&plan));CHECK(line.count==length && plan.count==length+1);
    Uint64 start=SDL_GetTicksNS();
    for(size_t i=0;i<100000;++i){size_t byte=i*613%length;SBCaret found,hit,moved;CHECK(sb_caret_find(&plan,byte,SB_CARET_BEFORE,0,&found));CHECK(fabsf(found.x-line.glyphs[byte].x)<0.01f);CHECK(sb_caret_nearest(&plan,found.x,0,&hit) && hit.byte==byte);CHECK(sb_caret_step(&plan,&found,1,&moved) && moved.byte==byte+1);}
    printf("100000 indexed caret find/hit/step queries in %.3f ms (20,000-byte line).\n",(double)(SDL_GetTicksNS()-start)/1e6);
    SBSelectionPlan small={0};OK(sb_caret_selection(&plan,12000,12003,&small));CHECK(small.count==3);sb_caret_selection_free(&small);
    SBShapedLine broken=line;broken.runs=NULL;SBCaretPlan rejected={0};CHECK(sb_caret_plan(paragraph,&broken,&rejected).code==SB_INVALID && !rejected.stops);
    sb_caret_plan_free(&plan);sb_shape_line_free(&line);sb_shape_paragraph_free(paragraph);sb_bidi_paragraph_free(bidi);free(text);return 0;
}
static int preview(const char *path,TTF_Font *latin,TTF_Font *hebrew,TTF_Font *arabic){
    const char *text="AV ffi é · אב12גד · ببب";
    const char *hebrew_text=strstr(text,"אב"),*arabic_text=strstr(text,"ببب");CHECK(hebrew_text && arabic_text);
    size_t hebrew_at=(size_t)(hebrew_text-text),tail=(size_t)(arabic_text-text),length=strlen(text);
    SBShapeFontSpan fonts[]={{0,hebrew_at,latin,TAG('L','a','t','n')},{hebrew_at,4,hebrew,TAG('H','e','b','r')},{hebrew_at+4,2,latin,TAG('L','a','t','n')},{hebrew_at+6,4,hebrew,TAG('H','e','b','r')},{hebrew_at+10,tail-hebrew_at-10,latin,TAG('L','a','t','n')},{tail,length-tail,arabic,TAG('A','r','a','b')}};
    SBCaretPlan carets={0};SBShapedLine line={0};SBTextParagraph *bidi=NULL;
    CHECK(!verify(text,fonts,6,0,length,&carets,&line,&bidi));
    SDL_Surface *surface=SDL_CreateSurface((int)ceilf(line.advance)+60,line.ascent+line.descent+70,SDL_PIXELFORMAT_ARGB8888);CHECK(surface);CHECK(SDL_ClearSurface(surface,0.025f,0.055f,0.1f,1));
    SBSelectionPlan selection={0};OK(sb_caret_selection(&carets,hebrew_at,hebrew_at+5,&selection));
    /* Selection rectangles from the same glyph plan, then the actual glyphs. */
    for(size_t i=0;i<selection.count;++i){SBSelectionSpan c=selection.spans[i];SDL_Rect r={(int)floorf(30+c.x),30,(int)ceilf(c.width),line.ascent+line.descent};CHECK(SDL_FillSurfaceRect(surface,&r,0xff284a65));}
    for(size_t i=0;i<line.count;++i){SBShapeGlyph *g=&line.glyphs[i];TTF_ImageType type;SDL_Surface *image=TTF_GetGlyphImageForIndex(g->font,g->index,&type);CHECK(image);SDL_Rect dest={(int)floorf(30+g->x+g->left),30+line.ascent+(int)floorf(g->y-g->top),image->w,image->h};CHECK(SDL_SetSurfaceBlendMode(image,SDL_BLENDMODE_BLEND) && SDL_BlitSurface(image,NULL,surface,&dest));SDL_DestroySurface(image);}
    for(size_t i=0;i<carets.count;++i){SBCaret c=carets.stops[i];SDL_Rect r={(int)floorf(30+c.x),20,1,8};CHECK(SDL_FillSurfaceRect(surface,&r,c.affinity==SB_CARET_AFTER ? 0xffffbb75 : 0xff80c3ee));}
    CHECK(SDL_SaveBMP(surface,path));SDL_DestroySurface(surface);sb_caret_selection_free(&selection);sb_caret_plan_free(&carets);sb_shape_line_free(&line);sb_bidi_paragraph_free(bidi);return 0;
}
int main(int argc,char **argv){CHECK(argc==3);CHECK(SDL_Init(0));CHECK(TTF_Init());
    for(unsigned size=0;size<3;++size){float points=18+9*size;TTF_Font *latin=open_font(argv[1],"NotoSans-Regular.ttf",points),*arabic=open_font(argv[1],"NotoSansArabic-Regular.ttf",points),*hebrew=open_font(argv[1],"NotoSansHebrew-Regular.ttf",points),*emoji=open_font(argv[1],"NotoEmoji-Variable.ttf",points);CHECK(latin && arabic && hebrew && emoji);CHECK(!examples(latin,arabic,hebrew,emoji));if(size==2){CHECK(!preview(argv[2],latin,hebrew,arabic));CHECK(!large_line(latin));}TTF_CloseFont(latin);TTF_CloseFont(arabic);TTF_CloseFont(hebrew);TTF_CloseFont(emoji);}
    for(unsigned direction=0;direction<2;++direction){SBTextParagraph *bidi=NULL;SBShapeParagraph *paragraph=NULL;SBShapedLine line={0};SBCaretPlan plan={0};
        OK(sb_bidi_paragraph_create("",0,direction ? SB_BIDI_RTL : SB_BIDI_LTR,&bidi));
        OK(sb_shape_paragraph_create(bidi,NULL,0,&paragraph));OK(sb_shape_paragraph_line(paragraph,0,0,&line));CHECK(line.base_level==direction);
        OK(sb_caret_plan(paragraph,&line,&plan));CHECK(plan.count==1 && plan.stops[0].byte==0 && plan.stops[0].x==0 && plan.stops[0].level==direction);
        SBCaret found;CHECK(sb_caret_find(&plan,0,SB_CARET_BEFORE,(unsigned char)direction,&found));CHECK(!sb_caret_step(&plan,&found,1,&found));
        SBSelectionPlan selection={0};OK(sb_caret_selection(&plan,0,0,&selection));CHECK(selection.count==0);
        sb_caret_plan_free(&plan);sb_shape_line_free(&line);sb_shape_paragraph_free(paragraph);sb_bidi_paragraph_free(bidi);
    }
    TTF_Quit();SDL_Quit();printf("%u visual caret/affinity/ligature/selection assertions passed.\n",checks);return 0;
}
