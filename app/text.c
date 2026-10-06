#include "text.h"
#include "platform.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define SB_TEXT_CACHE_ENTRIES 1024
#define SB_TEXT_CACHE_BYTES (32u * 1024u * 1024u)
#define SB_FALLBACK_COUNT 5

typedef struct {
    struct nk_font nk;
    TTF_Font *font, *fallback[SB_FALLBACK_COUNT];
    struct SBTextSystem *owner;
} SBTextFace;
typedef struct {
    SDL_Texture *texture;
    char *text;
    size_t length, bytes;
    unsigned face;
    int width, height, maximum;
    uint64_t stamp;
} SBTextCache;
struct SBTextSystem {
    SDL_Renderer *renderer;
    float density;
    SBTextFace faces[4];
    SBTextCache cache[SB_TEXT_CACHE_ENTRIES];
    size_t bytes;
    uint64_t frame;
};

static void cache_free(SBTextSystem *text, SBTextCache *entry) {
    SDL_DestroyTexture(entry->texture); free(entry->text);
    text->bytes-=entry->bytes; memset(entry,0,sizeof(*entry));
}
static void system_free(SBTextSystem *text) {
    if (!text) return;
    for (unsigned i=0;i<SB_TEXT_CACHE_ENTRIES;++i) cache_free(text,&text->cache[i]);
    for (unsigned i=0;i<4;++i) {
        TTF_CloseFont(text->faces[i].font);
        for (unsigned j=0;j<SB_FALLBACK_COUNT;++j) TTF_CloseFont(text->faces[i].fallback[j]);
    }
    free(text);
}
static float width(nk_handle handle, float height, const char *value, int length) {
    SBTextFace *face=handle.ptr; int w=0,h=0; (void)height;
    if (length<=0 || !value || !face) return 0;
    if (!TTF_GetStringSize(face->font,value,(size_t)length,&w,&h)) return 0;
    return (float)w/face->owner->density;
}
static TTF_Font *open_font(const char *path, float logical_height, float density) {
    TTF_Font *font=TTF_OpenFont(path,logical_height*density);
    if (!font) return NULL;
    int height=TTF_GetFontHeight(font);
    if (height<=0 || !TTF_SetFontSize(font,logical_height*density*logical_height*density/(float)height)) {
        TTF_CloseFont(font); return NULL;
    }
    return font;
}
SBStatus sb_ui_text_fonts(SBUi *ui, float scale, float density) {
    static const char *fallbacks[]={"NotoSansArabic-Regular.ttf","NotoSansHebrew-Regular.ttf",
        "NotoSansDevanagari-Regular.ttf","NotoSansSymbols2-Regular.ttf","NotoSansCJKjp-Regular.otf"};
    const float heights[]={15,18,26,17};
    bool first=ui->text==NULL;
    if (first && !TTF_Init()) return sb_error(SB_IO,"Textdarstellung: %s",SDL_GetError());
    SBTextSystem *next=calloc(1,sizeof(*next));
    if (!next) { if (first) TTF_Quit(); return sb_error(SB_MEMORY,"Textdarstellung benötigt mehr Speicher."); }
    next->renderer=ui->renderer; next->density=density; next->frame=1;
    char folder[SB_PATH_CAP],path[SB_PATH_CAP];
    snprintf(folder,sizeof(folder),"%s",ui->font_path);
    char *slash=strrchr(folder,'/');
    if (slash) *slash=0;
    else snprintf(folder,sizeof(folder),".");
    for (unsigned i=0;i<4;++i) {
        SBTextFace *face=&next->faces[i]; face->owner=next;
        if (i==3) { if (sb_path_join(path,sizeof(path),folder,"NotoSansMono-Regular.ttf").code!=SB_OK) goto failed; }
        else snprintf(path,sizeof(path),"%s",ui->font_path);
        face->font=open_font(path,heights[i]*scale,density);
        if (!face->font) goto failed;
        for (unsigned j=0;j<SB_FALLBACK_COUNT;++j) {
            if (sb_path_join(path,sizeof(path),folder,fallbacks[j]).code!=SB_OK) goto failed;
            face->fallback[j]=TTF_OpenFont(path,TTF_GetFontSize(face->font));
            if (!face->fallback[j] || !TTF_AddFallbackFont(face->font,face->fallback[j])) goto failed;
        }
        face->nk.handle.userdata=nk_handle_ptr(face);
        face->nk.handle.height=heights[i]*scale;
        face->nk.handle.width=width;
    }
    system_free(ui->text); ui->text=next;
    ui->normal=&next->faces[0].nk; ui->body=&next->faces[1].nk;
    ui->heading=&next->faces[2].nk; ui->code=&next->faces[3].nk;
    ui->scale=scale; ui->density=density;
    nk_style_set_font(ui->ctx,&ui->normal->handle);
    return sb_ok();
failed: {
    SBStatus error=sb_error(SB_IO,"Schrift konnte nicht geladen werden: %s",SDL_GetError());
    system_free(next); if (first) TTF_Quit(); return error;
}}

