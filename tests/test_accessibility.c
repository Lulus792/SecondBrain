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
static void frame_run(SBDesktop *d,bool draw) {
    nk_input_begin(d->ui.ctx); SDL_Event e;
    while (SDL_PollEvent(&e)) sb_desktop_event(d,&e);
    sb_desktop_tick(d,1.0f/60); nk_input_end(d->ui.ctx); sb_desktop_frame(d);
    if (draw) { sb_ui_draw(&d->ui); SDL_RenderPresent(d->ui.renderer); }
    else nk_clear(d->ui.ctx);
    sb_desktop_apply(d);
}
static void frame(SBDesktop *d) { frame_run(d,true); }
#if defined(_WIN32) || defined(SB_ATSPI_TEST)
/* Polling the real client still processes input, layout, native updates and
   actions. The next observed state is rasterized by frame()/capture. */
static void pump(void *context) { frame_run(context,false); }
#endif
static Uint64 phase_start;
static void checkpoint(const char *phase) {
    Uint64 now=SDL_GetTicks();
    printf("Native checkpoint: %s (%llu ms)\n",phase,(unsigned long long)(now-phase_start));
    fflush(stdout); phase_start=now;
}
static char *dump(SBDesktop *d) { accesskit_tree_update *tree=sb_accessibility_tree(d->accessibility); char *text=accesskit_tree_update_debug(tree); accesskit_tree_update_free(tree); return text; }
int main(int argc,char **argv) {
    phase_start=SDL_GetTicks(); CHECK(argc==3); SBDesktop d; char root[SB_PATH_CAP],suffix[90];
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
    ((void(*)(void *,SEL,void *))objc_msgSend)(editor,sel_registerName("setAccessibilityValue:"),string("e\xcc\x81")); frame(&d); frame(&d);
    ((void(*)(void *,SEL,NativeRange))objc_msgSend)(editor,sel_registerName("setAccessibilitySelectedTextRange:"),(NativeRange){1,1}); frame(&d); frame(&d);
    CHECK(d.text_edit.select_start==0 && d.text_edit.select_end==2);
    selected=utf8(send(editor,"accessibilitySelectedText")); CHECK(selected && !strcmp(selected,"e\xcc\x81"));
    ((void(*)(void *,SEL,void *))objc_msgSend)(editor,sel_registerName("setAccessibilityValue:"),string("# Native Notiz\n\nÜber den Provider bearbeitet.\n")); frame(&d); frame(&d);
    ((void(*)(void *,SEL))objc_msgSend)(save,sel_registerName("accessibilityPerformPress")); frame(&d); frame(&d); CHECK(!sb_app_dirty(&d.model));
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
    /* Read-only document structure and complete text are exposed by the real renderer. */
    const char *outline="# Struktur ü\n\n## Erster Abschnitt\n\nEin Absatz mit [Stand](../STATE.md).\n\n### Unterabschnitt\n\n```c\nint wert = 1;\n```\n\n####### Kein Titel\n\n";
    char structured[16000]; snprintf(structured,sizeof(structured),"%s",outline);
    size_t used=strlen(structured); memset(structured+used,'x',6000); used+=6000;
    strcpy(structured+used," Ende αΩ.\n\n## Letzter Abschnitt\n\nSchluss.\n");
    snprintf(d.model.editor,SB_TEXT_LIMIT+1,"%s",structured); OK(sb_app_save(&d.model));
    d.editing=false; d.card=true; frame(&d); frame(&d);
    unsigned headings=0,paragraphs=0,codes=0; char first_id[100]={0},last_id[100]={0};
    for (size_t i=0;i<d.passive_count;++i) {
        SBPassiveText *p=&d.passive[i]; if (strcmp(p->parent,"reader")) continue;
        if (p->role==ACCESSKIT_ROLE_HEADING) {
            ++headings;
            if (!strcmp(p->text,"Erster Abschnitt")) { CHECK(p->level==2); strcpy(first_id,p->id); }
            if (!strcmp(p->text,"Letzter Abschnitt")) strcpy(last_id,p->id);
        }
        if (p->role==ACCESSKIT_ROLE_PARAGRAPH) { ++paragraphs; if (strstr(p->text,"Ende αΩ")) CHECK(strlen(p->text)>6000); }
        if (p->role==ACCESSKIT_ROLE_CODE) ++codes;
    }
    CHECK(headings==4 && paragraphs>=4 && codes==1 && first_id[0] && last_id[0]);
    text=dump(&d); CHECK(strstr(text,"role: Heading") && strstr(text,"role: Paragraph") && strstr(text,"role: Code") && strstr(text,"Ende αΩ"));
    char dump_path[SB_PATH_CAP]; OK(sb_path_join(dump_path,sizeof(dump_path),root,"document-tree.txt"));
    FILE *tree_file=fopen(dump_path,"wb"); CHECK(tree_file); CHECK(fwrite(text,1,strlen(text),tree_file)==strlen(text)); fclose(tree_file);
    accesskit_string_free(text);
#ifdef __APPLE__
    void *native_heading=native_find(view,"Erster Abschnitt",0); CHECK(native_heading);
    CHECK(!strcmp(utf8(send(native_heading,"accessibilityRole")),heading_role ? utf8(*heading_role) : "Heading"));
    void *native_document=native_find(view,d.model.title,0); CHECK(native_document);
    SEL number_selector=sel_registerName("accessibilityNumberOfCharacters");
    size_t native_count=((size_t(*)(void *,SEL))objc_msgSend)(native_document,number_selector); CHECK(native_count>6000 && native_count<7000);
    const char *native_text=utf8(((void *(*)(void *,SEL,NativeRange))objc_msgSend)(native_document,sel_registerName("accessibilityStringForRange:"),(NativeRange){0,native_count}));
    CHECK(native_text && strstr(native_text,"Erster Abschnitt") && strstr(native_text,"Ende αΩ") && !strstr(native_text,"## Erster"));
    void *native_last=native_find(view,"Letzter Abschnitt",0); CHECK(native_last);
    ((void(*)(void *,SEL,void *))objc_msgSend)(native_last,sel_registerName("accessibilityPerformAction:"),string("AXScrollToVisible"));
    frame(&d); frame(&d); CHECK(d.scrolling[0].destination>0);
#elif defined(_WIN32) || defined(SB_ATSPI_TEST)
    CHECK(sb_native_probe(d.ui.window,"Erster Abschnitt",NULL,SB_NATIVE_READ_LEVEL,native_value,sizeof(native_value),pump,&d)); CHECK(!strcmp(native_value,"2"));
    char native_document[16000];
    CHECK(sb_native_probe(d.ui.window,d.model.title,NULL,SB_NATIVE_READ_DOCUMENT_TEXT,native_document,sizeof(native_document),pump,&d));
    CHECK(strstr(native_document,"Erster Abschnitt") && strstr(native_document,"Ende αΩ") && strlen(native_document)<7000 && !strstr(native_document,"## Erster"));
    CHECK(sb_native_probe(d.ui.window,"Letzter Abschnitt",NULL,SB_NATIVE_SCROLL_INTO_VIEW,NULL,0,pump,&d));
    frame(&d); frame(&d); CHECK(d.scrolling[0].destination>0);
#endif
    checkpoint("native document and heading queries");
    memset(&d.scrolling[0],0,sizeof(d.scrolling[0])); d.reset_reader=true; frame(&d); frame(&d);
    /* Keyboard heading jumps reuse the same offset and scroll path. */
    snprintf(d.focus,sizeof(d.focus),"reader");
    SDL_Event heading_key={0}; heading_key.type=SDL_EVENT_KEY_DOWN; heading_key.key.windowID=SDL_GetWindowID(d.ui.window);
    heading_key.key.key=SDLK_PAGEDOWN; heading_key.key.mod=SDL_KMOD_ALT; heading_key.key.down=true;
    sb_desktop_event(&d,&heading_key); frame(&d); CHECK(!strcmp(d.heading_cursor,first_id));
    sb_desktop_event(&d,&heading_key); frame(&d); CHECK(strcmp(d.heading_cursor,first_id));
    for (unsigned i=0;i<4;++i) { sb_desktop_event(&d,&heading_key); frame(&d); }
    CHECK(!strcmp(d.heading_cursor,last_id));
    for (unsigned i=0;i<40;++i) frame_run(&d,i==39);
    bool last_visible=false;
    for (size_t i=0;i<d.passive_count;++i) if (!strcmp(d.passive[i].id,last_id)) last_visible=d.passive[i].bounds.h>0;
    CHECK(last_visible && d.scrolling[0].position>0 && !sb_app_dirty(&d.model));
    const char *grammar="# Blockregeln ü ###\r\n\r\n  ~~~~c\r\n  ## Nur Code\r\n  ~~~\r\n  [Kein Link](../STATE.md)\r\n  ~~~~~\r\n\r\n2026 bleibt Absatz\rund wird fortgesetzt.\r\n\r\nUnterstrich-Titel\r\n---\r\n\r\n    # Eingerückter Code\r\n";
    strcpy(d.model.editor,grammar); OK(sb_app_save(&d.model));
    d.reset_reader=true; frame(&d); frame(&d);
    CHECK(!strcmp(d.model.title,"Blockregeln ü") && !strcmp(d.model.editor,grammar));
    headings=paragraphs=codes=0; bool literal_heading=false,literal_fence=false,literal_link=false,joined=false,setext=false;
    for (size_t i=0;i<d.passive_count;++i) {
        SBPassiveText *p=&d.passive[i]; if (strcmp(p->parent,"reader")) continue;
        if (p->role==ACCESSKIT_ROLE_HEADING) { ++headings; if (!strcmp(p->text,"Unterstrich-Titel")) setext=p->level==2; }
        if (p->role==ACCESSKIT_ROLE_CODE) {
            ++codes; literal_heading|=!strcmp(p->text,"## Nur Code");
            literal_fence|=!strcmp(p->text,"~~~"); literal_link|=!strcmp(p->text,"[Kein Link](../STATE.md)");
        }
        if (p->role==ACCESSKIT_ROLE_PARAGRAPH) { ++paragraphs; joined|=!strcmp(p->text,"2026 bleibt Absatz und wird fortgesetzt."); }
    }
    CHECK(headings==2 && codes==4 && paragraphs==1 && setext && literal_heading && literal_fence && literal_link && joined);
    for (size_t i=0;i<d.target_count;++i) CHECK(strncmp(d.targets[i].id,"link:",5));
    char *saved_grammar=NULL; OK(sb_note_load(&d.model.project,d.model.path,&saved_grammar,NULL));
    CHECK(!strcmp(saved_grammar,grammar) && !sb_app_dirty(&d.model)); free(saved_grammar);
#ifdef __APPLE__
    void *native_setext=native_find(view,"Unterstrich-Titel",0); CHECK(native_setext);
    CHECK(!strcmp(utf8(send(native_setext,"accessibilityRole")),heading_role ? utf8(*heading_role) : "Heading"));
#elif defined(_WIN32) || defined(SB_ATSPI_TEST)
    CHECK(sb_native_probe(d.ui.window,"Unterstrich-Titel",NULL,SB_NATIVE_READ_LEVEL,native_value,sizeof(native_value),pump,&d));
    CHECK(!strcmp(native_value,"2"));
#endif
    OK(sb_path_join(dump_path,sizeof(dump_path),root,"markdown-blocks.bmp"));
    CHECK(sb_ui_capture(&d.ui,dump_path).code==SB_OK);
    const char *separators="# Abschnitte\n\nVorher.\n\n***\n\nDanach.\n\nUntertitel\n---\n\n```\n___\n```\n";
    strcpy(d.model.editor,separators); OK(sb_app_save(&d.model)); d.reset_reader=true;
    for (unsigned size=0;size<2;++size) {
        OK(sb_ui_fonts(&d.ui,size ? 2 : 1)); frame(&d); frame(&d);
        unsigned rules=0; bool code_rule=false,underline_heading=false;
        for (size_t i=0;i<d.passive_count;++i) {
            SBPassiveText *p=&d.passive[i];
            if (p->role==ACCESSKIT_ROLE_SPLITTER) {
                ++rules; CHECK(!strcmp(p->parent,"reader") && !strcmp(p->text,"Abschnittstrennung"));
                CHECK(p->bounds.w>100 && p->bounds.h>0 && p->bounds.h<=2);
            }
            if (p->role==ACCESSKIT_ROLE_CODE && !strcmp(p->text,"___")) code_rule=true;
            if (p->role==ACCESSKIT_ROLE_HEADING && !strcmp(p->text,"Untertitel")) underline_heading=p->level==2;
        }
        CHECK(rules==1 && code_rule && underline_heading && !sb_app_dirty(&d.model));
        text=dump(&d); CHECK(strstr(text,"role: Splitter") && strstr(text,"orientation: Horizontal")); accesskit_string_free(text);
        for (size_t i=0;i<d.target_count;++i) CHECK(strncmp(d.targets[i].id,"reader:block:",13));
#ifdef __APPLE__
        void *native_separator=native_find(view,"Abschnittstrennung",0); CHECK(native_separator);
        CHECK(!strcmp(utf8(send(native_separator,"accessibilityRole")),"AXSplitter"));
#elif defined(_WIN32) || defined(SB_ATSPI_TEST)
        CHECK(sb_native_probe(d.ui.window,"Abschnittstrennung",NULL,SB_NATIVE_IS_SEPARATOR,NULL,0,pump,&d));
#endif
        OK(sb_path_join(dump_path,sizeof(dump_path),root,size ? "markdown-rules-large.bmp" : "markdown-rules.bmp"));
        CHECK(sb_ui_capture(&d.ui,dump_path).code==SB_OK);
    }
    SBStyleChoice previous_style=d.requested_style;
    sb_desktop_set_style(&d,(SBStyleChoice){.dark=false,.contrast=true}); frame(&d); frame(&d);
    CHECK(d.ui.contrast && !d.ui.dark);
    SDL_Surface *pixels=SDL_RenderReadPixels(d.ui.renderer,NULL); CHECK(pixels);
    bool separator_pixel=false;
    for (size_t i=0;i<d.passive_count;++i) if (d.passive[i].role==ACCESSKIT_ROLE_SPLITTER) {
        struct nk_rect bounds=d.passive[i].bounds; Uint8 red,green,blue,alpha;
        for (size_t j=0;j<d.target_count;++j) if (!strcmp(d.targets[j].id,"reader")) {
            struct nk_rect reader=d.targets[j].bounds;
            CHECK(bounds.x>reader.x+4*d.ui.scale);
            CHECK(bounds.x+bounds.w<reader.x+reader.w-4*d.ui.scale);
        }
        CHECK(SDL_ReadSurfacePixel(pixels,(int)((bounds.x+bounds.w/2)*d.ui.density),
            (int)((bounds.y+bounds.h/2)*d.ui.density),&red,&green,&blue,&alpha));
        struct nk_color ink=d.ui.ctx->style.text.color;
        separator_pixel=abs((int)red-ink.r)<=1 && abs((int)green-ink.g)<=1 && abs((int)blue-ink.b)<=1;
    }
    SDL_DestroySurface(pixels); CHECK(separator_pixel);
    OK(sb_path_join(dump_path,sizeof(dump_path),root,"markdown-rules-contrast.bmp")); CHECK(sb_ui_capture(&d.ui,dump_path).code==SB_OK);
    sb_desktop_set_style(&d,previous_style);
    char *saved_separators=NULL; OK(sb_note_load(&d.model.project,d.model.path,&saved_separators,NULL));
    CHECK(!strcmp(saved_separators,separators)); free(saved_separators);
    OK(sb_ui_fonts(&d.ui,1)); checkpoint("thematic separators and native role");
    char inline_source[1800]; const char *prefix=strchr(d.model.path,'/') ? "../" : "";
    snprintf(inline_source,sizeof(inline_source),"# [Titel](%sSTATE.md) `Code` und **Wort**\n\n`[Nur Code](%sPROJECT.md)` \\[Maskiert](%sQUESTIONS.md) ![Bild](%sSOURCES.md)\n[Stand](%sSTATE.md \"Quelle\")\n",prefix,prefix,prefix,prefix,prefix);
    strcpy(d.model.editor,inline_source); OK(sb_app_save(&d.model));
    d.reset_reader=true; frame(&d); frame(&d);
    CHECK(!strcmp(d.model.title,"Titel Code und Wort") && !strcmp(d.model.editor,inline_source));
    unsigned actual_links=0; bool preserved_code=false;
    for (size_t i=0;i<d.target_count;++i) if (!strncmp(d.targets[i].id,"link:",5)) {
        ++actual_links; CHECK(!strcmp(d.targets[i].label,actual_links==1 ? "Titel" : "Stand"));
    }
    for (size_t i=0;i<d.passive_count;++i) if (d.passive[i].role==ACCESSKIT_ROLE_PARAGRAPH && strstr(d.passive[i].text,"[Nur Code](")) preserved_code=true;
    CHECK(actual_links==2 && preserved_code);
    strcpy(d.focus,"link:1"); SDL_Event link_key={0}; link_key.type=SDL_EVENT_KEY_DOWN;
    link_key.key.key=SDLK_RETURN; link_key.key.down=true; link_key.key.windowID=SDL_GetWindowID(d.ui.window);
    sb_desktop_event(&d,&link_key); frame(&d); frame(&d);
    CHECK(d.model.source && strstr(d.model.source_path,"/STATE.md") && !sb_app_dirty(&d.model));
    link_key.key.key=SDLK_ESCAPE; sb_desktop_event(&d,&link_key); frame(&d); frame(&d);
    CHECK(!d.model.source && !strcmp(d.model.editor,inline_source));
    OK(sb_path_join(dump_path,sizeof(dump_path),root,"inline-links.bmp")); CHECK(sb_ui_capture(&d.ui,dump_path).code==SB_OK);
    char *previous_context=d.context; d.context=malloc(80); CHECK(d.context);
    strcpy(d.context,"# Sichtbarer Projektname\n\nKontext zum Lesen.\n");
    d.form=SB_FORM_CONTEXT; frame(&d); frame(&d); bool context_heading=false;
    for (size_t i=0;i<d.passive_count;++i) if (!strcmp(d.passive[i].parent,"reader") && !strcmp(d.passive[i].text,"Sichtbarer Projektname"))
        context_heading=d.passive[i].role==ACCESSKIT_ROLE_HEADING && d.passive[i].bounds.h>0;
    CHECK(context_heading); d.form=SB_FORM_NONE; free(d.context); d.context=previous_context; frame(&d);
    checkpoint("block and inline contracts");
    char table_source[2800];
    snprintf(table_source,sizeof(table_source),"# Tabellenprüfung\n\n| Aktion | Kürzel | Wert |\n| :--- | :---: | ---: |\n| [Stand](%sSTATE.md) | `Ctrl+S` | 7 |\n| Lange Beschreibung mit Wissen ü und mehreren Wörtern zum kontrollierten Umbruch | F6 | 8 | ignoriert |\n| Leer |\n\nDanach.\n",prefix);
    strcpy(d.model.editor,table_source); OK(sb_app_save(&d.model)); d.reset_reader=true; frame(&d); frame(&d);
    unsigned tables=0,rows=0,headers=0,cells=0; char table_id[100]={0}; bool empty_cell=false;
    for (size_t i=0;i<d.passive_count;++i) {
        SBPassiveText *p=&d.passive[i];
        if (p->role==ACCESSKIT_ROLE_TABLE) { ++tables; CHECK(p->rows==4 && p->columns==3); strcpy(table_id,p->id); }
        if (p->role==ACCESSKIT_ROLE_ROW) ++rows;
        if (p->role==ACCESSKIT_ROLE_COLUMN_HEADER) ++headers;
        if (p->role==ACCESSKIT_ROLE_CELL) { ++cells; empty_cell|=p->row==3 && p->column==2 && !*p->text; CHECK(!strstr(p->text,"ignoriert")); }
    }
    CHECK(tables==1 && rows==4 && headers==3 && cells==9 && empty_cell);
    float previous_row_y=-1;
    for (size_t i=0;i<d.passive_count;++i) if (d.passive[i].role==ACCESSKIT_ROLE_ROW && d.passive[i].bounds.h>0) {
        CHECK(d.passive[i].bounds.y>previous_row_y); previous_row_y=d.passive[i].bounds.y;
    }
    actual_links=0;
    for (size_t i=0;i<d.target_count;++i) if (!strncmp(d.targets[i].id,"link:",5)) { ++actual_links; CHECK(strstr(d.targets[i].parent,"reader:table:") && !strcmp(d.targets[i].label,"Stand")); }
    CHECK(actual_links==1 && !strcmp(d.model.editor,table_source) && !sb_app_dirty(&d.model));
    text=dump(&d); CHECK(strstr(text,"role: Table") && strstr(text,"role: ColumnHeader") && strstr(text,"row_count: 4") && strstr(text,"column_count: 3")); accesskit_string_free(text);
#ifdef __APPLE__
    void *native_table=native_find(view,"Tabelle",0); CHECK(native_table);
    printf("Native table role: %s\n",utf8(send(native_table,"accessibilityRole")));
    const char *table_role=utf8(send(native_table,"accessibilityRole")); CHECK(table_role && !strcmp(table_role,"AXTable"));
    void *native_rows=send(native_table,"accessibilityRows");
    CHECK(native_rows && ((size_t(*)(void *,SEL))objc_msgSend)(native_rows,sel_registerName("count"))==4);
    CHECK(((BOOL(*)(void *,SEL,SEL))objc_msgSend)(native_table,sel_registerName("isAccessibilitySelectorAllowed:"),sel_registerName("accessibilityRows")));
    for (size_t r=0;r<4;++r) {
        void *native_row=((void *(*)(void *,SEL,size_t))objc_msgSend)(native_rows,sel_registerName("objectAtIndex:"),r);
        CHECK(!strcmp(utf8(send(native_row,"accessibilityRole")),"AXRow"));
        void *native_cells=send(native_row,"accessibilityChildren");
        CHECK(native_cells && ((size_t(*)(void *,SEL))objc_msgSend)(native_cells,sel_registerName("count"))==3);
        for (size_t c=0;c<3;++c) {
            void *native_cell=((void *(*)(void *,SEL,size_t))objc_msgSend)(native_cells,sel_registerName("objectAtIndex:"),c);
            CHECK(!strcmp(utf8(send(native_cell,"accessibilityRole")),"AXCell"));
            const char *cell_value=utf8(send(native_cell,"accessibilityValue"));
            if (r==0) { const char *expected[]={"Aktion","Kürzel","Wert"}; CHECK(cell_value && !strcmp(cell_value,expected[c])); }
            if (r==1 && c==1) CHECK(cell_value && !strcmp(cell_value,"Ctrl+S"));
            if (r==3 && c>0) CHECK(!cell_value || !*cell_value);
        }
    }
#elif defined(_WIN32) || defined(SB_ATSPI_TEST)
    CHECK(sb_native_probe(d.ui.window,"Tabelle",NULL,SB_NATIVE_READ_TABLE_TREE,native_value,sizeof(native_value),pump,&d));
    CHECK(!strcmp(native_value,"4:3"));
    /* GridPattern / AT-SPI Table are absent in the pinned providers. Keep their
       absence visible; tree traversal above is a separate, narrower contract. */
    bool table_pattern=sb_native_probe(d.ui.window,"Tabelle",NULL,SB_NATIVE_READ_TABLE_SIZE,native_value,sizeof(native_value),pump,&d);
    if (table_pattern) CHECK(!strcmp(native_value,"4:3"));
    printf("Native table matrix interface: %s (tree checked separately).\n",table_pattern ? "available" : "unavailable; release gate open");
#endif
    OK(sb_path_join(dump_path,sizeof(dump_path),root,"table-grid.bmp")); CHECK(sb_ui_capture(&d.ui,dump_path).code==SB_OK);
    OK(sb_ui_fonts(&d.ui,2)); CHECK(SDL_SetWindowSize(d.ui.window,780,560)); frame(&d); frame(&d);
    CHECK(!strcmp(d.model.editor,table_source));
    for (size_t i=0;i<d.passive_count;++i) if (d.passive[i].role==ACCESSKIT_ROLE_TABLE) CHECK(!strcmp(d.passive[i].id,table_id));
    unsigned compact_headers=0; bool first_data_visible=false,hidden_header=false;
    for (size_t i=0;i<d.passive_count;++i) {
        SBPassiveText *p=&d.passive[i];
        if (p->role==ACCESSKIT_ROLE_COLUMN_HEADER) { ++compact_headers; CHECK(p->bounds.h==0); }
        if (p->role==ACCESSKIT_ROLE_ROW && p->row==0) hidden_header=p->bounds.h==0;
        if (p->role==ACCESSKIT_ROLE_CELL && p->row==1 && p->column==0) first_data_visible=p->bounds.h>0 && !strcmp(p->text,"Stand");
    }
    CHECK(compact_headers==3 && hidden_header && first_data_visible);
#ifdef __APPLE__
    native_table=native_find(view,"Tabelle",0); CHECK(native_table);
    native_rows=send(native_table,"accessibilityRows");
    CHECK(native_rows && ((size_t(*)(void *,SEL))objc_msgSend)(native_rows,sel_registerName("count"))==4);
#elif defined(_WIN32) || defined(SB_ATSPI_TEST)
    CHECK(sb_native_probe(d.ui.window,"Tabelle",NULL,SB_NATIVE_READ_TABLE_TREE,native_value,sizeof(native_value),pump,&d));
    CHECK(!strcmp(native_value,"4:3"));
#endif
    OK(sb_path_join(dump_path,sizeof(dump_path),root,"table-stacked.bmp")); CHECK(sb_ui_capture(&d.ui,dump_path).code==SB_OK);
    char last_cell[100]={0};
    for (size_t i=0;i<d.passive_count;++i) if (d.passive[i].role==ACCESSKIT_ROLE_CELL && d.passive[i].row==3 && d.passive[i].column==0) strcpy(last_cell,d.passive[i].id);
    CHECK(last_cell[0]);
#ifdef __APPLE__
    void *native_last_cell=native_find(view,last_cell,0); CHECK(native_last_cell);
    ((void(*)(void *,SEL,void *))objc_msgSend)(native_last_cell,sel_registerName("accessibilityPerformAction:"),string("AXScrollToVisible"));
#elif defined(_WIN32) || defined(SB_ATSPI_TEST)
    CHECK(sb_native_probe(d.ui.window,"Leer",NULL,SB_NATIVE_SCROLL_INTO_VIEW,NULL,0,pump,&d));
#endif
    for (unsigned i=0;i<40;++i) frame_run(&d,i==39);
    bool last_cell_visible=false;
    for (size_t i=0;i<d.passive_count;++i) if (!strcmp(d.passive[i].id,last_cell)) last_cell_visible=d.passive[i].bounds.h>0;
    CHECK(last_cell_visible && !strcmp(d.model.editor,table_source));
    OK(sb_path_join(dump_path,sizeof(dump_path),root,"table-stacked-end.bmp")); CHECK(sb_ui_capture(&d.ui,dump_path).code==SB_OK);
    d.expanded=true; d.reset_reader=true; frame(&d); frame(&d);
    OK(sb_path_join(dump_path,sizeof(dump_path),root,"table-expanded.bmp")); CHECK(sb_ui_capture(&d.ui,dump_path).code==SB_OK);
    d.expanded=false;
    checkpoint("table roles and scroll");
    char header_source[1000];
    snprintf(header_source,sizeof(header_source),"# Header-Verweise\n\n| [Aktion](%sPROJECT.md) | Wert |\n| --- | --- |\n| Tun | 3 |\n",prefix);
    strcpy(d.model.editor,header_source); OK(sb_app_save(&d.model)); d.reset_reader=true; frame(&d); frame(&d);
    bool header_visible=false,header_link=false;
    for (size_t i=0;i<d.passive_count;++i) if (d.passive[i].role==ACCESSKIT_ROLE_ROW && d.passive[i].row==0) header_visible=d.passive[i].bounds.h>0;
    for (size_t i=0;i<d.target_count;++i) if (!strcmp(d.targets[i].id,"link:0")) header_link=!strcmp(d.targets[i].label,"Aktion") && strstr(d.targets[i].parent,":cell:0");
    CHECK(header_visible && header_link && !sb_app_dirty(&d.model));
    OK(sb_path_join(dump_path,sizeof(dump_path),root,"table-header-link.bmp")); CHECK(sb_ui_capture(&d.ui,dump_path).code==SB_OK);
    strcpy(d.focus,"reader"); link_key.key.key=SDLK_TAB; sb_desktop_event(&d,&link_key); frame(&d); frame(&d);
    CHECK(!strcmp(d.focus,"link:0"));
    link_key.key.key=SDLK_RETURN; sb_desktop_event(&d,&link_key); frame(&d); frame(&d);
    CHECK(d.model.source && strstr(d.model.source_path,"/PROJECT.md"));
    link_key.key.key=SDLK_ESCAPE; sb_desktop_event(&d,&link_key); frame(&d); frame(&d);
    CHECK(!d.model.source && !strcmp(d.model.editor,header_source));
    strcpy(d.model.editor,table_source); OK(sb_app_save(&d.model)); d.reset_reader=true;
    OK(sb_ui_fonts(&d.ui,1)); CHECK(SDL_SetWindowSize(d.ui.window,1336,840)); frame(&d); frame(&d);
    strcpy(d.focus,"link:0"); link_key.key.key=SDLK_RETURN; sb_desktop_event(&d,&link_key); frame(&d); frame(&d);
    CHECK(d.model.source && strstr(d.model.source_path,"/STATE.md") && !sb_app_dirty(&d.model));
    link_key.key.key=SDLK_ESCAPE; sb_desktop_event(&d,&link_key); frame(&d); frame(&d);
    CHECK(!d.model.source && !strcmp(d.model.editor,table_source));
    snprintf(d.reveal_document,sizeof(d.reveal_document),"%s",last_id); d.reveal_document_context=d.semantic_context;
    d.form=SB_FORM_HELP; frame(&d); CHECK(!d.reveal_document[0]); text=dump(&d); CHECK(!strstr(text,"Erster Abschnitt") && !strstr(text,"Ende αΩ")); accesskit_string_free(text);
    d.form=SB_FORM_NONE; frame(&d);
    strcpy(d.model.editor,"Ungespeicherter Entwurf");
    OK(sb_app_request(&d.model,SB_ACT_NOTE,"STATE.md")); CHECK(d.model.guard); frame(&d);
    bool title_bounds=false; for (size_t i=0;i<d.passive_count;++i) if (!strcmp(d.passive[i].id,"modal-title")) title_bounds=d.passive[i].bounds.w>0 && d.passive[i].bounds.h>0;
    CHECK(title_bounds);
    text=dump(&d); CHECK(strstr(text,"Änderungen erhalten") && strstr(text,"Dieses Dokument enthält ungespeicherte Änderungen"));
    CHECK(!strstr(text,"Projektdokumente")); accesskit_string_free(text);
#ifdef SB_ATSPI_TEST
    CHECK(sb_native_cache_check());
#endif
    checkpoint("header links, keyboard and guards");
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
    while (sb_accessibility_next_action(a,&pending)) sb_accessibility_action_free(&pending);
    /* Many immutable paragraphs retain identities across value-only updates. */
    const size_t block_count=2000;
    SBAccessibleItem *blocks=calloc(block_count+1,sizeof(*blocks)); char (*block_ids)[40]=calloc(block_count,sizeof(*block_ids)); CHECK(blocks && block_ids);
    blocks[0]=(SBAccessibleItem){.id="document",.label="Langer Text",.value="",.role=ACCESSKIT_ROLE_DOCUMENT};
    for (size_t i=0;i<block_count;++i) {
        snprintf(block_ids[i],40,"paragraph:%zu",i);
        blocks[i+1]=(SBAccessibleItem){.id=block_ids[i],.label="",.value="Absatz ü.",.role=ACCESSKIT_ROLE_PARAGRAPH,.parent="document"};
    }
    sb_accessibility_update(a,"Test","",blocks,block_count+1,"",false,4);
    CHECK(sb_accessibility_submit(a,2020,ACCESSKIT_ACTION_SCROLL_INTO_VIEW,NULL,0,0));
    CHECK(sb_accessibility_next_action(a,&pending)); CHECK(!strcmp(pending.id,"paragraph:1999"));
    blocks[20].value="Geänderter Absatz"; sb_accessibility_update(a,"Test","",blocks,block_count+1,"",false,4);
    CHECK(sb_accessibility_current(a,&pending,4)); sb_accessibility_action_free(&pending);
    CHECK(!sb_accessibility_submit(a,2020,ACCESSKIT_ACTION_SET_VALUE,"Fremde Änderung",0,0));
    accesskit_tree_update *large_tree=sb_accessibility_tree(a); text=accesskit_tree_update_debug(large_tree); CHECK(strstr(text,"paragraph:1999") && strstr(text,"Geänderter Absatz"));
    accesskit_string_free(text); accesskit_tree_update_free(large_tree);
    free(blocks); free(block_ids);
    sb_accessibility_free(a); SDL_DestroyWindow(test_window); SDL_Quit();
    printf("%u accessibility assertions passed.\n",checks); return 0;
}
