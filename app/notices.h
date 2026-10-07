#ifndef SB_NOTICES_H
#define SB_NOTICES_H
#include "sb.h"
size_t sb_notice_count(void);
const char *sb_notice_name(size_t index);
SBStatus sb_notice_read(const char *font_path,size_t index,char **text);
#endif