void sb_ui_text_draw(struct nk_draw_list *list, const struct nk_command_text *command) {
    if (!command->font || command->length<=0 || !command->foreground.a) return;
    struct nk_rect rect=nk_rect(command->x,command->y,command->w,command->h);
    if (rect.x>=list->clip_rect.x+list->clip_rect.w || rect.y>=list->clip_rect.y+list->clip_rect.h ||
        rect.x+rect.w<=list->clip_rect.x || rect.y+rect.h<=list->clip_rect.y) return;
    SBTextFace *face=command->font->userdata.ptr;
    if (!face || !face->owner) return;
    SBTextSystem *text=face->owner;
    unsigned face_number=(unsigned)(face-text->faces);
    size_t length=(size_t)command->length;
    int maximum=(int)fminf(8192,ceilf((rect.w+2)*text->density));
    if (maximum<1) return;
    SBTextCache *entry=NULL;
    for (unsigned i=0;i<SB_TEXT_CACHE_ENTRIES;++i) {
        SBTextCache *candidate=&text->cache[i];
        if (candidate->texture && candidate->face==face_number && candidate->length==length && candidate->maximum==maximum &&
            !memcmp(candidate->text,command->string,length)) { entry=candidate; break; }
    }
    if (!entry) {
        /* All textures referenced by this frame stay alive until conversion draws them. */
        for (unsigned i=0;i<SB_TEXT_CACHE_ENTRIES;++i)
            if (!text->cache[i].texture) { entry=&text->cache[i]; break; }
        if (!entry) return;
        int measured=0; size_t fitting=0;
        if (maximum<1 || !TTF_MeasureString(face->font,command->string,length,maximum,&measured,&fitting) || !fitting) return;
        if (fitting<length) length=fitting;
        SDL_Surface *surface=TTF_RenderText_Blended(face->font,command->string,length,(SDL_Color){255,255,255,255});
        if (!surface) return;
        entry->text=malloc((size_t)command->length);
        if (entry->text) entry->texture=SDL_CreateTextureFromSurface(text->renderer,surface);
        if (!entry->text || !entry->texture) {
            SDL_DestroySurface(surface); free(entry->text); memset(entry,0,sizeof(*entry)); return;
        }
        memcpy(entry->text,command->string,(size_t)command->length);
        entry->length=(size_t)command->length; entry->face=face_number; entry->maximum=maximum;
        entry->width=surface->w; entry->height=surface->h;
        entry->bytes=(size_t)surface->w*(size_t)surface->h*4+entry->length;
        text->bytes+=entry->bytes;
        SDL_DestroySurface(surface);
        SDL_SetTextureBlendMode(entry->texture,SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(entry->texture,SDL_SCALEMODE_LINEAR);
    }
    entry->stamp=text->frame;
    rect.w=entry->width/text->density; rect.h=entry->height/text->density;
    nk_draw_list_add_image(list,nk_image_ptr(entry->texture),rect,command->foreground);
}
void sb_ui_text_frame_end(SBUi *ui) {
    SBTextSystem *text=ui->text; if (!text) return;
    /* Release unused entries promptly; no draw command still references them here. */
    for (unsigned i=0;i<SB_TEXT_CACHE_ENTRIES;++i)
        if (text->cache[i].texture && text->cache[i].stamp!=text->frame) cache_free(text,&text->cache[i]);
    if (text->bytes>SB_TEXT_CACHE_BYTES)
        for (unsigned i=0;i<SB_TEXT_CACHE_ENTRIES;++i) cache_free(text,&text->cache[i]);
    ++text->frame;
}
void sb_ui_text_free(SBUi *ui) {
    if (ui->text) { system_free(ui->text); ui->text=NULL; TTF_Quit(); }
}
