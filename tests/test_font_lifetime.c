/* Concurrent private fonts share SDL_ttf's FreeType library, not their faces. */
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>

enum { WORKERS=8, ROUNDS=128 };
typedef struct {
    const void *bytes;
    size_t length;
    SDL_AtomicInt arrived, generation, failed;
} Probe;
static bool rendezvous(Probe *p) {
    int generation=SDL_GetAtomicInt(&p->generation);
    if (SDL_AddAtomicInt(&p->arrived,1)==WORKERS-1) {
        SDL_SetAtomicInt(&p->arrived,0);
        SDL_SetAtomicInt(&p->generation,generation+1);
    } else {
        while (SDL_GetAtomicInt(&p->generation)==generation) {
            if (SDL_GetAtomicInt(&p->failed)) return false;
            SDL_Delay(0);
        }
    }
    return !SDL_GetAtomicInt(&p->failed);
}
static int worker(void *data) {
    Probe *p=data;
    for (unsigned i=0;i<ROUNDS;++i) {
        SDL_IOStream *io=SDL_IOFromConstMem(p->bytes,p->length);
        TTF_Font *font=io ? TTF_OpenFontIO(io,true,18+(float)(i%3)) : NULL;
        if (!font) {
            fprintf(stderr,"Font open failed: %s\n",SDL_GetError());
            SDL_SetAtomicInt(&p->failed,1);
        }
        bool ready=rendezvous(p);
        /* Each font is opened, used and closed by its owning worker. */
        if (ready && !TTF_FontHasGlyph(font,'A')) SDL_SetAtomicInt(&p->failed,1);
        TTF_CloseFont(font);
        if (!ready || !rendezvous(p)) return 1;
    }
    return 0;
}
int main(int argc,char **argv) {
    if (argc!=2 || !SDL_Init(0) || !TTF_Init()) return 1;
    Probe p={0};
    void *bytes=SDL_LoadFile(argv[1],&p.length);p.bytes=bytes;
    if (!bytes) { TTF_Quit();SDL_Quit();return 1; }
    SDL_Thread *threads[WORKERS]={0};
    for (unsigned i=0;i<WORKERS;++i) {
        threads[i]=SDL_CreateThread(worker,"Font lifetime probe",&p);
        if (!threads[i]) { SDL_SetAtomicInt(&p.failed,1);break; }
    }
    for (unsigned i=0;i<WORKERS;++i) if (threads[i]) {
        int result=0;SDL_WaitThread(threads[i],&result);
        if (result) SDL_SetAtomicInt(&p.failed,1);
    }
    bool failed=SDL_GetAtomicInt(&p.failed)!=0;
    SDL_free(bytes);TTF_Quit();SDL_Quit();
    if (!failed) printf("%u private font lifetimes across %u concurrent owners passed.\n",WORKERS*ROUNDS,WORKERS);
    return failed ? 1 : 0;
}
