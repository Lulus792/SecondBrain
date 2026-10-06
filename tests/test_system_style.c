#include "desktop.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef __linux__
#include <dbus/dbus.h>
static SDL_AtomicInt mode,quit;
static DBusHandlerResult portal_call(DBusConnection *bus,DBusMessage *call,void *userdata) {
    (void)userdata;
    if (!dbus_message_is_method_call(call,"org.freedesktop.portal.Settings","ReadAll")) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    int state=SDL_GetAtomicInt(&mode);
    DBusMessage *reply=state==2 ? dbus_message_new_error(call,DBUS_ERROR_FAILED,"Isolated test failure") : dbus_message_new_method_return(call);
    if (!reply) return DBUS_HANDLER_RESULT_NEED_MEMORY;
    if (state!=2) {
        DBusMessageIter root,namespaces,entry,values; dbus_message_iter_init_append(reply,&root);
        const char *namespace="org.freedesktop.appearance";
        dbus_message_iter_open_container(&root,DBUS_TYPE_ARRAY,"{sa{sv}}",&namespaces);
        dbus_message_iter_open_container(&namespaces,DBUS_TYPE_DICT_ENTRY,NULL,&entry);
        dbus_message_iter_append_basic(&entry,DBUS_TYPE_STRING,&namespace);
        dbus_message_iter_open_container(&entry,DBUS_TYPE_ARRAY,"{sv}",&values);
        const char *keys[]={"color-scheme","contrast","reduced-motion"};
        for (unsigned i=0;i<3;++i) {
            DBusMessageIter setting,variant; dbus_uint32_t value=state==3 ? 99 : state ? 1 : i ? 0 : 2;
            dbus_message_iter_open_container(&values,DBUS_TYPE_DICT_ENTRY,NULL,&setting);
            dbus_message_iter_append_basic(&setting,DBUS_TYPE_STRING,&keys[i]);
            dbus_message_iter_open_container(&setting,DBUS_TYPE_VARIANT,"u",&variant);
            dbus_message_iter_append_basic(&variant,DBUS_TYPE_UINT32,&value);
            dbus_message_iter_close_container(&setting,&variant); dbus_message_iter_close_container(&values,&setting);
        }
        dbus_message_iter_close_container(&entry,&values); dbus_message_iter_close_container(&namespaces,&entry); dbus_message_iter_close_container(&root,&namespaces);
    }
    dbus_connection_send(bus,reply,NULL); dbus_message_unref(reply); return DBUS_HANDLER_RESULT_HANDLED;
}
static int portal_loop(void *userdata) {
    DBusConnection *bus=userdata;
    while (!SDL_GetAtomicInt(&quit)) dbus_connection_read_write_dispatch(bus,50);
    return 0;
}
#endif
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"SYSTEM STYLE FAIL %d: %s\n",__LINE__,#x); return 1; } } while (0)
#define OK(x) CHECK((x).code==SB_OK)
static void frame(SBDesktop *d) { nk_input_begin(d->ui.ctx); SDL_Event e; while (SDL_PollEvent(&e)) sb_desktop_event(d,&e); nk_input_end(d->ui.ctx); sb_desktop_tick(d,1.0f/60); sb_desktop_frame(d); sb_ui_draw(&d->ui); SDL_RenderPresent(d->ui.renderer); sb_desktop_apply(d); }
int main(int argc,char **argv) {
    CHECK(argc==3);
    SBStyleChoice request={.dark=true}; SBSystemStyle system={.known=15,.dark=false,.motion=true,.solid=true,.contrast=true};
    SBStyleChoice result=sb_style_resolve(request,system);
    CHECK(result.dark && result.motion && result.solid && result.contrast && !request.motion);
    request.follow_theme=true; result=sb_style_resolve(request,system); CHECK(!result.dark);
    request.motion=true; result=sb_style_resolve(request,(SBSystemStyle){0}); CHECK(result.motion && result.dark);
    SBDesktop d; char root[SB_PATH_CAP],suffix[90],config[SB_PATH_CAP];
    snprintf(suffix,sizeof(suffix),"run-%lu-%lu",sb_process_id(),(unsigned long)time(NULL)); OK(sb_path_join(root,sizeof(root),argv[2],suffix)); OK(sb_fs_mkdirs(root));
    OK(sb_desktop_init(&d,root,argv[1],true)); OK(sb_path_join(config,sizeof(config),root,"settings.conf")); OK(sb_desktop_preferences(&d,config,true));
    OK(sb_app_new_project(&d.model,"projekt","Darstellung",NULL));
    OK(sb_app_new_note(&d.model,"knowledge","notiz","Kontrast und Bewegung"));
    strcpy(d.model.editor,"# Kontrast und Bewegung\n\nDie Einstellungen bleiben erhalten.\n\n## Lesen\n\nText und Bedienelemente sind klar erkennbar.\n"); OK(sb_app_save(&d.model));
    sb_desktop_set_style(&d,(SBStyleChoice){.dark=true,.contrast=true,.motion=true,.follow_theme=true}); frame(&d);
    CHECK(d.ui.contrast && d.solid && d.reduced_motion); OK(sb_desktop_store_preferences(&d));
    SBSettings settings; SBRevision revision; OK(sb_settings_load(config,&settings,&revision)); CHECK(settings.contrast && settings.follow_theme && settings.reduced_motion && !settings.solid);
    char path[SB_PATH_CAP]; OK(sb_path_join(path,sizeof(path),root,"high-contrast-dark.bmp")); OK(sb_ui_capture(&d.ui,path));
    sb_desktop_set_style(&d,(SBStyleChoice){.dark=false,.contrast=true}); frame(&d);
    OK(sb_path_join(path,sizeof(path),root,"high-contrast-light.bmp")); OK(sb_ui_capture(&d.ui,path));
    sb_desktop_set_style(&d,(SBStyleChoice){.dark=true,.follow_theme=true});
#ifdef __linux__
    CHECK(dbus_threads_init_default()); DBusError error; dbus_error_init(&error);
    DBusConnection *bus=dbus_bus_get_private(DBUS_BUS_SESSION,&error); CHECK(bus && !dbus_error_is_set(&error)); dbus_connection_set_exit_on_disconnect(bus,FALSE);
    CHECK(dbus_bus_request_name(bus,"org.freedesktop.portal.Desktop",DBUS_NAME_FLAG_DO_NOT_QUEUE,&error)==DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);
    CHECK(dbus_connection_add_filter(bus,portal_call,NULL,NULL)); SDL_Thread *thread=SDL_CreateThread(portal_loop,"test-settings-portal",bus); CHECK(thread);
    SBSystemStyle native=sb_system_style_read_native();
    unsigned portal_known=SB_SYS_THEME|SB_SYS_MOTION|SB_SYS_CONTRAST;
    CHECK((native.known&portal_known)==portal_known && !native.dark && !native.motion && !native.contrast);
    SDL_SetAtomicInt(&mode,1); native=sb_system_style_read_native(); CHECK(native.dark && native.motion && native.contrast);
    sb_system_style_free(d.style_monitor); d.style_monitor=sb_system_style_new(true); CHECK(d.style_monitor);
    Uint64 deadline=SDL_GetTicks()+5000;
    while (!(d.system_style.known&SB_SYS_MOTION) && SDL_GetTicks()<deadline) { frame(&d); SDL_Delay(10); }
    CHECK(d.reduced_motion && d.solid && d.ui.contrast && d.ui.dark);
    CHECK(!d.requested_style.motion && !d.requested_style.solid && !d.requested_style.contrast);
    SDL_SetAtomicInt(&mode,2); sb_system_style_snapshot(d.style_monitor,true); SDL_Delay(400); frame(&d);
    CHECK(d.reduced_motion && d.ui.contrast);
    SDL_SetAtomicInt(&mode,0); sb_system_style_snapshot(d.style_monitor,true); deadline=SDL_GetTicks()+5000;
    while (d.reduced_motion && SDL_GetTicks()<deadline) { frame(&d); SDL_Delay(10); }
    CHECK(!d.reduced_motion && !d.solid && !d.ui.contrast && !d.ui.dark);
    SDL_SetAtomicInt(&mode,3); native=sb_system_style_read_native(); CHECK(!(native.known&SB_SYS_THEME) && !native.motion && !native.contrast);
    sb_system_style_free(d.style_monitor); d.style_monitor=NULL;
    SDL_SetAtomicInt(&quit,1); SDL_WaitThread(thread,NULL); dbus_connection_close(bus); dbus_connection_unref(bus); dbus_error_free(&error);
    printf("Actual isolated Settings portal read, live update, failure retention and recovery tested.\n");
#else
    SBSystemStyle native=sb_system_style_read_native(); CHECK((native.known&14)==14);
    printf("Native OS accessibility preferences queried read-only: known mask=%u.\n",native.known);
#endif
    sb_desktop_free(&d); printf("%u system appearance assertions passed.\n",checks); return 0;
}
