#ifndef SB_NATIVE_PROBE_H
#define SB_NATIVE_PROBE_H
#include <SDL3/SDL.h>
enum { SB_NATIVE_PRESS,SB_NATIVE_SET_VALUE,SB_NATIVE_READ_VALUE,SB_NATIVE_READ_NAME };
bool sb_native_probe(SDL_Window *window,const char *label,const char *value,int operation,
    char *output,size_t capacity,void (*pump)(void *),void *context);
#endif
