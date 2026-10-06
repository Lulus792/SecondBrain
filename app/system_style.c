#include "system_style.h"
#include <stdlib.h>
#include <string.h>
#ifdef __APPLE__
#include <objc/runtime.h>
#include <objc/message.h>
#elif defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <dbus/dbus.h>
#include <gio/gio.h>
#endif
struct SBStyleMonitor {
    bool native; Uint64 next; SBSystemStyle last;
#ifdef __linux__
    SDL_Thread *thread; SDL_Mutex *mutex; SDL_Condition *condition; bool quit;
#endif
};
SBStyleChoice sb_style_resolve(SBStyleChoice r,SBSystemStyle s) {
    if (r.follow_theme && (s.known&SB_SYS_THEME)) r.dark=s.dark;
    if ((s.known&SB_SYS_MOTION) && s.motion) r.motion=true;
    if ((s.known&SB_SYS_CONTRAST) && s.contrast) r.contrast=true;
    if (((s.known&SB_SYS_TRANSPARENCY) && s.solid) || r.contrast) r.solid=true;
    return r;
}
static void merge(SBSystemStyle *last,SBSystemStyle s) {
    if (s.known&SB_SYS_THEME_NONE) last->known&=~(unsigned)SB_SYS_THEME;
    if (s.known&SB_SYS_THEME) last->known&=~(unsigned)SB_SYS_THEME_NONE;
    if (s.known&SB_SYS_THEME) last->dark=s.dark;
    if (s.known&SB_SYS_MOTION) last->motion=s.motion;
    if (s.known&SB_SYS_TRANSPARENCY) last->solid=s.solid;
    if (s.known&SB_SYS_CONTRAST) last->contrast=s.contrast;
    last->known|=s.known;
}
#ifdef __linux__
static void portal_value(SBSystemStyle *s,const char *name,DBusMessageIter *variant) {
    if (dbus_message_iter_get_arg_type(variant)!=DBUS_TYPE_VARIANT) return;
    DBusMessageIter value; dbus_message_iter_recurse(variant,&value);
    if (dbus_message_iter_get_arg_type(&value)!=DBUS_TYPE_UINT32) return;
    dbus_uint32_t n; dbus_message_iter_get_basic(&value,&n);
    if (!strcmp(name,"color-scheme")) { if (n==1 || n==2) { s->known|=SB_SYS_THEME; s->dark=n==1; } else s->known|=SB_SYS_THEME_NONE; }
    else if (!strcmp(name,"contrast")) { s->known|=SB_SYS_CONTRAST; s->contrast=n==1; }
    else if (!strcmp(name,"reduced-motion")) { s->known|=SB_SYS_MOTION; s->motion=n==1; }
}
static SBSystemStyle portal(void) {
    SBSystemStyle s={0}; DBusError error; dbus_error_init(&error);
    dbus_threads_init_default();
    DBusConnection *bus=dbus_bus_get(DBUS_BUS_SESSION,&error);
    DBusMessage *call=bus ? dbus_message_new_method_call("org.freedesktop.portal.Desktop","/org/freedesktop/portal/desktop","org.freedesktop.portal.Settings","ReadAll") : NULL;
    if (call) {
        DBusMessageIter args,filter; dbus_message_iter_init_append(call,&args);
        const char *namespace="org.freedesktop.appearance";
        bool encoded=dbus_message_iter_open_container(&args,DBUS_TYPE_ARRAY,"s",&filter) &&
            dbus_message_iter_append_basic(&filter,DBUS_TYPE_STRING,&namespace) && dbus_message_iter_close_container(&args,&filter);
        DBusMessage *reply=encoded ? dbus_connection_send_with_reply_and_block(bus,call,300,&error) : NULL;
        if (reply && !strcmp(dbus_message_get_signature(reply),"a{sa{sv}}")) {
            DBusMessageIter root,namespaces; dbus_message_iter_init(reply,&root); dbus_message_iter_recurse(&root,&namespaces);
            unsigned rows=0;
            while (dbus_message_iter_get_arg_type(&namespaces)==DBUS_TYPE_DICT_ENTRY && rows++<128) {
                DBusMessageIter entry; dbus_message_iter_recurse(&namespaces,&entry);
                const char *name; dbus_message_iter_get_basic(&entry,&name); dbus_message_iter_next(&entry);
                if (!strcmp(name,namespace)) {
                    DBusMessageIter settings; dbus_message_iter_recurse(&entry,&settings); unsigned keys=0;
                    while (dbus_message_iter_get_arg_type(&settings)==DBUS_TYPE_DICT_ENTRY && keys++<128) {
                        DBusMessageIter setting; dbus_message_iter_recurse(&settings,&setting);
                        const char *key; dbus_message_iter_get_basic(&setting,&key); dbus_message_iter_next(&setting);
                        portal_value(&s,key,&setting); dbus_message_iter_next(&settings);
                    }
                }
                dbus_message_iter_next(&namespaces);
            }
        }
        if (reply) dbus_message_unref(reply); dbus_message_unref(call);
    }
    if (bus) dbus_connection_unref(bus); dbus_error_free(&error); return s;
}
static void gnome(SBSystemStyle *s) {
    const char *desktop=SDL_getenv("XDG_CURRENT_DESKTOP");
    if (!desktop || !strstr(desktop,"GNOME")) return;
    GSettingsSchemaSource *source=g_settings_schema_source_get_default(); if (!source) return;
    GSettingsSchema *schema=g_settings_schema_source_lookup(source,"org.gnome.desktop.interface",TRUE);
    if (schema && g_settings_schema_has_key(schema,"enable-animations")) {
        GSettings *settings=g_settings_new_full(schema,NULL,NULL);
        bool reduced=!g_settings_get_boolean(settings,"enable-animations");
        if (!(s->known&SB_SYS_MOTION) || reduced) { s->known|=SB_SYS_MOTION; s->motion=reduced; }
        g_object_unref(settings);
    }
    if (schema) g_settings_schema_unref(schema);
}
#endif
SBSystemStyle sb_system_style_read_native(void) {
    SBSystemStyle s={0};
#ifdef __APPLE__
    void *workspace=((void *(*)(void *,SEL))objc_msgSend)(objc_getClass("NSWorkspace"),sel_registerName("sharedWorkspace"));
    const char *names[]={"accessibilityDisplayShouldReduceMotion","accessibilityDisplayShouldReduceTransparency","accessibilityDisplayShouldIncreaseContrast"};
    bool *values[]={&s.motion,&s.solid,&s.contrast}; const unsigned bits[]={SB_SYS_MOTION,SB_SYS_TRANSPARENCY,SB_SYS_CONTRAST};
    for (unsigned i=0;i<3;++i) {
        SEL selector=sel_registerName(names[i]);
        if (((BOOL(*)(void *,SEL,SEL))objc_msgSend)(workspace,sel_registerName("respondsToSelector:"),selector)) {
            *values[i]=((BOOL(*)(void *,SEL))objc_msgSend)(workspace,selector)!=0; s.known|=bits[i];
        }
    }
#elif defined(_WIN32)
    BOOL value=TRUE;
    if (SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION,0,&value,0)) { s.known|=SB_SYS_MOTION; s.motion=!value; }
    if (SystemParametersInfoW(SPI_GETDISABLEOVERLAPPEDCONTENT,0,&value,0)) { s.known|=SB_SYS_TRANSPARENCY; s.solid=value!=0; }
    HIGHCONTRASTW contrast={0}; contrast.cbSize=sizeof(contrast);
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST,sizeof(contrast),&contrast,0)) { s.known|=SB_SYS_CONTRAST; s.contrast=(contrast.dwFlags&HCF_HIGHCONTRASTON)!=0; }
#elif defined(__linux__)
    s=portal(); gnome(&s);
