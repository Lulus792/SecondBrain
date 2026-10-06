#ifndef SB_SPACE_H
#define SB_SPACE_H
#include <SDL3/SDL.h>
#include "graph.h"
typedef struct { float x, y, depth; unsigned group; bool visible, selected, focused; } SBPoint;
typedef struct { SDL_FRect rect; float radius; } SBGlass;
typedef struct {
    SDL_Texture *texture;
    unsigned char *base, *pixels, *sky;
    int width, height;
    bool dark, solid, sky_dark, sky_ready, cached;
    uint64_t fingerprint;
    SBPoint *points;
    size_t count;
    const SBGraph *graph;
    SBGlass glass[64];
    unsigned glass_count;
    float mouse_x, mouse_y;
} SBSpace;
bool sb_space_draw(SBSpace *space, SDL_Renderer *renderer, int width, int height);
void sb_space_free(SBSpace *space);
#endif
