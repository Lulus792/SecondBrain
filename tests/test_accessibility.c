#include "desktop.h"
#include "platform.h"
#include "native_probe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef __APPLE__
#include <objc/runtime.h>
#include <objc/message.h>
#include <dlfcn.h>
static void *send(void *object,const char *selector) {
    SEL method=sel_registerName(selector);
    if (!object || !((BOOL(*)(void *,SEL,SEL))objc_msgSend)(object,sel_registerName("respondsToSelector:"),method)) return NULL;
    return ((void *(*)(void *,SEL))objc_msgSend)(object,method);
}
static void *string(const char *text) { return ((void *(*)(void *,SEL,const char *))objc_msgSend)(objc_getClass("NSString"),sel_registerName("stringWithUTF8String:"),text); }
static const char *utf8(void *s) { return s ? ((const char *(*)(void *,SEL))objc_msgSend)(s,sel_registerName("UTF8String")) : NULL; }
static void *native_find(void *object,const char *label,unsigned depth) {
    if (!object || depth>8) return NULL;
    const char *name=utf8(send(object,"accessibilityLabel"));
    if (name && !strcmp(name,label)) return object;
    name=utf8(send(object,"accessibilityTitle")); if (name && !strcmp(name,label)) return object;
    name=utf8(send(object,"accessibilityDescription")); if (name && !strcmp(name,label)) return object;
    name=utf8(send(object,"accessibilityIdentifier")); if (name && !strcmp(name,label)) return object;
    name=utf8(send(object,"accessibilityValue")); if (name && !strcmp(name,label)) return object;
    void *children=send(object,"accessibilityChildren");
    size_t count=children ? ((size_t(*)(void *,SEL))objc_msgSend)(children,sel_registerName("count")) : 0;
    for (size_t i=0;i<count;++i) {
        void *child=((void *(*)(void *,SEL,size_t))objc_msgSend)(children,sel_registerName("objectAtIndex:"),i);
        void *found=native_find(child,label,depth+1); if (found) return found;
    }
    return NULL;
}
#endif
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"ACCESSIBILITY FAIL %d: %s\n",__LINE__,#x); return 1; } } while (0)
#define OK(x) CHECK((x).code==SB_OK)
static void frame(SBDesktop *d) { nk_input_begin(d->ui.ctx); SDL_Event e; while (SDL_PollEvent(&e)) sb_desktop_event(d,&e); sb_desktop_tick(d,1.0f/60); nk_input_end(d->ui.ctx); sb_desktop_frame(d); sb_ui_draw(&d->ui); SDL_RenderPresent(d->ui.renderer); sb_desktop_apply(d); }
static void pump(void *context) { frame(context); }
static char *dump(SBDesktop *d) { accesskit_tree_update *tree=sb_accessibility_tree(d->accessibility); char *text=accesskit_tree_update_debug(tree); accesskit_tree_update_free(tree); return text; }
int main(int argc,char **argv) {
    CHECK(argc==3); SBDesktop d; char root[SB_PATH_CAP],suffix[90];
    snprintf(suffix,sizeof(suffix),"run-%lu-%lu",sb_process_id(),(unsigned long)time(NULL)); OK(sb_path_join(root,sizeof(root),argv[2],suffix));
    OK(sb_desktop_init(&d,root,argv[1],true)); CHECK(d.accessibility!=NULL);
    OK(sb_app_new_project(&d.model,"projekt","Projekt ü",NULL)); OK(sb_app_new_note(&d.model,"knowledge","notiz","Notiz"));
    strcpy(d.model.editor,"# Notiz\n\nLesbarer Inhalt ü.\n"); OK(sb_app_save(&d.model)); frame(&d); frame(&d);
    char *text=dump(&d); CHECK(strstr(text,"Lesbarer Inhalt") && strstr(text,"Neue Notiz")); accesskit_string_free(text);
#ifdef __APPLE__
    void *window=SDL_GetPointerProperty(SDL_GetWindowProperties(d.ui.window),SDL_PROP_WINDOW_COCOA_WINDOW_POINTER,NULL);
    void *view=send(window,"contentView"); CHECK(view!=NULL);
    void *button=native_find(view,"Neue Notiz",0); CHECK(button!=NULL);
    CHECK(!strcmp(utf8(send(button,"accessibilityRole")),"AXButton"));
    ((void(*)(void *,SEL))objc_msgSend)(button,sel_registerName("accessibilityPerformPress")); frame(&d); frame(&d);
    CHECK(d.form==SB_FORM_NOTE);
    void *field=native_find(view,"Titel",0); CHECK(field!=NULL);
    ((void(*)(void *,SEL,void *))objc_msgSend)(field,sel_registerName("setAccessibilityValue:"),string("Native Notiz ü")); frame(&d); frame(&d);
    CHECK(!strcmp(d.name,"Native Notiz ü"));
    void *submit=native_find(view,"Anlegen",0); CHECK(submit!=NULL);
    ((void(*)(void *,SEL))objc_msgSend)(submit,sel_registerName("accessibilityPerformPress")); frame(&d); frame(&d);
    CHECK(d.form==SB_FORM_NONE && !strcmp(d.model.path,"knowledge/native-notiz-ue.md"));
    void *editor=native_find(view,"Dokument bearbeiten",0); CHECK(editor!=NULL);
    ((void(*)(void *,SEL,void *))objc_msgSend)(editor,sel_registerName("setAccessibilityValue:"),string("# Native Notiz\n\nÜber den Provider bearbeitet.\n")); frame(&d); frame(&d);
    CHECK(strstr(d.model.editor,"Provider bearbeitet") && sb_app_dirty(&d.model));
    void *save=native_find(view,"Speichern",0); CHECK(save!=NULL);
    ((void(*)(void *,SEL))objc_msgSend)(save,sel_registerName("accessibilityPerformPress")); frame(&d); frame(&d); CHECK(!sb_app_dirty(&d.model));
    typedef struct { size_t location,length; } NativeRange;
    ((void(*)(void *,SEL,NativeRange))objc_msgSend)(editor,sel_registerName("setAccessibilitySelectedTextRange:"),(NativeRange){2,6}); frame(&d); frame(&d);
    CHECK(d.text_edit.select_start==2 && d.text_edit.select_end==8);
    const char *selected=utf8(send(editor,"accessibilitySelectedText")); CHECK(selected && !strcmp(selected,"Native"));
    /* Native callback is queued; changing document before consumption must reject it. */
    ((void(*)(void *,SEL,void *))objc_msgSend)(editor,sel_registerName("setAccessibilityValue:"),string("Falsches Dokument überschreiben"));
    OK(sb_app_request(&d.model,SB_ACT_NOTE,"PROJECT.md")); frame(&d); frame(&d);
    CHECK(!strstr(d.model.editor,"Falsches Dokument") && !sb_app_dirty(&d.model));
    /* A value action is never applied to the read-only document surface. */
    d.editing=false; d.card=true; frame(&d);
    void *reader=native_find(view,d.model.title,0); CHECK(reader!=NULL);
    const char *read_value=utf8(send(reader,"accessibilityValue")); CHECK(read_value && strstr(read_value,"Projekt"));
    printf("Native macOS accessibility provider queried and used: button press, text value, project note creation, editor and save.\n");
#endif
#if defined(_WIN32) || defined(SB_ATSPI_TEST)
    CHECK(sb_native_probe(d.ui.window,"Neue Notiz",NULL,SB_NATIVE_PRESS,NULL,0,pump,&d)); CHECK(d.form==SB_FORM_NOTE);
    CHECK(sb_native_probe(d.ui.window,"Titel","Native Notiz ü",SB_NATIVE_SET_VALUE,NULL,0,pump,&d)); CHECK(!strcmp(d.name,"Native Notiz ü"));
    char native_value[1024];
    CHECK(sb_native_probe(d.ui.window,"Titel",NULL,SB_NATIVE_READ_VALUE,native_value,sizeof(native_value),pump,&d)); CHECK(!strcmp(native_value,"Native Notiz ü"));
    CHECK(sb_native_probe(d.ui.window,"Anlegen",NULL,SB_NATIVE_PRESS,NULL,0,pump,&d)); CHECK(d.form==SB_FORM_NONE && !strcmp(d.model.path,"knowledge/native-notiz-ue.md"));
    CHECK(sb_native_probe(d.ui.window,"Dokument bearbeiten","# Native Notiz\n\nÜber den Provider bearbeitet.\n",SB_NATIVE_SET_VALUE,NULL,0,pump,&d)); CHECK(strstr(d.model.editor,"Provider bearbeitet") && sb_app_dirty(&d.model));
    CHECK(sb_native_probe(d.ui.window,"Dokument bearbeiten",NULL,SB_NATIVE_READ_VALUE,native_value,sizeof(native_value),pump,&d)); CHECK(strstr(native_value,"Provider bearbeitet"));
    CHECK(sb_native_probe(d.ui.window,"Speichern",NULL,SB_NATIVE_PRESS,NULL,0,pump,&d)); CHECK(!sb_app_dirty(&d.model));
    printf("Native UIA/AT-SPI client queried and used: roles, button press, Unicode text value, note creation, editor and save.\n");
#elif !defined(__APPLE__)
    printf("Native AT-SPI client probe unavailable in this build; only snapshot/queue contract tested.\n");
#endif
    const char *help_line="Tab / Umschalt+Tab: Fokus · Enter/Leertaste: aktivieren";
    d.form=SB_FORM_HELP; frame(&d); frame(&d);
    text=dump(&d); CHECK(strstr(text,help_line) && strstr(text,"Tastaturhilfe") && !strstr(text,"Lesbarer Inhalt")); accesskit_string_free(text);
#ifdef __APPLE__
    void *help_heading=native_find(view,"modal-title",0); CHECK(help_heading);
    void **heading_role=dlsym(RTLD_DEFAULT,"NSAccessibilityHeadingRole");
    const char *actual_role=utf8(send(help_heading,"accessibilityRole")); CHECK(actual_role);
    if (heading_role) CHECK(!strcmp(actual_role,utf8(*heading_role)));
    else CHECK(!strcmp(actual_role,"Heading")); /* Older AppKit: observe adapter role, no assistive heading claim. */
    CHECK(native_find(view,help_line,0));
#elif defined(_WIN32) || defined(SB_ATSPI_TEST)
    CHECK(sb_native_probe(d.ui.window,help_line,NULL,SB_NATIVE_READ_NAME,native_value,sizeof(native_value),pump,&d)); CHECK(!strcmp(native_value,help_line));
#endif
    d.form=SB_FORM_NONE; frame(&d); frame(&d);
    text=dump(&d); CHECK(!strstr(text,help_line)); accesskit_string_free(text);
    strcpy(d.model.editor,"Ungespeicherter Entwurf");
    OK(sb_app_request(&d.model,SB_ACT_NOTE,"STATE.md")); CHECK(d.model.guard); frame(&d);
    bool title_bounds=false; for (size_t i=0;i<d.passive_count;++i) if (!strcmp(d.passive[i].id,"modal-title")) title_bounds=d.passive[i].bounds.w>0 && d.passive[i].bounds.h>0;
    CHECK(title_bounds);
    text=dump(&d); CHECK(strstr(text,"Änderungen erhalten") && strstr(text,"Dieses Dokument enthält ungespeicherte Änderungen"));
    CHECK(!strstr(text,"Projektdokumente")); accesskit_string_free(text);
#ifdef SB_ATSPI_TEST
    CHECK(sb_native_cache_check());
#endif
    sb_desktop_free(&d);
    /* Snapshot/queue contract is tested on every OS, independently of the native client. */
    CHECK(SDL_Init(SDL_INIT_VIDEO));
    SDL_Window *test_window=SDL_CreateWindow("Accessibility contract",400,300,SDL_WINDOW_HIDDEN); CHECK(test_window);
    SBAccessibility *a=sb_accessibility_new(test_window); CHECK(a);
    SBAccessibleItem item={.id="text",.label="Editor",.value="AüB",.bounds={0,0,200,100},.role=ACCESSKIT_ROLE_MULTILINE_TEXT_INPUT,.editable=true};
    sb_accessibility_update(a,"Test","text",&item,1,"",false,1);
    CHECK(sb_accessibility_submit(a,17,ACCESSKIT_ACTION_SET_TEXT_SELECTION,NULL,1,2));
    SBAccessibleAction pending; CHECK(sb_accessibility_next_action(a,&pending));
    CHECK(pending.anchor==1 && pending.caret==2 && sb_accessibility_current(a,&pending,1)); sb_accessibility_action_free(&pending);
    CHECK(!sb_accessibility_submit(a,17,ACCESSKIT_ACTION_SET_TEXT_SELECTION,NULL,1,4));
    CHECK(!sb_accessibility_submit(a,17,ACCESSKIT_ACTION_SET_VALUE,"\xff",0,0));
    CHECK(!sb_accessibility_submit(a,17,ACCESSKIT_ACTION_SET_VALUE,NULL,0,0));
    CHECK(sb_accessibility_submit(a,17,ACCESSKIT_ACTION_SET_VALUE,"Entwurf",0,0));
    sb_accessibility_update(a,"Test","text",&item,1,"",false,2);
    CHECK(sb_accessibility_next_action(a,&pending)); CHECK(!sb_accessibility_current(a,&pending,2)); sb_accessibility_action_free(&pending);
    CHECK(!sb_accessibility_submit(a,17,ACCESSKIT_ACTION_SET_VALUE,"Veralteter Knoten",0,0));
    item.editable=false; item.role=ACCESSKIT_ROLE_DOCUMENT;
    sb_accessibility_update(a,"Test","text",&item,1,"",false,2);
    CHECK(!sb_accessibility_submit(a,18,ACCESSKIT_ACTION_SET_VALUE,"Schreibgeschützt",0,0));
    for (unsigned i=0;i<64;++i) CHECK(sb_accessibility_submit(a,18,ACCESSKIT_ACTION_FOCUS,NULL,0,0));
    CHECK(!sb_accessibility_submit(a,18,ACCESSKIT_ACTION_FOCUS,NULL,0,0));
    unsigned drained=0; while (sb_accessibility_next_action(a,&pending)) { ++drained; sb_accessibility_action_free(&pending); }
    CHECK(drained==64); CHECK(sb_accessibility_submit(a,18,ACCESSKIT_ACTION_FOCUS,NULL,0,0));
    char caption[701]; memset(caption,'x',sizeof(caption)-1); caption[700]=0;
    item.id="caption"; item.label=caption; item.value=NULL; item.role=ACCESSKIT_ROLE_LABEL;
    sb_accessibility_update(a,"Test","",&item,1,"",false,3);
    accesskit_tree_update *caption_tree=sb_accessibility_tree(a); text=accesskit_tree_update_debug(caption_tree); CHECK(strstr(text,caption));
    accesskit_string_free(text); accesskit_tree_update_free(caption_tree);
    CHECK(!sb_accessibility_submit(a,19,ACCESSKIT_ACTION_FOCUS,NULL,0,0));
    CHECK(!sb_accessibility_submit(a,19,ACCESSKIT_ACTION_CLICK,NULL,0,0));
    sb_accessibility_free(a); SDL_DestroyWindow(test_window); SDL_Quit();
    printf("%u accessibility assertions passed.\n",checks); return 0;
}
