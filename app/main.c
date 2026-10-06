#include "desktop.h"
#include "version.h"
#include "platform.h"
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static SBStatus font_path(const char *assets, char *out) {
    char root[SB_PATH_CAP];
    const char *base = SDL_GetBasePath();
    if (assets) return sb_path_join(out, SB_PATH_CAP, assets, "fonts/NotoSans-Regular.ttf");
    if (!base) return sb_error(SB_IO, "Anwendungsordner ist nicht erreichbar.");
    SBStatus result = sb_path_join(root, sizeof(root), base, "assets");
    if (result.code == SB_OK) result = sb_path_join(out, SB_PATH_CAP, root, "fonts/NotoSans-Regular.ttf");
    if (result.code == SB_OK && sb_fs_kind(out) == 1) return result;
    /* The bundle stores resources separately from its executable. */
    result = sb_path_join(root, sizeof(root), base, "../Resources/assets");
    if (result.code == SB_OK) result = sb_path_join(out, SB_PATH_CAP, root, "fonts/NotoSans-Regular.ttf");
    return result;
}
int main(int argc, char **argv) {
    if (argc == 2 && !strcmp(argv[1], "--version")) { fputs(sb_build_info(), stdout); return 0; }
    SBDesktop desktop;
    char workspace[SB_PATH_CAP], font[SB_PATH_CAP], home[SB_PATH_CAP];
    const char *workspace_arg = NULL, *project_arg = NULL, *assets = NULL, *test_root = NULL, *snapshot = NULL, *settings_arg = NULL;
    SBStatus status;
    bool keyboard_test = false,backup_test=false;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--workspace") && i + 1 < argc) workspace_arg = argv[++i];
        else if (!strcmp(argv[i], "--project") && i + 1 < argc) project_arg = argv[++i];
        else if (!strcmp(argv[i], "--assets") && i + 1 < argc) assets = argv[++i];
        else if (!strcmp(argv[i], "--self-test") && i + 1 < argc) test_root = argv[++i];
        else if (!strcmp(argv[i], "--keyboard-test") && i + 1 < argc) { test_root = argv[++i]; keyboard_test = true; }
        else if (!strcmp(argv[i],"--backup-test") && i+1<argc) { test_root=argv[++i]; backup_test=true; }
        else if (!strcmp(argv[i],"--settings") && i+1<argc) settings_arg=argv[++i];
        else if (!strcmp(argv[i], "--snapshot") && i + 1 < argc) snapshot = argv[++i];
        else if (!strcmp(argv[i], "--help")) {
            printf("SecondBrain\n  --version\n  --workspace ORDNER\n  --project KENNUNG\n  --assets ASSETORDNER\n  --settings EINSTELLUNGSDATEI\n"
                   "  --self-test TESTORDNER\n  --keyboard-test TESTORDNER\n  --backup-test TESTORDNER\n  --snapshot BILD.bmp\n");
            return 0;
        } else { fprintf(stderr, "Unbekannte oder unvollständige Option: %s\n", argv[i]); return 2; }
    }
    if (test_root) {
        char suffix[100], directory[SB_PATH_CAP];
        snprintf(suffix, sizeof(suffix), "run-%lu-%lu", sb_process_id(), (unsigned long)time(NULL));
        status = sb_path_join(directory, sizeof(directory), test_root, suffix);
        if (status.code == SB_OK) status = sb_fs_mkdirs(directory);
        if (status.code == SB_OK) status = sb_path_join(workspace, sizeof(workspace), directory, "workspace");
        if (status.code != SB_OK) { fprintf(stderr, "%s\n", status.message); return 1; }
        status = font_path(assets, font);
        if (status.code == SB_OK) status = sb_desktop_init(&desktop, workspace, font, true);
        if (status.code != SB_OK) { fprintf(stderr, "%s\n", status.message); return 1; }
        int result = backup_test ? sb_desktop_backup_test(&desktop,directory) : keyboard_test ? sb_desktop_keyboard_test(&desktop, directory) : sb_desktop_self_test(&desktop, directory);
        sb_desktop_free(&desktop);
        return result;
    }
    bool explicit_workspace=workspace_arg!=NULL;
    char preference_path[SB_PATH_CAP]={0};
    SBStatus preferences=sb_ok();
    SBSettings startup_settings; SBRevision startup_revision;
    if (settings_arg || !snapshot) {
        char *directory=settings_arg ? NULL : SDL_GetPrefPath("Lulus792","SecondBrain");
        preferences=settings_arg ? sb_fs_absolute(settings_arg,preference_path,sizeof(preference_path)) :
            directory ? sb_path_join(preference_path,sizeof(preference_path),directory,"settings.conf") : sb_error(SB_IO,"Einstellungsordner ist nicht erreichbar.");
        SDL_free(directory);
        if (preferences.code==SB_OK) preferences=sb_settings_load(preference_path,&startup_settings,&startup_revision);
        if (!explicit_workspace && preferences.code==SB_OK && startup_settings.workspace[0] && sb_fs_kind(startup_settings.workspace)==2)
            workspace_arg=startup_settings.workspace;
    }
    if (workspace_arg) status = sb_fs_absolute(workspace_arg, workspace, sizeof(workspace));
    else {
        status = sb_fs_home(home, sizeof(home));
        if (status.code == SB_OK) status = sb_path_join(workspace, sizeof(workspace), home, "SecondBrain");
    }
    if (status.code == SB_OK) status = font_path(assets, font);
    if (status.code == SB_OK) status = sb_desktop_init(&desktop, workspace, font, snapshot != NULL);
    if (status.code != SB_OK) { fprintf(stderr, "%s\n", status.message); return 1; }
    if (settings_arg || !snapshot) {
        if (preferences.code==SB_OK) preferences=sb_desktop_preferences(&desktop,preference_path,explicit_workspace);
        if (preferences.code!=SB_OK) { desktop.message=preferences; fprintf(stderr,"%s\n",preferences.message); }
    }
    if (project_arg) {
        status = sb_app_request(&desktop.model, SB_ACT_PROJECT, project_arg);
        if (status.code != SB_OK) { fprintf(stderr, "%s\n", status.message); sb_desktop_free(&desktop); return 1; }
    }
    unsigned frames = 0;
    Uint64 last_tick=SDL_GetTicksNS();
    while (!desktop.model.quit) {
        SDL_Event event;
        nk_input_begin(desktop.ui.ctx);
        if (!snapshot && SDL_WaitEventTimeout(&event, sb_desktop_animating(&desktop) ? 16 : 100)) sb_desktop_event(&desktop, &event);
        while (SDL_PollEvent(&event)) sb_desktop_event(&desktop, &event);
        Uint64 now=SDL_GetTicksNS();
        sb_desktop_tick(&desktop,(float)((double)(now-last_tick)/1e9)); last_tick=now;
        nk_input_end(desktop.ui.ctx);
        sb_desktop_frame(&desktop);
        sb_ui_draw(&desktop.ui);
        if (snapshot && ++frames == 4) {
            status = sb_ui_capture(&desktop.ui, snapshot);
            desktop.model.quit = true;
        }
        SDL_RenderPresent(desktop.ui.renderer);
        sb_desktop_apply(&desktop);
        if (desktop.model.has_project) {
            char title[SB_NAME_CAP + 32];
            snprintf(title, sizeof(title), "%s — SecondBrain", desktop.model.project.name);
            SDL_SetWindowTitle(desktop.ui.window, title);
        }
    }
    preferences=sb_desktop_store_preferences(&desktop);
    if (preferences.code!=SB_OK) fprintf(stderr,"%s\n",preferences.message);
    sb_desktop_free(&desktop);
    if (status.code != SB_OK) { fprintf(stderr, "%s\n", status.message); return 1; }
    return 0;
}
