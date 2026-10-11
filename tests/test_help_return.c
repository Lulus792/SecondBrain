#include "desktop.h"
#include "platform.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"HELP RETURN %d: %s\n",__LINE__,#x); return 1; } } while (0)
#define OK(x) CHECK((x).code==SB_OK)
static void frame(SBDesktop *d) {
    nk_input_begin(d->ui.ctx);SDL_Event event;while(SDL_PollEvent(&event))sb_desktop_event(d,&event);
    sb_desktop_tick(d,1.0f/60);nk_input_end(d->ui.ctx);sb_desktop_frame(d);sb_ui_draw(&d->ui);SDL_RenderPresent(d->ui.renderer);sb_desktop_apply(d);
}
static void key(SBDesktop *d,SDL_Keycode code,SDL_Keymod modifiers) {
    SDL_Event event={0};event.type=SDL_EVENT_KEY_DOWN;event.key.key=code;event.key.mod=modifiers;event.key.down=true;
    sb_desktop_event(d,&event);frame(d);frame(d);
}
static bool reader_ready(SBDesktop *d) {
    Uint64 deadline=SDL_GetTicksNS()+UINT64_C(15000000000);
    for(;;){bool pending=false;
        for(size_t i=0;i<d->passive_count;++i)if(!strcmp(d->passive[i].id,"reader:preparing")){pending=true;break;}
        if(!pending)return true;
        if(SDL_GetTicksNS()>=deadline)return false;
        SDL_Delay(1);frame(d);
    }
}
static accesskit_node_id focused_node(SBDesktop *d) {
    accesskit_tree_update *tree=sb_accessibility_tree(d->accessibility);char *dump=accesskit_tree_update_debug(tree);accesskit_tree_update_free(tree);
    const char *p=dump ? strstr(dump,"focus: #") : NULL;accesskit_node_id id=p ? strtoull(p+8,NULL,10) : 0;accesskit_string_free(dump);return id;
}
static int capture(SBDesktop *d,const char *root,const char *name) {
    char path[SB_PATH_CAP];OK(sb_path_join(path,sizeof(path),root,name));OK(sb_ui_capture(&d->ui,path));printf("Help snapshot: %s\n",path);return 0;
}
static int excursion(SBDesktop *d,SBForm form,const char *focus,bool close_button) {
    d->form=form;strcpy(d->focus,focus);d->keyboard=true;frame(d);frame(d);
    strcpy(d->focus,focus);frame(d);CHECK(!strcmp(d->focus,focus));
    char before_focus[100];strcpy(before_focus,d->focus);SBStatus message=sb_error(SB_INVALID,"Bitte den Dateinamen prüfen.");d->message=message;frame(d);
    key(d,SDLK_F10,SDL_KMOD_SHIFT);CHECK(d->form==form);
    key(d,SDLK_F1,0);CHECK(d->form==SB_FORM_HELP);
    key(d,SDLK_F1,0);CHECK(d->form==SB_FORM_HELP);
    if(close_button){strcpy(d->activate,"cancel");frame(d);}else key(d,SDLK_ESCAPE,0);
    frame(d);
    CHECK(d->form==form && !strcmp(d->focus,before_focus));CHECK(d->message.code==message.code && !strcmp(d->message.message,message.message));
    CHECK(!strcmp(d->name,"Begonnene Notiz") && !strcmp(d->id,"eigener-name") && !strcmp(d->repository,"/existing/project"));
    CHECK(!strcmp(d->folder,"/existing/workspace") && !strcmp(d->backup_path,"/existing/backup.sbb") && !strcmp(d->restore_id,"restored"));
    return 0;
}
int main(int argc,char **argv) {
    CHECK(argc==3);char root[SB_PATH_CAP],suffix[80];snprintf(suffix,sizeof(suffix),"help-return-%lu",sb_process_id());OK(sb_path_join(root,sizeof(root),argv[2],suffix));
    SBDesktop d;OK(sb_desktop_init(&d,root,argv[1],true));OK(sb_app_new_project(&d.model,"help","Help",NULL));d.reduced_motion=true;frame(&d);
    d.command=SB_CMD_NEW_NOTE;sb_desktop_apply(&d);frame(&d);
    strcpy(d.name,"Begonnene Notiz");strcpy(d.id,"eigener-name");d.id_manual=true;strcpy(d.repository,"/existing/project");strcpy(d.folder,"/existing/workspace");strcpy(d.backup_path,"/existing/backup.sbb");strcpy(d.restore_id,"restored");d.note_section=2;
    CHECK(!excursion(&d,SB_FORM_NOTE,"form-name",false));CHECK(d.note_section==2 && d.id_manual);
    CHECK(!excursion(&d,SB_FORM_PROJECT,"form-repo",true));CHECK(!excursion(&d,SB_FORM_WORKSPACE,"form-folder",false));
    CHECK(!excursion(&d,SB_FORM_BACKUP,"backup-path",true));d.restore_checked=true;strcpy(d.checked_backup,d.backup_path);
    CHECK(!excursion(&d,SB_FORM_RESTORE,"restore-id",false));CHECK(d.restore_checked && !strcmp(d.checked_backup,d.backup_path));
    /* Native selection and replacement exercise the actual widget and queue. */
    d.form=SB_FORM_NOTE;strcpy(d.focus,"form-name");d.keyboard=true;frame(&d);frame(&d);accesskit_node_id node=focused_node(&d);CHECK(node>4);
    CHECK(sb_accessibility_submit(d.accessibility,node,ACCESSKIT_ACTION_SET_TEXT_SELECTION,NULL,9,0));frame(&d);
    CHECK(d.native_fields[1].anchor==9 && d.native_fields[1].caret==0);
    CHECK(!capture(&d,root,"form-before.bmp"));
    CHECK(sb_accessibility_submit(d.accessibility,node,ACCESSKIT_ACTION_SET_VALUE,"Stale help callback",0,0));key(&d,SDLK_F1,0);CHECK(!strcmp(d.name,"Begonnene Notiz"));
    CHECK(!capture(&d,root,"help.bmp"));
    key(&d,SDLK_ESCAPE,0);frame(&d);CHECK(d.form==SB_FORM_NOTE && !strcmp(d.focus,"form-name"));
    CHECK(d.native_fields[1].anchor==9 && d.native_fields[1].caret==0);
    CHECK(!capture(&d,root,"form-after.bmp"));
    node=focused_node(&d);CHECK(sb_accessibility_submit(d.accessibility,node,ACCESSKIT_ACTION_REPLACE_SELECTED_TEXT,"Neue",0,0));frame(&d);CHECK(!strcmp(d.name,"Neue Notiz"));
    CHECK(sb_accessibility_submit(d.accessibility,node,ACCESSKIT_ACTION_SET_VALUE,"A é 👩‍👩‍👧‍👦 אב Z",0,0));frame(&d);
    CHECK(sb_accessibility_submit(d.accessibility,node,ACCESSKIT_ACTION_SET_TEXT_SELECTION,NULL,10,3));frame(&d);
    CHECK(d.native_fields[1].anchor==12 && d.native_fields[1].caret==2);
    key(&d,SDLK_F1,0);key(&d,SDLK_ESCAPE,0);CHECK(d.native_fields[1].anchor==12 && d.native_fields[1].caret==2);
    node=focused_node(&d);CHECK(sb_accessibility_submit(d.accessibility,node,ACCESSKIT_ACTION_REPLACE_SELECTED_TEXT,"Ω",0,0));frame(&d);CHECK(!strcmp(d.name,"A Ω אב Z"));
    /* Large text in a small window makes the settings body genuinely scroll. */
    d.form=SB_FORM_SETTINGS;d.message=sb_ok();OK(sb_ui_fonts(&d.ui,1.5f));CHECK(SDL_SetWindowSize(d.ui.window,780,520) && SDL_SyncWindow(d.ui.window));
    int width,height;CHECK(SDL_GetWindowSize(d.ui.window,&width,&height) && width==780 && height==520);frame(&d);frame(&d);CHECK(d.scrolling[3].maximum>100);
    d.scrolling[3].position=d.scrolling[3].destination=d.scrolling[3].maximum/2;d.scrolling[3].active=true;frame(&d);float position=d.scrolling[3].position;
    key(&d,SDLK_F1,0);key(&d,SDLK_ESCAPE,0);CHECK(d.form==SB_FORM_SETTINGS && fabsf(d.scrolling[3].position-position)<1);frame(&d);CHECK(fabsf(d.scrolling[3].position-position)<1);
    /* A read-only modal document retains its separate reading bookmark. */
    d.context=calloc(1,16000);CHECK(d.context);for(unsigned i=0;i<100;++i)strcat(d.context,"Ein längerer Absatz mit **Wissen** und einer Quelle.\n\n");
    d.form=SB_FORM_CONTEXT;frame(&d);CHECK(reader_ready(&d));frame(&d);CHECK(d.scrolling[2].maximum>100);
    d.scrolling[2].position=d.scrolling[2].destination=d.scrolling[2].maximum/2;d.scrolling[2].active=true;frame(&d);position=d.scrolling[2].position;char reader_focus[100];strcpy(reader_focus,d.focus);
    key(&d,SDLK_F1,0);key(&d,SDLK_ESCAPE,0);CHECK(d.form==SB_FORM_CONTEXT && !strcmp(d.focus,reader_focus) && fabsf(d.scrolling[2].position-position)<1);
    d.form=SB_FORM_NONE;frame(&d);key(&d,SDLK_F10,SDL_KMOD_SHIFT);CHECK(d.form==SB_FORM_ACTIONS);key(&d,SDLK_DOWN,0);char focus[100];strcpy(focus,d.focus);
    key(&d,SDLK_F1,0);key(&d,SDLK_ESCAPE,0);frame(&d);CHECK(d.form==SB_FORM_ACTIONS && !strcmp(d.focus,focus));key(&d,SDLK_ESCAPE,0);CHECK(d.form==SB_FORM_NONE);
    key(&d,SDLK_F10,SDL_KMOD_SHIFT);strcpy(d.focus,"help-actions");strcpy(d.activate,"help-actions");frame(&d);frame(&d);CHECK(d.form==SB_FORM_HELP);
    key(&d,SDLK_ESCAPE,0);CHECK(d.form==SB_FORM_ACTIONS && !strcmp(d.focus,"help-actions"));key(&d,SDLK_ESCAPE,0);CHECK(d.form==SB_FORM_NONE);
    key(&d,SDLK_F1,0);key(&d,SDLK_ESCAPE,0);CHECK(d.form==SB_FORM_NONE);
    /* Help must not replace an unsaved-change decision. */
    d.model.guard=true;frame(&d);key(&d,SDLK_F1,0);CHECK(d.model.guard && d.form==SB_FORM_NONE);d.model.guard=false;
    /* Freeing the application during an excursion releases the saved selection. */
    d.command=SB_CMD_NEW_NOTE;sb_desktop_apply(&d);frame(&d);key(&d,SDLK_F1,0);CHECK(d.form==SB_FORM_HELP);sb_desktop_free(&d);
    printf("%u help/form/selection/scroll/shortcut assertions passed.\n",checks);return 0;
}
