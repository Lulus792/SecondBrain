#ifndef SB_DESKTOP_H
#define SB_DESKTOP_H
#include "ui.h"
#include "model.h"
#include "settings.h"
#include "dialog.h"
#include "backup_job.h"
#include "accessibility.h"
#include "system_style.h"

typedef enum { SB_FORM_NONE, SB_FORM_PROJECT, SB_FORM_NOTE, SB_FORM_WORKSPACE, SB_FORM_SETTINGS, SB_FORM_CONTEXT, SB_FORM_HELP, SB_FORM_ACTIONS, SB_FORM_PROJECTS, SB_FORM_FILTER,SB_FORM_BACKUP,SB_FORM_RESTORE,SB_FORM_ABOUT,SB_FORM_NOTICE_LIST,SB_FORM_NOTICE_TEXT } SBForm;
typedef enum {
    SB_CMD_NONE, SB_CMD_SAVE, SB_CMD_COPY, SB_CMD_NEW_PROJECT, SB_CMD_NEW_NOTE,
    SB_CMD_WORKSPACE, SB_CMD_SUBMIT, SB_CMD_CANCEL, SB_CMD_CONTEXT,
    SB_CMD_GUARD_SAVE, SB_CMD_GUARD_DISCARD, SB_CMD_GUARD_CANCEL,
    SB_CMD_ARCHIVE, SB_CMD_RELOAD, SB_CMD_THEME, SB_CMD_SCALE, SB_CMD_SOURCE,
    SB_CMD_BACKUP,SB_CMD_RESTORE,SB_CMD_INSPECT
} SBCommand;
typedef enum { SB_FOCUS_BUTTON, SB_FOCUS_TEXT, SB_FOCUS_MAP, SB_FOCUS_READER } SBFocusKind;
typedef struct { char id[100], label[SB_NAME_CAP]; struct nk_rect bounds; SBFocusKind kind; int group; unsigned order; char parent[100]; } SBTarget;
typedef struct { char id[100]; char *text; struct nk_rect bounds; accesskit_role role; unsigned order; int group; char parent[100]; unsigned level; float document_y; size_t row,column,rows,columns; } SBPassiveText;
typedef struct {
    float position, destination, pending, maximum, elastic;
    nk_uint applied;
    bool active, ready, used, measured, dragging;
    float width,height,grab;
    struct nk_rect track,thumb;
} SBScroll;
typedef struct {
    SBUi ui;
    SBApp model;
    SBNotes hits;
    char settings_path[SB_PATH_CAP];
    SBRevision settings_revision;
    bool settings_enabled;
    SBNativeDialogs *dialogs;
    SBAccessibility *accessibility;
    SBStyleMonitor *style_monitor;
    SBStyleChoice requested_style,applied_style;
    SBSystemStyle system_style;
    unsigned dialog_serial;
    SBBackupJob *backup;
    SBBackupJobState backup_state;
    SBBackupInfo restore_info;
    char backup_path[SB_PATH_CAP],checked_backup[SB_PATH_CAP],restore_id[65];
    bool restore_checked,quit_after_backup,backup_feedback_reset,backup_error_copied,backup_clipboard_failed;
    char search[256], searched[256], section[32];
    char name[SB_NAME_CAP], id[65], repository[SB_PATH_CAP], folder[SB_PATH_CAP];
    char command_value[SB_PATH_CAP];
    char *context,*notice;
    size_t notice_index;
    char reveal_document[100];
    uint64_t reveal_document_context,heading_context;
    char heading_cursor[100];
    struct nk_rect reader_title_bounds;
    SBStatus message;
    SBForm form;
    SBCommand command;
    SBAction navigation;
    bool editing, sidebar, id_manual, focus_search, focus_editor, next_edit, reset_reader;
    int form_focus, active_form_field, note_section;
    unsigned generation;
    float next_scale;
    SBTarget *targets;
    size_t target_count, target_capacity;
    SBPassiveText *passive;
    size_t passive_count,passive_capacity;
    unsigned semantic_order;
    uint64_t semantic_context;
    bool test;
    SBGraph graph;
    struct nk_text_edit text_edit;
    bool text_edit_ready;
    char graph_project[SB_PATH_CAP];
    bool graph_dirty, card, browser, solid, reduced_motion, focus_changed, keyboard, dragging, moved;
    float yaw, pitch, zoom, pan_x, pan_y, drag_x, drag_y;
    float view_yaw, view_pitch, view_zoom, view_pan_x, view_pan_y, seconds;
    SBScroll scrolling[4];
    bool follow_star, expanded;
    float focus_x,focus_y,focus_z,flight_from[3],flight_to[3],flight;
    float map_cx,map_cy,map_unit,map_target_cx,map_target_cy,map_target_unit;
    bool map_ready;
    size_t star, page, project_page;
    int layout_width,layout_height;
    float layout_scale;
    char focus[100], activate[100], saved_focus[100], source_focus[100];
    int focus_group, focus_scroll_frames;
    SBForm focus_form;
    bool focus_guard;
    struct nk_rect map_bounds;
} SBDesktop;

SBStatus sb_desktop_init(SBDesktop *desktop, const char *workspace, const char *font, bool testing);
void sb_desktop_free(SBDesktop *desktop);
SBStatus sb_desktop_preferences(SBDesktop *desktop, const char *path, bool explicit_workspace);
SBStatus sb_desktop_store_preferences(SBDesktop *desktop);
void sb_desktop_event(SBDesktop *desktop, const SDL_Event *event);
void sb_desktop_frame(SBDesktop *desktop);
void sb_desktop_tick(SBDesktop *desktop, float seconds);
bool sb_desktop_animating(const SBDesktop *desktop);
void sb_desktop_apply(SBDesktop *desktop);
void sb_desktop_set_style(SBDesktop *desktop,SBStyleChoice style);
int sb_desktop_keyboard_test(SBDesktop *desktop, const char *directory);
int sb_desktop_self_test(SBDesktop *desktop, const char *directory);
int sb_desktop_backup_test(SBDesktop *desktop,const char *directory);

#endif
