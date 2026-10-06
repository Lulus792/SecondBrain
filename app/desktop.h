#ifndef SB_DESKTOP_H
#define SB_DESKTOP_H
#include "ui.h"
#include "model.h"

typedef enum { SB_FORM_NONE, SB_FORM_PROJECT, SB_FORM_NOTE, SB_FORM_WORKSPACE, SB_FORM_SETTINGS, SB_FORM_CONTEXT, SB_FORM_HELP } SBForm;
typedef enum {
    SB_CMD_NONE, SB_CMD_SAVE, SB_CMD_COPY, SB_CMD_NEW_PROJECT, SB_CMD_NEW_NOTE,
    SB_CMD_WORKSPACE, SB_CMD_SUBMIT, SB_CMD_CANCEL, SB_CMD_CONTEXT,
    SB_CMD_GUARD_SAVE, SB_CMD_GUARD_DISCARD, SB_CMD_GUARD_CANCEL,
    SB_CMD_ARCHIVE, SB_CMD_RELOAD, SB_CMD_THEME, SB_CMD_SCALE, SB_CMD_SOURCE
} SBCommand;
typedef struct { char id[100]; struct nk_rect bounds; } SBTarget;
typedef struct {
    SBUi ui;
    SBApp model;
    SBNotes hits;
    char search[256], searched[256], section[32];
    char name[SB_NAME_CAP], id[65], repository[SB_PATH_CAP], folder[SB_PATH_CAP];
    char command_value[SB_PATH_CAP];
    char *context;
    SBStatus message;
    SBForm form;
    SBCommand command;
    SBAction navigation;
    bool editing, sidebar, id_manual, focus_search, focus_editor, next_edit, reset_reader;
    int form_focus, active_form_field, note_section;
    unsigned generation;
    float next_scale;
    SBTarget targets[256];
    size_t target_count;
    bool test;
} SBDesktop;

SBStatus sb_desktop_init(SBDesktop *desktop, const char *workspace, const char *font, bool testing);
void sb_desktop_free(SBDesktop *desktop);
void sb_desktop_event(SBDesktop *desktop, const SDL_Event *event);
void sb_desktop_frame(SBDesktop *desktop);
void sb_desktop_apply(SBDesktop *desktop);
int sb_desktop_self_test(SBDesktop *desktop, const char *directory);

#endif
