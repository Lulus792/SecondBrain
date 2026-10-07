#include "desktop.h"
#include "platform.h"
#include "version.h"
#include "notices.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
#define MOD SDL_KMOD_GUI
#else
#define MOD SDL_KMOD_CTRL
#endif
static struct { Uint64 layout,render; unsigned frames,drawn; } timing;
static bool observe_copy;
static char *copied_result;
static void frame_run(SBDesktop *d,bool draw) {
    Uint64 start=SDL_GetPerformanceCounter();
    SDL_Event event;
    nk_input_begin(d->ui.ctx);
    while (SDL_PollEvent(&event)) sb_desktop_event(d, &event);
    sb_desktop_tick(d,1.0f/60);
    nk_input_end(d->ui.ctx);
    sb_desktop_frame(d); Uint64 ready=SDL_GetPerformanceCounter();
    if (draw) { sb_ui_draw(&d->ui); SDL_RenderPresent(d->ui.renderer); ++timing.drawn; }
    else nk_clear(d->ui.ctx);
    Uint64 presented=SDL_GetPerformanceCounter();
    timing.layout+=ready-start; timing.render+=presented-ready;
    if (++timing.frames%100==0) {
        double frequency=(double)SDL_GetPerformanceFrequency();
        fprintf(stderr,"Keyboard frames %u (%u drawn): layout %.2fs, raster/present %.2fs.\n",timing.frames,timing.drawn,timing.layout/frequency,timing.render/frequency);
    }
    sb_desktop_apply(d);
}
static void frame(SBDesktop *d) { frame_run(d,true); }
static void key(SBDesktop *d, SDL_Keycode code, SDL_Keymod mod) {
    SDL_Event e = {0}; e.type = SDL_EVENT_KEY_DOWN; e.key.key = code; e.key.mod = mod;
    e.key.windowID = SDL_GetWindowID(d->ui.window); e.key.down = true;
    /* Both phases update real input, layout, native snapshot and model. The
       completed key-up state is rasterized; intermediate commands are discarded. */
    SDL_PushEvent(&e); frame_run(d,false);
    if (observe_copy) { SDL_free(copied_result); copied_result=SDL_GetClipboardText(); observe_copy=false; }
    e.type = SDL_EVENT_KEY_UP; e.key.down = false; SDL_PushEvent(&e); frame(d);
}
static void type(SBDesktop *d, const char *text) {
    size_t remaining=strlen(text);
    while (remaining) {
        char chunk[NK_INPUT_MAX]; size_t bytes=remaining<NK_INPUT_MAX-1 ? remaining : NK_INPUT_MAX-1;
        while (bytes && ((unsigned char)text[bytes]&0xc0)==0x80) --bytes;
        if (!bytes) return;
        memcpy(chunk,text,bytes); chunk[bytes]=0;
        SDL_Event e={0}; e.type=SDL_EVENT_TEXT_INPUT; e.text.text=chunk;
        e.text.windowID=SDL_GetWindowID(d->ui.window); SDL_PushEvent(&e); frame(d); frame(d);
        text+=bytes; remaining-=bytes;
    }
}
static void replace(SBDesktop *d, const char *text) {
    key(d,SDLK_A,MOD);
    if (*text) type(d,text); else key(d,SDLK_BACKSPACE,0);
}
static bool reach(SBDesktop *d, const char *id) {
    /* Search dismissal can shrink the target list mid-traversal. Keep a fixed
       budget that allows reaching controls in the newly restored layout. */
    size_t limit=2*d->target_count+4;
    for (size_t i = 0; i <= limit; ++i) {
        if (!strcmp(d->focus, id)) return true;
        key(d, SDLK_TAB, 0);
    }
    fprintf(stderr,"Unreachable focus: %s (current %s)\n",id,d->focus); return false;
}
static bool activate(SBDesktop *d, const char *id) {
    if (!reach(d,id)) return false;
    observe_copy=!strncmp(id,"copy-",5);
    key(d,SDLK_RETURN,0); return true;
}
static char *take_copy(void) { char *text=copied_result;copied_result=NULL;return text; }
static bool same_clipboard_text(const char *actual, const char *expected) {
    if (!actual) return false;
    while (*expected) {
        /* Windows' CF_UNICODETEXT uses CRLF even when SDL is given LF. */
        if (*expected=='\n' && actual[0]=='\r' && actual[1]=='\n') ++actual;
        if (*actual!=*expected) return false;
        ++actual; ++expected;
    }
    return !*actual;
}
int sb_desktop_keyboard_test(SBDesktop *d, const char *directory) {
    timing.layout=timing.render=0; timing.frames=timing.drawn=0;
    unsigned checks = 0; char path[SB_PATH_CAP], saved[SB_PATH_CAP]; char *text = NULL;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"KEYBOARD FAIL %d: %s (focus=%s form=%d path=%s: %s)\n",__LINE__,#x,d->focus,d->form,d->model.path,d->message.message); return 1; } } while (0)
    frame(d); frame(d);
    key(d,SDLK_N,MOD|SDL_KMOD_SHIFT);
    CHECK(d->form == SB_FORM_PROJECT && !strcmp(d->focus,"form-name"));
    CHECK(SDL_TextInputActive(d->ui.window));
    type(d,"Tastatur ü");
    CHECK(!strcmp(d->name,"Tastatur ü"));
    key(d,SDLK_TAB,0); CHECK(!strcmp(d->focus,"form-id"));
    key(d,SDLK_TAB,0); CHECK(!strcmp(d->focus,"form-repo"));
    type(d,directory);
    CHECK(!strcmp(d->repository,directory));
    key(d,SDLK_TAB,0); CHECK(!strcmp(d->focus,"cancel-footer"));
    key(d,SDLK_TAB,0); CHECK(!strcmp(d->focus,"submit"));
    key(d,SDLK_RETURN,0);
    CHECK(d->model.has_project && !strcmp(d->model.project.id,"tastatur-ue") && d->form == SB_FORM_NONE);
    key(d,SDLK_N,MOD); CHECK(SDL_TextInputActive(d->ui.window)); type(d,"Erkenntnis");
    CHECK(reach(d,"section-choice:inbox"));
    key(d,SDLK_SPACE,0); CHECK(d->note_section == 1);
    CHECK(activate(d,"submit"));
    CHECK(!strcmp(d->model.path,"inbox/erkenntnis.md") && d->editing);
    frame(d);
    CHECK(SDL_TextInputActive(d->ui.window));
    type(d,"Äther ü"); CHECK(strstr(d->model.editor,"Äther ü"));
    replace(d,"# Erkenntnis\n\n[Stand](../STATE.md)\nEine Eingabe ü.\n");
    CHECK(sb_app_dirty(&d->model) && strstr(d->model.editor,"Eingabe ü"));
    key(d,SDLK_S,MOD);
    CHECK(!sb_app_dirty(&d->model) && d->graph.edge_count);
    CHECK(reach(d,"editor")); key(d,SDLK_I,SDL_KMOD_CTRL);
    CHECK(sb_app_dirty(&d->model) && strchr(d->model.editor,'\t'));
    key(d,SDLK_Z,MOD); CHECK(!sb_app_dirty(&d->model));
#ifdef __APPLE__
    key(d,SDLK_Z,MOD|SDL_KMOD_SHIFT);
#else
    key(d,SDLK_Y,MOD);
#endif
    CHECK(sb_app_dirty(&d->model) && strchr(d->model.editor,'\t'));
    key(d,SDLK_Z,MOD); CHECK(!sb_app_dirty(&d->model));
    key(d,SDLK_F,MOD); CHECK(activate(d,"section:all"));
    CHECK(reach(d,"editor"));
    key(d,SDLK_TAB,0); CHECK(!strcmp(d->focus,"project-picker"));
    key(d,SDLK_F6,0); CHECK(!strcmp(d->focus,"galaxy") && !d->card);
    size_t before = d->star;
    key(d,SDLK_LEFT,SDL_KMOD_SHIFT); CHECK(d->yaw < 0 && d->star == before);
    CHECK(d->view_yaw < 0 && d->view_yaw > d->yaw);
    float animated=d->view_yaw;
    frame(d); CHECK(d->view_yaw < animated && d->view_yaw > d->yaw);
    key(d,SDLK_PLUS,0); CHECK(d->zoom > 1);
    key(d,SDLK_HOME,0); CHECK(d->yaw == 0 && d->zoom == 1);
    for (unsigned i=0;i<45;++i) frame(d);
    CHECK(d->view_yaw == 0 && d->view_zoom == 1);
    d->reduced_motion=true; key(d,SDLK_LEFT,SDL_KMOD_SHIFT);
    CHECK(d->view_yaw == d->yaw); key(d,SDLK_HOME,0); d->reduced_motion=false;
    key(d,SDLK_F6,0); CHECK(!strcmp(d->focus,"editor") && d->card);
    replace(d,"# Erkenntnis\n\nVor dem Wechsel speichern.\n");
    key(d,SDLK_TAB,0); key(d,SDLK_F6,0);
    /* Arrow navigation opens immediately, while a draft still requires a decision. */
    strcpy(saved,d->model.path); before = d->star;
    key(d,SDLK_UP,0);
    if (!d->model.guard) key(d,SDLK_RIGHT,0);
    CHECK(d->star != before && !strcmp(d->model.path,saved));
    CHECK(d->model.guard && sb_app_dirty(&d->model));
    CHECK(!strcmp(d->focus,"guard-save"));
    key(d,SDLK_TAB,SDL_KMOD_SHIFT); CHECK(!strcmp(d->focus,"guard-cancel"));
    key(d,SDLK_RETURN,0); CHECK(!d->model.guard && sb_app_dirty(&d->model));
    key(d,SDLK_RETURN,0); CHECK(d->model.guard);
    CHECK(activate(d,"guard-save"));
    CHECK(!d->model.guard && !sb_app_dirty(&d->model) && strcmp(d->model.path,saved));
    CHECK(sb_note_load(&d->model.project,saved,&text,NULL).code == SB_OK && strstr(text,"Vor dem Wechsel speichern"));
    free(text); text = NULL;
    for (unsigned i=0;i<3 && strcmp(d->focus,"galaxy");++i) key(d,SDLK_F6,0);
    CHECK(!strcmp(d->focus,"galaxy"));
    char opened[SB_PATH_CAP]; strcpy(opened,d->model.path);
    key(d,SDLK_LEFT,0);
    if (!strcmp(opened,d->model.path)) key(d,SDLK_RIGHT,0);
    if (!strcmp(opened,d->model.path)) key(d,SDLK_DOWN,0);
    CHECK(strcmp(opened,d->model.path) && !d->model.guard && d->card);
    CHECK(d->flight<1);
    float flight_x=d->focus_x, flight_z=d->focus_z;
    for (unsigned i=0;i<15;++i) frame(d);
    CHECK(d->focus_x!=flight_x || d->focus_z!=flight_z);
    key(d,SDLK_F,MOD); type(d,"Vor dem Wechsel");
    CHECK(d->hits.count == 1 && d->browser && !strcmp(d->focus,"search"));
    CHECK(activate(d,"note:inbox/erkenntnis.md")); CHECK(!strcmp(d->model.path,saved));
    key(d,SDLK_E,MOD); replace(d,"# Erkenntnis\n\n[Original](../../../Original.md)\n"); key(d,SDLK_S,MOD); key(d,SDLK_E,MOD);
    CHECK(sb_path_join(path,sizeof(path),directory,"Original.md").code == SB_OK);
    const char *source = "# Original\n\nUnveränderte Quelle ü.\n";
    CHECK(sb_fs_write_new(path,source,strlen(source)).code == SB_OK);
    CHECK(activate(d,"link:0")); CHECK(d->model.source && strstr(d->model.source,"Quelle ü"));
    key(d,SDLK_ESCAPE,0); CHECK(!d->model.source && !strcmp(d->model.path,saved) && !strcmp(d->focus,"link:0"));
    char many[24000] = "# Viele Quellen\n\n";
    for (unsigned i = 0; i < 300; ++i) {
        char row[70]; snprintf(row,sizeof(row),"[Original %u](../../../Original.md)\n",i);
        strcat(many,row);
    }
    key(d,SDLK_E,MOD); replace(d,many); key(d,SDLK_S,MOD); key(d,SDLK_E,MOD);
    key(d,SDLK_F10,SDL_KMOD_SHIFT);
    CHECK(d->form==SB_FORM_ACTIONS && !strcmp(d->focus,"context"));
    key(d,SDLK_DOWN,0); CHECK(!strcmp(d->focus,"reload"));
    key(d,SDLK_UP,0); CHECK(!strcmp(d->focus,"context"));
    key(d,SDLK_UP,0); CHECK(!strcmp(d->focus,"help-actions"));
    key(d,SDLK_ESCAPE,0); CHECK(d->form==SB_FORM_NONE);
    CHECK(d->target_count > 300);
    CHECK(reach(d,"reader"));
    key(d,SDLK_PAGEDOWN,0);
    float first_scroll=d->scrolling[0].position;
    CHECK(first_scroll > 0 && first_scroll < 220);
    frame(d); CHECK(d->scrolling[0].position > first_scroll && d->scrolling[0].position < 220);
    for (unsigned i=0;i<35;++i) frame(d);
    CHECK(d->scrolling[0].applied == 220);
    d->reduced_motion=true; key(d,SDLK_PAGEDOWN,0);
    CHECK(d->scrolling[0].applied == 440 && !d->scrolling[0].active);
    d->reduced_motion=false; key(d,SDLK_PAGEUP,0);
    float reversed=d->scrolling[0].position;
    CHECK(reversed > 220 && reversed < 440);
    frame(d); CHECK(d->scrolling[0].position < reversed);

    key(d,SDLK_F6,0); CHECK(!strcmp(d->focus,"project-picker"));
    key(d,SDLK_TAB,SDL_KMOD_SHIFT); CHECK(!strcmp(d->focus,"link:299"));
    key(d,SDLK_RETURN,0); CHECK(d->model.source && strstr(d->model.source,"Quelle ü"));
    key(d,SDLK_ESCAPE,0); CHECK(!d->model.source && !strcmp(d->focus,"link:299"));
    key(d,SDLK_PAGEDOWN,0);
    key(d,SDLK_C,MOD|SDL_KMOD_SHIFT); CHECK(d->form == SB_FORM_CONTEXT);
    CHECK(activate(d,"copy-context"));
    text = take_copy(); CHECK(text && strstr(text,"Tastatur ü")); SDL_free(text); text = NULL;
    key(d,SDLK_ESCAPE,0); CHECK(d->form == SB_FORM_NONE);
    SDL_SetWindowSize(d->ui.window,780,560); frame(d); frame(d);
    key(d,SDLK_COMMA,MOD); CHECK(d->form == SB_FORM_SETTINGS);
    CHECK(activate(d,"font-plus") && activate(d,"font-plus") && d->ui.scale == 1.5f);
    CHECK(activate(d,"transparency") && d->solid);
    CHECK(activate(d,"about") && d->form == SB_FORM_ABOUT);
    CHECK(activate(d,"copy-version"));
    text=take_copy(); CHECK(text && same_clipboard_text(text,sb_build_info()) && !strstr(text,d->model.project.root)); SDL_free(text); text=NULL;
    CHECK(sb_path_join(path,sizeof(path),directory,"about-small.bmp").code == SB_OK);
    CHECK(sb_ui_capture(&d->ui,path).code == SB_OK);
    CHECK(activate(d,"notice-list") && d->form==SB_FORM_NOTICE_LIST);
    /* Resource checks cover all files; UI also exercises both large collections. */
    const size_t notice_samples[]={0,14,sb_notice_count()-2,sb_notice_count()-1};
    for (size_t sample=0;sample<sizeof(notice_samples)/sizeof(*notice_samples);++sample) {
        size_t i=notice_samples[sample];
        char notice_id[100]; snprintf(notice_id,sizeof(notice_id),"notice:%zu",i);
        CHECK(activate(d,notice_id) && d->form==SB_FORM_NOTICE_TEXT && d->notice_index==i);
        CHECK(d->notice && strlen(d->notice)>20);
        CHECK(activate(d,"copy-notice"));
        text=take_copy(); CHECK(same_clipboard_text(text,d->notice)); SDL_free(text); text=NULL;
        CHECK(reach(d,"reader"));
        key(d,SDLK_PAGEDOWN,0);
        CHECK(d->scrolling[2].maximum==0 || d->scrolling[2].destination>0);
        if (i==14) {
            CHECK(sb_path_join(path,sizeof(path),directory,"license-small.bmp").code==SB_OK);
            CHECK(sb_ui_capture(&d->ui,path).code==SB_OK);
        }
        CHECK(activate(d,"notices-back") && d->form==SB_FORM_NOTICE_LIST);
    }
    CHECK(sb_path_join(path,sizeof(path),directory,"licenses-small.bmp").code==SB_OK);
    CHECK(sb_ui_capture(&d->ui,path).code==SB_OK);
    CHECK(activate(d,"notices-about") && d->form==SB_FORM_ABOUT);
    CHECK(activate(d,"about-back") && d->form == SB_FORM_SETTINGS);
    CHECK(activate(d,"about") && d->form == SB_FORM_ABOUT);
    key(d,SDLK_ESCAPE,0); CHECK(d->form == SB_FORM_NONE);
    CHECK(activate(d,"filter") && d->form == SB_FORM_FILTER);
    CHECK(activate(d,"section:all") && d->form == SB_FORM_NONE);
    key(d,SDLK_F1,0); CHECK(d->form == SB_FORM_HELP);
    CHECK(activate(d,"cancel") && d->form == SB_FORM_NONE);
    key(d,SDLK_F6,0); key(d,SDLK_F6,0); key(d,SDLK_F6,0);
    CHECK(!strcmp(d->focus,"project-picker"));
    key(d,SDLK_TAB,SDL_KMOD_SHIFT); CHECK(!strcmp(d->focus,"link:299")); frame(d); frame(d);
    struct nk_window *detail = d->ui.ctx->begin;
    while (detail && strcmp(detail->name_string,"Detail")) detail = detail->next;
    struct nk_rect last = nk_rect(0,0,0,0);
    for (size_t i = 0; i < d->target_count; ++i) if (!strcmp(d->targets[i].id,"link:299")) last = d->targets[i].bounds;
    CHECK(detail && last.y >= detail->bounds.y && last.y+last.h <= detail->bounds.y+detail->bounds.h);
    CHECK(sb_path_join(path,sizeof(path),directory,"keyboard-small.bmp").code == SB_OK);
    nk_input_begin(d->ui.ctx); nk_input_end(d->ui.ctx); sb_desktop_frame(d); sb_ui_draw(&d->ui);
    CHECK(sb_ui_capture(&d->ui,path).code == SB_OK); SDL_RenderPresent(d->ui.renderer);
    strcpy(saved,d->model.path);
    key(d,SDLK_E,MOD); replace(d,"# Meine Fassung\n\nTrotz externer Änderung erhalten.\n");
    char *draft=malloc(strlen(d->model.editor)+1); CHECK(draft!=NULL); strcpy(draft,d->model.editor);
    key(d,SDLK_COMMA,MOD);
    CHECK(activate(d,"about") && activate(d,"notice-list") && activate(d,"notice:0"));
    key(d,SDLK_ESCAPE,0);
    CHECK(d->form==SB_FORM_NONE && sb_app_dirty(&d->model) && !strcmp(d->model.editor,draft) && !strcmp(d->model.path,saved));
    free(draft);
    CHECK(sb_note_save(&d->model.project,saved,"# Extern\n",d->model.revision,NULL).code == SB_OK);
    key(d,SDLK_S,MOD); CHECK(d->message.code == SB_CONFLICT && sb_app_dirty(&d->model));
    CHECK(activate(d,"actions")); CHECK(activate(d,"save-copy"));
    CHECK(strstr(d->model.path,"knowledge/kopie-") && !sb_app_dirty(&d->model));
    CHECK(sb_note_load(&d->model.project,saved,&text,NULL).code == SB_OK && !strcmp(text,"# Extern\n"));
    free(text); text = NULL;
    CHECK(activate(d,"actions") && d->form == SB_FORM_ACTIONS);
    CHECK(activate(d,"archive")); CHECK(!strncmp(d->model.path,"archive/",8));
    key(d,SDLK_O,MOD); CHECK(d->form == SB_FORM_WORKSPACE && !strcmp(d->focus,"form-folder"));
    CHECK(SDL_TextInputActive(d->ui.window));
    replace(d,d->model.workspace); key(d,SDLK_RETURN,0); CHECK(d->form == SB_FORM_NONE);
    CHECK(activate(d,"project-picker")); CHECK(activate(d,"project:tastatur-ue"));
    CHECK(d->model.has_project && d->form == SB_FORM_NONE);
    key(d,SDLK_Q,MOD); CHECK(d->model.quit);
    printf("%u assertions passed with keyboard-only SDL events; no mouse events injected.\n",checks);
    return 0;
#undef CHECK
}
