#include "text.h"
#include "platform.h"
#include "grapheme.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define SB_TEXT_CACHE_ENTRIES 1024
#define SB_TEXT_CACHE_BYTES (32u * 1024u * 1024u)
#define SB_FALLBACK_COUNT 7
#define SB_TEXT_FACES 32
typedef struct SBGlyphTexture {SBShapeGlyph *signature;size_t count;uint64_t hash;int left,top,w,h;SDL_Texture *texture;size_t bytes;uint64_t stamp;struct SBGlyphTexture *next;} SBGlyphTexture;
#define SB_GLYPH_BUCKETS 1024
#define SB_GLYPH_BYTES (32u*1024u*1024u)

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
    int width, height, maximum,offset;
    uint64_t stamp;
} SBTextCache;
/* Bounded, exact-key measurement cache. Font systems own all entries and
   discard them on font/density changes; hash collisions compare full bytes. */
typedef struct {
    char *text;size_t length;uint64_t hash;unsigned face;
    float width,ascent,descent;bool width_ready,metrics_ready;
} SBMeasureCache;
#define SB_MEASURE_CACHE_ENTRIES 4096
#define SB_MEASURE_CACHE_LENGTH 2048
struct SBTextSystem {
    SDL_Renderer *renderer;
    TTF_TextEngine *surface_engine;
    float density;
    SBTextFace faces[SB_TEXT_FACES];
    SBTextCache cache[SB_TEXT_CACHE_ENTRIES];
    SBMeasureCache measures[SB_MEASURE_CACHE_ENTRIES];
    SBGlyphTexture *glyphs[SB_GLYPH_BUCKETS];size_t glyph_bytes;
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
    for (unsigned i=0;i<SB_MEASURE_CACHE_ENTRIES;++i) free(text->measures[i].text);
    for(unsigned i=0;i<SB_GLYPH_BUCKETS;++i){SBGlyphTexture *entry=text->glyphs[i];while(entry){SBGlyphTexture *next=entry->next;SDL_DestroyTexture(entry->texture);free(entry->signature);free(entry);entry=next;}}
    TTF_DestroySurfaceTextEngine(text->surface_engine);
    for (unsigned i=0;i<SB_TEXT_FACES;++i) {
        TTF_CloseFont(text->faces[i].font);
        for (unsigned j=0;j<SB_FALLBACK_COUNT;++j) TTF_CloseFont(text->faces[i].fallback[j]);
    }
    free(text);
}
typedef struct { size_t start,end; TTF_Font *font; } SBFontRun;
typedef struct { SBTextFace *face; SBGrapheme reader; SBGraphemeBoundary boundary; TTF_Font *previous; bool available; } SBFontRuns;
static bool ignored(Uint32 cp) {
    return cp==0x200d || cp==0x200c || (cp>=0xfe00 && cp<=0xfe0f) || (cp>=0xe0020 && cp<=0xe007f) || (cp>=0xe0100 && cp<=0xe01ef);
}
static bool covers(TTF_Font *font,const char *text,size_t length) {
    const char *end=text+length;
    while (text<end) { Uint32 cp=(Uint32)SDL_StepUTF8(&text,NULL); if (!ignored(cp) && !TTF_FontHasGlyph(font,cp)) return false; }
    return true;
}
static TTF_Font *cluster_font(SBTextFace *face,const char *text,size_t length,TTF_Font *previous) {
    const char *at=text,*end=text+length; bool emoji=false,neutral=true;
    while (at<end) {
        Uint32 cp=(Uint32)SDL_StepUTF8(&at,NULL);
        emoji|=cp>=0x1f000 || (cp>=0x2600 && cp<=0x27bf) || cp==0xfe0f || cp==0x20e3;
        if (cp>=0x80 || (cp>='A' && cp<='Z') || (cp>='a' && cp<='z') || (cp>='0' && cp<='9')) neutral=false;
    }
    if (emoji && covers(face->fallback[3],text,length)) return face->fallback[3];
    if (neutral && previous && previous!=face->fallback[3] && covers(previous,text,length)) return previous;
    if (covers(face->font,text,length)) return face->font;
    for (unsigned i=0;i<SB_FALLBACK_COUNT;++i) if (covers(face->fallback[i],text,length)) return face->fallback[i];
    return face->font;
}
TTF_Font *sb_ui_cluster_font(const struct nk_user_font *font,const char *value,size_t length,TTF_Font *previous){SBTextFace *face=font ? font->userdata.ptr : NULL;return face ? cluster_font(face,value,length,previous) : NULL;}
static uint64_t glyph_hash(const SBShapedLine *line) {
    uint64_t hash=1469598103934665603ULL;
    for(size_t i=0;i<line->count;++i){const SBShapeGlyph *g=&line->glyphs[i];uint32_t x,y;memcpy(&x,&g->x,4);memcpy(&y,&g->y,4);
        hash=(hash^(uintptr_t)g->font)*1099511628211ULL;hash=(hash^g->index)*1099511628211ULL;hash=(hash^x)*1099511628211ULL;hash=(hash^y)*1099511628211ULL;}
    return hash;
}
static bool glyph_equal(const SBGlyphTexture *entry,const SBShapedLine *line) {
    if(entry->count!=line->count)return false;
    for(size_t i=0;i<line->count;++i){const SBShapeGlyph *a=&entry->signature[i],*b=&line->glyphs[i];
        if(a->font!=b->font || a->index!=b->index || a->x!=b->x || a->y!=b->y || a->left!=b->left || a->top!=b->top || a->width!=b->width || a->height!=b->height)return false;}
    return true;
}
static SBGlyphTexture *glyph_texture(SBTextSystem *text,const SBShapedLine *line,int left,int top,int w,int h) {
    uint64_t hash=glyph_hash(line);size_t bucket=hash%SB_GLYPH_BUCKETS;
    for(SBGlyphTexture *e=text->glyphs[bucket];e;e=e->next)if(e->hash==hash && e->left==left && e->top==top && e->w==w && e->h==h && glyph_equal(e,line)){e->stamp=text->frame;return e;}
    SDL_Surface *surface=SDL_CreateSurface(w,h,SDL_PIXELFORMAT_ARGB8888);if(!surface)return NULL;
    SDL_ClearSurface(surface,0,0,0,0);
    for(size_t i=0;i<line->count;++i){const SBShapeGlyph *g=&line->glyphs[i];if(!g->width || !g->height)continue;
        float x=floorf(g->x+g->left)-left,y=floorf(g->y-g->top)-top;
        if(x>=w || y>=h || x+g->width<=0 || y+g->height<=0)continue;
        TTF_ImageType type;SDL_Surface *image=TTF_GetGlyphImageForIndex(g->font,g->index,&type);if(!image){SDL_DestroySurface(surface);return NULL;}
        SDL_Rect dest={(int)x,(int)y,image->w,image->h};
        bool copied=SDL_SetSurfaceBlendMode(image,SDL_BLENDMODE_BLEND) && SDL_BlitSurface(image,NULL,surface,&dest);
        SDL_DestroySurface(image);if(!copied){SDL_DestroySurface(surface);return NULL;}
    }
    /* Compositing into a transparent tile produces premultiplied RGB. */
    for(int y=0;y<h;++y){Uint32 *row=(Uint32 *)((Uint8 *)surface->pixels+y*surface->pitch);for(int x=0;x<w;++x){Uint32 p=row[x],a=p>>24;if(a && a<255){Uint32 r=SDL_min(255,(((p>>16)&255)*255+a/2)/a),g=SDL_min(255,(((p>>8)&255)*255+a/2)/a),b=SDL_min(255,((p&255)*255+a/2)/a);row[x]=(a<<24)|(r<<16)|(g<<8)|b;}}}
    SBGlyphTexture *e=calloc(1,sizeof(*e));
    if(e)e->signature=malloc(line->count*sizeof(*e->signature));
    if(e && e->signature)e->texture=SDL_CreateTextureFromSurface(text->renderer,surface);
    SDL_DestroySurface(surface);
    if(!e || !e->texture){if(e)free(e->signature);free(e);return NULL;}
    memcpy(e->signature,line->glyphs,line->count*sizeof(*e->signature));e->count=line->count;e->hash=hash;e->left=left;e->top=top;e->w=w;e->h=h;
    e->bytes=(size_t)w*h*4+line->count*sizeof(*e->signature);e->stamp=text->frame;
    SDL_SetTextureBlendMode(e->texture,SDL_BLENDMODE_BLEND);SDL_SetTextureScaleMode(e->texture,SDL_SCALEMODE_LINEAR);
    e->next=text->glyphs[bucket];text->glyphs[bucket]=e;text->glyph_bytes+=e->bytes;return e;
}
bool sb_ui_shaped_draw(SBUi *ui,const SBShapedLine *line,float x,float baseline,struct nk_color color) {
    if(!ui || !ui->text || !line)return false;
    struct nk_command_buffer *canvas=nk_window_get_canvas(ui->ctx);struct nk_rect clip=canvas->clip;float density=ui->text->density;
    float left=0,right=0,top=0,bottom=0;bool ink=false;
    for(size_t i=0;i<line->count;++i){const SBShapeGlyph *g=&line->glyphs[i];if(!g->width || !g->height)continue;
        float gx=floorf(g->x+g->left),gy=floorf(g->y-g->top);
        if(!ink){left=gx;right=gx+g->width;top=gy;bottom=gy+g->height;ink=true;}
        else{left=fminf(left,gx);right=fmaxf(right,gx+g->width);top=fminf(top,gy);bottom=fmaxf(bottom,gy+g->height);}
    }
    if(!ink || x+right/density<=clip.x || x+left/density>=clip.x+clip.w || baseline+bottom/density<=clip.y || baseline+top/density>=clip.y+clip.h)return true;
    if(right-left>8192){left=fmaxf(left,floorf((clip.x-x)*density)-2);right=fminf(right,ceilf((clip.x+clip.w-x)*density)+2);}
    if(bottom-top>8192){top=fmaxf(top,floorf((clip.y-baseline)*density)-2);bottom=fminf(bottom,ceilf((clip.y+clip.h-baseline)*density)+2);}
    if((double)left<INT_MIN || (double)right>INT_MAX || (double)top<INT_MIN || (double)bottom>INT_MAX)return false;
    for(int y=(int)top;y<(int)bottom;){int h=SDL_min(8192,(int)bottom-y);
        for(int at=(int)left;at<(int)right;){int w=SDL_min(8192,(int)right-at);SBGlyphTexture *image=glyph_texture(ui->text,line,at,y,w,h);if(!image)return false;
            struct nk_image handle=nk_image_ptr(image->texture);nk_draw_image(canvas,nk_rect(x+at/density,baseline+y/density,w/density,h/density),&handle,color);at+=w;}
        y+=h;
    }return true;
}
static bool runs_init(SBFontRuns *runs,SBTextFace *face,const char *text,size_t length) {
    *runs=(SBFontRuns){.face=face};
    if (!sb_grapheme_init(&runs->reader,text,length)) return false;
    runs->available=sb_grapheme_next(&runs->reader,&runs->boundary); return true;
}
static bool run_next(SBFontRuns *runs,SBFontRun *run) {
    if (!runs->available) return false;
    size_t start=runs->boundary.byte; SBGraphemeBoundary next;
    if (!sb_grapheme_next(&runs->reader,&next)) { runs->available=false; return false; }
    TTF_Font *font=cluster_font(runs->face,runs->reader.text+start,next.byte-start,runs->previous);
    *run=(SBFontRun){start,next.byte,font}; runs->boundary=next; runs->previous=font;
    for (;;) {
        SBGrapheme saved=runs->reader;
        if (!sb_grapheme_next(&runs->reader,&next)) break;
        TTF_Font *candidate=cluster_font(runs->face,runs->reader.text+runs->boundary.byte,next.byte-runs->boundary.byte,font);
        if (candidate!=font) { runs->reader=saved; return true; }
        run->end=next.byte; runs->boundary=next;
    }
    runs->available=false; return true;
}
static SBMeasureCache *measure_entry(SBTextFace *face,const char *value,size_t length) {
    if(length>SB_MEASURE_CACHE_LENGTH)return NULL;
    unsigned index=(unsigned)(face-face->owner->faces);uint64_t hash=sb_hash(value,length)^((uint64_t)index*UINT64_C(0x9e3779b97f4a7c15));
    SBMeasureCache *entry=&face->owner->measures[hash%SB_MEASURE_CACHE_ENTRIES];
    if(entry->text && entry->hash==hash && entry->face==index && entry->length==length && !memcmp(entry->text,value,length))return entry;
    char *copy=malloc(length+1);if(!copy)return NULL;memcpy(copy,value,length);copy[length]=0;
    free(entry->text);*entry=(SBMeasureCache){.text=copy,.length=length,.hash=hash,.face=index};return entry;
}
static float width(nk_handle handle, float height, const char *value, int length) {
    SBTextFace *face=handle.ptr; (void)height;
    if (length<=0 || !value || !face) return 0;
    SBMeasureCache *memo=measure_entry(face,value,(size_t)length);if(memo && memo->width_ready)return memo->width;
    SBFontRuns runs; SBFontRun run; int total=0;
    if (!runs_init(&runs,face,value,(size_t)length)) return 0;
    while (run_next(&runs,&run)) { int w=0,h=0; if (!TTF_GetStringSize(run.font,value+run.start,run.end-run.start,&w,&h)) return 0; total+=w; }
    float measured=(float)total/face->owner->density;if(memo){memo->width=measured;memo->width_ready=true;}return measured;
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
        "NotoSansDevanagari-Regular.ttf","NotoEmoji-Variable.ttf","NotoSansSymbols2-Regular.ttf","NotoSansMath-Regular.ttf","NotoSansCJKjp-Regular.otf"};
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
        SBTextFace *face=&next->faces[i*8]; face->owner=next;
        if (i==3) { if (sb_path_join(path,sizeof(path),folder,"NotoSansMono-Regular.ttf").code!=SB_OK) goto failed; }
        else snprintf(path,sizeof(path),"%s",ui->font_path);
        face->font=open_font(path,heights[i]*scale,density);
        if (!face->font) goto failed;
        for (unsigned j=0;j<SB_FALLBACK_COUNT;++j) {
            if (sb_path_join(path,sizeof(path),folder,fallbacks[j]).code!=SB_OK) goto failed;
            face->fallback[j]=TTF_OpenFont(path,TTF_GetFontSize(face->font));
            if (!face->fallback[j]) goto failed;
        }
        face->nk.handle.userdata=nk_handle_ptr(face);
        face->nk.handle.height=heights[i]*scale;
        face->nk.handle.width=width;
    }
    sb_ui_styled_cache_clear(ui);system_free(ui->text); ui->text=next;
    ui->normal=&next->faces[0].nk; ui->body=&next->faces[8].nk;
    ui->heading=&next->faces[16].nk; ui->code=&next->faces[24].nk;
    ui->scale=scale; ui->density=density;
    nk_style_set_font(ui->ctx,&ui->normal->handle);
    return sb_ok();
