#ifndef SB_SYSTEM_STYLE_H
#define SB_SYSTEM_STYLE_H
#include "palette.h"
enum { SB_SYS_THEME=1,SB_SYS_MOTION=2,SB_SYS_TRANSPARENCY=4,SB_SYS_CONTRAST=8,SB_SYS_THEME_NONE=16,SB_SYS_PALETTE=32 };
typedef struct { unsigned known; bool dark,motion,solid,contrast; SBPalette palette; } SBSystemStyle;
typedef struct { bool dark,motion,solid,contrast,follow_theme; } SBStyleChoice;
typedef struct SBStyleMonitor SBStyleMonitor;
SBStyleChoice sb_style_resolve(SBStyleChoice requested,SBSystemStyle system);
SBPalette sb_style_palette(SBStyleChoice effective,SBSystemStyle system);
SBPalette sb_system_palette_read_native(void);
void sb_system_style_merge(SBSystemStyle *last,SBSystemStyle update);
SBSystemStyle sb_system_style_read_native(void);
SBStyleMonitor *sb_system_style_new(bool native);
SBSystemStyle sb_system_style_snapshot(SBStyleMonitor *monitor,bool force);
void sb_system_style_free(SBStyleMonitor *monitor);
#endif
