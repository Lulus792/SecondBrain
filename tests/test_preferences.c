#include "desktop.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
static void frame(SBDesktop *d) {
    nk_input_begin(d->ui.ctx); nk_input_end(d->ui.ctx); sb_desktop_tick(d,1.0f/60);
    sb_desktop_frame(d); sb_ui_draw(&d->ui); SDL_RenderPresent(d->ui.renderer); sb_desktop_apply(d);
}
int main(int argc,char **argv) {
    unsigned checks=0; SBDesktop d; SBStatus status; char root[SB_PATH_CAP], workspace[SB_PATH_CAP], alternative[SB_PATH_CAP],config[SB_PATH_CAP],suffix[90],absolute[SB_PATH_CAP];
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"PREFERENCES FAIL %d: %s\n",__LINE__,#x); return 1; } } while (0)
#define OK(x) do { status=(x); CHECK(status.code==SB_OK); } while (0)
    CHECK(argc==3); snprintf(suffix,sizeof(suffix),"run-%lu-%lu",sb_process_id(),(unsigned long)time(NULL));
    OK(sb_path_join(root,sizeof(root),argv[2],suffix)); OK(sb_fs_mkdirs(root));
    OK(sb_path_join(workspace,sizeof(workspace),root,"Arbeitsordner ü"));
    OK(sb_path_join(alternative,sizeof(alternative),root,"anderer Ordner"));
    OK(sb_path_join(config,sizeof(config),root,"settings.conf"));
    OK(sb_desktop_init(&d,workspace,argv[1],true));
    OK(sb_desktop_preferences(&d,config,true));
    OK(sb_app_new_project(&d.model,"projekt","Projekt ü",NULL));
    OK(sb_app_new_note(&d.model,"knowledge","notiz","Notiz"));
    strcpy(d.model.editor,"# Notiz\n\nBleibt erhalten.\n"); OK(sb_app_save(&d.model));
    sb_ui_theme(&d.ui,false); OK(sb_ui_fonts(&d.ui,1.5f)); d.solid=true; d.reduced_motion=true;
    SDL_SetWindowSize(d.ui.window,780,560); frame(&d);
    OK(sb_desktop_store_preferences(&d)); OK(sb_fs_absolute(workspace,absolute,sizeof(absolute)));
    sb_desktop_free(&d);
    OK(sb_desktop_init(&d,alternative,argv[1],true));
    OK(sb_desktop_preferences(&d,config,false)); frame(&d);
    CHECK(!strcmp(d.model.workspace,absolute) && !strcmp(d.model.project.id,"projekt") && !strcmp(d.model.path,"knowledge/notiz.md"));
    CHECK(strstr(d.model.editor,"Bleibt erhalten") && !sb_app_dirty(&d.model));
    int w,h; SDL_GetWindowSize(d.ui.window,&w,&h);
    CHECK(w==780 && h==560 && d.ui.scale==1.5f && !d.ui.dark && d.solid && d.reduced_motion);
    /* Folder reply changes only the open form; an obsolete reply changes nothing. */
    d.form=SB_FORM_WORKSPACE; d.dialog_serial=42; strcpy(d.folder,"vorher"); frame(&d);
    SBDialogReply *reply=SDL_calloc(1,sizeof(*reply)); CHECK(reply!=NULL);
    reply->serial=41; strcpy(reply->value,"veraltet"); SDL_Event e={0}; e.type=sb_dialogs_event(d.dialogs); e.user.data1=reply;
    sb_desktop_event(&d,&e); CHECK(!strcmp(d.folder,"vorher"));
    reply=SDL_calloc(1,sizeof(*reply)); reply->serial=42; e.user.data1=reply;
    sb_desktop_event(&d,&e); CHECK(!strcmp(d.folder,"vorher"));
    reply=SDL_calloc(1,sizeof(*reply)); reply->serial=42; reply->error=true;
    strcpy(reply->value,"Testfehler"); e.user.data1=reply;
    sb_desktop_event(&d,&e); CHECK(d.message.code==SB_IO && !strcmp(d.folder,"vorher"));
    reply=SDL_calloc(1,sizeof(*reply)); reply->serial=42; strcpy(reply->value,absolute); e.user.data1=reply;
    sb_desktop_event(&d,&e); CHECK(!strcmp(d.folder,absolute) && !strcmp(d.focus,"form-folder"));
    d.form=SB_FORM_NONE; strcpy(d.folder,"vorher");
    reply=SDL_calloc(1,sizeof(*reply)); reply->serial=42; strcpy(reply->value,absolute); e.user.data1=reply;
    sb_desktop_event(&d,&e); CHECK(!strcmp(d.folder,"vorher"));
    d.command=SB_CMD_CANCEL; sb_desktop_apply(&d);
    d.command=SB_CMD_WORKSPACE; sb_desktop_apply(&d); strcpy(d.folder,"erneut");
    reply=SDL_calloc(1,sizeof(*reply)); reply->serial=42; strcpy(reply->value,absolute); e.user.data1=reply;
    sb_desktop_event(&d,&e); CHECK(!strcmp(d.folder,"erneut"));
    frame(&d);
    char screenshot[SB_PATH_CAP];
    OK(sb_path_join(screenshot,sizeof(screenshot),root,"folder-150.bmp"));
    OK(sb_ui_capture(&d.ui,screenshot));
    printf("Folder preview: %s\n",screenshot);
    OK(sb_ui_fonts(&d.ui,2)); frame(&d);
    for (unsigned i=0;i<8 && strcmp(d.focus,"choose-folder");++i) {
        SDL_Event key={0}; key.type=SDL_EVENT_KEY_DOWN; key.key.key=SDLK_TAB;
        nk_input_begin(d.ui.ctx); sb_desktop_event(&d,&key); nk_input_end(d.ui.ctx);
        frame(&d);
    }
    CHECK(!strcmp(d.focus,"choose-folder"));
    frame(&d); frame(&d); frame(&d);
    OK(sb_path_join(screenshot,sizeof(screenshot),root,"folder-200.bmp"));
    OK(sb_ui_capture(&d.ui,screenshot));
    printf("Large folder preview: %s\n",screenshot);
    /* Destruction drains queued replies, without accessing their old owner. */
    reply=SDL_calloc(1,sizeof(*reply)); reply->serial=42; e.user.data1=reply; CHECK(SDL_PushEvent(&e));
    sb_desktop_free(&d);
    OK(sb_desktop_init(&d,alternative,argv[1],true));
    OK(sb_desktop_preferences(&d,config,true));
    CHECK(strcmp(d.model.workspace,absolute) && !d.model.has_project && d.ui.scale==1.5f);
    sb_desktop_free(&d);
    /* A missing old workspace is preserved until the user chooses another one. */
    SBSettings settings; SBRevision revision;
    OK(sb_settings_load(config,&settings,&revision));
    OK(sb_path_join(settings.workspace,sizeof(settings.workspace),root,"nicht vorhanden"));
    OK(sb_settings_save(config,&settings,revision,NULL));
    OK(sb_desktop_init(&d,alternative,argv[1],true));
    CHECK(sb_desktop_preferences(&d,config,false).code==SB_NOT_FOUND && !d.settings_enabled);
    CHECK(sb_fs_kind(settings.workspace)==0);
    OK(sb_desktop_store_preferences(&d));
    SBSettings before; OK(sb_settings_load(config,&before,&revision));
    CHECK(!strcmp(settings.workspace,before.workspace));
    d.command=SB_CMD_WORKSPACE; sb_desktop_apply(&d); strcpy(d.folder,alternative);
    d.command=SB_CMD_SUBMIT; sb_desktop_apply(&d); CHECK(d.settings_enabled && d.form==SB_FORM_NONE);
    OK(sb_desktop_store_preferences(&d));
    OK(sb_settings_load(config,&before,&revision));
    OK(sb_fs_absolute(alternative,absolute,sizeof(absolute))); CHECK(!strcmp(before.workspace,absolute));
    sb_desktop_free(&d);
    printf("%u preferences assertions passed: restart, explicit workspace, isolated configuration and asynchronous replies.\n",checks);
    return 0;
}
