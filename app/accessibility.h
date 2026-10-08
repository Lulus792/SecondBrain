#ifndef SB_ACCESSIBILITY_H
#define SB_ACCESSIBILITY_H
#include "ui.h"
#include <accesskit.h>
typedef struct SBAccessibility SBAccessibility;
typedef struct {
    const char *id,*label,*value;
    struct nk_rect bounds;
    accesskit_role role;
    bool editable,selected;
    size_t anchor,caret;
    const char *parent;
    unsigned level;
    uint64_t order;
    size_t row,column,rows,columns;
    const SBTextSpan *styles; size_t style_count; float font_size;
    const SBNativeText *native_text;
} SBAccessibleItem;
typedef struct {
    char id[100]; accesskit_action action;
    char *value;
    size_t anchor,caret;
    uint64_t generation;
} SBAccessibleAction;
SBAccessibility *sb_accessibility_new(SDL_Window *window);
void sb_accessibility_free(SBAccessibility *accessibility);
void sb_accessibility_update(SBAccessibility *accessibility,const char *title,const char *focus,
    const SBAccessibleItem *items,size_t count,const char *message,bool modal,uint64_t context);
bool sb_accessibility_next_action(SBAccessibility *accessibility,SBAccessibleAction *action);
void sb_accessibility_action_free(SBAccessibleAction *action);
bool sb_accessibility_current(SBAccessibility *accessibility,const SBAccessibleAction *action,uint64_t context);
/* Adapter/native provider tests share the exact production snapshot and queue. */
accesskit_tree_update *sb_accessibility_tree(SBAccessibility *accessibility);
void sb_accessibility_request(SBAccessibility *accessibility,accesskit_action_request *request);
bool sb_accessibility_submit(SBAccessibility *accessibility,accesskit_node_id node,accesskit_action action,const char *value,size_t anchor,size_t caret);
/* Native run-local selectable units are mapped to the editor's scalar ABI. */
bool sb_accessibility_select(SBAccessibility *accessibility,accesskit_node_id node,accesskit_text_selection selection);
#endif
