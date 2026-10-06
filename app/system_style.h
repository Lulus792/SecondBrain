#ifndef SB_SYSTEM_STYLE_H
#define SB_SYSTEM_STYLE_H
#include <SDL3/SDL.h>
enum { SB_SYS_THEME=1,SB_SYS_MOTION=2,SB_SYS_TRANSPARENCY=4,SB_SYS_CONTRAST=8,SB_SYS_THEME_NONE=16 };
typedef struct { unsigned known; bool dark,motion,solid,contrast; } SBSystemStyle;
typedef struct { bool dark,motion,solid,contrast,follow_theme; } SBStyleChoice;
typedef struct SBStyleMonitor SBStyleMonitor;
SBStyleChoice sb_style_resolve(SBStyleChoice requested,SBSystemStyle system);
SBSystemStyle sb_system_style_read_native(void);
SBStyleMonitor *sb_system_style_new(bool native);
SBSystemStyle sb_system_style_snapshot(SBStyleMonitor *monitor,bool force);
void sb_system_style_free(SBStyleMonitor *monitor);
#endif
