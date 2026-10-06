#ifndef SB_DIALOG_H
#define SB_DIALOG_H
#include "ui.h"
typedef struct SBNativeDialogs SBNativeDialogs;
typedef struct { unsigned serial; bool error; char value[SB_PATH_CAP]; } SBDialogReply;
SBNativeDialogs *sb_dialogs_new(void);
void sb_dialogs_free(SBNativeDialogs *dialogs);
Uint32 sb_dialogs_event(const SBNativeDialogs *dialogs);
unsigned sb_dialog_folder(SBNativeDialogs *dialogs, SDL_Window *window, const char *path);
#endif
