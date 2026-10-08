#include "desktop.h"
#include "platform.h"
#include "grapheme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
static unsigned checks,opened;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"CAPTION %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
static bool mock_url(const char *url){if(!strcmp(url,"https://example.test/hidden"))++opened;return true;}
static SBTarget *target(SBDesktop *d,const char *id){for(size_t i=0;i<d->target_count;++i)if(!strcmp(d->targets[i].id,id))return &d->targets[i];return NULL;}
static void layout(SBDesktop *d){nk_input_begin(d->ui.ctx);SDL_Event event;while(SDL_PollEvent(&event)){}nk_input_end(d->ui.ctx);sb_desktop_tick(d,1.0f/60);sb_desktop_frame(d);}
static void finish(SBDesktop *d){sb_ui_draw(&d->ui);SDL_RenderPresent(d->ui.renderer);sb_desktop_apply(d);}
static bool whole_prefix(const char *original,const char *prefix,size_t n){if(memcmp(original,prefix,n))return false;SBGrapheme reader;SBGraphemeBoundary b;if(!sb_grapheme_init(&reader,original,strlen(original)))return false;while(sb_grapheme_next(&reader,&b))if(b.byte==n)return true;return false;}
int main(int argc,char **argv){CHECK(argc==3);char root[SB_PATH_CAP],suffix[80];snprintf(suffix,sizeof(suffix),"captions-%lu-%lu",sb_process_id(),(unsigned long)time(NULL));OK(sb_path_join(root,sizeof(root),argv[2],suffix));
    SBDesktop d;OK(sb_desktop_init(&d,root,argv[1],true));d.open_url=mock_url;OK(sb_app_new_project(&d.model,"test","Beschriftungen",NULL));OK(sb_app_new_note(&d.model,"knowledge","labels","Labels"));
    char label[2000]="CCCC";for(unsigned i=0;i<30;++i)strcat(label,"👩‍👩‍👧‍👦");CHECK(strlen(label)>SB_NAME_CAP);
    char content[12000];snprintf(content,sizeof(content),"# Labels\n\n[%s](https://example.test/visible)\n\n",label);
    for(unsigned i=0;i<80;++i)strcat(content,"Ein weiterer Absatz mit Inhalt.\n\n");strcat(content,"[Verdeckter Link](https://example.test/hidden)\n");
    strcpy(d.model.editor,content);OK(sb_app_save(&d.model));uint64_t original=sb_hash(d.model.editor,strlen(d.model.editor));
    layout(&d);finish(&d);layout(&d);
    SBTarget *visible=target(&d,"link:0"),*hidden=target(&d,"link:1");CHECK(visible && hidden);CHECK(whole_prefix(label,visible->label,strlen(visible->label)));CHECK(hidden->bounds.y>840);
    unsigned captions=0;const struct nk_command *command;
    nk_foreach(command,d.ui.ctx)if(command->type==NK_COMMAND_TEXT){const struct nk_command_text *text=(const struct nk_command_text *)command;
        if(text->length>=4 && !memcmp(text->string,"CCCC",4)){size_t n=(size_t)text->length;CHECK(n>=3 && !memcmp(text->string+n-3,"…",3));CHECK(whole_prefix(label,text->string,n-3));++captions;}
        CHECK(!(text->length==(int)strlen("Verdeckter Link") && !memcmp(text->string,"Verdeckter Link",(size_t)text->length)));
    }CHECK(captions==1);finish(&d);
    /* All logical targets survive visual culling and accept keyboard activation. */
    strcpy(d.activate,"link:1");layout(&d);finish(&d);CHECK(opened==1 && sb_hash(d.model.editor,strlen(d.model.editor))==original && !sb_app_dirty(&d.model));
    d.scrolling[0].position=d.scrolling[0].destination=d.scrolling[0].maximum;d.scrolling[0].active=false;layout(&d);finish(&d);layout(&d);
    visible=target(&d,"link:0");CHECK(visible && visible->bounds.y+visible->bounds.h<0);float x=visible->bounds.x+visible->bounds.w/2,y=visible->bounds.y+visible->bounds.h/2;finish(&d);
    d.hover_label[0]=0;d.hover_hash=0;d.keyboard=false;nk_input_begin(d.ui.ctx);nk_input_motion(d.ui.ctx,x,y);nk_input_end(d.ui.ctx);sb_desktop_frame(&d);CHECK(!d.hover_label[0] && !d.hint_visible);finish(&d);
    CHECK(sb_hash(d.model.editor,strlen(d.model.editor))==original);sb_desktop_free(&d);printf("%u caption/grapheme/hidden-link/activation/hover assertions passed.\n",checks);return 0;
}
