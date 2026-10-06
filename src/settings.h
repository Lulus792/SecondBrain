#ifndef SB_SETTINGS_H
#define SB_SETTINGS_H
#include "sb.h"
typedef struct {
    char workspace[SB_PATH_CAP], project[65], note[SB_PATH_CAP];
    unsigned font_percent, width, height;
    bool dark, solid, reduced_motion;
} SBSettings;
void sb_settings_defaults(SBSettings *settings);
SBStatus sb_settings_load(const char *path, SBSettings *out, SBRevision *revision);
SBStatus sb_settings_save(const char *path, const SBSettings *settings, SBRevision expected, SBRevision *saved);
#endif