#endif
    return s;
}
#ifdef __linux__
static int worker(void *userdata) {
    SBStyleMonitor *m=userdata;
    for (;;) {
        SBSystemStyle style=sb_system_style_read_native();
        SDL_LockMutex(m->mutex); merge(&m->last,style);
        if (!m->quit) SDL_WaitConditionTimeout(m->condition,m->mutex,1000);
        bool quit=m->quit; SDL_UnlockMutex(m->mutex); if (quit) break;
    }
    return 0;
}
#endif
SBStyleMonitor *sb_system_style_new(bool native) {
    SBStyleMonitor *m=calloc(1,sizeof(*m)); if (!m) return NULL; m->native=native;
#ifdef __linux__
    m->mutex=SDL_CreateMutex(); m->condition=SDL_CreateCondition();
    if (!m->mutex || !m->condition) { sb_system_style_free(m); return NULL; }
    if (native) { m->thread=SDL_CreateThread(worker,"system-appearance",m); if (!m->thread) { sb_system_style_free(m); return NULL; } }
#endif
    return m;
}
SBSystemStyle sb_system_style_snapshot(SBStyleMonitor *m,bool force) {
    SBSystemStyle s={0}; if (!m || !m->native) return s;
#ifdef __linux__
    SDL_LockMutex(m->mutex); s=m->last; if (force) SDL_SignalCondition(m->condition); SDL_UnlockMutex(m->mutex);
#else
    Uint64 now=SDL_GetTicks(); if (force || now>=m->next) { merge(&m->last,sb_system_style_read_native()); m->next=now+1000; } s=m->last;
#endif
    SDL_SystemTheme theme=SDL_GetSystemTheme();
    if (!(s.known&SB_SYS_THEME) && theme!=SDL_SYSTEM_THEME_UNKNOWN) { s.known|=SB_SYS_THEME; s.dark=theme==SDL_SYSTEM_THEME_DARK; }
    return s;
}
void sb_system_style_free(SBStyleMonitor *m) {
    if (!m) return;
#ifdef __linux__
    if (m->thread) { SDL_LockMutex(m->mutex); m->quit=true; SDL_SignalCondition(m->condition); SDL_UnlockMutex(m->mutex); SDL_WaitThread(m->thread,NULL); }
    if (m->condition) SDL_DestroyCondition(m->condition); if (m->mutex) SDL_DestroyMutex(m->mutex);
#endif
    free(m);
}
