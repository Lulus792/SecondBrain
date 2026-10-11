#include "desktop.h"
#include "text.h"
#include "platform.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"READER NATIVE %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
#define TAG(a,b,c,d) (((uint32_t)(a)<<24)|((uint32_t)(b)<<16)|((uint32_t)(c)<<8)|(uint32_t)(d))
static bool box(const SBNativeText *text,size_t byte,SBNativeCharBox *out){
    for(size_t i=0;i<text->count;++i){const SBNativeTextRun *r=&text->runs[i];size_t at=r->span.offset;
        for(size_t j=0;j<r->characters;++j){if(at==byte && r->geometry){*out=(SBNativeCharBox){at,r->lengths[j],r->level&1 ? r->x+r->width-r->positions[j]-r->widths[j] : r->x+r->positions[j],r->y,r->widths[j],r->height,r->level};return true;}at+=r->lengths[j];}}
    return false;
}
static int reference(SBUi *ui){
    const char *source="AB **ب**بب CD";SBInline reader;SBStyledText styled;OK(sb_inline_init(&reader,source,strlen(source)));OK(sb_inline_styled(&reader,0,strlen(source),&styled));
    const struct nk_user_font *base=&ui->body->handle,*bold=sb_ui_text_style(ui,base,SB_TEXT_BOLD);
    TTF_Font *latin=sb_ui_cluster_font(base,"A",1,NULL),*arabic=sb_ui_cluster_font(base,"ب",2,NULL),*arabic_bold=sb_ui_cluster_font(bold,"ب",2,NULL);CHECK(latin && arabic && arabic_bold);
    SBTextParagraph *bidi=NULL;SBShapeParagraph *paragraph=NULL;SBShapedLine line={0};SBCaretPlan carets={0};
    OK(sb_bidi_paragraph_create(styled.text,strlen(styled.text),SB_BIDI_AUTO_LTR,&bidi));
    SBShapeFontSpan fonts[]={{0,3,latin,TAG('L','a','t','n')},{3,2,arabic_bold,TAG('A','r','a','b')},{5,5,arabic,TAG('A','r','a','b')},{10,2,latin,TAG('L','a','t','n')}};
    OK(sb_shape_line(bidi,0,strlen(styled.text),fonts,4,&line));OK(sb_shape_paragraph_create(bidi,fonts,4,&paragraph));OK(sb_caret_plan(paragraph,&line,&carets));
    nk_flags alignments[]={NK_TEXT_LEFT,NK_TEXT_CENTERED,NK_TEXT_RIGHT};
    for(size_t alignment=0;alignment<3;++alignment){struct nk_rect rect=nk_rect(37.25f,-83.5f,800,80);SBNativeText native={0},moved={0};
        CHECK(sb_ui_styled_native(ui,base,&styled,rect,alignments[alignment],&native));
        float available=rect.w-2*ui->ctx->style.text.padding.x,shift=alignment==2 ? available-line.advance/ui->density : alignment==1 ? (available-line.advance/ui->density)/2 : 0;
        for(size_t i=0;i<carets.cluster_count;++i){SBCaretCluster c=carets.clusters[i];SBNativeCharBox actual;CHECK(box(&native,c.byte,&actual));
            CHECK(fabsf(actual.x-(rect.x+ui->ctx->style.text.padding.x+shift+c.left/ui->density))<0.01f);
            CHECK(fabsf(actual.width-(c.right-c.left)/ui->density)<0.01f && actual.level==c.level);
            CHECK(fabsf(actual.y-(rect.y+ui->ctx->style.text.padding.y))<0.01f && fabsf(actual.height-fmaxf(base->height,(line.ascent+line.descent)/ui->density))<0.01f);}
        rect.x+=14.75f;rect.y+=121.25f;CHECK(sb_ui_styled_native(ui,base,&styled,rect,alignments[alignment],&moved));CHECK(native.count==moved.count);
        for(size_t i=0;i<native.count;++i){CHECK(fabsf(moved.runs[i].x-native.runs[i].x-14.75f)<0.01f && fabsf(moved.runs[i].y-native.runs[i].y-121.25f)<0.01f);CHECK(native.runs[i].span.style==moved.runs[i].span.style && native.runs[i].characters==moved.runs[i].characters);}
        native.runs[0].x=999;sb_native_text_free(&native);sb_native_text_free(&moved);
        CHECK(sb_ui_styled_native(ui,base,&styled,rect,alignments[alignment],&native) && native.runs[0].x!=999);sb_native_text_free(&native);
    }
    sb_caret_plan_free(&carets);sb_shape_line_free(&line);sb_shape_paragraph_free(paragraph);sb_bidi_paragraph_free(bidi);sb_styled_free(&styled);sb_inline_free(&reader);return 0;
}
static void frame(SBDesktop *d){nk_input_begin(d->ui.ctx);SDL_Event event;while(SDL_PollEvent(&event))sb_desktop_event(d,&event);sb_desktop_tick(d,1.0f/60);nk_input_end(d->ui.ctx);sb_desktop_frame(d);sb_ui_draw(&d->ui);SDL_RenderPresent(d->ui.renderer);sb_desktop_apply(d);}
int main(int argc,char **argv){
    CHECK(argc==3);char root[SB_PATH_CAP],suffix[80];snprintf(suffix,sizeof(suffix),"reader-native-%lu",sb_process_id());OK(sb_path_join(root,sizeof(root),argv[2],suffix));SBDesktop d;OK(sb_desktop_init(&d,root,argv[1],true));
    for(unsigned scale=0;scale<3;++scale){OK(sb_ui_fonts(&d.ui,1+0.5f*scale));CHECK(!reference(&d.ui));
        const char *raw="  AV ffi  \r\n \nאבג  \n";SBTextSpan code={0,strlen(raw),SB_TEXT_CODE};SBNativeText native={0};
        CHECK(sb_ui_wrap_native(&d.ui,&d.ui.code->handle,raw,strlen(raw),&code,1,nk_rect(11,23,85,600),&native));
        size_t bytes=0;bool zero=false,rtl=false;for(size_t i=0;i<native.count;++i){SBNativeTextRun *r=&native.runs[i];CHECK(r->span.offset==bytes && r->span.style==SB_TEXT_CODE);bytes+=r->span.length;
            for(size_t j=0;j<r->characters;++j){CHECK(r->geometry && r->height>0 && isfinite(r->positions[j]));zero|=r->widths[j]==0;rtl|=(r->level&1)!=0;}}
        CHECK(bytes==strlen(raw) && zero && rtl);sb_native_text_free(&native);
    }
    OK(sb_ui_fonts(&d.ui,1));OK(sb_app_new_project(&d.model,"reader","Reader",NULL));OK(sb_app_new_note(&d.model,"knowledge","note","Geometry"));
    strcpy(d.model.editor,"# Geometry\n\nAB **ب**بب CD é 👩‍💻\n\n> Quoted אבג\n\n- Nested é\n\n```c\nAV ffi\r\nאבג\n```\n\n| Left | Right |\n| :--- | ---: |\n| é | אבג |\n");OK(sb_app_save(&d.model));d.reduced_motion=true;d.card=true;d.expanded=true;strcpy(d.focus,"reader");d.keyboard=true;frame(&d);frame(&d);
    bool paragraph=false,code=false,cell=false;for(size_t i=0;i<d.passive_count;++i){SBPassiveText *p=&d.passive[i];
        if(p->role!=ACCESSKIT_ROLE_PARAGRAPH && p->role!=ACCESSKIT_ROLE_CODE && p->role!=ACCESSKIT_ROLE_CELL && p->role!=ACCESSKIT_ROLE_COLUMN_HEADER)continue;
        if(p->bounds.w<=0 && p->role==ACCESSKIT_ROLE_COLUMN_HEADER)continue;CHECK(p->native.count);size_t bytes=0;
        for(size_t j=0;j<p->native.count;++j){SBNativeTextRun *r=&p->native.runs[j];CHECK(r->span.offset==bytes);bytes+=r->span.length;if(r->characters)CHECK(r->geometry && r->height>0);}
        CHECK(bytes==strlen(p->text));paragraph|=p->role==ACCESSKIT_ROLE_PARAGRAPH;code|=p->role==ACCESSKIT_ROLE_CODE;cell|=p->role==ACCESSKIT_ROLE_CELL;
    }CHECK(paragraph && code && cell);
    char picture[SB_PATH_CAP];OK(sb_path_join(picture,sizeof(picture),root,"reader-native.bmp"));OK(sb_ui_capture(&d.ui,picture));sb_desktop_free(&d);printf("%u native reader/independent glyph/alignment/scroll/style/source assertions passed.\n",checks);return 0;
}
