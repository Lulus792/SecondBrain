#include "accessibility.h"
#include "native_text.h"
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
static IMP mac_row_count_original,mac_column_count_original,mac_cell_original;
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
static unsigned long mac_count(void *array) {
    return array ? ((unsigned long(*)(void *,SEL))objc_msgSend)(array,sel_registerName("count")) : 0;
}
static void *mac_at(void *array,unsigned long index) {
    return index<mac_count(array) ? ((void *(*)(void *,SEL,unsigned long))objc_msgSend)(array,sel_registerName("objectAtIndex:"),index) : NULL;
}
static long mac_row_count(void *self,SEL selector) {
    if(mac_table(self))return (long)mac_count(mac_rows(self,sel_registerName("accessibilityRows")));
    return mac_row_count_original ? ((long(*)(void *,SEL))mac_row_count_original)(self,selector) : 0;
}
static long mac_column_count(void *self,SEL selector) {
    if(!mac_table(self))return mac_column_count_original ? ((long(*)(void *,SEL))mac_column_count_original)(self,selector) : 0;
    void *rows=mac_rows(self,sel_registerName("accessibilityRows"));unsigned long count=0;
    for(unsigned long i=0;i<mac_count(rows);++i){
        void *row=mac_at(rows,i),*cells=((void *(*)(void *,SEL))objc_msgSend)(row,sel_registerName("accessibilityChildren"));
        unsigned long columns=mac_count(cells);if(columns>count)count=columns;
    }
    return (long)count;
}
static void *mac_cell(void *self,SEL selector,long column,long row) {
    if(!mac_table(self))return mac_cell_original ? ((void *(*)(void *,SEL,long,long))mac_cell_original)(self,selector,column,row) : NULL;
    if(column<0 || row<0)return NULL;
    void *rows=mac_rows(self,sel_registerName("accessibilityRows"));void *selected=mac_at(rows,(unsigned long)row);
    if(!selected)return NULL;
    void *cells=((void *(*)(void *,SEL))objc_msgSend)(selected,sel_registerName("accessibilityChildren"));
    return mac_at(cells,(unsigned long)column);
}
static void mac_replace(Class node,const char *name,IMP replacement,const char *encoding,IMP *original) {
    SEL selector=sel_registerName(name);Method method=class_getInstanceMethod(node,selector);
    if(method){*original=method_getImplementation(method);method_setImplementation(method,replacement);}
    else class_addMethod(node,selector,replacement,encoding);
}
static BOOL mac_allowed(void *self,SEL selector,SEL requested) {
    if (mac_table(self) && (requested==sel_registerName("accessibilityRows") ||
        requested==sel_registerName("accessibilityRowCount") || requested==sel_registerName("accessibilityColumnCount") ||
        requested==sel_registerName("accessibilityCellForColumn:row:"))) return YES;
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
            mac_replace(node,"accessibilityRowCount",(IMP)mac_row_count,"q@:",&mac_row_count_original);
            mac_replace(node,"accessibilityColumnCount",(IMP)mac_column_count,"q@:",&mac_column_count_original);
            mac_replace(node,"accessibilityCellForColumn:row:",(IMP)mac_cell,"@@:qq",&mac_cell_original);
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
typedef struct { SBNativeTextRun text; accesskit_node_id node; } StyledRun;
typedef struct {
    char id[100],parent[100]; char *label,*value;
    unsigned level; size_t row,column,rows,columns;
    accesskit_node_id node; struct nk_rect bounds; accesskit_role role;
    bool editable,selected; size_t anchor,caret;
    StyledRun *runs; size_t run_count; float font_size;
    SBTextSpan *source_styles;size_t style_count;
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
    const char *value,const SBNativeTextRun *run,accesskit_rect bounds,float font_size) {
    size_t length=run->span.length;unsigned style=run->span.style;
    accesskit_node *text=accesskit_node_new(ACCESSKIT_ROLE_TEXT_RUN);
    accesskit_node_set_value_with_length(text,value,length); accesskit_node_set_bounds(text,bounds);
    if (font_size>0) {
        accesskit_node_set_font_size(text,font_size);
        accesskit_node_set_font_family(text,style&SB_TEXT_CODE ? "Noto Sans Mono" : "Noto Sans");
        accesskit_node_set_font_weight(text,style&SB_TEXT_BOLD ? 700 : 400);
        if (style&SB_TEXT_ITALIC) accesskit_node_set_italic(text);
    }
    accesskit_node_set_character_lengths(text,run->characters,run->lengths);
    accesskit_node_set_word_starts(text,run->word_count,run->words);
    accesskit_node_push_child(parent,id); accesskit_tree_update_push_node(tree,id,text);
}
static accesskit_text_position native_position(const Item *v,size_t scalar) {
    size_t run=0;while(run+1<v->run_count && v->runs[run+1].text.scalars[0]<=scalar)++run;
    const StyledRun *r=&v->runs[run];size_t lo=0,hi=r->text.characters;
    while(lo<hi){size_t mid=lo+(hi-lo)/2;if(r->text.scalars[mid]<scalar)lo=mid+1;else hi=mid;}
    return (accesskit_text_position){r->node,lo};
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
            if (v->run_count) {
                for (size_t j=0;j<v->run_count;++j) {
                    StyledRun run=v->runs[j];
                    text_run(tree,node,run.node,v->value+run.text.span.offset,&run.text,rect,v->font_size);
                }
            }
            if (v->run_count && v->editable && v->role==ACCESSKIT_ROLE_MULTILINE_TEXT_INPUT) {
                accesskit_text_selection selection={native_position(v,v->anchor),native_position(v,v->caret)};
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
static bool submit_locked(SBAccessibility *a,Item *item,accesskit_action kind,const char *value,size_t anchor,size_t caret) {
    size_t bytes=value ? strlen(value)+1 : 0;
    if (value && (bytes>SB_TEXT_LIMIT || !sb_utf8_valid(value,bytes-1))) return false;
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
    return accepted;
}
bool sb_accessibility_submit(SBAccessibility *a,accesskit_node_id node,accesskit_action kind,const char *value,size_t anchor,size_t caret) {
    SDL_LockMutex(a->mutex);Item *item=NULL;
    for(size_t i=0;i<a->count;++i)if(a->items[i].node==node || run_id(a->items[i].node)==node){item=&a->items[i];break;}
    bool accepted=submit_locked(a,item,kind,value,anchor,caret);SDL_UnlockMutex(a->mutex);return accepted;
}
static bool source_position(const Item *item,accesskit_text_position position,size_t *scalar) {
    for(size_t i=0;i<item->run_count;++i)if(item->runs[i].node==position.node){
        if(position.character_index>item->runs[i].text.characters)return false;
        *scalar=item->runs[i].text.scalars[position.character_index];return true;
    }return false;
}
bool sb_accessibility_select(SBAccessibility *a,accesskit_node_id node,accesskit_text_selection selection) {
    SDL_LockMutex(a->mutex);Item *item=NULL;size_t anchor=0,caret=0;
    for(size_t i=0;i<a->count;++i)if(a->items[i].node==node){item=&a->items[i];break;}
    bool accepted=item && source_position(item,selection.anchor,&anchor) && source_position(item,selection.focus,&caret) && submit_locked(a,item,ACCESSKIT_ACTION_SET_TEXT_SELECTION,NULL,anchor,caret);
    SDL_UnlockMutex(a->mutex);return accepted;
}
void sb_accessibility_request(SBAccessibility *a,accesskit_action_request *request) {
    const char *value=NULL; size_t anchor=0,caret=0;
    SDL_LockMutex(a->mutex);Item *item=NULL;bool valid=true;
    for(size_t i=0;i<a->count;++i)if(a->items[i].node==request->target_node || run_id(a->items[i].node)==request->target_node){item=&a->items[i];break;}
    if(request->action==ACCESSKIT_ACTION_SET_TEXT_SELECTION)valid=request->data.has_value && request->data.value.tag==ACCESSKIT_ACTION_DATA_SET_TEXT_SELECTION;
    if (request->data.has_value && request->data.value.tag==ACCESSKIT_ACTION_DATA_VALUE) value=request->data.value.value;
    if (request->data.has_value && request->data.value.tag==ACCESSKIT_ACTION_DATA_SET_TEXT_SELECTION) {
        accesskit_text_selection selection=request->data.value.set_text_selection;
        valid=item && source_position(item,selection.anchor,&anchor) && source_position(item,selection.focus,&caret);
    }
    if(valid)submit_locked(a,item,request->action,value,anchor,caret);
    SDL_UnlockMutex(a->mutex);
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
static void clear_items(SBAccessibility *a) { for (size_t i=0;i<a->count;++i) { free(a->items[i].label); free(a->items[i].value); free(a->items[i].runs);free(a->items[i].source_styles); } free(a->items); a->items=NULL; a->count=0; }
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
            next[i].font_size=items[i].font_size;
            bool styled=items[i].styles && items[i].style_count<=SB_INLINE_LIMIT && !items[i].editable;
            next[i].style_count=styled ? items[i].style_count : 0;
            if(next[i].style_count){next[i].source_styles=malloc(next[i].style_count*sizeof(*next[i].source_styles));
                if(!next[i].source_styles)complete=false;else memcpy(next[i].source_styles,items[i].styles,next[i].style_count*sizeof(*next[i].source_styles));}
            bool structured=false;
            if(items[i].role==ACCESSKIT_ROLE_DOCUMENT && (!items[i].parent || !items[i].parent[0]))
                for(size_t j=0;j<count;++j)if(j!=i && items[j].parent && !strcmp(items[j].parent,items[i].id)){structured=true;break;}
            if(items[i].value && !structured){
                Item *old=context==a->context && i<a->count && !strcmp(items[i].id,a->items[i].id) ? &a->items[i] : NULL;
                bool reuse=old && old->value && old->run_count && !strcmp(old->value,items[i].value) && old->style_count==next[i].style_count;
                for(size_t j=0;reuse && j<next[i].style_count;++j){SBTextSpan x=old->source_styles[j],y=items[i].styles[j];reuse=x.offset==y.offset && x.length==y.length && x.style==y.style;}
                SBNativeText layout={0};
                if(reuse){next[i].run_count=old->run_count;next[i].runs=malloc(old->run_count*sizeof(*next[i].runs));
                    if(!next[i].runs)complete=false;else memcpy(next[i].runs,old->runs,old->run_count*sizeof(*next[i].runs));}
                else if(sb_native_text(items[i].value,strlen(items[i].value),styled ? items[i].styles : NULL,next[i].style_count,&layout).code==SB_OK){
                    next[i].runs=calloc(layout.count,sizeof(*next[i].runs));
                    if(!next[i].runs)complete=false;else{next[i].run_count=layout.count;
                        for(size_t j=0;j<layout.count;++j){char id[100];snprintf(id,sizeof(id),"@text:%llu:%zu",(unsigned long long)next[i].node,layout.runs[j].span.offset);
                            accesskit_node_id node=j ? identify(a,id,context) : next[i].node;if(!node)complete=false;
                            next[i].runs[j]=(StyledRun){layout.runs[j],run_id(node)};}}
                    sb_native_text_free(&layout);
                }else complete=false;
            }
            snprintf(next[i].parent,sizeof(next[i].parent),"%s",items[i].parent ? items[i].parent : ""); next[i].level=items[i].level;
            next[i].row=items[i].row; next[i].column=items[i].column; next[i].rows=items[i].rows; next[i].columns=items[i].columns;
            next[i].bounds=items[i].bounds; next[i].role=items[i].role; next[i].editable=items[i].editable; next[i].selected=items[i].selected; next[i].anchor=items[i].anchor; next[i].caret=items[i].caret;
        }
        if (!complete) { for (size_t i=0;i<count;++i) { free(next[i].label); free(next[i].value); free(next[i].runs); free(next[i].source_styles); } free(next); SDL_UnlockMutex(a->mutex); return; }
        bool controls=context!=a->context || count!=a->count;
        if (!controls) for (size_t i=0;i<count;++i) if (strcmp(a->items[i].id,next[i].id) || strcmp(a->items[i].parent,next[i].parent) || a->items[i].role!=next[i].role) { controls=true; break; }
        if(!controls)for(size_t i=0;i<count;++i)if(next[i].editable && ((!next[i].value)!=(!a->items[i].value) || (next[i].value && strcmp(next[i].value,a->items[i].value)))){controls=true;break;}
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
