#ifndef SB_PREPARE_H
#define SB_PREPARE_H
#include "text.h"
typedef struct SBPrepareJob SBPrepareJob;
typedef enum {SB_PREPARE_PENDING,SB_PREPARE_READY,SB_PREPARE_FAILED,SB_PREPARE_CANCELLED} SBPrepareState;
/* Isolated shaping prototype. Takes the snapshot only on successful start.
   Caller source may change immediately. Validation/shaping run on the worker;
   no UI/renderer calls there. This prototype returns one unwrapped line. */
SBPrepareJob *sb_prepare_start(SBFontSnapshot *font,const char *source,size_t length,uint64_t context);
SBPrepareState sb_prepare_state(SBPrepareJob *job,SBStatus *status);
void sb_prepare_cancel(SBPrepareJob *job);
/* Transfers a complete result only for exact current source/context/fonts.
   No source file is modified. Output must be empty and is owned on success. */
SBStatus sb_prepare_take(SBPrepareJob *job,SBUi *ui,const char *source,size_t length,uint64_t context,SBShapedLine *out);
/* Waits if necessary. Retargeting should cancel, retire, then free after done. */
void sb_prepare_free(SBPrepareJob *job);
typedef struct SBPrepareLayout SBPrepareLayout;
/* Takes the text snapshot only on successful start. Owns source/spans. Role:
   0 normal, 1 body, 2 heading, 3 code. Padding/gap are logical window units. */
SBPrepareLayout *sb_prepare_layout_start(SBTextSnapshot *fonts,const SBStyledText *text,
    unsigned role,float width,struct nk_vec2 padding,float gap,bool terminal_line,uint64_t context);
SBPrepareState sb_prepare_layout_state(SBPrepareLayout *job,SBStatus *status);
void sb_prepare_layout_cancel(SBPrepareLayout *job);
SBStatus sb_prepare_layout_take(SBPrepareLayout *job,SBUi *ui,const SBStyledText *text,
    unsigned role,float width,struct nk_vec2 padding,float gap,bool terminal_line,uint64_t context,SBStyledPlan *out);
void sb_prepare_layout_free(SBPrepareLayout *job);
#endif
