#include "accessibility.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#if defined(__APPLE__)
#include <objc/runtime.h>
#include <objc/message.h>
#include <dlfcn.h>
/* AccessKit 0.23.1 returns the literal "Heading". New AppKit exposes its actual
   heading role; preserve that native contract without changing the C tree role. */
static SDL_SpinLock mac_role_lock;
static IMP mac_children_original,mac_role_original,mac_rows_original,mac_allowed_original;
static void *mac_heading_role;
static void *mac_role(void *self,SEL selector) {
    SDL_LockSpinlock(&mac_role_lock); IMP original=mac_role_original; SDL_UnlockSpinlock(&mac_role_lock);
    void *role=((void *(*)(void *,SEL))original)(self,selector);
    const char *name=role ? ((const char *(*)(void *,SEL))objc_msgSend)(role,sel_registerName("UTF8String")) : NULL;
    return mac_heading_role && name && !strcmp(name,"Heading") ? mac_heading_role : role;
}
/* The pinned provider exposes Table/Row/Cell roles, but its rows selector
   only handles selectable containers. Document rows are the table children. */
static bool mac_table(void *self) {
    void *role=((void *(*)(void *,SEL))objc_msgSend)(self,sel_registerName("accessibilityRole"));
    const char *name=role ? ((const char *(*)(void *,SEL))objc_msgSend)(role,sel_registerName("UTF8String")) : NULL;
    return name && !strcmp(name,"AXTable");
}
static void *mac_rows(void *self,SEL selector) {
    if (mac_table(self)) return ((void *(*)(void *,SEL))objc_msgSend)(self,sel_registerName("accessibilityChildren"));
    return ((void *(*)(void *,SEL))mac_rows_original)(self,selector);
}
static BOOL mac_allowed(void *self,SEL selector,SEL requested) {
    if (requested==sel_registerName("accessibilityRows") && mac_table(self)) return YES;
    return ((BOOL(*)(void *,SEL,SEL))mac_allowed_original)(self,selector,requested);
}
static void *mac_children(void *self,SEL selector) {
    SDL_LockSpinlock(&mac_role_lock); IMP original=mac_children_original; SDL_UnlockSpinlock(&mac_role_lock);
    void *children=((void *(*)(void *,SEL))original)(self,selector);
    SDL_LockSpinlock(&mac_role_lock);
    if (!mac_role_original) {
        Class node=objc_getClass("AccessKitNode");
        Method method=node ? class_getInstanceMethod(node,sel_registerName("accessibilityRole")) : NULL;
        if (method) { mac_role_original=method_getImplementation(method); method_setImplementation(method,(IMP)mac_role); }
        Method rows=node ? class_getInstanceMethod(node,sel_registerName("accessibilityRows")) : NULL;
        Method allowed=node ? class_getInstanceMethod(node,sel_registerName("isAccessibilitySelectorAllowed:")) : NULL;
        if (rows && allowed) {
            mac_rows_original=method_getImplementation(rows); mac_allowed_original=method_getImplementation(allowed);
            method_setImplementation(rows,(IMP)mac_rows); method_setImplementation(allowed,(IMP)mac_allowed);
        }
    }
    SDL_UnlockSpinlock(&mac_role_lock); return children;
}
static void mac_native_roles(void *window) {
    void **role=dlsym(RTLD_DEFAULT,"NSAccessibilityHeadingRole");
    void *view=((void *(*)(void *,SEL))objc_msgSend)(window,sel_registerName("contentView"));
    Class view_class=object_getClass(view); SEL selector=sel_registerName("accessibilityChildren");
    Method method=class_getInstanceMethod(view_class,selector);
    SDL_LockSpinlock(&mac_role_lock);
    if (method && !mac_children_original) {
        mac_heading_role=role ? *role : NULL; mac_children_original=method_getImplementation(method);
        if (!class_addMethod(view_class,selector,(IMP)mac_children,method_getTypeEncoding(method))) method_setImplementation(method,(IMP)mac_children);
    }
    SDL_UnlockSpinlock(&mac_role_lock);
}
#endif
typedef struct { char id[100]; accesskit_node_id node; uint64_t context; } Identity;
typedef struct { SBTextSpan span; accesskit_node_id node; } StyledRun;
typedef struct {
    char id[100],parent[100]; char *label,*value;
    unsigned level; size_t row,column,rows,columns;
    accesskit_node_id node; struct nk_rect bounds; accesskit_role role;
    bool editable,selected; size_t anchor,caret;
    StyledRun *runs; size_t run_count; float font_size;
} Item;
typedef struct Pending { SBAccessibleAction action; struct Pending *next; } Pending;
struct SBAccessibility {
    SDL_Window *window; SDL_Mutex *mutex;
    Identity *identities; size_t identity_count,identity_capacity;
    size_t *identity_slots,slot_capacity;
    Item *items; size_t count; char title[SB_NAME_CAP],focus[100],message[512];
    uint64_t generation,signature,context; accesskit_node_id next_node; bool modal,alive;
    Pending *head,*tail; size_t queued,queued_bytes;
#if defined(__APPLE__)
    accesskit_macos_subclassing_adapter *adapter;
#elif defined(_WIN32)
    accesskit_windows_subclassing_adapter *adapter;
#else
    accesskit_unix_adapter *adapter;
#endif
};
static char *copy(const char *s) { if (!s) return NULL; size_t n=strlen(s)+1; char *p=malloc(n); if (p) memcpy(p,s,n); return p; }
static uint64_t identity_hash(const char *id,uint64_t context) {
    return sb_hash(id,strlen(id))^context;
}
static bool identity_grow(SBAccessibility *a) {
    size_t capacity=a->slot_capacity ? a->slot_capacity*2 : 64;
    size_t *slots=calloc(capacity,sizeof(*slots)); if (!slots) return false;
    for (size_t i=0;i<a->identity_count;++i) {
        size_t slot=(size_t)identity_hash(a->identities[i].id,a->identities[i].context)&(capacity-1);
        while (slots[slot]) slot=(slot+1)&(capacity-1);
        slots[slot]=i+1;
    }
    free(a->identity_slots); a->identity_slots=slots; a->slot_capacity=capacity; return true;
}
static accesskit_node_id identify(SBAccessibility *a,const char *id,uint64_t context) {
    if (strlen(id)>=sizeof(a->identities[0].id)) return 0;
    if (!a->slot_capacity || a->identity_count+1>a->slot_capacity*3/4) if (!identity_grow(a)) return 0;
    size_t slot=(size_t)identity_hash(id,context)&(a->slot_capacity-1);
    while (a->identity_slots[slot]) {
        Identity *entry=&a->identities[a->identity_slots[slot]-1];
        if (entry->context==context && !strcmp(entry->id,id)) return entry->node;
        slot=(slot+1)&(a->slot_capacity-1);
    }
    if (a->identity_count==a->identity_capacity) {
        size_t capacity=a->identity_capacity ? a->identity_capacity*2 : 64;
        Identity *items=realloc(a->identities,capacity*sizeof(*items)); if (!items) return 0;
        a->identities=items; a->identity_capacity=capacity;
    }
    Identity *v=&a->identities[a->identity_count]; snprintf(v->id,sizeof(v->id),"%s",id);
    v->node=16+(++a->next_node); v->context=context; a->identity_slots[slot]=++a->identity_count;
    return v->node;
}
static accesskit_node_id run_id(accesskit_node_id node) { return UINT64_C(0x8000000000000000)|node; }
static size_t characters(const char *s) { size_t n=0; for (;s && *s;++s) if (((unsigned char)*s&0xc0)!=0x80) ++n; return n; }
static bool permits(const Item *v,accesskit_action action) {
    switch (action) {
    case ACCESSKIT_ACTION_SCROLL_INTO_VIEW:
        if (v->parent[0]) return true;
        /* fall through */
    case ACCESSKIT_ACTION_FOCUS:
        return v->role==ACCESSKIT_ROLE_BUTTON || v->role==ACCESSKIT_ROLE_LINK || v->role==ACCESSKIT_ROLE_DOCUMENT ||
            v->role==ACCESSKIT_ROLE_GROUP || v->role==ACCESSKIT_ROLE_TEXT_INPUT || v->role==ACCESSKIT_ROLE_SEARCH_INPUT ||
            v->role==ACCESSKIT_ROLE_MULTILINE_TEXT_INPUT || v->role==ACCESSKIT_ROLE_LIST_BOX_OPTION;
    case ACCESSKIT_ACTION_CLICK: return v->role==ACCESSKIT_ROLE_BUTTON || v->role==ACCESSKIT_ROLE_LINK || v->role==ACCESSKIT_ROLE_LIST_BOX_OPTION;
    case ACCESSKIT_ACTION_SET_VALUE: return v->editable;
    case ACCESSKIT_ACTION_REPLACE_SELECTED_TEXT: case ACCESSKIT_ACTION_SET_TEXT_SELECTION:
        return v->editable && v->role==ACCESSKIT_ROLE_MULTILINE_TEXT_INPUT;
    case ACCESSKIT_ACTION_SCROLL_UP: case ACCESSKIT_ACTION_SCROLL_DOWN:
        return v->role==ACCESSKIT_ROLE_DOCUMENT || v->role==ACCESSKIT_ROLE_MULTILINE_TEXT_INPUT;
    default: return false;
    }
}
typedef struct { const char *id; size_t index; } ParentIndex;
static int parent_order(const void *left,const void *right) {
    const ParentIndex *a=left,*b=right; return strcmp(a->id,b->id);
}
static size_t parent_find(const ParentIndex *index,size_t count,const char *id) {
    size_t lo=0,hi=count;
    while (lo<hi) { size_t mid=lo+(hi-lo)/2; if (strcmp(index[mid].id,id)<0) lo=mid+1; else hi=mid; }
    return lo<count && !strcmp(index[lo].id,id) ? index[lo].index : SIZE_MAX;
}
static bool parent_allowed(const Item *parent,const Item *child) {
    if (parent->role==ACCESSKIT_ROLE_DOCUMENT && !parent->parent[0]) return true;
    if (parent->role==ACCESSKIT_ROLE_BLOCKQUOTE || parent->role==ACCESSKIT_ROLE_LIST_ITEM) return true;
    if (parent->role==ACCESSKIT_ROLE_LIST && child->role==ACCESSKIT_ROLE_LIST_ITEM) return true;
    if (parent->role==ACCESSKIT_ROLE_TABLE && child->role==ACCESSKIT_ROLE_ROW) return true;
    if (parent->role==ACCESSKIT_ROLE_ROW && (child->role==ACCESSKIT_ROLE_CELL || child->role==ACCESSKIT_ROLE_COLUMN_HEADER)) return true;
    return (parent->role==ACCESSKIT_ROLE_CELL || parent->role==ACCESSKIT_ROLE_COLUMN_HEADER) && child->role==ACCESSKIT_ROLE_LINK;
}
static void text_run(accesskit_tree_update *tree,accesskit_node *parent,accesskit_node_id id,
    const char *value,size_t length,accesskit_rect bounds,unsigned style,float font_size) {
    accesskit_node *text=accesskit_node_new(ACCESSKIT_ROLE_TEXT_RUN);
    accesskit_node_set_value_with_length(text,value,length); accesskit_node_set_bounds(text,bounds);
    if (font_size>0) {
        accesskit_node_set_font_size(text,font_size);
        accesskit_node_set_font_family(text,style&SB_TEXT_CODE ? "Noto Sans Mono" : "Noto Sans");
        accesskit_node_set_font_weight(text,style&SB_TEXT_BOLD ? 700 : 400);
        if (style&SB_TEXT_ITALIC) accesskit_node_set_italic(text);
    }
    size_t n=0; for (size_t i=0;i<length;++i) if (((unsigned char)value[i]&0xc0)!=0x80) ++n;
    uint8_t *lengths=n ? malloc(n) : NULL;
    if (lengths) {
        size_t at=0; for (size_t i=0;i<length;) {
            unsigned char c=(unsigned char)value[i]; unsigned size=c<0x80 ? 1 : c<0xe0 ? 2 : c<0xf0 ? 3 : 4;
            lengths[at++]=(uint8_t)size; i+=size;
        }
        accesskit_node_set_character_lengths(text,n,lengths); free(lengths);
    }
    accesskit_node_push_child(parent,id); accesskit_tree_update_push_node(tree,id,text);
}
static accesskit_tree_update *build_locked(SBAccessibility *a) {
    accesskit_node_id focus=1;
    for (size_t i=0;i<a->count;++i) if (!strcmp(a->items[i].id,a->focus)) focus=a->items[i].node;
    accesskit_tree_update *tree=accesskit_tree_update_with_capacity_and_focus(a->count*2+3,focus);
    accesskit_tree_info *info=accesskit_tree_info_new(1); accesskit_tree_update_set_tree_info(tree,info);
    accesskit_node *root=accesskit_node_new(ACCESSKIT_ROLE_WINDOW); accesskit_node_set_label(root,a->title);
    accesskit_node *container=accesskit_node_new(a->modal ? ACCESSKIT_ROLE_DIALOG : ACCESSKIT_ROLE_GROUP);
    if (a->modal) accesskit_node_set_modal(container);
    const char *region=a->modal ? "Aktuelle Aufgabe" : "Projektarbeitsfläche";
    if (a->modal) for (size_t i=0;i<a->count;++i) if (!strcmp(a->items[i].id,"modal-title")) region=a->items[i].label;
    accesskit_node_set_label(container,region); accesskit_node_push_child(root,2);
    accesskit_node *documents=NULL;
    accesskit_node **nodes=a->count ? calloc(a->count,sizeof(*nodes)) : NULL;
    ParentIndex *parents=a->count ? malloc(a->count*sizeof(*parents)) : NULL;
    if (a->count && (!nodes || !parents)) {
        free(nodes); free(parents);
        accesskit_tree_update_set_focus(tree,1);
        accesskit_tree_update_push_node(tree,2,container); accesskit_tree_update_push_node(tree,1,root); return tree;
    }
    for (size_t i=0;i<a->count;++i) parents[i]=(ParentIndex){a->items[i].id,i};
    if (a->count) qsort(parents,a->count,sizeof(*parents),parent_order);
    for (size_t i=0;i<a->count;++i) {
        Item *v=&a->items[i]; accesskit_node *node=nodes[i]=accesskit_node_new(v->role);
        accesskit_node_set_label(node,v->label);
        accesskit_node_set_author_id(node,v->id);
        if (v->role==ACCESSKIT_ROLE_HEADING) accesskit_node_set_level(node,(v->level ? v->level : 1)-1);
        if (v->role==ACCESSKIT_ROLE_SPLITTER) accesskit_node_set_orientation(node,ACCESSKIT_ORIENTATION_HORIZONTAL);
        if (v->role==ACCESSKIT_ROLE_TABLE) { accesskit_node_set_row_count(node,v->rows); accesskit_node_set_column_count(node,v->columns); }
        if (v->role==ACCESSKIT_ROLE_ROW) accesskit_node_set_row_index(node,v->row);
        if (v->role==ACCESSKIT_ROLE_CELL || v->role==ACCESSKIT_ROLE_COLUMN_HEADER) {
            accesskit_node_set_row_index(node,v->row); accesskit_node_set_column_index(node,v->column);
            accesskit_node_set_row_span(node,1); accesskit_node_set_column_span(node,1);
        }
        if (v->parent[0]) accesskit_node_set_is_line_breaking_object(node);
        accesskit_rect rect={v->bounds.x,v->bounds.y,v->bounds.x+v->bounds.w,v->bounds.y+v->bounds.h}; accesskit_node_set_bounds(node,rect);
        if (permits(v,ACCESSKIT_ACTION_FOCUS)) accesskit_node_add_action(node,ACCESSKIT_ACTION_FOCUS);
        if (permits(v,ACCESSKIT_ACTION_SCROLL_INTO_VIEW)) accesskit_node_add_action(node,ACCESSKIT_ACTION_SCROLL_INTO_VIEW);
        if (v->role==ACCESSKIT_ROLE_BUTTON || v->role==ACCESSKIT_ROLE_LINK || v->role==ACCESSKIT_ROLE_LIST_BOX_OPTION) accesskit_node_add_action(node,ACCESSKIT_ACTION_CLICK);
        if (v->role==ACCESSKIT_ROLE_LIST_BOX_OPTION) accesskit_node_set_selected(node,v->selected);
        if (v->value) {
            accesskit_node_set_value(node,v->value);
            if (!v->editable) accesskit_node_set_read_only(node);
            else {
                accesskit_node_add_action(node,ACCESSKIT_ACTION_SET_VALUE);
                if (v->role==ACCESSKIT_ROLE_MULTILINE_TEXT_INPUT) {
                    accesskit_node_add_action(node,ACCESSKIT_ACTION_REPLACE_SELECTED_TEXT);
                    accesskit_node_add_action(node,ACCESSKIT_ACTION_SET_TEXT_SELECTION);
                }
            }
            if (v->role==ACCESSKIT_ROLE_DOCUMENT || v->role==ACCESSKIT_ROLE_MULTILINE_TEXT_INPUT) {
                accesskit_node_add_action(node,ACCESSKIT_ACTION_SCROLL_UP); accesskit_node_add_action(node,ACCESSKIT_ACTION_SCROLL_DOWN);
            }
            bool structured=false;
            if (v->role==ACCESSKIT_ROLE_DOCUMENT && !v->parent[0])
                for (size_t j=0;j<a->count;++j) if (j!=i && !strcmp(a->items[j].parent,v->id)) { structured=true; break; }
            if (!structured) {
            if (v->run_count && !v->editable) {
                for (size_t j=0;j<v->run_count;++j) {
                    StyledRun run=v->runs[j];
                    text_run(tree,node,run.node,v->value+run.span.offset,run.span.length,rect,run.span.style,v->font_size);
                }
            } else text_run(tree,node,run_id(v->node),v->value,strlen(v->value),rect,0,0);
            if (v->editable && v->role==ACCESSKIT_ROLE_MULTILINE_TEXT_INPUT) {
                accesskit_text_selection selection={{run_id(v->node),v->anchor},{run_id(v->node),v->caret}};
                accesskit_node_set_text_selection(node,selection);
            }
        }
        }
    }
    /* Parent nodes remain owned here until all child edges have been added. */
    for (size_t i=0;i<a->count;++i) {
        Item *v=&a->items[i]; bool nested=false;
        if (v->parent[0]) {
            size_t j=parent_find(parents,a->count,v->parent);
            if (j!=SIZE_MAX && i!=j && parent_allowed(&a->items[j],v)) { accesskit_node_push_child(nodes[j],v->node); nested=true; }
        }
        if (!nested && !strncmp(v->id,"star:",5)) {
            if (!documents) { documents=accesskit_node_new(ACCESSKIT_ROLE_LIST_BOX); accesskit_node_set_label(documents,"Projektdokumente"); accesskit_node_push_child(container,4); }
            accesskit_node_push_child(documents,v->node);
        } else if (!nested) accesskit_node_push_child(container,v->node);
    }
    for (size_t i=0;i<a->count;++i) accesskit_tree_update_push_node(tree,a->items[i].node,nodes[i]);
    free(nodes); free(parents);
    if (documents) accesskit_tree_update_push_node(tree,4,documents);
    if (a->message[0]) {
        accesskit_node *status=accesskit_node_new(ACCESSKIT_ROLE_LABEL); accesskit_node_set_value(status,a->message); accesskit_node_set_live(status,ACCESSKIT_LIVE_POLITE);
        accesskit_node_push_child(container,3); accesskit_tree_update_push_node(tree,3,status);
    }
    accesskit_tree_update_push_node(tree,2,container); accesskit_tree_update_push_node(tree,1,root); return tree;
}
accesskit_tree_update *sb_accessibility_tree(SBAccessibility *a) { SDL_LockMutex(a->mutex); accesskit_tree_update *t=build_locked(a); SDL_UnlockMutex(a->mutex); return t; }
static accesskit_tree_update *factory(void *userdata) { return sb_accessibility_tree(userdata); }
bool sb_accessibility_submit(SBAccessibility *a,accesskit_node_id node,accesskit_action kind,const char *value,size_t anchor,size_t caret) {
    size_t bytes=value ? strlen(value)+1 : 0;
    if (value && (bytes>SB_TEXT_LIMIT || !sb_utf8_valid(value,bytes-1))) return false;
    SDL_LockMutex(a->mutex);
    Item *item=NULL;
    for (size_t i=0;i<a->count;++i) if (a->items[i].node==node || run_id(a->items[i].node)==node) { item=&a->items[i]; break; }
    bool accepted=false;
    bool data_valid=(kind!=ACCESSKIT_ACTION_SET_VALUE && kind!=ACCESSKIT_ACTION_REPLACE_SELECTED_TEXT) || value;
    if (kind==ACCESSKIT_ACTION_SET_TEXT_SELECTION && item && (anchor>characters(item->value) || caret>characters(item->value))) data_valid=false;
    if (a->alive && item && permits(item,kind) && data_valid && a->queued<64 && bytes<=2*SB_TEXT_LIMIT-a->queued_bytes) {
        Pending *p=calloc(1,sizeof(*p));
        if (p) {
            strcpy(p->action.id,item->id); p->action.action=kind; p->action.generation=a->generation;
            p->action.value=copy(value); p->action.anchor=anchor; p->action.caret=caret;
            if (value && !p->action.value) free(p);
            else { if (a->tail) a->tail->next=p; else a->head=p; a->tail=p; ++a->queued; a->queued_bytes+=bytes; accepted=true; }
        }
    }
    SDL_UnlockMutex(a->mutex); return accepted;
}
void sb_accessibility_request(SBAccessibility *a,accesskit_action_request *request) {
    const char *value=NULL; size_t anchor=0,caret=0;
    if (request->data.has_value && request->data.value.tag==ACCESSKIT_ACTION_DATA_VALUE) value=request->data.value.value;
    if (request->data.has_value && request->data.value.tag==ACCESSKIT_ACTION_DATA_SET_TEXT_SELECTION) {
        accesskit_text_selection selection=request->data.value.set_text_selection;
        if (selection.anchor.node!=run_id(request->target_node) || selection.focus.node!=run_id(request->target_node)) {
            accesskit_action_request_free(request); return;
        }
        anchor=request->data.value.set_text_selection.anchor.character_index; caret=request->data.value.set_text_selection.focus.character_index;
    }
    sb_accessibility_submit(a,request->target_node,request->action,value,anchor,caret);
    accesskit_action_request_free(request);
}
static void action(accesskit_action_request *request,void *userdata) { sb_accessibility_request(userdata,request); }
#ifndef __APPLE__
static void deactivate(void *userdata) { (void)userdata; }
#endif
SBAccessibility *sb_accessibility_new(SDL_Window *window) {
    SBAccessibility *a=calloc(1,sizeof(*a)); if (!a) return NULL;
    a->mutex=SDL_CreateMutex(); if (!a->mutex) { free(a); return NULL; } a->alive=true; a->window=window; strcpy(a->title,"SecondBrain");
    SDL_PropertiesID properties=SDL_GetWindowProperties(window);
#if defined(__APPLE__)
    void *native=SDL_GetPointerProperty(properties,SDL_PROP_WINDOW_COCOA_WINDOW_POINTER,NULL);
    if (native) {
        static bool forwarded=false;
        if (!forwarded) { accesskit_macos_add_focus_forwarder_to_window_class(class_getName(object_getClass(native))); forwarded=true; }
        a->adapter=accesskit_macos_subclassing_adapter_for_window(native,factory,a,action,a);
        mac_native_roles(native);
    }
#elif defined(_WIN32)
    HWND native=SDL_GetPointerProperty(properties,SDL_PROP_WINDOW_WIN32_HWND_POINTER,NULL);
    if (native) a->adapter=accesskit_windows_subclassing_adapter_new(native,factory,a,action,a);
#else
    (void)properties; a->adapter=accesskit_unix_adapter_new(factory,a,action,a,deactivate,a);
#endif
    return a;
}
static void clear_items(SBAccessibility *a) { for (size_t i=0;i<a->count;++i) { free(a->items[i].label); free(a->items[i].value); free(a->items[i].runs); } free(a->items); a->items=NULL; a->count=0; }
void sb_accessibility_update(SBAccessibility *a,const char *title,const char *focus,const SBAccessibleItem *items,size_t count,const char *message,bool modal,uint64_t context) {
    if (!a) return;
    uint64_t signature=sb_hash(title,strlen(title))^sb_hash(focus,strlen(focus))^sb_hash(message,strlen(message))^(uint64_t)modal;
    for (size_t i=0;i<count;++i) { signature=signature*1099511628211ULL^sb_hash(items[i].id,strlen(items[i].id))^sb_hash(items[i].label,strlen(items[i].label))^sb_hash((const char *)&items[i].bounds,sizeof(items[i].bounds))^items[i].role^items[i].anchor^(items[i].caret<<1)^(uint64_t)items[i].selected^((uint64_t)items[i].editable<<8); if (items[i].value) signature^=sb_hash(items[i].value,strlen(items[i].value)); if (items[i].parent) signature^=sb_hash(items[i].parent,strlen(items[i].parent)); signature^=(uint64_t)items[i].level<<16;
        signature^=sb_hash((const char *)&items[i].font_size,sizeof(items[i].font_size));
        for (size_t j=0;items[i].styles && items[i].style_count<=SB_INLINE_LIMIT && j<items[i].style_count;++j) {
            SBTextSpan span=items[i].styles[j];
            signature=signature*1099511628211ULL^span.offset^span.length^span.style;
        } signature^=items[i].row^((uint64_t)items[i].column<<16)^((uint64_t)items[i].rows<<32)^((uint64_t)items[i].columns<<48); }
    SDL_LockMutex(a->mutex);
    bool changed=signature!=a->signature || context!=a->context;
    if (changed) {
        if (context!=a->context) { free(a->identities); free(a->identity_slots); a->identities=NULL; a->identity_slots=NULL; a->identity_count=a->identity_capacity=a->slot_capacity=0; }
        Item *next=calloc(count,sizeof(*next));
        if (count && !next) { SDL_UnlockMutex(a->mutex); return; }
        bool complete=true;
        for (size_t i=0;i<count;++i) {
            snprintf(next[i].id,sizeof(next[i].id),"%s",items[i].id); next[i].label=copy(items[i].label); if (!next[i].label) complete=false;
            next[i].value=copy(items[i].value); if (items[i].value && !next[i].value) complete=false;
            next[i].node=identify(a,items[i].id,context); if (!next[i].node) complete=false;
            if (items[i].value && items[i].style_count && !items[i].editable) {
                size_t length=strlen(items[i].value),covered=0;
                bool valid=items[i].styles && items[i].style_count<=SB_INLINE_LIMIT;
                for (size_t j=0;valid && j<items[i].style_count;++j) {
                    SBTextSpan span=items[i].styles[j];
                    valid=span.offset==covered && span.length && span.length<=length-covered;
                    if (valid) valid=sb_utf8_valid(items[i].value+span.offset,span.length);
                    if (valid) covered+=span.length;
                }
                valid=valid && covered==length;
                if (valid) {
                    next[i].runs=calloc(items[i].style_count,sizeof(*next[i].runs));
                    if (!next[i].runs) complete=false;
                    else {
                        next[i].run_count=items[i].style_count; next[i].font_size=items[i].font_size;
                        for (size_t j=0;j<items[i].style_count;++j) {
                            char id[100]; snprintf(id,sizeof(id),"@text:%llu:%zu",(unsigned long long)next[i].node,items[i].styles[j].offset);
                            accesskit_node_id node=j ? identify(a,id,context) : next[i].node;
                            if (!node) complete=false;
                            next[i].runs[j]=(StyledRun){items[i].styles[j],run_id(node)};
                        }
                    }
                }
            }
            snprintf(next[i].parent,sizeof(next[i].parent),"%s",items[i].parent ? items[i].parent : ""); next[i].level=items[i].level;
            next[i].row=items[i].row; next[i].column=items[i].column; next[i].rows=items[i].rows; next[i].columns=items[i].columns;
            next[i].bounds=items[i].bounds; next[i].role=items[i].role; next[i].editable=items[i].editable; next[i].selected=items[i].selected; next[i].anchor=items[i].anchor; next[i].caret=items[i].caret;
        }
        if (!complete) { for (size_t i=0;i<count;++i) { free(next[i].label); free(next[i].value); free(next[i].runs); } free(next); SDL_UnlockMutex(a->mutex); return; }
        bool controls=context!=a->context || count!=a->count;
        if (!controls) for (size_t i=0;i<count;++i) if (strcmp(a->items[i].id,next[i].id) || strcmp(a->items[i].parent,next[i].parent) || a->items[i].role!=next[i].role) { controls=true; break; }
        clear_items(a); a->items=next; a->count=count; a->signature=signature; a->context=context;
        if (controls) ++a->generation;
        snprintf(a->title,sizeof(a->title),"%s",title); snprintf(a->focus,sizeof(a->focus),"%s",focus); snprintf(a->message,sizeof(a->message),"%s",message); a->modal=modal;
    }
    SDL_UnlockMutex(a->mutex);
    if (!a->adapter) return;
#if defined(__APPLE__)
    accesskit_macos_queued_events *focus_events=accesskit_macos_subclassing_adapter_update_view_focus_state(a->adapter,(SDL_GetWindowFlags(a->window)&SDL_WINDOW_INPUT_FOCUS)!=0);
    if (focus_events) accesskit_macos_queued_events_raise(focus_events);
    if (changed) { accesskit_macos_queued_events *e=accesskit_macos_subclassing_adapter_update_if_active(a->adapter,factory,a); if (e) accesskit_macos_queued_events_raise(e); }
#elif defined(_WIN32)
    if (changed) { accesskit_windows_queued_events *e=accesskit_windows_subclassing_adapter_update_if_active(a->adapter,factory,a); if (e) accesskit_windows_queued_events_raise(e); }
#else
    int x=0,y=0,w=0,h=0,top=0,left=0,bottom=0,right=0; SDL_GetWindowPosition(a->window,&x,&y); SDL_GetWindowSize(a->window,&w,&h); SDL_GetWindowBordersSize(a->window,&top,&left,&bottom,&right);
    accesskit_rect outer={x-left,y-top,x+w+right,y+h+bottom},inner={x,y,x+w,y+h};
    accesskit_unix_adapter_set_root_window_bounds(a->adapter,outer,inner); accesskit_unix_adapter_update_window_focus_state(a->adapter,(SDL_GetWindowFlags(a->window)&SDL_WINDOW_INPUT_FOCUS)!=0);
    if (changed) accesskit_unix_adapter_update_if_active(a->adapter,factory,a);
#endif
}
bool sb_accessibility_next_action(SBAccessibility *a,SBAccessibleAction *out) {
    if (!a) return false; SDL_LockMutex(a->mutex); Pending *p=a->head;
    if (p) { a->head=p->next; if (!a->head) a->tail=NULL; --a->queued; if (p->action.value) a->queued_bytes-=strlen(p->action.value)+1; *out=p->action; free(p); }
    SDL_UnlockMutex(a->mutex); return p!=NULL;
}
bool sb_accessibility_current(SBAccessibility *a,const SBAccessibleAction *action,uint64_t context) { SDL_LockMutex(a->mutex); bool valid=action->generation==a->generation && context==a->context; SDL_UnlockMutex(a->mutex); return valid; }
void sb_accessibility_action_free(SBAccessibleAction *action) { free(action->value); memset(action,0,sizeof(*action)); }
void sb_accessibility_free(SBAccessibility *a) {
    if (!a) return;
    SDL_LockMutex(a->mutex); a->alive=false; SDL_UnlockMutex(a->mutex);
#if defined(__APPLE__)
    if (a->adapter) accesskit_macos_subclassing_adapter_free(a->adapter);
#elif defined(_WIN32)
    if (a->adapter) accesskit_windows_subclassing_adapter_free(a->adapter);
#else
    if (a->adapter) accesskit_unix_adapter_free(a->adapter);
#endif
    Pending *p=a->head; while (p) { Pending *next=p->next; sb_accessibility_action_free(&p->action); free(p); p=next; }
    clear_items(a); free(a->identities); free(a->identity_slots); SDL_DestroyMutex(a->mutex); free(a);
}
