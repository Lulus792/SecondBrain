#include "desktop.h"
#include "platform.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"READER ACTIONS %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
static SBTarget *find(SBDesktop *d,const char *id){for(size_t i=0;i<d->target_count;++i)if(!strcmp(d->targets[i].id,id))return &d->targets[i];return NULL;}
static void layout(SBDesktop *d){nk_input_begin(d->ui.ctx);SDL_Event e;while(SDL_PollEvent(&e))sb_desktop_event(d,&e);nk_input_end(d->ui.ctx);sb_desktop_tick(d,1.0f/60);sb_desktop_frame(d);}
static void finish(SBDesktop *d){sb_ui_draw(&d->ui);SDL_RenderPresent(d->ui.renderer);sb_desktop_apply(d);}
static int caption(SBDesktop *d,SBTarget *target){
    const struct nk_command *command;int painted=0;
    nk_foreach(command,d->ui.ctx)if(command->type==NK_COMMAND_TEXT){
        const struct nk_command_text *text=(const struct nk_command_text *)command;
        if(!text->length || text->x<target->bounds.x || text->x>=target->bounds.x+target->bounds.w || text->y<target->bounds.y || text->y>=target->bounds.y+target->bounds.h)continue;
        if(text->length!=(int)strlen(target->label) || memcmp(text->string,target->label,(size_t)text->length)){
            fprintf(stderr,"Partial action %s: %.*s, expected %s\n",target->id,text->length,text->string,target->label);return -1;
        }
        ++painted;
    }
    return painted;
}
int main(int argc,char **argv){
    CHECK(argc==3);char root[SB_PATH_CAP],suffix[90];snprintf(suffix,sizeof(suffix),"reader-actions-%lu-%lu",sb_process_id(),(unsigned long)time(NULL));OK(sb_path_join(root,sizeof(root),argv[2],suffix));
    SBDesktop d;OK(sb_desktop_init(&d,root,argv[1],true));OK(sb_app_new_project(&d.model,"review","Readeraktionen",NULL));OK(sb_app_new_note(&d.model,"knowledge","reader","Eigene Erkenntnis"));
    const char *original="# Eigene Erkenntnis\n\nDie Aktionen bleiben erkennbar.\n";strcpy(d.model.editor,original);OK(sb_app_save(&d.model));d.card=true;d.reduced_motion=true;
    unsigned widths[]={1000,780,1336};
    for(unsigned scale=0;scale<3;++scale)for(unsigned width=0;width<3;++width)for(unsigned state=0;state<4;++state){
        sb_app_source_close(&d.model);strcpy(d.model.editor,original);OK(sb_app_save(&d.model));d.editing=false;d.expanded=false;d.browser=false;d.card=true;d.form=SB_FORM_NONE;
        OK(sb_ui_fonts(&d.ui,1+scale*0.5f));CHECK(SDL_SetWindowSize(d.ui.window,(int)widths[width],840));CHECK(SDL_SyncWindow(d.ui.window));
        if(state==1 || state==2)strcat(d.model.editor,"Entwurf.\n");
        if(state==2)d.editing=true;
        if(state==3)OK(sb_app_source(&d.model,"../PROJECT.md"));
        layout(&d);finish(&d);layout(&d);
        const char *ids[4]={state==3 ? "source-back" : state==2 ? "read" : "edit","expand","actions",NULL};
        unsigned count=state==3 ? 2 : state==0 ? 3 : 4;
        if(count==4){ids[1]="save";ids[2]="expand";ids[3]="actions";}
        float right=d.ui.card_bounds.x+d.ui.card_bounds.w;
        for(unsigned i=0;i<count;++i){SBTarget *t=find(&d,ids[i]);CHECK(t && t->label[0]);CHECK(t->bounds.x>=d.ui.card_bounds.x && t->bounds.x+t->bounds.w<=right+1);CHECK(t->bounds.w>20*d.ui.scale);
            if(i){SBTarget *previous=find(&d,ids[i-1]);CHECK(t->bounds.x>=previous->bounds.x+previous->bounds.w);CHECK(fabsf(t->bounds.y-previous->bounds.y)<1);}
            int text=caption(&d,t);if(text<0){char picture[SB_PATH_CAP];OK(sb_path_join(picture,sizeof(picture),root,"partial-before.bmp"));finish(&d);OK(sb_ui_capture(&d.ui,picture));fprintf(stderr,"Snapshot: %s\n",picture);}CHECK(text>=0);
            if(scale==1 && width==0 && state==0 && !i)CHECK(text==1);
        }
        finish(&d);
        if(scale==1 && width==0 && (state==0 || state==2)){char picture[SB_PATH_CAP];OK(sb_path_join(picture,sizeof(picture),root,state==0 ? "reader-actions.bmp" : "editor-actions.bmp"));OK(sb_ui_capture(&d.ui,picture));printf("Snapshot: %s\n",picture);}
    }
    sb_app_source_close(&d.model);strcpy(d.model.editor,original);OK(sb_app_save(&d.model));d.editing=false;d.expanded=false;d.card=true;layout(&d);finish(&d);
    strcpy(d.activate,"actions");layout(&d);finish(&d);CHECK(d.form==SB_FORM_ACTIONS);
    SDL_Event key={0};key.type=SDL_EVENT_KEY_DOWN;key.key.windowID=SDL_GetWindowID(d.ui.window);key.key.key=SDLK_ESCAPE;key.key.down=true;sb_desktop_event(&d,&key);layout(&d);finish(&d);CHECK(d.form==SB_FORM_NONE);
    strcpy(d.activate,"expand");layout(&d);finish(&d);CHECK(d.expanded);strcpy(d.activate,"edit");layout(&d);finish(&d);CHECK(d.editing);
    CHECK(!sb_app_dirty(&d.model));sb_desktop_free(&d);printf("%u reader action caption/geometry/activation assertions passed.\n",checks);return 0;
}
