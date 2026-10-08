#include "text.h"
#include "platform.h"
#include "grapheme.h"
#include "scripts.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

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
/* Bounded, exact-key measurement cache. Font systems own all entries and
   discard them on font/density changes; hash collisions compare full bytes. */
typedef struct {
    char *text;size_t length;uint64_t hash;unsigned face;
    float width,ascent,descent;bool width_ready,metrics_ready;
} SBMeasureCache;
#define SB_MEASURE_CACHE_ENTRIES 4096
#define SB_MEASURE_CACHE_LENGTH 2048
struct SBTextSystem {
    SBUi *ui;
    SDL_Renderer *renderer;
    float density;
    SBTextFace faces[SB_TEXT_FACES];
    SBMeasureCache measures[SB_MEASURE_CACHE_ENTRIES];
    SBGlyphTexture *glyphs[SB_GLYPH_BUCKETS];size_t glyph_bytes;
    void *plain;size_t plain_bytes;
    uint64_t frame;
};

static void plain_system_free(SBTextSystem *text);
static void system_free(SBTextSystem *text) {
    if (!text) return;
    plain_system_free(text);
    for (unsigned i=0;i<SB_MEASURE_CACHE_ENTRIES;++i) free(text->measures[i].text);
    for(unsigned i=0;i<SB_GLYPH_BUCKETS;++i){SBGlyphTexture *entry=text->glyphs[i];while(entry){SBGlyphTexture *next=entry->next;SDL_DestroyTexture(entry->texture);free(entry->signature);free(entry);entry=next;}}
    for (unsigned i=0;i<SB_TEXT_FACES;++i) {
        TTF_CloseFont(text->faces[i].font);
        for (unsigned j=0;j<SB_FALLBACK_COUNT;++j) TTF_CloseFont(text->faces[i].fallback[j]);
    }
    free(text);
}
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
#include "plain_text.inc"
static void plain_system_free(SBTextSystem *text){SBPlainPlan *plans=text->plain;if(plans){for(unsigned i=0;i<SB_PLAIN_ENTRIES;++i)plain_free(&plans[i]);free(plans);}}
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
    SBPlainPlan scratch={0};SBPlainPlan *plan=plain_get(face,value,(size_t)length,&scratch);
    float measured=plan ? plan->advance/face->owner->density : 0;plain_free(&scratch);
    if(plan && memo){memo->width=measured;memo->width_ready=true;}return measured;
}
SBUi *sb_ui_font_owner(const struct nk_user_font *font){if(!font || font->width!=width)return NULL;SBTextFace *face=font->userdata.ptr;return face && face->owner ? face->owner->ui : NULL;}
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
    next->ui=ui;next->renderer=ui->renderer; next->density=density; next->frame=1;
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
    sb_ui_edit_cache_clear(ui);sb_ui_styled_cache_clear(ui);system_free(ui->text); ui->text=next;
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
    if(measured)*measured=0;
    if(!font || !value || !measured || available<=0 || length>INT_MAX || !sb_ui_font_owner(font))return 0;
    float full=font->width(font->userdata,font->height,value,(int)length);
    if(full<=available){*measured=full;return length;}
    SBGrapheme reader;SBGraphemeBoundary boundary;size_t *ends=NULL,count=0,capacity=0;
    if(!sb_grapheme_init(&reader,value,length))return 0;
    while(sb_grapheme_next(&reader,&boundary)){
        if(count==capacity){size_t next=capacity ? capacity*2 : 32;size_t *grown=realloc(ends,next*sizeof(*grown));if(!grown){free(ends);return 0;}ends=grown;capacity=next;}
        ends[count++]=boundary.byte;
    }
    size_t low=0,high=count;
    while(low<high){size_t mid=low+(high-low)/2;float w=font->width(font->userdata,font->height,value,(int)ends[mid]);
        if(w<=available)low=mid+1;else high=mid;
    }
    size_t fitting=low ? ends[low-1] : 0;free(ends);
    *measured=font->width(font->userdata,font->height,value,(int)fitting);return fitting;
}
bool sb_ui_text_metrics(const struct nk_user_font *font,const char *value,size_t length,float *ascent,float *descent) {
    if(!font || !value || !ascent || !descent || !sb_ui_font_owner(font))return false;
    SBTextFace *face=font->userdata.ptr;SBMeasureCache *memo=measure_entry(face,value,length);
    if(memo && memo->metrics_ready){*ascent=memo->ascent;*descent=memo->descent;return true;}
    SBPlainPlan scratch={0};SBPlainPlan *plan=plain_get(face,value,length,&scratch);if(!plan)return false;
    *ascent=plan->ascent/face->owner->density;*descent=plan->descent/face->owner->density;plain_free(&scratch);
    if(memo){memo->ascent=*ascent;memo->descent=*descent;memo->metrics_ready=true;}return true;
}

