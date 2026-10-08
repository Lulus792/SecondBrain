/* SecondBrain UI extension to pinned SDL_ttf 3.2.2. Own code: MIT.
   Borrows the existing HarfBuzz/FreeType state; changes no font setting or
   SDL_ttf text cache. The styled reader uses this shared geometry. */
#include "ttf_shape.h"
#if TTF_USE_HARFBUZZ
#include <hb-ot.h>
static hb_codepoint_t sb_ttf_mirror(hb_unicode_funcs_t *funcs,hb_codepoint_t cp,void *data) {
    SBTTFMirror *mirror=data;Uint32 paired=(*mirror)(cp);(void)funcs;
    return paired && paired<=0x10ffff && !(paired>=0xd800 && paired<=0xdfff) ? paired : cp;
}
#endif
void sb_ttf_shape_free(SBTTFShape *shape) {
    if(shape){SDL_free(shape->glyphs);SDL_memset(shape,0,sizeof(*shape));}
}
bool sb_ttf_shape_range(TTF_Font *font,const char *context,size_t length,
    size_t byte,size_t range,bool rtl,Uint32 script,SBTTFMirror mirror,SBTTFShape *out) {
    TTF_CHECK_FONT(font,false);
    if(!context || !out || out->glyphs || out->count || !range || length>INT_MAX ||
       byte>length || range>length-byte ||
       (((unsigned char)context[byte]&0xc0)==0x80) ||
       (byte+range<length && ((unsigned char)context[byte+range]&0xc0)==0x80))
        return SDL_SetError("Invalid contextual shaping range");
#if TTF_USE_HARFBUZZ
    hb_buffer_t *buffer=hb_buffer_create();
    if(!hb_buffer_allocation_successful(buffer)){hb_buffer_destroy(buffer);return SDL_SetError("Shaping buffer allocation failed");}
    if(mirror) {
        hb_unicode_funcs_t *unicode=hb_unicode_funcs_create(hb_unicode_funcs_get_default());
        if(unicode==hb_unicode_funcs_get_empty()){hb_buffer_destroy(buffer);return SDL_OutOfMemory();}
        hb_unicode_funcs_set_mirroring_func(unicode,sb_ttf_mirror,&mirror,NULL);
        hb_unicode_funcs_make_immutable(unicode);
        hb_buffer_set_unicode_funcs(buffer,unicode);hb_unicode_funcs_destroy(unicode);
    }
    hb_buffer_set_language(buffer,font->hb_language);
    hb_buffer_set_direction(buffer,rtl ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
    hb_buffer_set_script(buffer,hb_script_from_iso15924_tag(script));
    hb_buffer_set_cluster_level(buffer,HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
    hb_buffer_add_utf8(buffer,context,(int)length,(unsigned int)byte,(int)range);
    hb_buffer_guess_segment_properties(buffer);
    hb_feature_t kern={HB_TAG('k','e','r','n'),font->enable_kerning,HB_FEATURE_GLOBAL_START,HB_FEATURE_GLOBAL_END};
    if(!hb_shape_full(font->hb_font,buffer,&kern,1,NULL) || !hb_buffer_allocation_successful(buffer)){
        hb_buffer_destroy(buffer);return SDL_SetError("Contextual shaping failed");
    }
    unsigned count=0;
    const hb_glyph_info_t *info=hb_buffer_get_glyph_infos(buffer,&count);
    const hb_glyph_position_t *positions=hb_buffer_get_glyph_positions(buffer,NULL);
    SBTTFGlyph *glyphs=count ? SDL_calloc(count,sizeof(*glyphs)) : NULL;
    if(count && !glyphs){hb_buffer_destroy(buffer);return SDL_OutOfMemory();}
    Sint64 x=0,y=0;
    int bold=TTF_HANDLE_STYLE_BOLD(font) ? F26Dot6(font->glyph_overhang) : 0;
    for(unsigned i=0;i<count;++i) {
        c_glyph *metrics=NULL;
        if(info[i].cluster<byte || info[i].cluster>=byte+range ||
           !Find_GlyphByIndex(font,info[i].codepoint,0,0,0,0,0,0,&metrics,NULL)) {
            SDL_free(glyphs);hb_buffer_destroy(buffer);return SDL_SetError("Invalid shaped glyph or cluster");
        }
        glyphs[i]=(SBTTFGlyph){info[i].codepoint,info[i].cluster,
            (float)((double)x+positions[i].x_offset)/64.0f,
            -(float)((double)y+positions[i].y_offset)/64.0f,
            (float)((double)positions[i].x_advance+bold)/64.0f,
            metrics->sz_left,metrics->sz_top,metrics->sz_width,metrics->sz_rows};
        x+=(Sint64)positions[i].x_advance+bold;y+=positions[i].y_advance;
    }
    hb_buffer_destroy(buffer);
    *out=(SBTTFShape){glyphs,count,(float)((double)x/64.0),font->ascent,-font->descent};
    return true;
#else
    (void)rtl;(void)script;(void)mirror;
    return SDL_SetError("Contextual shaping requires HarfBuzz");
#endif
}

bool sb_ttf_ligature_carets(TTF_Font *font,Uint32 glyph,bool rtl,float *positions,
    size_t capacity,size_t *count) {
    TTF_CHECK_FONT(font,false);
    if(!count || capacity>UINT_MAX || capacity>SIZE_MAX/sizeof(Sint32) || (capacity && !positions))return SDL_SetError("Invalid ligature caret output");
#if TTF_USE_HARFBUZZ
    unsigned wanted=(unsigned)capacity;
    hb_position_t *values=wanted ? SDL_malloc(wanted*sizeof(*values)) : NULL;
    if(wanted && !values)return SDL_OutOfMemory();
    *count=hb_ot_layout_get_ligature_carets(font->hb_font,rtl ? HB_DIRECTION_RTL : HB_DIRECTION_LTR,glyph,0,&wanted,values);
    for(unsigned i=0;i<wanted;++i)positions[i]=(float)values[i]/64.0f;
    SDL_free(values);return true;
#else
    (void)glyph;(void)rtl;(void)positions;(void)capacity;
    return SDL_SetError("Ligature carets require HarfBuzz");
#endif
}
