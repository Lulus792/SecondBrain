#include "desktop.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void frame(SBDesktop *d) {
    SDL_Event event;
    nk_input_begin(d->ui.ctx);
    while (SDL_PollEvent(&event)) sb_desktop_event(d, &event);
    sb_desktop_tick(d,1.0f/60);
    nk_input_end(d->ui.ctx);
    sb_desktop_frame(d);
    sb_ui_draw(&d->ui);
    SDL_RenderPresent(d->ui.renderer);
    sb_desktop_apply(d);
}
static bool click(SBDesktop *d, const char *id) {
    struct nk_rect bounds = nk_rect(0, 0, 0, 0);
    bool found = false;
    for (size_t i = d->target_count; i > 0; --i)
        if (!strcmp(d->targets[i - 1].id, id)) { bounds = d->targets[i - 1].bounds; found = true; break; }
    if (!found) { fprintf(stderr, "Control not found: %s\n", id); return false; }
    SDL_Event event = {0};
    event.type = SDL_EVENT_MOUSE_MOTION; event.motion.windowID = SDL_GetWindowID(d->ui.window);
    event.motion.x = bounds.x + bounds.w / 2; event.motion.y = bounds.y + bounds.h / 2;
    SDL_PushEvent(&event); frame(d);
    event.type = SDL_EVENT_MOUSE_BUTTON_DOWN; event.button.windowID = SDL_GetWindowID(d->ui.window);
    event.button.button = SDL_BUTTON_LEFT; event.button.down = true;
    event.button.x = bounds.x + bounds.w / 2; event.button.y = bounds.y + bounds.h / 2;
    SDL_PushEvent(&event); frame(d);
    event.type = SDL_EVENT_MOUSE_BUTTON_UP; event.button.down = false;
    SDL_PushEvent(&event); frame(d);
    frame(d);
    return true;
}
static void key(SBDesktop *d, SDL_Keycode key, SDL_Keymod modifiers) {
    SDL_Event event = {0};
    event.type = SDL_EVENT_KEY_DOWN; event.key.windowID = SDL_GetWindowID(d->ui.window);
    event.key.key = key; event.key.mod = modifiers; event.key.down = true;
    SDL_PushEvent(&event); frame(d);
    event.type = SDL_EVENT_KEY_UP; event.key.down = false;
    SDL_PushEvent(&event); frame(d);
}
static void type(SBDesktop *d, const char *text) {
    size_t remaining=strlen(text);
    while (remaining) {
        char chunk[NK_INPUT_MAX]; size_t bytes=remaining<NK_INPUT_MAX-1 ? remaining : NK_INPUT_MAX-1;
        while (bytes && ((unsigned char)text[bytes]&0xc0)==0x80) --bytes;
        if (!bytes) return;
        memcpy(chunk,text,bytes); chunk[bytes]=0;
        SDL_Event event={0}; event.type=SDL_EVENT_TEXT_INPUT; event.text.windowID=SDL_GetWindowID(d->ui.window); event.text.text=chunk;
        SDL_PushEvent(&event); frame(d); frame(d); text+=bytes; remaining-=bytes;
    }
}
static bool replace(SBDesktop *d, const char *id, const char *text) {
#ifdef __APPLE__
    SDL_Keymod modifier = SDL_KMOD_GUI;
#else
    SDL_Keymod modifier = SDL_KMOD_CTRL;
#endif
    if (!click(d,id)) return false;
    key(d,SDLK_A,modifier);
    if (*text) type(d,text); else key(d,SDLK_BACKSPACE,0);
    return true;
}
static bool capture(SBDesktop *d, const char *directory, const char *name) {
    char path[SB_PATH_CAP];
    if (sb_path_join(path, sizeof(path), directory, name).code != SB_OK) return false;
    /* ReadPixels is called before Present, using the same render path as the app. */
    nk_input_begin(d->ui.ctx); nk_input_end(d->ui.ctx);
    sb_desktop_frame(d); sb_ui_draw(&d->ui);
    SBStatus result = sb_ui_capture(&d->ui, path);
    SDL_RenderPresent(d->ui.renderer);
    return result.code == SB_OK;
}
int sb_desktop_self_test(SBDesktop *d, const char *directory) {
    unsigned checks = 0;
    char path[SB_PATH_CAP], *text = NULL;
    size_t length = 0;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr, "GUI FAIL line %d: %s (form=%d, path=%s, message=%s)\n", \
    __LINE__, #x, d->form, d->model.path, d->message.message); capture(d,directory,"failure.bmp"); return 1; } } while (0)
    frame(d); frame(d);
    CHECK(click(d, "new-project"));
    CHECK(d->form == SB_FORM_PROJECT);
    CHECK(click(d, "form-name")); type(d, "Beispielprojekt ü");
    CHECK(!strcmp(d->name, "Beispielprojekt ü") && !strcmp(d->id, "beispielprojekt-ue"));
    CHECK(click(d, "submit"));
    CHECK(d->model.has_project && !strcmp(d->model.project.id, "beispielprojekt-ue"));
    CHECK(click(d, "new-note"));
    CHECK(d->form == SB_FORM_NOTE);
    CHECK(click(d, "form-name")); type(d, "Energie");
    CHECK(click(d, "submit"));
    CHECK(!strcmp(d->model.path, "knowledge/energie.md") && d->editing);
    CHECK(replace(d, "editor", "# Energie und ihre Bedeutung für die Simulation in Beispielprojekt\n\nMessung ü. Ein belegter Befund.\n"));
    CHECK(sb_app_dirty(&d->model) && strstr(d->model.editor, "Messung ü."));
    CHECK(click(d, "save"));
    CHECK(!sb_app_dirty(&d->model));
    CHECK(click(d,"read"));
    CHECK(d->expanded && !d->editing && !sb_app_dirty(&d->model));
    CHECK(click(d,"expand") && !d->expanded);
    CHECK(capture(d, directory, "large-lumen.bmp"));
    CHECK(click(d, "search")); type(d, "Messung");
    CHECK(!strcmp(d->search, "Messung") && d->hits.count == 1);
    CHECK(replace(d, "search", ""));
    CHECK(click(d, "edit"));
    CHECK(replace(d, "editor", "# Energie\n\nUngespeicherte Änderung.\n"));
    CHECK(click(d,"list"));
    CHECK(click(d, "section:overview"));
    CHECK(click(d, "note:STATE.md"));
    CHECK(d->model.guard && !strcmp(d->model.path, "knowledge/energie.md"));
    CHECK(click(d, "guard-cancel"));
    CHECK(!d->model.guard && sb_app_dirty(&d->model));
    CHECK(click(d, "note:STATE.md"));
    CHECK(click(d, "guard-discard"));
    CHECK(!strcmp(d->model.path, "STATE.md") && !sb_app_dirty(&d->model));
#ifdef __APPLE__
    key(d, SDLK_C, SDL_KMOD_GUI | SDL_KMOD_SHIFT);
#else
    key(d, SDLK_C, SDL_KMOD_CTRL | SDL_KMOD_SHIFT);
#endif
    CHECK(d->form == SB_FORM_CONTEXT && d->context && strstr(d->context, "PROJECT.md"));
    CHECK(click(d, "copy-context"));
    text = SDL_GetClipboardText();
    CHECK(text && strstr(text, "Beispielprojekt ü")); SDL_free(text); text = NULL;
    key(d, SDLK_ESCAPE, 0);
    CHECK(d->form == SB_FORM_NONE);

    CHECK(sb_path_join(path, sizeof(path), directory, "Reference.md").code == SB_OK);
    char reference[4096]="# Referenzquelle\n\nEin Originalbefund.\n";
    for (unsigned i=0;i<40;++i) strcat(reference,"\nWeitere Beobachtung aus der Originalquelle.\n");
    CHECK(sb_fs_write_new(path,reference,strlen(reference)).code == SB_OK);
    CHECK(click(d, "note:SOURCES.md"));
    CHECK(click(d, "edit"));
    CHECK(replace(d, "editor", "# Quellen\n\n[Referenz](../../Reference.md)\n"));
    CHECK(click(d, "save") && click(d, "read"));
    CHECK(click(d, "link:0"));
    CHECK(d->model.source && strstr(d->model.source, "Originalbefund"));
    SDL_Event wheel={0}; wheel.type=SDL_EVENT_MOUSE_WHEEL; wheel.wheel.y=-0.5f;
    for (size_t i=0;i<d->target_count;++i) if (!strcmp(d->targets[i].id,"reader")) {
        wheel.wheel.mouse_x=d->targets[i].bounds.x+30; wheel.wheel.mouse_y=d->targets[i].bounds.y+30;
    }
    SDL_PushEvent(&wheel); frame(d);
    CHECK(d->scrolling[0].position > 0 && d->scrolling[0].position < 25);
    float first_scroll=d->scrolling[0].position;
    frame(d); CHECK(d->scrolling[0].position > first_scroll);
    for (unsigned i=0;i<35;++i) frame(d);
    CHECK(d->scrolling[0].applied == 25);
    wheel.wheel.y=1; SDL_PushEvent(&wheel); frame(d);
    CHECK(d->scrolling[0].position < 25);
    for (unsigned i=0;i<35;++i) frame(d);
    CHECK(d->scrolling[0].applied == 0 && !d->scrolling[0].active);
    CHECK(d->model.source && !strcmp(d->model.source,reference));
    /* Repeated wheel input at the lower edge must not move the stored position up. */
    wheel.wheel.y=-1000; SDL_PushEvent(&wheel); frame(d);
    for (unsigned i=0;i<45;++i) frame(d);
    nk_uint bottom=d->scrolling[0].applied;
    CHECK(bottom>0 && bottom==(nk_uint)d->scrolling[0].maximum);
    for (unsigned i=0;i<8;++i) {
        wheel.wheel.y=-1; SDL_PushEvent(&wheel); frame(d);
        CHECK(d->scrolling[0].applied==bottom && d->scrolling[0].destination==(float)bottom);
    }
    for (unsigned i=0;i<45;++i) frame(d);
    CHECK(d->scrolling[0].applied==bottom && d->scrolling[0].elastic==0);
    CHECK(!strcmp(d->model.source,reference));


    CHECK(click(d, "source-back"));
    CHECK(!d->model.source && !strcmp(d->model.path, "SOURCES.md"));

    CHECK(click(d, "new-project"));
    CHECK(click(d, "form-name")); type(d, "Zweites");
    CHECK(click(d, "submit"));
    CHECK(d->model.projects.count == 2 && !strcmp(d->model.project.id, "zweites"));
    CHECK(click(d, "project-picker"));
    CHECK(click(d, "project:beispielprojekt-ue"));
    CHECK(!strcmp(d->model.project.id, "beispielprojekt-ue"));
    CHECK(click(d, "section:knowledge"));
    CHECK(click(d, "note:knowledge/energie.md"));
    CHECK(strstr(d->model.editor, "Messung ü.") && !sb_app_dirty(&d->model));
    CHECK(sb_note_load(&d->model.project, d->model.path, &text, NULL).code == SB_OK);
    CHECK(strstr(text, "Messung ü.") && !strstr(text, "Ungespeicherte Änderung"));
    free(text); text = NULL;
    CHECK(click(d, "settings"));
    CHECK(click(d, "theme"));
    CHECK(!d->ui.dark);
    CHECK(capture(d, directory, "settings-light.bmp"));
    CHECK(click(d, "theme") && d->ui.dark);
    CHECK(click(d, "font-plus") && click(d, "font-plus"));
    CHECK(d->ui.scale == 1.5f);
    key(d, SDLK_ESCAPE, 0);
    SDL_SetWindowSize(d->ui.window, 780, 560);
    frame(d); frame(d);
    CHECK(capture(d, directory, "small-dark.bmp"));
    CHECK(sb_ui_fonts(&d->ui,2).code==SB_OK); frame(d); frame(d);
    struct nk_window *bar=nk_window_find(d->ui.ctx,"Lumen tools");
    CHECK(bar!=NULL);
    const char *toolbar[]={"project-picker","search","clear-search","list","filter","new-note","new-project","settings","help"};
    for (size_t k=0;k<sizeof(toolbar)/sizeof(*toolbar);++k) {
        bool found=false;
        for (size_t i=0;i<d->target_count;++i) if (!strcmp(d->targets[i].id,toolbar[k])) {
            struct nk_rect r=d->targets[i].bounds;
            found=r.x>=bar->bounds.x && r.y>=bar->bounds.y && r.x+r.w<=bar->bounds.x+bar->bounds.w+0.5f && r.y+r.h<=bar->bounds.y+bar->bounds.h+0.5f;
        }
        CHECK(found);
    }
    CHECK(capture(d,directory,"small-200.bmp"));
    CHECK(sb_ui_fonts(&d->ui,1.5f).code==SB_OK); frame(d); frame(d);

    CHECK(!sb_app_dirty(&d->model) && !strcmp(d->model.path, "knowledge/energie.md"));
    CHECK(sb_path_join(path, sizeof(path), directory, "Reference.md").code == SB_OK);
    CHECK(sb_fs_read(path, &text, &length).code == SB_OK);
    CHECK(!strcmp(text,reference));
    free(text); text = NULL;
    SDL_SetWindowSize(d->ui.window, 1336, 840);
    CHECK(sb_ui_fonts(&d->ui, 1).code == SB_OK);
    frame(d); frame(d);
    CHECK(click(d, "edit"));
    CHECK(replace(d, "editor", "# Energie\n\nMeine Fassung trotz Konflikt.\n"));
    CHECK(sb_note_save(&d->model.project, d->model.path, "# Extern\n", d->model.revision, NULL).code == SB_OK);
    CHECK(click(d, "save"));
    CHECK(d->message.code == SB_CONFLICT && sb_app_dirty(&d->model));
    CHECK(click(d, "actions") && click(d, "save-copy"));
    CHECK(!sb_app_dirty(&d->model) && strstr(d->model.path, "knowledge/kopie-") && strstr(d->model.editor, "trotz Konflikt"));
    CHECK(sb_note_load(&d->model.project, "knowledge/energie.md", &text, NULL).code == SB_OK);
    CHECK(!strcmp(text, "# Extern\n")); free(text); text = NULL;
    CHECK(click(d, "actions") && click(d, "archive"));
    CHECK(!strncmp(d->model.path,"archive/kopie-",14) && !strcmp(d->section,"all"));
    CHECK(click(d,"filter")); CHECK(click(d,"section:archive"));
    CHECK(!strcmp(d->section,"archive"));
    CHECK(click(d,"filter")); CHECK(click(d,"section:all"));
    CHECK(!strcmp(d->section,"all"));
    CHECK(click(d, "edit"));
    CHECK(replace(d, "editor", "# Übergabe\n\nBeim Beenden gespeichert.\n"));
    SDL_Event quit = {0}; quit.type = SDL_EVENT_QUIT;
    SDL_PushEvent(&quit); frame(d); frame(d);
    CHECK(d->model.guard && !d->model.quit);
    CHECK(click(d, "guard-cancel") && !d->model.quit && sb_app_dirty(&d->model));
    SDL_PushEvent(&quit); frame(d); frame(d);
    CHECK(click(d, "guard-save") && d->model.quit && !sb_app_dirty(&d->model));
    CHECK(sb_note_load(&d->model.project, d->model.path, &text, NULL).code == SB_OK);
    CHECK(strstr(text, "Beim Beenden gespeichert")); free(text);
    printf("%u desktop assertions passed through SDL mouse, keyboard and clipboard events.\nScreenshots: %s\n", checks, directory);
    return 0;
#undef CHECK
}
