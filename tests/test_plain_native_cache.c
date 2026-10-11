#include "text.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"PLAIN NATIVE CACHE %d: %s\n",__LINE__,#x);return 1;}}while(0)
static struct nk_command_text *command(SBUi *ui,unsigned key) {
    size_t length=60000;struct nk_command_text *c=calloc(1,sizeof(*c)+length+1);if(!c)return NULL;
    c->font=&ui->normal->handle;c->foreground=nk_rgb(230,240,250);c->x=17;c->y=29;c->length=(int)length;
    memset(c->string,'A'+key%20,length);c->string[length]=0;return c;
}
static bool complete(const SBNativeText *text,size_t length) {
    size_t covered=0;for(size_t i=0;i<text->count;++i){const SBNativeTextRun *r=&text->runs[i];
        if(!r->geometry || r->span.offset!=covered || !r->characters || r->characters>SB_NATIVE_CHAR_LIMIT)return false;
        size_t bytes=0;for(size_t j=0;j<r->characters;++j){if(r->lengths[j]!=1 || r->widths[j]<=0)return false;bytes+=r->lengths[j];}
        if(bytes!=r->span.length)return false;covered+=bytes;
    }return covered==length;
}
int main(int argc,char **argv) {
    CHECK(argc==2);SBUi ui;CHECK(sb_ui_init(&ui,argv[1],400,300,true).code==SB_OK);
    struct nk_command_text *first=command(&ui,0);CHECK(first);SBNativeText held={0};CHECK(sb_ui_text_command_native(&ui,first,&held));CHECK(complete(&held,60000));
    /* Actual large exports force eviction under the production 16 MiB budget.
       Outputs retain their own geometry even when their source entry is gone. */
    for(unsigned i=1;i<12;++i){struct nk_command_text *next=command(&ui,i);CHECK(next);SBNativeText out={0};
        CHECK(sb_ui_text_command_native(&ui,next,&out));CHECK(complete(&out,60000));sb_native_text_free(&out);free(next);CHECK(complete(&held,60000));}
    SBNativeText again={0};CHECK(sb_ui_text_command_native(&ui,first,&again));CHECK(complete(&again,60000));CHECK(again.count==held.count);
    for(size_t i=0;i<held.count;++i){const SBNativeTextRun *a=&held.runs[i],*b=&again.runs[i];CHECK(a->span.offset==b->span.offset && a->span.length==b->span.length && a->x==b->x && a->y==b->y && a->width==b->width && a->height==b->height);CHECK(!memcmp(a->positions,b->positions,sizeof(a->positions)) && !memcmp(a->widths,b->widths,sizeof(a->widths)));}
    again.runs[0].x=999;sb_native_text_free(&again);CHECK(held.runs[0].x==17);
    free(first);sb_native_text_free(&held);sb_ui_shutdown(&ui);printf("%u native plain-text cache pressure/ownership/geometry assertions passed.\n",checks);return 0;
}
