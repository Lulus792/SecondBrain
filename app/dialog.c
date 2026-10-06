#include "dialog.h"
#include <string.h>
#include <stdio.h>
struct SBNativeDialogs { SDL_Mutex *mutex; SDL_AtomicInt references; Uint32 event; bool alive; unsigned serial; };
typedef struct { SBNativeDialogs *owner; unsigned serial; } Request;
static void release(SBNativeDialogs *d) {
    if (SDL_AtomicDecRef(&d->references)) { SDL_DestroyMutex(d->mutex); SDL_free(d); }
}
SBNativeDialogs *sb_dialogs_new(void) {
    SBNativeDialogs *d=SDL_calloc(1,sizeof(*d));
    if (!d) return NULL;
    d->mutex=SDL_CreateMutex(); d->event=SDL_RegisterEvents(1); d->alive=true;
    SDL_SetAtomicInt(&d->references,1);
    if (!d->mutex || !d->event) { if (d->mutex) SDL_DestroyMutex(d->mutex); SDL_free(d); return NULL; }
    return d;
}
Uint32 sb_dialogs_event(const SBNativeDialogs *d) { return d ? d->event : 0; }
static void completed(void *userdata,const char *const *files,int filter) {
    (void)filter;
    Request *request=userdata; SBNativeDialogs *d=request->owner;
    SBDialogReply *reply=SDL_calloc(1,sizeof(*reply));
    if (reply) {
        reply->serial=request->serial; reply->error=files==NULL;
        const char *value=files ? files[0] : SDL_GetError();
        if (value) {
            if (strlen(value)>=sizeof(reply->value) || !sb_utf8_valid(value,strlen(value))) {
                reply->error=true; value="Der ausgewählte Pfad ist ungültig oder zu lang.";
            }
            snprintf(reply->value,sizeof(reply->value),"%s",value);
        }
        SDL_LockMutex(d->mutex);
        SDL_Event e={0}; e.type=d->event; e.user.data1=reply;
        if (!d->alive || !SDL_PushEvent(&e)) SDL_free(reply);
        SDL_UnlockMutex(d->mutex);
    }
    SDL_free(request); release(d);
}
unsigned sb_dialog_folder(SBNativeDialogs *d,SDL_Window *window,const char *path) {
    if (!d) return 0;
    Request *request=SDL_malloc(sizeof(*request));
    if (!request) return 0;
    request->owner=d; request->serial=++d->serial;
    SDL_AtomicIncRef(&d->references);
    unsigned serial=request->serial;
    SDL_ShowOpenFolderDialog(completed,request,window,path,false);
    return serial;
}
void sb_dialogs_free(SBNativeDialogs *d) {
    if (!d) return;
    SDL_LockMutex(d->mutex); d->alive=false; SDL_UnlockMutex(d->mutex);
    SDL_Event e;
    while (SDL_PeepEvents(&e,1,SDL_GETEVENT,d->event,d->event)>0) SDL_free(e.user.data1);
    release(d);
}
