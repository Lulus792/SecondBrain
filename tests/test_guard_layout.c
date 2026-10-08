#include "desktop.h"
#include "platform.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"GUARD %d: %s\n",__LINE__,#x); return 1; } } while (0)
#define OK(x) CHECK((x).code==SB_OK)
static SBTarget *target(SBDesktop *d,const char *id) {
    for(size_t i=0;i<d->target_count;++i) if(!strcmp(d->targets[i].id,id)) return &d->targets[i];
    return NULL;
}
static void frame(SBDesktop *d) {
    nk_input_begin(d->ui.ctx); SDL_Event event; while(SDL_PollEvent(&event)) {}
    nk_input_end(d->ui.ctx); sb_desktop_tick(d,1.0f/60); sb_desktop_frame(d);
    sb_ui_draw(&d->ui); SDL_RenderPresent(d->ui.renderer); sb_desktop_apply(d);
}
static void settle(SBDesktop *d) { for(unsigned i=0;i<45;++i) frame(d); }
static void event(SBDesktop *d,SDL_Event *e) {
    nk_input_begin(d->ui.ctx); sb_desktop_event(d,e); nk_input_end(d->ui.ctx);
}
static void key(SBDesktop *d,SDL_Keycode value) {
    SDL_Event e={0}; e.type=SDL_EVENT_KEY_DOWN; e.key.key=value; event(d,&e);
}
static void wheel(SBDesktop *d,float x,float y,float delta) {
    SDL_Event e={0}; e.type=SDL_EVENT_MOUSE_WHEEL;
    e.wheel.mouse_x=x; e.wheel.mouse_y=y; e.wheel.y=delta; event(d,&e);
}
static bool contained(struct nk_rect item,struct nk_rect box) {
    return item.w>0 && item.h>0 && item.x>=box.x && item.y>=box.y &&
        item.x+item.w<=box.x+box.w+0.5f && item.y+item.h<=box.y+box.h+0.5f;
}
int main(int argc,char **argv) {
    CHECK(argc==3); char root[SB_PATH_CAP],suffix[80];
    snprintf(suffix,sizeof(suffix),"guard-%lu-%lu",sb_process_id(),(unsigned long)time(NULL));
    OK(sb_path_join(root,sizeof(root),argv[2],suffix));
    SBDesktop d; OK(sb_desktop_init(&d,root,argv[1],true));
    OK(sb_app_new_project(&d.model,"test","Dialogprüfung",NULL));
    OK(sb_app_new_note(&d.model,"knowledge","draft","Entwurf"));
    strcat(d.model.editor,"Ungespeicherter Zusatz.\n");
    uint64_t draft=sb_hash(d.model.editor,strlen(d.model.editor));
    SBRevision original=d.model.revision;
    SDL_SetWindowSize(d.ui.window,640,480); SDL_SyncWindow(d.ui.window);
    int width,height; SDL_GetWindowSize(d.ui.window,&width,&height);
    CHECK(width>=640 && height>=480);
    for(unsigned scale=0;scale<3;++scale) for(unsigned conflict=0;conflict<2;++conflict) {
        OK(sb_ui_fonts(&d.ui,1+0.5f*scale));
        OK(sb_app_request(&d.model,SB_ACT_QUIT,NULL)); CHECK(d.model.guard);
        d.message=sb_ok();
        if(conflict) {
            char error[1000]={0};
            for(unsigned i=0;i<18;++i) strcat(error,"Die Datei konnte nicht gespeichert werden.\n");
            d.message=sb_error(SB_CONFLICT,"%s",error);
        }
        frame(&d); frame(&d);
        printf("scale %.1f conflict %u modal %.1fx%.1f body %.1f maximum %.1f message %zu\n",d.ui.scale,conflict,d.modal_bounds.w,d.modal_bounds.h,d.scrolling[3].height,d.scrolling[3].maximum,strlen(d.message.message));
        CHECK(!strcmp(d.focus,"guard-save"));
        CHECK(contained(d.modal_bounds,nk_rect(0,0,(float)width,(float)height)));
        const char *ids[]={"cancel","guard-save","guard-discard","guard-copy","guard-cancel"};
        struct nk_rect bounds[5];
        for(unsigned i=0;i<5;++i) {
            SBTarget *t=target(&d,ids[i]);
            if(i==3 && !conflict) { CHECK(!t); continue; }
            CHECK(t && contained(t->bounds,d.modal_bounds)); bounds[i]=t->bounds;
        }
        CHECK(bounds[0].y+bounds[0].h<bounds[1].y);
        CHECK(bounds[1].y+bounds[1].h<bounds[2].y);
        CHECK(bounds[2].y+bounds[2].h<bounds[4].y);
        if(conflict) CHECK(bounds[3].y+bounds[3].h<bounds[4].y);
        CHECK(d.scrolling[3].ready && d.scrolling[3].measured);
        CHECK(d.scrolling[3].position==0);
        if(conflict && scale) CHECK(d.scrolling[3].maximum>0);
        if(scale==2 && !conflict) {
            char path[SB_PATH_CAP]; OK(sb_path_join(path,sizeof(path),root,"normal-large-text.bmp")); OK(sb_ui_capture(&d.ui,path));
        }
        if(conflict && d.scrolling[3].maximum>0) {
            wheel(&d,0,0,-4); CHECK(d.scrolling[3].pending==0);
            float x=d.modal_bounds.x+50,y=d.scrolling[3].track.y+10;
            wheel(&d,x,y,-1); CHECK(d.scrolling[3].pending==50);
            frame(&d); CHECK(d.scrolling[3].position>0 && d.scrolling[3].position<50);
            settle(&d); CHECK(fabsf(d.scrolling[3].position-fminf(50,d.scrolling[3].maximum))<1);
            float after_down=fminf(d.scrolling[3].maximum,270);
            key(&d,SDLK_PAGEDOWN); CHECK(d.scrolling[3].pending==220); settle(&d);
            CHECK(fabsf(d.scrolling[3].position-after_down)<1);
            key(&d,SDLK_PAGEUP); CHECK(d.scrolling[3].pending==-220); settle(&d);
            CHECK(fabsf(d.scrolling[3].position-fmaxf(0,after_down-220))<1);
            SBScroll *s=&d.scrolling[3];
            SDL_Event e={0}; e.type=SDL_EVENT_MOUSE_BUTTON_DOWN; e.button.button=SDL_BUTTON_LEFT;
            e.button.x=s->thumb.x+2; e.button.y=s->thumb.y+s->thumb.h/2; event(&d,&e);
            CHECK(s->dragging);
            e.type=SDL_EVENT_MOUSE_MOTION; e.motion.y=s->track.y+s->track.h-s->thumb.h+s->grab;
            event(&d,&e); CHECK(s->position==s->maximum);
            e.type=SDL_EVENT_MOUSE_BUTTON_UP; e.button.button=SDL_BUTTON_LEFT; event(&d,&e);
            CHECK(!s->dragging); frame(&d); CHECK(s->position==s->maximum);
            float maximum=s->maximum;
            for(unsigned i=0;i<4;++i) { wheel(&d,x,y,-4); settle(&d); CHECK(s->position==maximum && s->maximum==maximum); }
            for(unsigned i=0;i<5;++i) {
                SBTarget *t=target(&d,ids[i]); CHECK(t && !memcmp(&t->bounds,&bounds[i],sizeof(t->bounds)));
            }
            const char *tab_order[]={"guard-discard","guard-copy","guard-cancel","cancel","guard-save"};
            for(unsigned i=0;i<5;++i) {
                key(&d,SDLK_TAB); frame(&d); CHECK(!strcmp(d.focus,tab_order[i]));
                CHECK(d.model.guard && !d.model.quit && sb_app_dirty(&d.model));
                CHECK(d.scrolling[3].position==maximum);
            }
            if(scale==2) {
                char path[SB_PATH_CAP]; OK(sb_path_join(path,sizeof(path),root,"conflict-bottom.bmp")); OK(sb_ui_capture(&d.ui,path));
            }
        }
        /* Closing only cancels the pending operation; it never discards the draft. */
        if(conflict) { strcpy(d.activate,"cancel"); frame(&d); }
        else { key(&d,SDLK_ESCAPE); frame(&d); }
        CHECK(!d.model.guard && !d.model.quit && d.model.pending.kind==SB_ACT_NONE);
        CHECK(sb_app_dirty(&d.model) && sb_hash(d.model.editor,strlen(d.model.editor))==draft);
        CHECK(original.hash==d.model.revision.hash && original.length==d.model.revision.length && original.exists==d.model.revision.exists);
        frame(&d);
    }
    /* Reopening after overflow resets the body and retains the safe default. */
    OK(sb_app_request(&d.model,SB_ACT_QUIT,NULL)); d.message=sb_ok(); frame(&d); frame(&d);
    CHECK(d.scrolling[3].position==0 && !strcmp(d.focus,"guard-save"));
    key(&d,SDLK_RETURN); frame(&d);
    CHECK(!d.model.guard && d.model.quit && !sb_app_dirty(&d.model));
    CHECK(sb_hash(d.model.editor,strlen(d.model.editor))==draft);
    printf("%u guard layout/scroll/keyboard/draft assertions passed at %dx%d.\n",checks,width,height);
    sb_desktop_free(&d); return 0;
}
