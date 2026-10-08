#include "ui.h"
#include "text.h"
#include <math.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"TEXT FAIL %d: %s\n",__LINE__,#x); sb_ui_shutdown(&ui); return 1; } } while (0)
static float measure(struct nk_font *font,const char *text) {
    return font->handle.width(font->handle.userdata,font->handle.height,text,(int)strlen(text));
}
typedef struct {const char *text;bool bold,italic,both,code,arabic;float latin_baseline,arabic_baseline;} StyleProbe;
static bool inspect_style(void *user,const SBShapedLine *line,size_t offset,float y) {
    StyleProbe *p=user;
    for(size_t i=0;i<line->count;++i){const SBShapeGlyph *g=&line->glyphs[i];size_t byte=offset+g->byte;TTF_FontStyleFlags style=TTF_GetFontStyle(g->font);
        const char *words[]={"fett","kursiv","beides","Code","سلام"};
        for(unsigned word=0;word<5;++word){const char *at=strstr(p->text,words[word]);if(!at || byte!=(size_t)(at-p->text))continue;
            if(word==0){p->bold=(style&TTF_STYLE_BOLD)!=0;p->latin_baseline=y+line->ascent;}
            if(word==1)p->italic=(style&TTF_STYLE_ITALIC)!=0;
            if(word==2)p->both=(style&(TTF_STYLE_ITALIC|TTF_STYLE_BOLD))==(TTF_STYLE_ITALIC|TTF_STYLE_BOLD);
            if(word==3){const char *family=TTF_GetFontFamilyName(g->font);p->code=family && strstr(family,"Mono");}
            if(word==4){p->arabic=(style&TTF_STYLE_BOLD)!=0;p->arabic_baseline=y+line->ascent;}
        }
    }return true;
}
int main(int argc,char **argv) {
    SBUi ui; if (argc!=3) return 2;
    SBStatus status=sb_ui_init(&ui,argv[1],1000,900,true);
    if (status.code!=SB_OK) { fprintf(stderr,"%s\n",status.message); return 1; }
    ui.space.dark=true;
    const char *samples[]={"Wissen · é · Ελληνικά · Кириллица", "知识 · 知識 · 지식",
        "سلام", "שלום", "ज्ञान", "数学 ∑ ⊆ ℝ · ♟", "👩‍💻 · 🇩🇪 · 👍🏽 · 👨‍👩‍👧‍👦", "Latein + 각 + क्ष + 👩‍💻"};
    for (unsigned size=0;size<3;++size) {
        float scale=1+0.5f*size;
        CHECK(sb_ui_fonts(&ui,scale).code==SB_OK);
        float joined=measure(ui.body,"سلام");
        float separated=measure(ui.body,"س")+measure(ui.body,"ل")+measure(ui.body,"ا")+measure(ui.body,"م");
        printf("Scale %.1f: Arabic joined %.2f, separated %.2f\n",scale,joined,separated);
        CHECK(joined>0 && joined<separated-1);
        printf("Emoji scale %.1f: woman %.2f, laptop %.2f, joined %.2f, flag %.2f, thumbs %.2f, toned %.2f, family %.2f\n",scale,
            measure(ui.body,"👩"),measure(ui.body,"💻"),measure(ui.body,"👩‍💻"),measure(ui.body,"🇩🇪"),measure(ui.body,"👍"),measure(ui.body,"👍🏽"),measure(ui.body,"👨‍👩‍👧‍👦"));
        CHECK(measure(ui.body,"知识")>measure(ui.body,"??")*1.3f);
        CHECK(measure(ui.body,"지식")>measure(ui.body,"??")*1.3f);
        CHECK(measure(ui.code,"知识")>measure(ui.code,"??")*1.3f);
        CHECK(measure(ui.body,"é")<=measure(ui.body,"e")+1);
        float emoji=measure(ui.body,"👩"); CHECK(emoji>measure(ui.body,"?")*1.1f);
        CHECK(fabsf(measure(ui.body,"👩‍💻")-emoji)<1);
        /* Bounding ink can extend a pixel beyond the common emoji advance. */
        float family=measure(ui.body,"👨‍👩‍👧‍👦");
        CHECK(fabsf(family-emoji)<=2);
        CHECK(family<(measure(ui.body,"👨")+measure(ui.body,"👩")+measure(ui.body,"👧")+measure(ui.body,"👦"))*0.5f);
        CHECK(fabsf(measure(ui.body,"🇩🇪")-emoji)<1);
        CHECK(fabsf(measure(ui.body,"👍🏽")-emoji)<1);
        CHECK(fabsf(measure(ui.code,"👩‍💻")-measure(ui.code,"👩"))<1);
        CHECK(fabsf(measure(ui.body,"각")-measure(ui.body,"각"))<1);
        CHECK(fabsf(measure(ui.body,"ASCII 👩‍💻 done")-(measure(ui.body,"ASCII ")+emoji+measure(ui.body," done")))<scale*4);
        const struct nk_user_font *bold=sb_ui_text_style(&ui,&ui.body->handle,SB_TEXT_BOLD);
        const struct nk_user_font *italic=sb_ui_text_style(&ui,&ui.body->handle,SB_TEXT_ITALIC);
        const struct nk_user_font *both=sb_ui_text_style(&ui,&ui.body->handle,SB_TEXT_ITALIC|SB_TEXT_BOLD);
        CHECK(bold!=&ui.body->handle && italic!=bold && both!=italic && both!=bold);
        CHECK(bold->width(bold->userdata,bold->height,"Wissen",6)>measure(ui.body,"Wissen"));
        CHECK(bold==sb_ui_text_style(&ui,&ui.body->handle,SB_TEXT_BOLD));
        const char *rich_source="Normal · *kursiv* · **fett** · ***beides*** · `Code` · **سلام 👩‍💻**";
        SBInline rich_reader; SBStyledText rich;
        CHECK(sb_inline_init(&rich_reader,rich_source,strlen(rich_source)).code==SB_OK);
        CHECK(sb_inline_styled(&rich_reader,0,strlen(rich_source),&rich).code==SB_OK);
        const char *cluster_source="*e*́ 👩*‍*💻";
        SBInline cluster_reader; SBStyledText cluster_text;
        CHECK(sb_inline_init(&cluster_reader,cluster_source,strlen(cluster_source)).code==SB_OK);
        CHECK(sb_inline_styled(&cluster_reader,0,strlen(cluster_source),&cluster_text).code==SB_OK);
        nk_input_begin(ui.ctx); nk_input_end(ui.ctx);
        if (nk_begin(ui.ctx,"Schriften",nk_rect(0,0,1000,900),NK_WINDOW_NO_SCROLLBAR)) {
            nk_style_set_font(ui.ctx,&ui.body->handle);
            for (unsigned i=0;i<sizeof(samples)/sizeof(*samples);++i) {
                nk_layout_row_dynamic(ui.ctx,35*scale,1); nk_label(ui.ctx,samples[i],NK_TEXT_LEFT);
            }
            nk_style_set_font(ui.ctx,&ui.code->handle);
            nk_layout_row_dynamic(ui.ctx,35*scale,1); nk_label(ui.ctx,"code: Wissen · 知识 · سلام",NK_TEXT_LEFT);
            nk_style_set_font(ui.ctx,&ui.body->handle);
            nk_layout_row_dynamic(ui.ctx,sb_ui_styled_height(&ui,&ui.body->handle,&rich,950),1);
            sb_ui_styled_draw(&ui,&ui.body->handle,&rich);
            nk_layout_row_dynamic(ui.ctx,sb_ui_styled_height(&ui,&ui.body->handle,&cluster_text,950),1);
            sb_ui_styled_draw(&ui,&ui.body->handle,&cluster_text);
        }
        nk_end(ui.ctx);
        /* Styled drawing now emits cached glyph images. Inspect the shared
           source/font geometry rather than assuming one text command per style. */
        StyleProbe probe={.text=rich.text};
        CHECK(sb_ui_styled_geometry(&ui,&ui.body->handle,&rich,950,inspect_style,&probe));
        CHECK(probe.bold && probe.italic && probe.both && probe.code);
        CHECK(probe.arabic && fabsf(probe.latin_baseline-probe.arabic_baseline)<=1);
        SBTextSpan *normalized=NULL;size_t normalized_count=0;
        CHECK(sb_ui_styled_spans(&cluster_text,&normalized,&normalized_count));
        const char *emoji_at=strstr(cluster_text.text,"👩‍💻");CHECK(emoji_at!=NULL);
        size_t emoji_byte=(size_t)(emoji_at-cluster_text.text);
        bool complete_accent=false,complete_emoji=false;
        for(size_t i=0;i<normalized_count;++i){SBTextSpan span=normalized[i];
            if(span.offset==0 && span.length>=3)complete_accent=(span.style&SB_TEXT_ITALIC)!=0;
            if(span.offset<=emoji_byte && span.offset+span.length>=emoji_byte+11)complete_emoji=(span.style==0);
        }
        free(normalized);CHECK(complete_accent && complete_emoji);
        CHECK(sb_ui_text_style(&ui,&ui.heading->handle,SB_TEXT_CODE)->height==ui.heading->handle.height);
        const char *compound[]={"≫⃒","≪⃒"},*base_symbols[]={"≫","≪"};
        for (unsigned m=0;m<2;++m) {
            float combined=ui.body->handle.width(ui.body->handle.userdata,ui.body->handle.height,compound[m],(int)strlen(compound[m]));
            float base_width=ui.body->handle.width(ui.body->handle.userdata,ui.body->handle.height,base_symbols[m],(int)strlen(base_symbols[m]));
            CHECK(combined>0 && fabsf(combined-base_width)<=2*scale);
        }
        sb_ui_draw(&ui);
        if (size==2) CHECK(sb_ui_capture(&ui,argv[2]).code==SB_OK);
        SDL_RenderPresent(ui.renderer);
        sb_styled_free(&rich); sb_inline_free(&rich_reader);
        sb_styled_free(&cluster_text); sb_inline_free(&cluster_reader);
    }
    sb_ui_shutdown(&ui);
    printf("%u text assertions passed: contextual Arabic shaping, CJK/Hangul fallback, combining mark and font scales.\n",checks);
    return 0;
}
