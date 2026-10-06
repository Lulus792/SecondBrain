#include "version.h"
#include "sb_version.h"
const char *sb_version(void) { return SB_VERSION; }
const char *sb_build_info(void) {
    return "SecondBrain " SB_VERSION "\n"
           "Build: " SB_SOURCE_REVISION "\n"
           "System: " SB_BUILD_SYSTEM " / " SB_BUILD_ARCH "\n"
           "Configuration: " SB_BUILD_CONFIGURATION "\n";
}
