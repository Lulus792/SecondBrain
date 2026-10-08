/* Font bytes are immutable and shared; every instance has private IO streams,
   FreeType/HarfBuzz fonts and one owning thread. No renderer is shared. */
#include "ttf_shape.h"
typedef struct {float size;TTF_FontStyleFlags style;TTF_HintingFlags hinting;int outline;bool kerning,sdf;Uint32 generation;char *language;} SBFontSettings;
struct SBFontSnapshot {SBFontAssets *assets;uint64_t epoch;unsigned face;SBFontSettings settings[8];};
struct SBFontInstance {SBTextFace face;SDL_ThreadID owner;SBFontAssets *assets;};
SBFontSnapshot *sb_ui_font_snapshot(const struct nk_user_font *font){
    SBUi *ui=sb_ui_font_owner(font);if(!ui || !ui->text || ui->text->owner!=SDL_GetCurrentThreadID())return NULL;SBTextFace *face=font->userdata.ptr;
    SBFontSnapshot *s=calloc(1,sizeof(*s));if(!s)return NULL;
    s->assets=ui->text->assets;SDL_AtomicIncRef(&s->assets->refs);s->epoch=ui->font_epoch;s->face=(unsigned)(face-ui->text->faces);
    for(unsigned i=0;i<8;++i){TTF_Font *f=i ? face->fallback[i-1] : face->font;
        s->settings[i]=(SBFontSettings){TTF_GetFontSize(f),TTF_GetFontStyle(f),TTF_GetFontHinting(f),TTF_GetFontOutline(f),TTF_GetFontKerning(f),TTF_GetFontSDF(f),TTF_GetFontGeneration(f),NULL};
        const char *language=sb_ttf_font_language(f);if(language){size_t n=strlen(language)+1;s->settings[i].language=malloc(n);if(!s->settings[i].language){sb_font_snapshot_free(s);return NULL;}memcpy(s->settings[i].language,language,n);}}
    return s;
}
void sb_font_snapshot_free(SBFontSnapshot *s){if(s){for(unsigned i=0;i<8;++i)free(s->settings[i].language);font_assets_free(s->assets);free(s);}}
void sb_font_instance_free(SBFontInstance *p){if(!p)return;SDL_assert(p->owner==SDL_GetCurrentThreadID());TTF_CloseFont(p->face.font);for(unsigned i=0;i<SB_FALLBACK_COUNT;++i)TTF_CloseFont(p->face.fallback[i]);font_assets_free(p->assets);free(p);TTF_Quit();}
SBStatus sb_font_snapshot_open(const SBFontSnapshot *s,SBFontInstance **out){
    if(!s || !out)return sb_error(SB_INVALID,"Schriftkopie fehlt.");*out=NULL;
    if(!TTF_Init())return sb_error(SB_IO,"Schriftinitialisierung fehlgeschlagen: %s",SDL_GetError());
    SBFontInstance *p=calloc(1,sizeof(*p));if(!p){TTF_Quit();return sb_error(SB_MEMORY,"Kein Speicher für Schriftkopie.");}p->owner=SDL_GetCurrentThreadID();p->assets=s->assets;SDL_AtomicIncRef(&p->assets->refs);
    for(unsigned i=0;i<8;++i){const SBFontSettings *c=&s->settings[i];unsigned asset=i ? i+1 : (s->face/8==3 || (s->face&SB_TEXT_CODE) ? 1 : 0);
        TTF_Font *f=font_asset_open(&s->assets->files[asset],c->size);if(!i)p->face.font=f;else p->face.fallback[i-1]=f;
        if(!f)goto failed;TTF_SetFontStyle(f,c->style);TTF_SetFontHinting(f,c->hinting);TTF_SetFontKerning(f,c->kerning);
        if(!TTF_SetFontOutline(f,c->outline) || !TTF_SetFontSDF(f,c->sdf) || !TTF_SetFontLanguage(f,c->language))goto failed;
    }
    *out=p;return sb_ok();
failed:{SBStatus status=sb_error(SB_IO,"Schriftkopie konnte nicht geladen werden: %s",SDL_GetError());sb_font_instance_free(p);return status;}
}
TTF_Font *sb_font_instance_select(SBFontInstance *p,const char *text,size_t length,TTF_Font *previous){
    if(!p || !text || p->owner!=SDL_GetCurrentThreadID())return NULL;return cluster_font(&p->face,text,length,previous);
}
unsigned sb_font_instance_token(const SBFontInstance *p,const TTF_Font *font){if(p && font){if(p->face.font==font)return 0;for(unsigned i=0;i<SB_FALLBACK_COUNT;++i)if(p->face.fallback[i]==font)return i+1;}return UINT_MAX;}
TTF_Font *sb_font_snapshot_bind(const SBFontSnapshot *s,SBUi *ui,unsigned token){
    if(!s || !ui || !ui->text || ui->text->owner!=SDL_GetCurrentThreadID() || token>=8 || s->epoch!=ui->font_epoch || s->assets!=ui->text->assets)return NULL;
    SBTextFace *face=&ui->text->faces[s->face];TTF_Font *f=token ? face->fallback[token-1] : face->font;
    return f && TTF_GetFontGeneration(f)==s->settings[token].generation ? f : NULL;
}
