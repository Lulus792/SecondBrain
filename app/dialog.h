#ifndef SB_DIALOG_H
#define SB_DIALOG_H
#include "ui.h"
typedef struct SBNativeDialogs SBNativeDialogs;
typedef enum { SB_DIALOG_FOLDER,SB_DIALOG_SAVE_BACKUP,SB_DIALOG_OPEN_BACKUP } SBDialogKind;
typedef struct { unsigned serial; SBDialogKind kind; bool error; char value[SB_PATH_CAP]; } SBDialogReply;
SBNativeDialogs *sb_dialogs_new(void);
void sb_dialogs_free(SBNativeDialogs *dialogs);
Uint32 sb_dialogs_event(const SBNativeDialogs *dialogs);
unsigned sb_dialog_folder(SBNativeDialogs *dialogs, SDL_Window *window, const char *path);
unsigned sb_dialog_backup(SBNativeDialogs *dialogs,SDL_Window *window,const char *path,bool save);
#endif
