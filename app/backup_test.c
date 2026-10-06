#include "desktop.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __APPLE__
#define MOD SDL_KMOD_GUI
#else
#define MOD SDL_KMOD_CTRL
#endif
static void frame(SBDesktop *d) {
    SDL_Event e; nk_input_begin(d->ui.ctx);
    while (SDL_PollEvent(&e)) sb_desktop_event(d,&e);
    sb_desktop_tick(d,1.0f/60); nk_input_end(d->ui.ctx); sb_desktop_frame(d); sb_ui_draw(&d->ui);
    SDL_RenderPresent(d->ui.renderer); sb_desktop_apply(d);
}
static void key(SBDesktop *d,SDL_Keycode code,SDL_Keymod mod) {
    SDL_Event e={0}; e.type=SDL_EVENT_KEY_DOWN; e.key.key=code; e.key.mod=mod; e.key.down=true; e.key.windowID=SDL_GetWindowID(d->ui.window);
    SDL_PushEvent(&e); frame(d); e.type=SDL_EVENT_KEY_UP; e.key.down=false; SDL_PushEvent(&e); frame(d);
}
static bool reach(SBDesktop *d,const char *id) {
    for (unsigned i=0;i<64;++i) { if (!strcmp(d->focus,id)) { frame(d); frame(d); return true; } key(d,SDLK_TAB,0); }
    fprintf(stderr,"Backup focus unreachable: %s (at %s)\n",id,d->focus); return false;
}
static bool activate(SBDesktop *d,const char *id) { if (!reach(d,id)) return false; key(d,SDLK_RETURN,0); return true; }
static bool replace(SBDesktop *d,const char *id,const char *text) {
    if (!reach(d,id) || !SDL_SetClipboardText(text)) return false;
    key(d,SDLK_A,MOD); key(d,SDLK_V,MOD); return true;
}
static bool wait_job(SBDesktop *d) {
    Uint64 until=SDL_GetTicks()+30000;
    while (d->backup && SDL_GetTicks()<until) { SDL_Delay(1); frame(d); }
    return d->backup==NULL;
}
static bool capture(SBDesktop *d,const char *directory,const char *name) {
    char path[SB_PATH_CAP]; if (sb_path_join(path,sizeof(path),directory,name).code!=SB_OK) return false;
    nk_input_begin(d->ui.ctx); nk_input_end(d->ui.ctx); sb_desktop_frame(d); sb_ui_draw(&d->ui);
    SBStatus s=sb_ui_capture(&d->ui,path); SDL_RenderPresent(d->ui.renderer); return s.code==SB_OK;
}
int sb_desktop_backup_test(SBDesktop *d,const char *directory) {
    unsigned checks=0; char archive[SB_PATH_CAP],changed[SB_PATH_CAP],full[SB_PATH_CAP],note[SB_PATH_CAP]; SBStatus status;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"BACKUP UI FAIL %d: %s (focus=%s form=%d status=%s)\n",__LINE__,#x,d->focus,d->form,d->message.message); capture(d,directory,"failure.bmp"); return 1; } } while (0)
#define OK(x) do { status=(x); CHECK(status.code==SB_OK); } while (0)
    OK(sb_path_join(archive,sizeof(archive),directory,"Sicherung ü.sbbackup"));
    OK(sb_path_join(changed,sizeof(changed),directory,"Weitere Sicherung.sbbackup"));
    OK(sb_app_new_project(&d->model,"projekt","Projekt ü",NULL)); OK(sb_app_new_note(&d->model,"knowledge","notiz","Notiz"));
    OK(sb_path_join(full,sizeof(full),d->model.project.root,"attachment.bin"));
    SBFile *attachment=NULL; OK(sb_file_open(full,true,&attachment));
    unsigned char block[65536]; memset(block,42,sizeof(block));
    SBStatus written=sb_ok();
    for (unsigned i=0;i<128 && written.code==SB_OK;++i) written=sb_file_write(attachment,block,sizeof(block));
    OK(sb_file_close(attachment,written.code==SB_OK)); OK(written);
    strcpy(d->model.editor,"# Notiz\n\nGespeicherter Inhalt.\n"); OK(sb_app_save(&d->model)); frame(d);
    key(d,SDLK_E,MOD); CHECK(d->editing); CHECK(replace(d,"editor","# Notiz\n\nOffener Entwurf wird gesichert.\n")); CHECK(sb_app_dirty(&d->model));
    key(d,SDLK_F10,SDL_KMOD_SHIFT); CHECK(activate(d,"backup-project") && d->form==SB_FORM_BACKUP);
    CHECK(replace(d,"backup-path",archive)); CHECK(capture(d,directory,"backup-large.bmp"));
    CHECK(reach(d,"submit"));
    SDL_Event start={0}; start.type=SDL_EVENT_KEY_DOWN; start.key.key=SDLK_RETURN; start.key.down=true; start.key.windowID=SDL_GetWindowID(d->ui.window);
    SDL_PushEvent(&start); frame(d);
    if (d->backup) {
        sb_backup_job_snapshot(d->backup,&d->backup_state);
        if (!d->backup_state.done) { CHECK(sb_desktop_animating(d)); CHECK(capture(d,directory,"backup-progress.bmp")); }
    }
    start.type=SDL_EVENT_KEY_UP; start.key.down=false; SDL_PushEvent(&start); frame(d);
    CHECK(wait_job(d) && d->message.code==SB_OK && d->form==SB_FORM_NONE && !sb_app_dirty(&d->model)); CHECK(sb_fs_kind(archive)==1);
    SBBackupInfo info; OK(sb_backup_inspect(archive,&info,NULL,NULL)); CHECK(!strcmp(info.id,"projekt"));
    key(d,SDLK_E,MOD); if (!d->editing) key(d,SDLK_E,MOD);
    CHECK(replace(d,"editor","# Notiz\n\nDieser neue Entwurf bleibt offen.\n")); CHECK(sb_app_dirty(&d->model));
    key(d,SDLK_F10,SDL_KMOD_SHIFT); CHECK(activate(d,"restore-project") && d->form==SB_FORM_RESTORE);
    CHECK(replace(d,"backup-path",archive)); CHECK(activate(d,"submit")); CHECK(wait_job(d) && d->restore_checked && !strcmp(d->restore_id,"projekt-kopie"));
    CHECK(capture(d,directory,"restore-large.bmp"));
    CHECK(SDL_SetWindowSize(d->ui.window,780,560) && SDL_SyncWindow(d->ui.window)); OK(sb_ui_fonts(&d->ui,2)); frame(d);
    CHECK(reach(d,"restore-id")); CHECK(capture(d,directory,"restore-small-200.bmp"));
    CHECK(activate(d,"submit")); CHECK(wait_job(d) && d->message.code==SB_OK && d->form==SB_FORM_NONE);
    CHECK(sb_app_dirty(&d->model) && strstr(d->model.editor,"bleibt offen") && !strcmp(d->model.project.id,"projekt") && d->model.projects.count==2);
    OK(sb_path_join(note,sizeof(note),d->model.workspace,"projekt-kopie/knowledge/notiz.md")); char *text; size_t length;
    OK(sb_fs_read(note,&text,&length)); CHECK(strstr(text,"wird gesichert") && !strstr(text,"bleibt offen")); free(text);
    CHECK(SDL_SetWindowSize(d->ui.window,1336,840) && SDL_SyncWindow(d->ui.window)); OK(sb_ui_fonts(&d->ui,1)); frame(d);
    CHECK(activate(d,"project-picker")); frame(d); bool disambiguated=false;
    for (size_t i=0;i<d->target_count;++i) if (!strcmp(d->targets[i].id,"project:projekt-kopie") && strstr(d->targets[i].label,"projekt-kopie")) disambiguated=true;
    CHECK(disambiguated); key(d,SDLK_ESCAPE,0);
    /* Existing destination: keep preview and open draft; let the user change the name. */
    key(d,SDLK_F10,SDL_KMOD_SHIFT); CHECK(activate(d,"restore-project")); CHECK(replace(d,"backup-path",archive)); CHECK(activate(d,"submit")); CHECK(wait_job(d) && d->restore_checked);
    CHECK(replace(d,"restore-id","projekt")); CHECK(activate(d,"submit")); CHECK(wait_job(d) && d->message.code==SB_EXISTS && d->restore_checked && sb_app_dirty(&d->model));
    CHECK(activate(d,"backup-copy-error")); char *copied=SDL_GetClipboardText(); CHECK(copied && !strcmp(copied,d->message.message) && d->backup_error_copied); SDL_free(copied);
    CHECK(replace(d,"restore-id","neu"));
    /* Change the actual archive after preview, without silently importing new contents. */
    OK(sb_path_join(full,sizeof(full),d->model.project.root,"knowledge/notiz.md")); OK(sb_path_join(note,sizeof(note),directory,"replacement.md"));
    const char *external="# Notiz\n\nExtern geändert.\n"; OK(sb_fs_write_new(note,external,strlen(external))); OK(sb_fs_replace(note,full));
    OK(sb_backup_create(&d->model.project,changed,NULL,NULL)); OK(sb_fs_replace(changed,archive));
    CHECK(activate(d,"submit")); CHECK(wait_job(d) && d->message.code==SB_CONFLICT && !d->restore_checked);
    OK(sb_path_join(note,sizeof(note),d->model.workspace,"neu")); CHECK(sb_fs_kind(note)==0 && sb_app_dirty(&d->model)); key(d,SDLK_ESCAPE,0);
    /* A save conflict prevents starting backup and keeps the editor draft. */
    key(d,SDLK_F10,SDL_KMOD_SHIFT); CHECK(activate(d,"backup-project")); CHECK(replace(d,"backup-path",changed)); CHECK(activate(d,"submit"));
    CHECK(!d->backup && d->message.code==SB_CONFLICT && sb_app_dirty(&d->model) && !sb_fs_kind(changed)); key(d,SDLK_ESCAPE,0);
    /* Native dialog reply integration: wrong kind and closed owner are ignored. */
    key(d,SDLK_F10,SDL_KMOD_SHIFT); CHECK(activate(d,"restore-project")); d->dialog_serial=700;
    SBDialogReply *reply=SDL_calloc(1,sizeof(*reply)); CHECK(reply!=NULL); reply->serial=700; reply->kind=SB_DIALOG_SAVE_BACKUP; strcpy(reply->value,archive);
    SDL_Event e={0}; e.type=sb_dialogs_event(d->dialogs); e.user.data1=reply; sb_desktop_event(d,&e); CHECK(!d->backup_path[0]);
    reply=SDL_calloc(1,sizeof(*reply)); reply->serial=700; reply->kind=SB_DIALOG_OPEN_BACKUP; strcpy(reply->value,archive); e.user.data1=reply;
    sb_desktop_event(d,&e); sb_desktop_apply(d); CHECK(wait_job(d) && d->restore_checked && !strcmp(d->backup_path,archive));
    key(d,SDLK_ESCAPE,0); reply=SDL_calloc(1,sizeof(*reply)); reply->serial=700; reply->kind=SB_DIALOG_OPEN_BACKUP; strcpy(reply->value,"veraltet"); e.user.data1=reply;
    sb_desktop_event(d,&e); CHECK(strcmp(d->backup_path,"veraltet"));
    /* Quit while a job runs waits for cancellation, then protects the open draft. */
    d->command=SB_CMD_RESTORE; sb_desktop_apply(d); strcpy(d->backup_path,archive); d->command=SB_CMD_INSPECT; sb_desktop_apply(d);
    e.type=SDL_EVENT_KEY_DOWN; e.key.key=SDLK_Q; e.key.mod=MOD;
    nk_input_begin(d->ui.ctx); sb_desktop_event(d,&e); nk_input_end(d->ui.ctx);
    CHECK(wait_job(d) && d->model.guard && !d->model.quit && sb_app_dirty(&d->model));
    frame(d); CHECK(activate(d,"guard-cancel")); CHECK(!d->model.guard && !d->model.quit && sb_app_dirty(&d->model));
    printf("%u backup UI assertions passed with keyboard events: save, preview, restore, preserved drafts, conflicts, stale replies and cancellation/quit.\n",checks);
    printf("Backup UI screenshots: %s\n",directory); return 0;
}
