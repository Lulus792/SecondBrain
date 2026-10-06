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
static void close_atspi(void) { (void)atspi_exit(); }
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
    int initialized=atspi_init();
    if (initialized!=0 && initialized!=1) return false;
    if (initialized==0) atexit(close_atspi);
    atspi_set_timeout(3000,10000);
    AtspiAccessible *element=NULL;
    Uint64 deadline=SDL_GetTicks()+15000;
    while (!element && SDL_GetTicks()<deadline) {
        AtspiAccessible *desktop=atspi_get_desktop(0);
        if (desktop) {
            atspi_accessible_clear_cache(desktop);
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
        if (p->operation==SB_NATIVE_PRESS) {
            AtspiAction *action=atspi_accessible_get_action_iface(element);
            if (atspi_accessible_get_role(element,NULL)==ATSPI_ROLE_PUSH_BUTTON && action) success=atspi_action_do_action(action,0,&error);
        } else if (p->operation==SB_NATIVE_SET_VALUE) {
            AtspiEditableText *edit=atspi_accessible_get_editable_text_iface(element);
            if (edit) success=atspi_editable_text_set_text_contents(edit,p->value,&error);
        } else {
            AtspiText *text=atspi_accessible_get_text_iface(element);
            char *value=text ? atspi_text_get_text(text,0,-1,&error) : NULL;
            if (value && strlen(value)<p->capacity) { strcpy(p->output,value); success=true; }
            g_free(value);
        }
        g_object_unref(element);
    }
    if (!success) fprintf(stderr,"AT-SPI operation %d on '%s' failed: %s\n",p->operation,p->label,error ? error->message : "object/pattern not found");
    g_clear_error(&error); return success;
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
