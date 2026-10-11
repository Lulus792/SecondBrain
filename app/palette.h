#ifndef SB_PALETTE_H
#define SB_PALETTE_H
#include <SDL3/SDL.h>
#include <string.h>
enum {SB_P_WINDOW,SB_P_TEXT,SB_P_BUTTON,SB_P_BUTTON_TEXT,SB_P_HIGHLIGHT,SB_P_HIGHLIGHT_TEXT,SB_P_LINK,SB_P_DISABLED,SB_P_COUNT};
typedef struct {bool valid;SDL_Color color[SB_P_COUNT];} SBPalette;
static inline bool sb_palette_equal(const SBPalette *a,const SBPalette *b){return a->valid==b->valid && (!a->valid || !memcmp(a->color,b->color,sizeof(a->color)));}
#endif