void sb_ui_text_draw(struct nk_draw_list *list, const struct nk_command_text *command) {
    if(!command->font || command->length<=0 || !command->foreground.a)return;
    SBTextFace *face=command->font->userdata.ptr;if(!face || !face->owner)return;
    SBTextSystem *text=face->owner;float density=text->density;
    struct nk_rect clip=list->clip_rect;
    if(command->x>=clip.x+clip.w || command->y>=clip.y+clip.h || command->x+command->w<=clip.x || command->y+command->h<=clip.y)return;
    SBPlainPlan scratch={0};SBPlainPlan *plan=plain_get(face,command->string,(size_t)command->length,&scratch);
    if(!plan)return;
    for(size_t row=0;row<plan->count;++row){const SBPlainLine *item=&plan->lines[row];const SBShapedLine *line=&item->shape;
        float baseline=command->y+(item->y+line->ascent)/density;
        float left=0,right=0,top=0,bottom=0;bool ink=false;
        for(size_t i=0;i<line->count;++i){const SBShapeGlyph *g=&line->glyphs[i];if(!g->width || !g->height)continue;
            float gx=floorf(g->x+g->left),gy=floorf(g->y-g->top);
            if(!ink){left=gx;right=gx+g->width;top=gy;bottom=gy+g->height;ink=true;}
            else{left=fminf(left,gx);right=fmaxf(right,gx+g->width);top=fminf(top,gy);bottom=fmaxf(bottom,gy+g->height);}
        }
        if(!ink || command->x+right/density<=clip.x || command->x+left/density>=clip.x+clip.w || baseline+bottom/density<=clip.y || baseline+top/density>=clip.y+clip.h)continue;
        if(right-left>8192){left=fmaxf(left,floorf((clip.x-command->x)*density)-2);right=fminf(right,ceilf((clip.x+clip.w-command->x)*density)+2);}
        if(bottom-top>8192){top=fmaxf(top,floorf((clip.y-baseline)*density)-2);bottom=fminf(bottom,ceilf((clip.y+clip.h-baseline)*density)+2);}
        if((double)left<INT_MIN || (double)right>INT_MAX || (double)top<INT_MIN || (double)bottom>INT_MAX)continue;
        for(int y=(int)top;y<(int)bottom;){int h=SDL_min(8192,(int)bottom-y);
            for(int x=(int)left;x<(int)right;){int w=SDL_min(8192,(int)right-x);SBGlyphTexture *image=glyph_texture(text,line,x,y,w,h);if(!image)break;
                nk_draw_list_add_image(list,nk_image_ptr(image->texture),nk_rect(command->x+x/density,baseline+y/density,w/density,h/density),command->foreground);x+=w;}
            y+=h;
        }
    }
    plain_free(&scratch);
}
void sb_ui_text_frame_end(SBUi *ui) {
    SBTextSystem *text=ui->text; if (!text) return;
    if(text->glyph_bytes>SB_GLYPH_BYTES)for(unsigned i=0;i<SB_GLYPH_BUCKETS;++i){SBGlyphTexture **at=&text->glyphs[i];while(*at){SBGlyphTexture *e=*at;if(e->stamp<text->frame || text->glyph_bytes>SB_GLYPH_BYTES){*at=e->next;text->glyph_bytes-=e->bytes;SDL_DestroyTexture(e->texture);free(e->signature);free(e);}else at=&e->next;}}
    ++text->frame;
}
void sb_ui_text_free(SBUi *ui) {
    sb_ui_edit_cache_clear(ui);
    sb_ui_styled_cache_clear(ui);
    if (ui->text) { system_free(ui->text); ui->text=NULL; TTF_Quit(); }
}