failed: {
    SBStatus error=sb_error(SB_IO,"Schrift konnte nicht geladen werden: %s",SDL_GetError());
    system_free(next); if (first) TTF_Quit(); return error;
}}

const struct nk_user_font *sb_ui_text_style(SBUi *ui,const struct nk_user_font *base,unsigned style) {
    SBTextFace *original=base->userdata.ptr;
    if (!original || original->owner!=ui->text) return base;
    unsigned flags=style&7,role=(unsigned)(original-ui->text->faces)/8;
    if (role==3) flags&=3;
    SBTextFace *source=&ui->text->faces[role*8],*face=&ui->text->faces[role*8+flags];
    if (!flags) return &source->nk.handle;
    if (!face->font) {
        face->owner=ui->text; face->nk.handle=source->nk.handle; face->nk.handle.userdata=nk_handle_ptr(face);
        face->font=TTF_CopyFont(flags&SB_TEXT_CODE ? ui->text->faces[24].font : source->font);
        if (!face->font) return &source->nk.handle;
        if ((flags&SB_TEXT_CODE) && !TTF_SetFontSize(face->font,TTF_GetFontSize(source->font))) {
            TTF_CloseFont(face->font); face->font=NULL; return &source->nk.handle;
        }
        TTF_FontStyleFlags ttf=(flags&SB_TEXT_BOLD ? TTF_STYLE_BOLD : 0)|(flags&SB_TEXT_ITALIC ? TTF_STYLE_ITALIC : 0);
        TTF_SetFontStyle(face->font,ttf);
        for (unsigned j=0;j<SB_FALLBACK_COUNT;++j) {
            face->fallback[j]=TTF_CopyFont(source->fallback[j]);
            if (!face->fallback[j]) {
                TTF_CloseFont(face->font); face->font=NULL;
                for (unsigned k=0;k<SB_FALLBACK_COUNT;++k) { TTF_CloseFont(face->fallback[k]); face->fallback[k]=NULL; }
                return &source->nk.handle;
            }
            TTF_SetFontStyle(face->fallback[j],ttf);
        }
    }
    return &face->nk.handle;
}
size_t sb_ui_text_fit(const struct nk_user_font *font,const char *value,size_t length,float available,float *measured) {
    SBTextFace *face=font->userdata.ptr; SBFontRuns runs; SBFontRun run;
    if (measured) *measured=0;
    if (!face || !measured || available<=0 || !runs_init(&runs,face,value,length)) return 0;
    int maximum=(int)fminf(8192,floorf(available*face->owner->density)),total=0; size_t fitting=0;
    while (total<maximum && run_next(&runs,&run)) {
        size_t fit=0; int w=0;
        if (!TTF_MeasureString(run.font,value+run.start,run.end-run.start,maximum-total,&w,&fit)) return 0;
        fitting=run.start+fit; total+=w;
        if (fit<run.end-run.start) break;
    }
    SBGrapheme reader; SBGraphemeBoundary boundary; size_t whole=0;
    if (!sb_grapheme_init(&reader,value,length)) return 0;
    while (sb_grapheme_next(&reader,&boundary) && boundary.byte<=fitting) whole=boundary.byte;
    *measured=font->width(font->userdata,font->height,value,(int)whole); return whole;
}
bool sb_ui_text_metrics(const struct nk_user_font *font,const char *value,size_t length,float *ascent,float *descent) {
    SBTextFace *face=font->userdata.ptr; SBFontRuns runs; SBFontRun run;
    if (!face || !ascent || !descent || !runs_init(&runs,face,value,length)) return false;
    SBMeasureCache *memo=measure_entry(face,value,length);
    if(memo && memo->metrics_ready){*ascent=memo->ascent;*descent=memo->descent;return true;}
    int above=TTF_GetFontAscent(face->font),below=-TTF_GetFontDescent(face->font);
    while (run_next(&runs,&run)) {
        above=SDL_max(above,TTF_GetFontAscent(run.font)); below=SDL_max(below,-TTF_GetFontDescent(run.font));
    }
    *ascent=above/face->owner->density; *descent=below/face->owner->density;
    if(memo){memo->ascent=*ascent;memo->descent=*descent;memo->metrics_ready=true;}return true;
}

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
    int offset=(int)fmaxf(0,floorf((list->clip_rect.x-rect.x)*text->density)-2);
    int maximum=(int)fminf(8192,ceilf((fminf(rect.x+rect.w+2,list->clip_rect.x+list->clip_rect.w+2)-rect.x)*text->density)-offset);
    if (maximum<1) return;
    SBTextCache *entry=NULL;
    for (unsigned i=0;i<SB_TEXT_CACHE_ENTRIES;++i) {
        SBTextCache *candidate=&text->cache[i];
        if (candidate->texture && candidate->face==face_number && candidate->length==length && candidate->maximum==maximum && candidate->offset==offset &&
            !memcmp(candidate->text,command->string,length)) { entry=candidate; break; }
    }
    if (!entry) {
        /* All textures referenced by this frame stay alive until conversion draws them. */
        for (unsigned i=0;i<SB_TEXT_CACHE_ENTRIES;++i)
            if (!text->cache[i].texture) { entry=&text->cache[i]; break; }
        if (!entry) return;
        SBFontRuns runs; SBFontRun run; int measured=0,ascent=0,descent=0,total=0;
        if (!runs_init(&runs,face,command->string,length)) return;
        while (run_next(&runs,&run)) {
            int w=0,h=0; if (!TTF_GetStringSize(run.font,command->string+run.start,run.end-run.start,&w,&h)) return;
            if (measured<maximum) measured=SDL_min(maximum,measured+w);
            total+=w;
            ascent=SDL_max(ascent,TTF_GetFontAscent(run.font)); descent=SDL_max(descent,-TTF_GetFontDescent(run.font));
        }
        if (measured<=0 || ascent+descent<=0) return;
        /* Match SDL_ttf's raster format; avoid an extra channel conversion. */
        SDL_Surface *surface=SDL_CreateSurface(measured,ascent+descent,SDL_PIXELFORMAT_ARGB8888);
        if (!surface) return;
        if (!SDL_ClearSurface(surface,0,0,0,0)) { SDL_DestroySurface(surface); return; }
        runs_init(&runs,face,command->string,length); int x=0;
        bool tile=offset>0 || total>maximum;
        if(tile && !text->surface_engine)text->surface_engine=TTF_CreateSurfaceTextEngine();
        if(tile && !text->surface_engine){SDL_DestroySurface(surface);return;}
        while ((tile ? x<offset+maximum : x<maximum) && run_next(&runs,&run)) {
            int w=0,h=0; size_t fitting=0,run_length=run.end-run.start;
            if(tile){
                if(!TTF_GetStringSize(run.font,command->string+run.start,run_length,&w,&h)){SDL_DestroySurface(surface);return;}
                if(x+w>offset){
                    /* Shape the complete run, then paint just its visible tile. */
                    TTF_Text *shaped=TTF_CreateText(text->surface_engine,run.font,command->string+run.start,run_length);
                    bool drawn=shaped && TTF_SetTextColor(shaped,255,255,255,255) && TTF_DrawSurfaceText(shaped,x-offset,ascent-TTF_GetFontAscent(run.font),surface);
                    TTF_DestroyText(shaped);
                    if(!drawn){SDL_DestroySurface(surface);return;}
                }
                x+=w;continue;
            }
            if (!TTF_MeasureString(run.font,command->string+run.start,run_length,maximum-x,&w,&fitting)) { SDL_DestroySurface(surface); return; }
            if (!fitting) break;
            SDL_Surface *piece=TTF_RenderText_Blended(run.font,command->string+run.start,fitting,(SDL_Color){255,255,255,255});
            if (!piece) { SDL_DestroySurface(surface); return; }
            SDL_Rect destination={x,ascent-TTF_GetFontAscent(run.font),piece->w,piece->h};
            /* Preserve straight alpha; the final texture draw blends exactly once. */
            bool copied=SDL_SetSurfaceBlendMode(piece,SDL_BLENDMODE_NONE) && SDL_BlitSurface(piece,NULL,surface,&destination);
            if (!copied) { SDL_DestroySurface(piece); SDL_DestroySurface(surface); return; }
            SDL_DestroySurface(piece); x+=w;
            if (fitting<run_length) break;
        }
        if(tile){
            /* Surface compositing produces premultiplied RGB. Textures below
               blend straight alpha, including any color glyphs. */
            for(int y=0;y<surface->h;++y){Uint32 *pixels=(Uint32 *)((Uint8 *)surface->pixels+y*surface->pitch);
                for(int i=0;i<surface->w;++i){Uint32 p=pixels[i],a=p>>24;if(a && a<255){
                    Uint32 r=SDL_min(255,(((p>>16)&255)*255+a/2)/a),g=SDL_min(255,(((p>>8)&255)*255+a/2)/a),b=SDL_min(255,((p&255)*255+a/2)/a);
                    pixels[i]=(a<<24)|(r<<16)|(g<<8)|b;
                }}
            }
        }
        entry->text=malloc((size_t)command->length);
        if (entry->text) entry->texture=SDL_CreateTextureFromSurface(text->renderer,surface);
        if (!entry->text || !entry->texture) {
            SDL_DestroySurface(surface); free(entry->text); memset(entry,0,sizeof(*entry)); return;
        }
        memcpy(entry->text,command->string,(size_t)command->length);
        entry->length=(size_t)command->length; entry->face=face_number; entry->maximum=maximum;entry->offset=offset;
        entry->width=surface->w; entry->height=surface->h;
        entry->bytes=(size_t)surface->w*(size_t)surface->h*4+entry->length;
        text->bytes+=entry->bytes;
        SDL_DestroySurface(surface);
        SDL_SetTextureBlendMode(entry->texture,SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(entry->texture,SDL_SCALEMODE_LINEAR);
    }
    entry->stamp=text->frame;
    rect.x+=entry->offset/text->density;
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
    if(text->glyph_bytes>SB_GLYPH_BYTES)for(unsigned i=0;i<SB_GLYPH_BUCKETS;++i){SBGlyphTexture **at=&text->glyphs[i];while(*at){SBGlyphTexture *e=*at;if(e->stamp<text->frame || text->glyph_bytes>SB_GLYPH_BYTES){*at=e->next;text->glyph_bytes-=e->bytes;SDL_DestroyTexture(e->texture);free(e->signature);free(e);}else at=&e->next;}}
    ++text->frame;
}
void sb_ui_text_free(SBUi *ui) {
    sb_ui_styled_cache_clear(ui);
    if (ui->text) { system_free(ui->text); ui->text=NULL; TTF_Quit(); }
}
