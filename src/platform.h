#ifndef SB_PLATFORM_H
#define SB_PLATFORM_H

#include "sb.h"

/* 0: absent, 1: regular file, 2: directory, 3: symlink/reparse point, -1: error. */
int sb_fs_kind(const char *path);
typedef SBStatus (*SBVisit)(const char *name, int kind, void *userdata);
SBStatus sb_fs_list(const char *path, SBVisit visitor, void *userdata);
SBStatus sb_fs_read(const char *path, char **out, size_t *length);
SBStatus sb_fs_write_new(const char *path, const char *data, size_t length);
SBStatus sb_fs_replace(const char *from, const char *to);
SBStatus sb_fs_move_new(const char *from, const char *to);
SBStatus sb_fs_mkdir(const char *path);
SBStatus sb_fs_mkdirs(const char *path);
SBStatus sb_fs_remove(const char *path);
SBStatus sb_fs_absolute(const char *path, char *out, size_t capacity);
unsigned long sb_process_id(void);
SBStatus sb_fs_home(char *out, size_t capacity);

#endif
