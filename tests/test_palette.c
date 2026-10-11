#include "desktop.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"PALETTE %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
void sb_test_space_fault(unsigned);
static SDL_Surface *pixels;
static bool color_equal(SDL_Color a,struct nk_color b){return a.r==b.r && a.g==b.g && a.b==b.b && b.a==255;}
static bool sample(SBDesktop *d,float x,float y,SDL_Color expected){
    int w,h;SDL_GetWindowSize(d->ui.window,&w,&h);int px=(int)(x*pixels->w/w),py=(int)(y*pixels->h/h);
    if(px<0 || py<0 || px>=pixels->w || py>=pixels->h)return false;
    unsigned char *at=(unsigned char *)pixels->pixels+py*pixels->pitch+px*4;
    return at[0]==expected.r && at[1]==expected.g && at[2]==expected.b && at[3]==255;
}
static SBTarget *target(SBDesktop *d,const char *name){for(size_t i=0;i<d->target_count;++i)if(!strcmp(d->targets[i].id,name))return &d->targets[i];return NULL;}
static bool text_color(SBDesktop *d,const char *text,SDL_Color expected){
    const struct nk_command *cmd;nk_foreach(cmd,d->ui.ctx)if(cmd->type==NK_COMMAND_TEXT){const struct nk_command_text *t=(const struct nk_command_text *)cmd;
        if(t->length==(int)strlen(text) && !memcmp(t->string,text,(size_t)t->length) && color_equal(expected,t->foreground))return true;}
    return false;
}
static void frame(SBDesktop *d,SBSystemStyle system){
    nk_input_begin(d->ui.ctx);SDL_Event e;while(SDL_PollEvent(&e))sb_desktop_event(d,&e);nk_input_end(d->ui.ctx);
    sb_desktop_tick(d,1.0f/60);sb_desktop_apply_system_style(d,system,false);sb_desktop_frame(d);
}
static bool draw(SBDesktop *d){
    sb_ui_draw(&d->ui);SDL_DestroySurface(pixels);SDL_Surface *raw=SDL_RenderReadPixels(d->ui.renderer,NULL);
    pixels=raw ? SDL_ConvertSurface(raw,SDL_PIXELFORMAT_RGBA32) : NULL;SDL_DestroySurface(raw);SDL_RenderPresent(d->ui.renderer);sb_desktop_apply(d);return pixels!=NULL;
}
static SBPalette fixture(unsigned which){
    static const unsigned char rgb[][SB_P_COUNT][3]={
        {{18,9,43},{253,244,170},{48,15,80},{244,250,255},{232,150,36},{12,8,32},{130,230,255},{157,146,163}},
        {{248,238,215},{30,15,5},{221,205,175},{30,15,5},{84,38,16},{255,248,228},{20,36,140},{110,80,58}},
        {{1,32,18},{226,255,235},{12,53,33},{230,251,170},{230,251,170},{1,32,18},{180,221,255},{111,150,119}}};
    SBPalette p={.valid=true};for(unsigned i=0;i<SB_P_COUNT;++i)p.color[i]=(SDL_Color){rgb[which][i][0],rgb[which][i][1],rgb[which][i][2],255};return p;
}
int main(int argc,char **argv){
    CHECK(argc==3 || (argc==4 && !strcmp(argv[3],"native")));SBPalette p=fixture(0);
    SBSystemStyle system={.known=SB_SYS_CONTRAST|SB_SYS_PALETTE,.contrast=true,.palette=p};SBStyleChoice request={.dark=true};
    SBStyleChoice effective=sb_style_resolve(request,system);CHECK(effective.contrast && effective.solid && !request.contrast);
    SBPalette selected=sb_style_palette(effective,system);CHECK(sb_palette_equal(&selected,&p));
    CHECK(!sb_style_palette(effective,(SBSystemStyle){0}).valid);
    SBSystemStyle inactive=system;inactive.contrast=false;CHECK(!sb_style_palette((SBStyleChoice){.contrast=true},inactive).valid);
    SBSystemStyle held=system;sb_system_style_merge(&held,(SBSystemStyle){.known=SB_SYS_MOTION,.motion=true});CHECK(held.contrast && held.palette.valid && sb_palette_equal(&held.palette,&p));
    SBPalette changed=fixture(1);sb_system_style_merge(&held,(SBSystemStyle){.known=SB_SYS_PALETTE,.palette=changed});CHECK(sb_palette_equal(&held.palette,&changed));
    sb_system_style_merge(&held,(SBSystemStyle){.known=SB_SYS_CONTRAST,.contrast=false});CHECK(!(held.known&SB_SYS_PALETTE) && !held.palette.valid);
#ifdef _WIN32
    SBPalette native=sb_system_palette_read_native();CHECK(native.valid);
    const int roles[]={COLOR_WINDOW,COLOR_WINDOWTEXT,COLOR_3DFACE,COLOR_BTNTEXT,COLOR_HIGHLIGHT,COLOR_HIGHLIGHTTEXT,COLOR_HOTLIGHT,COLOR_GRAYTEXT};
    for(unsigned i=0;i<SB_P_COUNT;++i){COLORREF expected=GetSysColor(roles[i]);CHECK(native.color[i].r==GetRValue(expected) && native.color[i].g==GetGValue(expected) && native.color[i].b==GetBValue(expected) && native.color[i].a==255);}
    puts("Actual Win32 system color roles read without changing system settings.");
#else
    CHECK(!sb_system_palette_read_native().valid);
#endif
    char root[SB_PATH_CAP],suffix[80],config[SB_PATH_CAP];snprintf(suffix,sizeof(suffix),"palette-%lu-%lu",sb_process_id(),(unsigned long)time(NULL));OK(sb_path_join(root,sizeof(root),argv[2],suffix));
    SBDesktop d;OK(sb_desktop_init(&d,root,argv[1],argc==3));printf("Renderer: %s; density %.2f\n",SDL_GetRendererName(d.ui.renderer),d.ui.density);
    OK(sb_app_new_project(&d.model,"project","Palette",NULL));OK(sb_app_new_note(&d.model,"knowledge","note","Text in Systemfarben"));
    strcpy(d.model.editor,"# Systemfarben\n\nLesbarer Inhalt und Auswahl.\n");OK(sb_app_save(&d.model));
    OK(sb_path_join(config,sizeof(config),root,"settings.conf"));OK(sb_desktop_preferences(&d,config,true));d.form=SB_FORM_NONE;d.browser=false;
    /* Different palette colors, window sizes and font sizes exercise actual
       command colors and rendered backgrounds, rather than palette constants alone. */
    const int sizes[][2]={{1000,700},{1336,840}};
    for(unsigned palette=0;palette<3;++palette)for(unsigned size=0;size<2;++size)for(unsigned scale=0;scale<2;++scale){
        p=fixture(palette);system=(SBSystemStyle){.known=SB_SYS_CONTRAST|SB_SYS_PALETTE,.contrast=true,.palette=p};
        CHECK(SDL_SetWindowSize(d.ui.window,sizes[size][0],sizes[size][1]));CHECK(SDL_SyncWindow(d.ui.window));OK(sb_ui_fonts(&d.ui,scale ? 1.5f : 1));
        SDL_Event move={0};move.type=SDL_EVENT_MOUSE_MOTION;move.motion.windowID=SDL_GetWindowID(d.ui.window);move.motion.x=1;move.motion.y=1;CHECK(SDL_PushEvent(&move));
        frame(&d,system);CHECK(d.ui.palette.valid && d.ui.contrast && d.solid && !d.requested_style.contrast);
        CHECK(color_equal(p.color[SB_P_TEXT],d.ui.ctx->style.text.color));CHECK(color_equal(p.color[SB_P_HIGHLIGHT],d.ui.ctx->style.edit.selected_normal));CHECK(color_equal(p.color[SB_P_HIGHLIGHT_TEXT],d.ui.ctx->style.edit.selected_text_normal));
        CHECK(text_color(&d,"Palette",p.color[SB_P_BUTTON_TEXT]));SBTarget *button=target(&d,"project-picker");CHECK(button);struct nk_rect b=button->bounds;
        CHECK(draw(&d));CHECK(sample(&d,0,0,p.color[SB_P_WINDOW]));CHECK(sample(&d,b.x+b.w/2,b.y+4,p.color[SB_P_BUTTON]));
        sb_test_space_fault(6);frame(&d,system);CHECK(draw(&d));CHECK(!d.ui.material_ready && sample(&d,0,0,p.color[SB_P_WINDOW]));
        sb_test_space_fault(0);frame(&d,system);CHECK(draw(&d));CHECK(d.ui.material_ready);
        move.motion.x=b.x+b.w/2;move.motion.y=b.y+b.h/2;CHECK(SDL_PushEvent(&move));frame(&d,system);CHECK(text_color(&d,"Palette",p.color[SB_P_HIGHLIGHT_TEXT]));CHECK(draw(&d));CHECK(sample(&d,b.x+b.w/2,b.y+4,p.color[SB_P_HIGHLIGHT]));
        /* Functional star cores use the same native foreground on the sky. */
        bool star=false;for(size_t i=0;i<d.ui.space.count;++i)if(d.ui.space.points[i].visible){int w,h;SDL_GetWindowSize(d.ui.window,&w,&h);
            int x=(int)(d.ui.space.points[i].x*d.ui.space.width/w),y=(int)(d.ui.space.points[i].y*d.ui.space.height/h);
            if(x>=0 && y>=0 && x<d.ui.space.width && y<d.ui.space.height){unsigned char *rgb=d.ui.space.base+((size_t)y*d.ui.space.width+x)*4;
                CHECK(rgb[0]==p.color[SB_P_TEXT].r && rgb[1]==p.color[SB_P_TEXT].g && rgb[2]==p.color[SB_P_TEXT].b);star=true;break;}}
        CHECK(star);
        /* A color-only update must invalidate cached sky/material, theme and
           even an inactive retained card waiting for a ready document. */
        d.ui.capture_pending=true;frame(&d,system);CHECK(draw(&d));CHECK(d.ui.outgoing_texture);
        system.palette=fixture((palette+1)%3);frame(&d,system);CHECK(!d.ui.outgoing_texture && !d.ui.transitioning);CHECK(draw(&d));CHECK(sample(&d,0,0,system.palette.color[SB_P_WINDOW]));
        CHECK(!d.ui.transitioning);OK(sb_desktop_store_preferences(&d));SBSettings saved;SBRevision revision;OK(sb_settings_load(config,&saved,&revision));CHECK(!saved.contrast && saved.dark==d.requested_style.dark);
        frame(&d,(SBSystemStyle){.known=SB_SYS_CONTRAST,.contrast=false});CHECK(!d.ui.palette.valid && !d.ui.contrast);CHECK(draw(&d));
    }
    char image[SB_PATH_CAP];OK(sb_path_join(image,sizeof(image),root,"palette.bmp"));frame(&d,system);CHECK(draw(&d));CHECK(SDL_SaveBMP(pixels,image));
    SDL_DestroySurface(pixels);sb_desktop_free(&d);printf("%u native-role/palette/update/selection/button/pixel/preference assertions passed. Artifacts: %s\n",checks,root);return 0;
}
