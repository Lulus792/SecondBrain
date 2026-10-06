#ifndef SB_APP_MODEL_H
#define SB_APP_MODEL_H
#include "sb.h"

typedef enum {
    SB_ACT_NONE, SB_ACT_PROJECT, SB_ACT_NOTE, SB_ACT_WORKSPACE,
    SB_ACT_RELOAD, SB_ACT_ARCHIVE, SB_ACT_QUIT
} SBActionKind;
typedef enum { SB_KEEP_EDITING, SB_SAVE_CHANGES, SB_DISCARD_CHANGES } SBDecision;
typedef struct { SBActionKind kind; char value[SB_PATH_CAP]; } SBAction;
typedef struct {
    char workspace[SB_PATH_CAP];
    SBProjects projects;
    SBProject project;
    SBNotes notes;
    bool has_project, guard, quit, source_directory;
    char path[SB_PATH_CAP], title[SB_NAME_CAP];
    char *editor;
    SBRevision revision;
    char *source;
    char source_path[SB_PATH_CAP], source_title[SB_NAME_CAP];
    SBAction pending;
    unsigned generation;
} SBApp;

SBStatus sb_app_init(SBApp *app, const char *workspace);
SBStatus sb_app_refresh_projects(SBApp *app);
void sb_app_free(SBApp *app);
bool sb_app_dirty(const SBApp *app);
SBStatus sb_app_request(SBApp *app, SBActionKind kind, const char *value);
SBStatus sb_app_decide(SBApp *app, SBDecision decision);
SBStatus sb_app_save(SBApp *app);
SBStatus sb_app_save_copy(SBApp *app);
SBStatus sb_app_new_project(SBApp *app, const char *id, const char *name, const char *repository);
SBStatus sb_app_new_note(SBApp *app, const char *section, const char *id, const char *title);
SBStatus sb_app_source(SBApp *app, const char *link);
void sb_app_source_close(SBApp *app);
void sb_app_slug(const char *title, char *out, size_t capacity);

#endif
