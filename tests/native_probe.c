#include "native_probe.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#ifdef _WIN32
#define COBJMACROS
#include <windows.h>
#include <ole2.h>
#include <initguid.h>
#include <uiautomation.h>
#elif defined(SB_ATSPI_TEST)
#include <atspi/atspi.h>
#include <atspi/atspi-application.h>
static DBusConnection *cache_connection;
static char *cache_owner;
static unsigned cache_added,cache_removed,cache_invalid;
static void close_atspi(void) {
    if (cache_connection) { dbus_connection_close(cache_connection); dbus_connection_unref(cache_connection); }
    g_free(cache_owner); (void)atspi_exit();
}
static bool cache_open(const char *owner) {
    if (cache_connection) return !strcmp(cache_owner,owner);
    DBusError error; dbus_error_init(&error);
    DBusConnection *session=dbus_bus_get(DBUS_BUS_SESSION,&error);
    DBusMessage *call=dbus_message_new_method_call("org.a11y.Bus","/org/a11y/bus","org.a11y.Bus","GetAddress");
    DBusMessage *reply=session && call ? dbus_connection_send_with_reply_and_block(session,call,3000,&error) : NULL;
    const char *address=NULL;
    if (reply && dbus_message_get_args(reply,&error,DBUS_TYPE_STRING,&address,DBUS_TYPE_INVALID)) cache_connection=dbus_connection_open_private(address,&error);
    if (call) dbus_message_unref(call); if (reply) dbus_message_unref(reply); if (session) dbus_connection_unref(session);
    if (cache_connection && !dbus_bus_register(cache_connection,&error)) { dbus_connection_close(cache_connection); dbus_connection_unref(cache_connection); cache_connection=NULL; }
    if (cache_connection) {
        dbus_connection_set_exit_on_disconnect(cache_connection,FALSE);
        cache_owner=g_strdup(owner);
        char rule[512]; snprintf(rule,sizeof(rule),"type='signal',interface='org.a11y.atspi.Cache',sender='%s'",owner);
        dbus_bus_add_match(cache_connection,rule,&error); dbus_connection_flush(cache_connection);
    }
    bool success=cache_connection && !dbus_error_is_set(&error);
    if (!success) fprintf(stderr,"Cache observer setup failed: %s\n",error.message ? error.message : "no connection");
    dbus_error_free(&error); return success;
}
static void cache_drain(void) {
    if (!cache_connection) return;
    dbus_connection_read_write(cache_connection,50);
    DBusMessage *message;
    while ((message=dbus_connection_pop_message(cache_connection))) {
        const char *sender=dbus_message_get_sender(message);
        if (sender && !strcmp(sender,cache_owner) && dbus_message_get_type(message)==DBUS_MESSAGE_TYPE_SIGNAL) {
            const char *expected=NULL;
            if (dbus_message_is_signal(message,"org.a11y.atspi.Cache","AddAccessible")) { ++cache_added; expected="((so)(so)(so)iiassusau)"; }
            if (dbus_message_is_signal(message,"org.a11y.atspi.Cache","RemoveAccessible")) { ++cache_removed; expected="(so)"; }
            if (expected && strcmp(dbus_message_get_signature(message),expected)) {
                ++cache_invalid; if (cache_invalid<=3) fprintf(stderr,"Cache signal failed: %s has signature %s, expected %s\n",dbus_message_get_member(message),dbus_message_get_signature(message),expected);
            }
        }
        dbus_message_unref(message);
    }
}
#endif
typedef struct {
    void *native; const char *label,*value; int operation;
    char *output; size_t capacity; SDL_AtomicInt done; bool result;
} Probe;
#ifdef _WIN32
static wchar_t *wide(const char *s) {
    int count=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s,-1,NULL,0);
    wchar_t *p=count ? malloc((size_t)count*sizeof(*p)) : NULL;
    if (p && !MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s,-1,p,count)) { free(p); return NULL; }
    return p;
}
static bool query(Probe *p) {
    if (FAILED(CoInitializeEx(NULL,COINIT_MULTITHREADED))) return false;
    IUIAutomation *client=NULL; IUIAutomationElement *root=NULL,*element=NULL;
    IUIAutomationCondition *condition=NULL; IUIAutomationInvokePattern *invoke=NULL;
    IUIAutomationValuePattern *field=NULL; bool success=false;
    HRESULT hr=CoCreateInstance(&CLSID_CUIAutomation,NULL,CLSCTX_INPROC_SERVER,&IID_IUIAutomation,(void **)&client);
    if (FAILED(hr)) goto done;
    hr=IUIAutomation_ElementFromHandle(client,(HWND)p->native,&root); if (FAILED(hr) || !root) goto done;
    wchar_t *label=wide(p->label); if (!label) goto done;
    VARIANT value; VariantInit(&value); value.vt=VT_BSTR; value.bstrVal=SysAllocString(label); free(label);
    hr=IUIAutomation_CreatePropertyCondition(client,UIA_NamePropertyId,value,&condition); VariantClear(&value);
    if (FAILED(hr) || !condition) goto done;
    hr=IUIAutomationElement_FindFirst(root,TreeScope_Descendants,condition,&element); if (FAILED(hr) || !element) goto done;
    if (p->operation==SB_NATIVE_PRESS) {
        CONTROLTYPEID role=0; hr=IUIAutomationElement_get_CurrentControlType(element,&role);
        if (FAILED(hr) || role!=UIA_ButtonControlTypeId) goto done;
        hr=IUIAutomationElement_GetCurrentPatternAs(element,UIA_InvokePatternId,&IID_IUIAutomationInvokePattern,(void **)&invoke);
        if (SUCCEEDED(hr) && invoke) success=SUCCEEDED(IUIAutomationInvokePattern_Invoke(invoke));
    } else if (p->operation==SB_NATIVE_SCROLL_INTO_VIEW) {
        IUIAutomationScrollItemPattern *pattern=NULL;
        hr=IUIAutomationElement_GetCurrentPatternAs(element,UIA_ScrollItemPatternId,&IID_IUIAutomationScrollItemPattern,(void **)&pattern);
        if (SUCCEEDED(hr) && pattern) hr=IUIAutomationScrollItemPattern_ScrollIntoView(pattern);
        success=SUCCEEDED(hr); if (pattern) IUIAutomationScrollItemPattern_Release(pattern);
    } else if (p->operation==SB_NATIVE_READ_TABLE_TREE) {
        IUIAutomationCondition *all=NULL; IUIAutomationElementArray *rows=NULL;
        CONTROLTYPEID role=0; int row_count=0,column_count=-1;
        hr=IUIAutomationElement_get_CurrentControlType(element,&role);
        if (SUCCEEDED(hr) && role==UIA_TableControlTypeId) hr=IUIAutomation_CreateTrueCondition(client,&all); else hr=E_FAIL;
        if (SUCCEEDED(hr)) hr=IUIAutomationElement_FindAll(element,TreeScope_Children,all,&rows);
        if (SUCCEEDED(hr) && rows) hr=IUIAutomationElementArray_get_Length(rows,&row_count);
        for (int r=0;SUCCEEDED(hr) && r<row_count;++r) {
            IUIAutomationElement *row=NULL; IUIAutomationElementArray *cells=NULL; int count=0;
            hr=IUIAutomationElementArray_GetElement(rows,r,&row);
            if (SUCCEEDED(hr)) hr=IUIAutomationElement_FindAll(row,TreeScope_Children,all,&cells);
            if (SUCCEEDED(hr) && cells) hr=IUIAutomationElementArray_get_Length(cells,&count);
            if (SUCCEEDED(hr) && (column_count<0 || column_count==count)) column_count=count; else hr=E_FAIL;
            if (cells) IUIAutomationElementArray_Release(cells);
            if (row) IUIAutomationElement_Release(row);
        }
        if (SUCCEEDED(hr) && row_count>0 && column_count>0 && p->capacity) {
            snprintf(p->output,p->capacity,"%d:%d",row_count,column_count); success=true;
        }
        if (rows) IUIAutomationElementArray_Release(rows);
        if (all) IUIAutomationCondition_Release(all);
    } else if (p->operation==SB_NATIVE_READ_TABLE_SIZE) {
        IUIAutomationGridPattern *pattern=NULL; int rows=0,columns=0;
        hr=IUIAutomationElement_GetCurrentPatternAs(element,UIA_GridPatternId,&IID_IUIAutomationGridPattern,(void **)&pattern);
        if (SUCCEEDED(hr) && pattern) hr=IUIAutomationGridPattern_get_CurrentRowCount(pattern,&rows);
        if (SUCCEEDED(hr) && pattern) hr=IUIAutomationGridPattern_get_CurrentColumnCount(pattern,&columns);
        if (SUCCEEDED(hr) && pattern && p->capacity) { snprintf(p->output,p->capacity,"%d:%d",rows,columns); success=true; }
        if (pattern) IUIAutomationGridPattern_Release(pattern);
    } else if (p->operation==SB_NATIVE_IS_SEPARATOR) {
        CONTROLTYPEID role=0; hr=IUIAutomationElement_get_CurrentControlType(element,&role);
        success=SUCCEEDED(hr) && role==UIA_SeparatorControlTypeId;
    } else if (p->operation==SB_NATIVE_READ_LEVEL) {
        VARIANT level; VariantInit(&level);
        hr=IUIAutomationElement_GetCurrentPropertyValue(element,UIA_LevelPropertyId,&level);
        if (SUCCEEDED(hr) && level.vt==VT_I4 && p->capacity) {
            snprintf(p->output,p->capacity,"%ld",level.lVal); success=true;
        }
        VariantClear(&level);
    } else if (p->operation>=SB_NATIVE_FIND_FIRST && p->operation<=SB_NATIVE_FIND_LIMITED) {
        IUIAutomationTextPattern *pattern=NULL; IUIAutomationTextRange *document=NULL,*scope=NULL,*found=NULL,*prefix=NULL;
        hr=IUIAutomationElement_GetCurrentPatternAs(element,UIA_TextPatternId,&IID_IUIAutomationTextPattern,(void **)&pattern);
        if (SUCCEEDED(hr) && pattern) hr=IUIAutomationTextPattern_get_DocumentRange(pattern,&document); else hr=E_FAIL;
        if (SUCCEEDED(hr) && document) {
            if (p->operation==SB_NATIVE_FIND_LIMITED) {
                BSTR beta=SysAllocString(L"beta");
                hr=IUIAutomationTextRange_FindText(document,beta,FALSE,FALSE,&scope);SysFreeString(beta);
                if (SUCCEEDED(hr) && !scope) hr=E_FAIL;
            } else hr=IUIAutomationTextRange_Clone(document,&scope);
        }
        wchar_t *needle=wide(p->value);BSTR word=needle ? SysAllocString(needle) : NULL;free(needle);
        BOOL backward=p->operation==SB_NATIVE_FIND_LAST || p->operation==SB_NATIVE_FIND_LAST_NO_CASE;
        BOOL folded=p->operation==SB_NATIVE_FIND_NO_CASE || p->operation==SB_NATIVE_FIND_LAST_NO_CASE;
        if (SUCCEEDED(hr) && scope && word) hr=IUIAutomationTextRange_FindText(scope,word,backward,folded,&found);else hr=E_FAIL;
        BSTR value=NULL,before=NULL;
        if (SUCCEEDED(hr) && !found && p->capacity) { snprintf(p->output,p->capacity,"<none>");success=true; }
        else if (SUCCEEDED(hr) && found) {
            hr=IUIAutomationTextRange_GetText(found,-1,&value);
            if (SUCCEEDED(hr)) hr=IUIAutomationTextRange_Clone(document,&prefix);
            if (SUCCEEDED(hr) && prefix) hr=IUIAutomationTextRange_MoveEndpointByRange(prefix,TextPatternRangeEndpoint_End,found,TextPatternRangeEndpoint_Start);
            if (SUCCEEDED(hr) && prefix) hr=IUIAutomationTextRange_GetText(prefix,-1,&before);
            char text[512];
            if (SUCCEEDED(hr) && value && p->capacity && WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,value,-1,text,sizeof(text),NULL,NULL)>0) {
                snprintf(p->output,p->capacity,"%u:%s",SysStringLen(before),text);success=true;
            }
        }
        SysFreeString(value);SysFreeString(before);SysFreeString(word);
        if (prefix) IUIAutomationTextRange_Release(prefix);if (found) IUIAutomationTextRange_Release(found);if (scope) IUIAutomationTextRange_Release(scope);
        if (document) IUIAutomationTextRange_Release(document);if (pattern) IUIAutomationTextPattern_Release(pattern);
    } else if (p->operation==SB_NATIVE_TEXT_STYLE) {
        IUIAutomationTextPattern *pattern=NULL; IUIAutomationTextRange *document=NULL,*range=NULL;
        hr=IUIAutomationElement_GetCurrentPatternAs(element,UIA_TextPatternId,&IID_IUIAutomationTextPattern,(void **)&pattern);
        if (SUCCEEDED(hr) && pattern) hr=IUIAutomationTextPattern_get_DocumentRange(pattern,&document);
        wchar_t *needle=wide(p->value); BSTR word=needle ? SysAllocString(needle) : NULL; free(needle);
        if (SUCCEEDED(hr) && document && word) hr=IUIAutomationTextRange_FindText(document,word,FALSE,FALSE,&range); else hr=E_FAIL;
        if (SUCCEEDED(hr) && !range) { fprintf(stderr,"UIA FindText returned no range for '%s'.\n",p->value); hr=E_FAIL; }
        VARIANT weight,italic,family,size; VariantInit(&weight);VariantInit(&italic);VariantInit(&family);VariantInit(&size);
        if (SUCCEEDED(hr) && range) hr=IUIAutomationTextRange_GetAttributeValue(range,UIA_FontWeightAttributeId,&weight);
        if (SUCCEEDED(hr) && range) hr=IUIAutomationTextRange_GetAttributeValue(range,UIA_IsItalicAttributeId,&italic);
        if (SUCCEEDED(hr) && range) hr=IUIAutomationTextRange_GetAttributeValue(range,UIA_FontNameAttributeId,&family);
        if (SUCCEEDED(hr) && range) hr=IUIAutomationTextRange_GetAttributeValue(range,UIA_FontSizeAttributeId,&size);
        char name[160];
        if (SUCCEEDED(hr) && range && weight.vt==VT_I4 && italic.vt==VT_BOOL && family.vt==VT_BSTR && size.vt==VT_R8 &&
            WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,family.bstrVal,-1,name,sizeof(name),NULL,NULL)>0 && p->capacity) {
            snprintf(p->output,p->capacity,"%ld|%d|%s|%.1f",weight.lVal,italic.boolVal!=VARIANT_FALSE,name,size.dblVal);success=true;
        }
        VariantClear(&weight);VariantClear(&italic);VariantClear(&family);VariantClear(&size);SysFreeString(word);
        if (range) IUIAutomationTextRange_Release(range);if (document) IUIAutomationTextRange_Release(document);if (pattern) IUIAutomationTextPattern_Release(pattern);
    } else if (p->operation==SB_NATIVE_READ_DOCUMENT_TEXT) {
        IUIAutomationTextPattern *pattern=NULL; IUIAutomationTextRange *range=NULL; BSTR value=NULL;
        hr=IUIAutomationElement_GetCurrentPatternAs(element,UIA_TextPatternId,&IID_IUIAutomationTextPattern,(void **)&pattern);
        if (SUCCEEDED(hr) && pattern) hr=IUIAutomationTextPattern_get_DocumentRange(pattern,&range);
        if (SUCCEEDED(hr) && range) hr=IUIAutomationTextRange_GetText(range,-1,&value);
        if (SUCCEEDED(hr) && value && p->capacity<=INT_MAX)
            success=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,value,-1,p->output,(int)p->capacity,NULL,NULL)>0;
        SysFreeString(value); if (range) IUIAutomationTextRange_Release(range); if (pattern) IUIAutomationTextPattern_Release(pattern);
    } else if (p->operation==SB_NATIVE_READ_NAME) {
        BSTR text=NULL; hr=IUIAutomationElement_get_CurrentName(element,&text);
        if (SUCCEEDED(hr) && text && p->capacity<=INT_MAX) success=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,text,-1,p->output,(int)p->capacity,NULL,NULL)>0;
        SysFreeString(text);
    } else {
        hr=IUIAutomationElement_GetCurrentPatternAs(element,UIA_ValuePatternId,&IID_IUIAutomationValuePattern,(void **)&field);
        if (FAILED(hr) || !field) goto done;
        if (p->operation==SB_NATIVE_SET_VALUE) {
            wchar_t *text=wide(p->value); if (text) { success=SUCCEEDED(IUIAutomationValuePattern_SetValue(field,text)); free(text); }
        } else {
            BSTR text=NULL; hr=IUIAutomationValuePattern_get_CurrentValue(field,&text);
            if (SUCCEEDED(hr) && text && p->capacity<=INT_MAX) success=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,text,-1,p->output,(int)p->capacity,NULL,NULL)>0;
            SysFreeString(text);
        }
    }
