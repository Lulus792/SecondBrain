#include "desktop.h"
#include "platform.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
static unsigned checks;
#define CHECK(x) do {++checks;if(!(x)){fprintf(stderr,"NAVIGATION %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
#ifdef __APPLE__
#define MOD SDL_KMOD_GUI
#else
#define MOD SDL_KMOD_CTRL
#endif
static void frame(SBDesktop *d){nk_input_begin(d->ui.ctx);SDL_Event e;while(SDL_PollEvent(&e))sb_desktop_event(d,&e);sb_desktop_tick(d,1.0f/60);nk_input_end(d->ui.ctx);sb_desktop_frame(d);sb_ui_draw(&d->ui);SDL_RenderPresent(d->ui.renderer);sb_desktop_apply(d);}
static void key(SBDesktop *d,SDL_Keycode k,SDL_Keymod mod){SDL_Event e={0};e.type=SDL_EVENT_KEY_DOWN;e.key.windowID=SDL_GetWindowID(d->ui.window);e.key.key=k;e.key.mod=mod;e.key.down=true;SDL_PushEvent(&e);frame(d);e.type=SDL_EVENT_KEY_UP;e.key.down=false;SDL_PushEvent(&e);frame(d);}
static void type(SBDesktop *d,const char *value){SDL_Event e={0};e.type=SDL_EVENT_TEXT_INPUT;e.text.windowID=SDL_GetWindowID(d->ui.window);e.text.text=value;SDL_PushEvent(&e);frame(d);frame(d);}
static SBTarget *target(SBDesktop *d,const char *id){for(size_t i=0;i<d->target_count;++i)if(!strcmp(d->targets[i].id,id))return &d->targets[i];return NULL;}
static bool click(SBDesktop *d,const char *id){SBTarget *t=target(d,id);if(!t)return false;SDL_Event e={0};float x=t->bounds.x+t->bounds.w/2,y=t->bounds.y+t->bounds.h/2;e.type=SDL_EVENT_MOUSE_MOTION;e.motion.windowID=SDL_GetWindowID(d->ui.window);e.motion.x=x;e.motion.y=y;SDL_PushEvent(&e);frame(d);e.type=SDL_EVENT_MOUSE_BUTTON_DOWN;e.button.windowID=SDL_GetWindowID(d->ui.window);e.button.button=SDL_BUTTON_LEFT;e.button.down=true;e.button.x=x;e.button.y=y;SDL_PushEvent(&e);frame(d);e.type=SDL_EVENT_MOUSE_BUTTON_UP;e.button.down=false;SDL_PushEvent(&e);frame(d);return true;}
static uint64_t styled_image(SBUi *ui,SBStyledText *text,float width,nk_flags alignment,bool uncached,float *height){
    ui->styled_cache_disabled=uncached;nk_input_begin(ui->ctx);nk_input_end(ui->ctx);
    if(nk_begin(ui->ctx,"Cache proof",nk_rect(0,0,1000,900),NK_WINDOW_NO_SCROLLBAR)){
        nk_style_set_font(ui->ctx,&ui->body->handle);*height=sb_ui_styled_height(ui,&ui->body->handle,text,width);
        nk_layout_row_static(ui->ctx,*height,width,1);sb_ui_styled_aligned(ui,&ui->body->handle,text,alignment);
    }nk_end(ui->ctx);sb_ui_draw(ui);
    SDL_Surface *raw=SDL_RenderReadPixels(ui->renderer,NULL),*pixels=raw ? SDL_ConvertSurface(raw,SDL_PIXELFORMAT_RGBA32) : NULL;uint64_t hash=0;
    if(pixels)for(int y=0;y<pixels->h;++y)hash=hash*1099511628211ULL^sb_hash((char *)pixels->pixels+y*pixels->pitch,(size_t)pixels->w*4);
    SDL_DestroySurface(pixels);SDL_DestroySurface(raw);SDL_RenderPresent(ui->renderer);return hash;
}
int main(int argc,char **argv){
    CHECK(argc==3 || argc==4);SBUi ui;OK(sb_ui_init(&ui,argv[1],1000,900,true));ui.space.dark=true;
    const char *samples[]={"Ein langer Absatz mit **kräftigen Wörtern**, *kursiven Teilen* und `Code`, der zuverlässig auf mehrere Zeilen umbricht.","*e*́ 👩*‍*💻 **سلام** · 知识 · Ελληνικά · שלום · ज्ञान · ≫⃒", "***Mehrere Stile*** und sehrlangeswortdasübermehrerezeilenlaufenmussdamitderumbruchgeprüftwird und Ende.","**fett** normal *kursiv* und `inline code`"};
    if(argc==3)for(unsigned scale=0;scale<2;++scale){OK(sb_ui_fonts(&ui,scale ? 2 : 1));CHECK(!ui.styled_cache);for(unsigned sample=0;sample<4;++sample){SBInline reader;SBStyledText styled;OK(sb_inline_init(&reader,samples[sample],strlen(samples[sample])));OK(sb_inline_styled(&reader,0,strlen(samples[sample]),&styled));
        for(unsigned w=0;w<2;++w)for(unsigned a=0;a<3;++a){nk_flags align=a==0 ? NK_TEXT_LEFT : a==1 ? NK_TEXT_CENTERED : NK_TEXT_RIGHT;float h0,h1,h2;float width=w ? 650 : 240;
            uint64_t original=styled_image(&ui,&styled,width,align,true,&h0),cold=styled_image(&ui,&styled,width,align,false,&h1),warm=styled_image(&ui,&styled,width,align,false,&h2);
            CHECK(original && original==cold && cold==warm && h0==h1 && h1==h2);
        }sb_styled_free(&styled);sb_inline_free(&reader);
    }}sb_ui_shutdown(&ui);
    char root[SB_PATH_CAP],suffix[80];snprintf(suffix,sizeof(suffix),"navigation-%lu-%lu",sb_process_id(),(unsigned long)time(NULL));OK(sb_path_join(root,sizeof(root),argv[2],suffix));
    SBDesktop d;OK(sb_desktop_init(&d,root,argv[1],true));OK(sb_app_new_project(&d.model,"one","Navigation",NULL));OK(sb_app_new_note(&d.model,"knowledge","a","Alpha"));frame(&d);frame(&d);
    d.reduced_motion=true;sb_desktop_focus_start(&d);CHECK(!strcmp(d.focus,"galaxy"));char previous[SB_PATH_CAP];strcpy(previous,d.model.path);SBStar *graph=d.graph.stars;
    key(&d,SDLK_LEFT,0);if(!strcmp(previous,d.model.path))key(&d,SDLK_RIGHT,0);CHECK(strcmp(previous,d.model.path) && d.graph.stars==graph);
    OK(sb_app_request(&d.model,SB_ACT_NOTE,"knowledge/a.md"));frame(&d);frame(&d);
    d.browser=false;key(&d,SDLK_F,MOD);type(&d,"Alpha");CHECK(d.search_session && d.browser && !strcmp(d.search,"Alpha"));
    SBTarget *clear=target(&d,"clear-search");CHECK(clear);CHECK(fabsf(d.search_bounds.x+d.search_bounds.w-clear->bounds.x-clear->bounds.w/2-20*d.ui.scale)<1);
    for(unsigned i=0;i<25 && strcmp(d.focus,"note:knowledge/a.md");++i)key(&d,SDLK_TAB,0);
    CHECK(!strcmp(d.focus,"note:knowledge/a.md") && d.search_session && !strcmp(d.search,"Alpha"));
    key(&d,SDLK_ESCAPE,0);CHECK(!d.browser && !d.search_session && !d.search[0]);
    d.browser=true;key(&d,SDLK_F,MOD);type(&d,"Alpha");key(&d,SDLK_ESCAPE,0);CHECK(d.browser && !d.search_session && !d.search[0]);
    /* Search restores both panes, including an initially expanded reader.
       Clearing and starting again must remember the restored state anew. */
    for(unsigned state=0;state<4;++state) {
        bool browser=(state&1)!=0,expanded=(state&2)!=0;
        d.browser=browser;d.expanded=expanded;frame(&d);
        key(&d,SDLK_F,MOD);type(&d,"Alpha");
        CHECK(d.search_session && d.browser && !d.expanded);
        CHECK(click(&d,"clear-search"));
        CHECK(!d.search_session && !d.search[0] && d.browser==browser && d.expanded==expanded);
        type(&d,"Alpha");CHECK(d.search_session && d.browser && !d.expanded);
        key(&d,SDLK_ESCAPE,0);
        CHECK(!d.search_session && !d.search[0] && d.browser==browser && d.expanded==expanded);
    }
    d.expanded=false;
    d.browser=false;key(&d,SDLK_F,MOD);type(&d,"Alpha");CHECK(click(&d,"reader"));CHECK(!d.browser && !d.search_session && !d.search[0]);
    /* Leaving search through unused chrome must restore the prior list state. */
    key(&d,SDLK_F,MOD);type(&d,"Alpha");
    SDL_Event outside={0};outside.type=SDL_EVENT_MOUSE_BUTTON_DOWN;
    outside.button.windowID=SDL_GetWindowID(d.ui.window);outside.button.button=SDL_BUTTON_LEFT;
    outside.button.down=true;outside.button.x=4;outside.button.y=4;SDL_PushEvent(&outside);frame(&d);
    outside.type=SDL_EVENT_MOUSE_BUTTON_UP;outside.button.down=false;SDL_PushEvent(&outside);frame(&d);
    CHECK(!d.search_session && !d.browser && !d.search[0]);
    key(&d,SDLK_F,MOD);type(&d,"Alpha");CHECK(click(&d,"clear-search"));if(d.browser || d.search_session || d.search[0])fprintf(stderr,"CLEAR state browser=%d session=%d query=%s focus=%s\n",d.browser,d.search_session,d.search,d.focus);CHECK(!d.browser && !d.search_session && !d.search[0]);
    key(&d,SDLK_E,MOD);CHECK(d.editing);SBTarget *edit=target(&d,"editor");CHECK(edit);SDL_Event motion={0};motion.type=SDL_EVENT_MOUSE_MOTION;motion.motion.windowID=SDL_GetWindowID(d.ui.window);motion.motion.x=edit->bounds.x+20;motion.motion.y=edit->bounds.y+20;SDL_PushEvent(&motion);frame(&d);
    CHECK(d.ui.pointer_text && d.ui.text_cursor && SDL_GetCursor()==d.ui.text_cursor && d.ui.ctx->style.edit.cursor_size<3);
    d.ui.caret_epoch=SDL_GetTicksNS();SDL_Delay(600);frame(&d);CHECK(d.ui.ctx->style.edit.cursor_normal.a==0);type(&d,"Text");CHECK(d.ui.ctx->style.edit.cursor_normal.a>0);
    key(&d,SDLK_S,MOD);CHECK(!sb_app_dirty(&d.model));
    d.expanded=true;d.browser=false;key(&d,SDLK_F,MOD);type(&d,"Alpha");
    OK(sb_app_new_project(&d.model,"two","Second project",NULL));frame(&d);CHECK(!d.search_session && !d.browser && !d.expanded);
    /* Real desktop routing: preedit does not search or submit a form. */
    key(&d,SDLK_F,MOD);CHECK(!d.search[0]);
    SDL_Event composing={0};composing.type=SDL_EVENT_TEXT_EDITING;
    composing.edit.windowID=SDL_GetWindowID(d.ui.window);composing.edit.text="Second";composing.edit.start=6;
    SDL_PushEvent(&composing);frame(&d);CHECK(!d.search[0] && d.search_session);
    key(&d,SDLK_ESCAPE,0);CHECK(d.search_session && !d.search[0]);
    key(&d,SDLK_ESCAPE,0);CHECK(!d.search_session);
    key(&d,SDLK_N,MOD);CHECK(d.form==SB_FORM_NOTE);size_t notes_before=d.model.notes.count;
    composing.edit.text="新しいノート";composing.edit.start=6;SDL_PushEvent(&composing);frame(&d);
    CHECK(!d.name[0]);key(&d,SDLK_RETURN,0);CHECK(d.form==SB_FORM_NOTE && d.model.notes.count==notes_before);
    type(&d,"新しいノート");CHECK(!strcmp(d.name,"新しいノート") && d.form==SB_FORM_NOTE);
    key(&d,SDLK_ESCAPE,0);CHECK(d.form==SB_FORM_NONE);
    /* Commit, Tab and more text in one batch keep the two fields separate. */
    key(&d,SDLK_N,MOD|SDL_KMOD_SHIFT);CHECK(d.form==SB_FORM_PROJECT);CHECK(!strcmp(d.focus,"form-name") && d.ui.input_area_applied && SDL_TextInputActive(d.ui.window));
    composing.edit.text="Alpha";composing.edit.start=5;SDL_PushEvent(&composing);frame(&d);CHECK(sb_ui_composition_active(&d.ui) && d.ui.input_area_applied && d.ui.input_status.code==SB_OK);
    SDL_Event committed={0};committed.type=SDL_EVENT_TEXT_INPUT;
    committed.text.windowID=SDL_GetWindowID(d.ui.window);committed.text.text="Alpha";
    SDL_Event tab={0};tab.type=SDL_EVENT_KEY_DOWN;tab.key.windowID=SDL_GetWindowID(d.ui.window);tab.key.key=SDLK_TAB;tab.key.down=true;
    SDL_Event after=committed;after.text.text="b";
    SDL_PushEvent(&committed);SDL_PushEvent(&tab);SDL_PushEvent(&after);frame(&d);if(strcmp(d.name,"Alpha"))fprintf(stderr,"ORDER name=%s id=%s focus=%s status=%d input=%d %s\n",d.name,d.id,d.focus,d.message.code,d.ui.input_status.code,d.ui.input_status.message);CHECK(!strcmp(d.name,"Alpha"));
    frame(&d);frame(&d);frame(&d);CHECK(!strcmp(d.focus,"form-id") && !strcmp(d.name,"Alpha") && strchr(d.id,'b'));
    key(&d,SDLK_ESCAPE,0);
    /* Ordinary typing has the same field-ownership guarantee as IME commits. */
    key(&d,SDLK_N,MOD|SDL_KMOD_SHIFT);CHECK(d.form==SB_FORM_PROJECT);
    committed.text.text="Gamma";after.text.text="d";
    SDL_PushEvent(&committed);SDL_PushEvent(&tab);SDL_PushEvent(&after);frame(&d);
    CHECK(!strcmp(d.name,"Gamma"));frame(&d);frame(&d);frame(&d);
    CHECK(!strcmp(d.name,"Gamma") && !strcmp(d.focus,"form-id") && strchr(d.id,'d'));
    key(&d,SDLK_ESCAPE,0);
    /* Printable key bursts must not add a frame for every character. */
    key(&d,SDLK_N,MOD|SDL_KMOD_SHIFT);CHECK(d.form==SB_FORM_PROJECT);
    committed.text.text="A";SDL_PushEvent(&committed);
    const char *burst[]={"a","b","c","d","e","f","g","h"};
    for(size_t i=0;i<8;++i){SDL_Event press=tab;press.key.key=(SDL_Keycode)burst[i][0];SDL_PushEvent(&press);SDL_Event letter=committed;letter.text.text=burst[i];SDL_PushEvent(&letter);press.type=SDL_EVENT_KEY_UP;press.key.down=false;SDL_PushEvent(&press);}
    frame(&d);frame(&d);CHECK(!strcmp(d.name,"Aabcdefgh"));
    key(&d,SDLK_ESCAPE,0);
    /* Focus and caret actions before input also establish a fresh binding. */
    key(&d,SDLK_N,MOD|SDL_KMOD_SHIFT);CHECK(d.form==SB_FORM_PROJECT);
    after.text.text="q";SDL_PushEvent(&tab);SDL_PushEvent(&after);frame(&d);frame(&d);frame(&d);
    CHECK(!d.name[0] && !strcmp(d.focus,"form-id") && strchr(d.id,'q'));
    char original_id[65],expected_id[65];snprintf(original_id,sizeof(original_id),"%s",d.id);snprintf(expected_id,sizeof(expected_id),"z%.63s",original_id);
    SDL_Event left=tab;left.key.key=SDLK_LEFT;after.text.text="z";
    SDL_PushEvent(&left);SDL_PushEvent(&after);left.type=SDL_EVENT_KEY_UP;left.key.down=false;SDL_PushEvent(&left);
    frame(&d);frame(&d);frame(&d);frame(&d);CHECK(!strcmp(d.id,expected_id));
    key(&d,SDLK_ESCAPE,0);
    /* Opening a form and immediately typing must bind to the new form. */
    SDL_Event create=tab;create.key.key=SDLK_N;create.key.mod=MOD|SDL_KMOD_SHIFT;
    after.text.text="Sofort";SDL_PushEvent(&create);SDL_PushEvent(&after);frame(&d);
    CHECK(d.form==SB_FORM_PROJECT && sb_desktop_animating(&d));
    frame(&d);frame(&d);frame(&d);CHECK(!strcmp(d.name,"Sofort"));
    key(&d,SDLK_ESCAPE,0);
    /* A fresh desktop with a remembered project has keyboard focus itself. */
    sb_desktop_free(&d);OK(sb_desktop_init(&d,root,argv[1],true));
    CHECK(d.model.has_project && d.keyboard && !strcmp(d.focus,"galaxy"));frame(&d);frame(&d);
    strcpy(previous,d.model.path);key(&d,SDLK_LEFT,0);if(!strcmp(previous,d.model.path))key(&d,SDLK_RIGHT,0);
    CHECK(strcmp(previous,d.model.path));
    key(&d,SDLK_N,MOD|SDL_KMOD_SHIFT);type(&d,"Drittes Projekt");key(&d,SDLK_RETURN,0);frame(&d);
    CHECK(d.form==SB_FORM_NONE && !strcmp(d.model.project.id,"drittes-projekt") && !strcmp(d.focus,"galaxy"));
    sb_desktop_free(&d);printf("%u navigation assertions passed; %s.\n",checks,argc==3 ? "48 cached/uncached raster cases" : "navigation-only run");return 0;
}
