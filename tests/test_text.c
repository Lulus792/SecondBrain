#include "ui.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"TEXT FAIL %d: %s\n",__LINE__,#x); sb_ui_shutdown(&ui); return 1; } } while (0)
static float measure(struct nk_font *font,const char *text) {
    return font->handle.width(font->handle.userdata,font->handle.height,text,(int)strlen(text));
}
int main(int argc,char **argv) {
    SBUi ui; if (argc!=3) return 2;
    SBStatus status=sb_ui_init(&ui,argv[1],1000,650,true);
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
        CHECK(fabsf(measure(ui.body,"👨‍👩‍👧‍👦")-emoji)<1);
        CHECK(fabsf(measure(ui.body,"🇩🇪")-emoji)<1);
        CHECK(fabsf(measure(ui.body,"👍🏽")-emoji)<1);
        CHECK(fabsf(measure(ui.code,"👩‍💻")-measure(ui.code,"👩"))<1);
        CHECK(fabsf(measure(ui.body,"각")-measure(ui.body,"각"))<1);
        CHECK(fabsf(measure(ui.body,"ASCII 👩‍💻 done")-(measure(ui.body,"ASCII ")+emoji+measure(ui.body," done")))<scale*4);
        nk_input_begin(ui.ctx); nk_input_end(ui.ctx);
        if (nk_begin(ui.ctx,"Schriften",nk_rect(0,0,1000,650),NK_WINDOW_NO_SCROLLBAR)) {
            nk_style_set_font(ui.ctx,&ui.body->handle);
            for (unsigned i=0;i<sizeof(samples)/sizeof(*samples);++i) {
                nk_layout_row_dynamic(ui.ctx,35*scale,1); nk_label(ui.ctx,samples[i],NK_TEXT_LEFT);
            }
            nk_style_set_font(ui.ctx,&ui.code->handle);
            nk_layout_row_dynamic(ui.ctx,35*scale,1); nk_label(ui.ctx,"code: Wissen · 知识 · سلام",NK_TEXT_LEFT);
        }
        nk_end(ui.ctx); sb_ui_draw(&ui);
        if (size==2) CHECK(sb_ui_capture(&ui,argv[2]).code==SB_OK);
        SDL_RenderPresent(ui.renderer);
    }
    sb_ui_shutdown(&ui);
    printf("%u text assertions passed: contextual Arabic shaping, CJK/Hangul fallback, combining mark and font scales.\n",checks);
    return 0;
}