done:
    if (!success) fprintf(stderr,"UIA operation %d on '%s' failed (HRESULT 0x%08lx).\n",p->operation,p->label,(unsigned long)hr);
    if (field) IUIAutomationValuePattern_Release(field);
    if (invoke) IUIAutomationInvokePattern_Release(invoke);
    if (element) IUIAutomationElement_Release(element);
    if (condition) IUIAutomationCondition_Release(condition);
    if (root) IUIAutomationElement_Release(root);
    if (client) IUIAutomation_Release(client);
    CoUninitialize(); return success;
}
#elif defined(SB_ATSPI_TEST)
static AtspiAccessible *find(AtspiAccessible *object,const char *label,unsigned depth) {
    if (!object || depth>12) return NULL;
    char *name=atspi_accessible_get_name(object,NULL);
    bool match=name && !strcmp(name,label); g_free(name);
    if (match) return g_object_ref(object);
    int count=atspi_accessible_get_child_count(object,NULL);
    for (int i=0;i<count;++i) {
        AtspiAccessible *child=atspi_accessible_get_child_at_index(object,i,NULL);
        AtspiAccessible *found=find(child,label,depth+1); if (child) g_object_unref(child);
        if (found) return found;
    }
    return NULL;
}
static bool query(Probe *p) {
    if (!dbus_threads_init_default()) return false;
    int initialized=atspi_init();
    if (initialized!=0 && initialized!=1) return false;
    if (initialized==0) atexit(close_atspi);
    atspi_set_timeout(3000,10000);
    AtspiAccessible *element=NULL;
    Uint64 deadline=SDL_GetTicks()+15000;
    while (!element && SDL_GetTicks()<deadline) {
        /* Let the real client consume cache/property events; do not bypass its cache. */
        for (unsigned i=0;i<256 && g_main_context_pending(NULL);++i) g_main_context_iteration(NULL,FALSE);
        AtspiAccessible *desktop=atspi_get_desktop(0);
        if (desktop) {
            int count=atspi_accessible_get_child_count(desktop,NULL);
            for (int i=0;i<count && !element;++i) {
                AtspiAccessible *app=atspi_accessible_get_child_at_index(desktop,i,NULL);
                if (app && atspi_accessible_get_process_id(app,NULL)==sb_process_id()) element=find(app,p->label,0);
                if (app) g_object_unref(app);
            }
            g_object_unref(desktop);
        }
        if (!element) SDL_Delay(50);
    }
    bool success=false; GError *error=NULL;
    if (element) {
        AtspiObject *object=ATSPI_OBJECT(element);
        if (!object->app || !cache_open(object->app->bus_name)) { g_object_unref(element); return false; }
        if (p->operation==SB_NATIVE_PRESS) {
            AtspiAction *action=atspi_accessible_get_action_iface(element);
            if (atspi_accessible_get_role(element,NULL)==ATSPI_ROLE_PUSH_BUTTON && action) success=atspi_action_do_action(action,0,&error);
            if (action) g_object_unref(action);
        } else if (p->operation==SB_NATIVE_SET_VALUE) {
            AtspiEditableText *edit=atspi_accessible_get_editable_text_iface(element);
            if (edit) success=atspi_editable_text_set_text_contents(edit,p->value,&error);
            if (edit) g_object_unref(edit);
        } else if (p->operation==SB_NATIVE_SCROLL_INTO_VIEW) {
            AtspiComponent *component=atspi_accessible_get_component_iface(element);
            if (component) success=atspi_component_scroll_to(component,ATSPI_SCROLL_ANYWHERE,&error);
            if (component) g_object_unref(component);
        } else if (p->operation==SB_NATIVE_READ_TABLE_TREE) {
            int rows=atspi_accessible_get_child_count(element,&error),columns=-1;
            bool valid=!error && atspi_accessible_get_role(element,NULL)==ATSPI_ROLE_TABLE;
            for (int r=0;valid && r<rows;++r) {
                AtspiAccessible *row=atspi_accessible_get_child_at_index(element,r,&error);
                int count=row ? atspi_accessible_get_child_count(row,&error) : 0;
                valid=row && !error && atspi_accessible_get_role(row,NULL)==ATSPI_ROLE_TABLE_ROW && (columns<0 || columns==count);
                columns=count; if (row) g_object_unref(row);
            }
            if (valid && rows>0 && columns>0 && p->capacity) {
                snprintf(p->output,p->capacity,"%d:%d",rows,columns); success=true;
            }
        } else if (p->operation==SB_NATIVE_READ_TABLE_SIZE) {
            AtspiTable *table=atspi_accessible_get_table_iface(element);
            if (table) {
                int rows=atspi_table_get_n_rows(table,&error),columns=error ? 0 : atspi_table_get_n_columns(table,&error);
                if (!error && p->capacity) { snprintf(p->output,p->capacity,"%d:%d",rows,columns); success=true; }
                g_object_unref(table);
            }
        } else if (p->operation==SB_NATIVE_IS_SEPARATOR) {
            success=atspi_accessible_get_role(element,&error)==ATSPI_ROLE_SEPARATOR && !error;
        } else if (p->operation==SB_NATIVE_READ_LEVEL) {
            GHashTable *attributes=atspi_accessible_get_attributes(element,&error);
            const char *level=attributes ? g_hash_table_lookup(attributes,"level") : NULL;
            if (level && strlen(level)<p->capacity) { strcpy(p->output,level); success=true; }
            if (attributes) g_hash_table_unref(attributes);
        } else if (p->operation==SB_NATIVE_TEXT_STYLE) {
            AtspiText *text=atspi_accessible_get_text_iface(element);
            char *content=text ? atspi_text_get_text(text,0,-1,&error) : NULL;
            const char *word=content ? strstr(content,p->value) : NULL; int offset=0,start=0,end=0;
            if (word) for (const char *c=content;c<word;++c) if (((unsigned char)*c&0xc0)!=0x80) ++offset;
            GHashTable *attrs=word && !error ? atspi_text_get_text_attributes(text,offset,&start,&end,&error) : NULL;
            const char *weight=attrs ? g_hash_table_lookup(attrs,"weight") : NULL;
            const char *style=attrs ? g_hash_table_lookup(attrs,"style") : NULL;
            const char *family=attrs ? g_hash_table_lookup(attrs,"family-name") : NULL;
            const char *size=attrs ? g_hash_table_lookup(attrs,"size") : NULL;
            if (!error && weight && family && size && start<=offset && end>offset && p->capacity) {
                snprintf(p->output,p->capacity,"%s|%d|%s|%.1f",weight,style && !strcmp(style,"italic"),family,strtod(size,NULL)); success=true;
            }
            if (attrs) g_hash_table_unref(attrs);g_free(content);if (text) g_object_unref(text);
        } else if (p->operation==SB_NATIVE_READ_NAME) {
            char *name=atspi_accessible_get_name(element,&error);
            if (name && strlen(name)<p->capacity) { strcpy(p->output,name); success=true; }
            g_free(name);
        } else {
            AtspiText *text=atspi_accessible_get_text_iface(element);
            char *value=text ? atspi_text_get_text(text,0,-1,&error) : NULL;
            if (value && strlen(value)<p->capacity) { strcpy(p->output,value); success=true; }
            g_free(value);
            if (text) g_object_unref(text);
        }
        g_object_unref(element);
    }
    if (!success) fprintf(stderr,"AT-SPI operation %d on '%s' failed: %s\n",p->operation,p->label,error ? error->message : "object/pattern not found");
    g_clear_error(&error); cache_drain(); return success;
}
#else
static bool query(Probe *p) { (void)p; return false; }
#endif
static int worker(void *userdata) {
    Probe *p=userdata; p->result=query(p); SDL_SetAtomicInt(&p->done,1); return 0;
}
bool sb_native_probe(SDL_Window *window,const char *label,const char *value,int operation,char *output,size_t capacity,void (*pump)(void *),void *context) {
    Probe p={0}; p.label=label; p.value=value; p.operation=operation; p.output=output; p.capacity=capacity;
#ifdef _WIN32
    p.native=SDL_GetPointerProperty(SDL_GetWindowProperties(window),SDL_PROP_WINDOW_WIN32_HWND_POINTER,NULL);
#else
    (void)window;
#endif
    SDL_Thread *thread=SDL_CreateThread(worker,"native-accessibility-client",&p); if (!thread) return false;
    Uint64 deadline=SDL_GetTicks()+25000;
    while (!SDL_GetAtomicInt(&p.done)) {
        if (SDL_GetTicks()>deadline) { fprintf(stderr,"Native accessibility client timed out.\n"); exit(2); }
        pump(context); SDL_Delay(10);
    }
    SDL_WaitThread(thread,NULL); pump(context); pump(context); return p.result;
}
bool sb_native_cache_check(void) {
#ifdef SB_ATSPI_TEST
    cache_drain();
    if (!cache_connection) return false;
    DBusError error; dbus_error_init(&error);
    DBusMessage *call=dbus_message_new_method_call(cache_owner,"/org/a11y/atspi/cache","org.a11y.atspi.Cache","GetItems");
    DBusMessage *reply=call ? dbus_connection_send_with_reply_and_block(cache_connection,call,3000,&error) : NULL;
    bool bulk=reply && !strcmp(dbus_message_get_signature(reply),"a((so)(so)(so)iiassusau)");
    if (bulk) {
        DBusMessageIter array,items; dbus_message_iter_init(reply,&array); dbus_message_iter_recurse(&array,&items);
        bulk=dbus_message_iter_get_arg_type(&items)==DBUS_TYPE_STRUCT;
    }
    if (!bulk) fprintf(stderr,"Cache GetItems failed: %s\n",error.message ? error.message : reply ? dbus_message_get_signature(reply) : "no reply");
    if (call) dbus_message_unref(call); if (reply) dbus_message_unref(reply); dbus_error_free(&error);
    printf("Native cache: %u additions, %u removals, %u invalid signatures, bulk=%s\n",cache_added,cache_removed,cache_invalid,bulk ? "valid" : "invalid");
    return bulk && cache_added && cache_removed && !cache_invalid;
#else
    return true;
#endif
}
