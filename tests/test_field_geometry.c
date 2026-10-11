#include "desktop.h"
#include "platform.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"FIELD GEOMETRY %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
static void frame_event(SBDesktop *d,const SDL_Event *event){
    nk_input_begin(d->ui.ctx);SDL_Event e;while(SDL_PollEvent(&e))sb_desktop_event(d,&e);nk_input_end(d->ui.ctx);
    if(event){nk_input_begin(d->ui.ctx);sb_desktop_event(d,event);nk_input_end(d->ui.ctx);}
    sb_desktop_frame(d);sb_ui_draw(&d->ui);SDL_RenderPresent(d->ui.renderer);
}
static void frame(SBDesktop *d){frame_event(d,NULL);}
static const SBTarget *target(SBDesktop *d,const char *id){
    for(size_t i=0;i<d->target_count;++i)if(!strcmp(d->targets[i].id,id))return &d->targets[i];return NULL;
}
static int geometry(SBDesktop *d,size_t index,const char *id,const char *source){
    SBFieldText *field=&d->native_fields[index];const SBTarget *t=target(d,id);
    CHECK(t && field->source && !strcmp(field->source,source) && field->text.count);
    size_t bytes=0;float width=0,y=field->text.runs[0].y;bool rtl=false;
    for(size_t i=0;i<field->text.count;++i){SBNativeTextRun *r=&field->text.runs[i];
        CHECK(r->geometry && r->span.offset==bytes && r->height>0 && fabsf(r->y-y)<0.01f);
        CHECK(r->y>=t->bounds.y-1 && r->y+r->height<=t->bounds.y+t->bounds.h+1);
        CHECK(isfinite(r->x) && r->width>=0);bytes+=r->span.length;width+=r->width;rtl|=(r->level&1)!=0;
    }
    CHECK(bytes==strlen(source));
    float measured=d->ui.normal->handle.width(d->ui.normal->handle.userdata,d->ui.normal->handle.height,source,(int)strlen(source));
    CHECK(fabsf(width-measured)<0.1f);
    if(strstr(source,"אב"))CHECK(rtl);
    return 0;
}
int main(int argc,char **argv){
    CHECK(argc==3);char root[SB_PATH_CAP],suffix[80];snprintf(suffix,sizeof(suffix),"field-geometry-%lu",sb_process_id());OK(sb_path_join(root,sizeof(root),argv[2],suffix));
    SBDesktop d;OK(sb_desktop_init(&d,root,argv[1],true));OK(sb_app_new_project(&d.model,"fields","Fields",NULL));
    d.keyboard=true;d.reduced_motion=true;d.browser=true;strcpy(d.focus,"galaxy");frame(&d);
    const char *samples[]={"AV ffi e\xcc\x81", "אבג 123 مرحبا", "👩‍👩‍👧‍👦 x", ""};
    for(unsigned scale=0;scale<3;++scale){OK(sb_ui_fonts(&d.ui,1+0.5f*scale));
        for(size_t sample=0;sample<4;++sample){
            strcpy(d.search,samples[sample]);strcpy(d.focus,"search");d.focus_search=true;frame(&d);CHECK(!geometry(&d,0,"search",d.search));
            d.form=SB_FORM_PROJECT;d.form_focus=1;d.id_manual=true;strcpy(d.name,samples[sample]);strcpy(d.id,"separate-id");strcpy(d.repository,"/different/location/ffi");strcpy(d.focus,"form-name");frame(&d);
            CHECK(!geometry(&d,1,"form-name",d.name));CHECK(!geometry(&d,2,"form-id",d.id));CHECK(!geometry(&d,3,"form-repo",d.repository));
            /* The shared transient edit now contains the final field. Earlier
               fields must retain their own source, positions and direction. */
            CHECK(strcmp(d.native_fields[1].source,d.native_fields[3].source));
            accesskit_tree_update *tree=sb_accessibility_tree(d.accessibility);char *dump=accesskit_tree_update_debug(tree);accesskit_tree_update_free(tree);
            CHECK(dump && strstr(dump,"character_positions:") && strstr(dump,"character_widths:"));accesskit_string_free(dump);
            d.form=SB_FORM_NONE;strcpy(d.focus,"search");frame(&d);CHECK(!d.native_fields[1].source && !d.native_fields[2].text.count && !d.native_fields[3].source);
        }
    }
    OK(sb_ui_fonts(&d.ui,1));
    d.form=SB_FORM_WORKSPACE;d.form_focus=1;strcpy(d.folder,"/Users/Example/אבג");strcpy(d.focus,"form-folder");frame(&d);CHECK(!geometry(&d,4,"form-folder",d.folder));
    d.form=SB_FORM_BACKUP;d.form_focus=1;strcpy(d.backup_path,"/tmp/archive.sbrain");strcpy(d.focus,"backup-path");frame(&d);CHECK(!geometry(&d,5,"backup-path",d.backup_path));
    d.form=SB_FORM_RESTORE;d.form_focus=2;d.restore_checked=true;strcpy(d.checked_backup,d.backup_path);strcpy(d.restore_id,"restored");strcpy(d.focus,"restore-id");frame(&d);CHECK(!geometry(&d,6,"restore-id",d.restore_id));CHECK(!geometry(&d,5,"backup-path",d.backup_path));
    /* A long active source scrolls horizontally without moving its native row. */
    d.form=SB_FORM_PROJECT;d.form_focus=3;d.id_manual=true;memset(d.repository,'W',600);d.repository[600]=0;strcpy(d.focus,"form-repo");frame(&d);CHECK(!geometry(&d,3,"form-repo",d.repository));
    strcpy(d.focus,"form-repo");frame(&d);
    SDL_Event end={0};end.type=SDL_EVENT_KEY_DOWN;end.key.windowID=SDL_GetWindowID(d.ui.window);end.key.key=SDLK_END;end.key.down=true;CHECK(SDL_PushEvent(&end));frame(&d);CHECK(!geometry(&d,3,"form-repo",d.repository));
    const SBTarget *t=target(&d,"form-repo");if(t && d.native_fields[3].text.runs[0].x>=t->bounds.x)fprintf(stderr,"Long field: focus=%s cursor=%d active=%u scroll=%.2f source_caret=%zu x=%.2f left=%.2f width=%.2f\n",d.focus,d.ui.ctx->text_edit.cursor,d.ui.ctx->text_edit.active,d.ui.ctx->text_edit.scrollbar.x,d.native_fields[3].caret,d.native_fields[3].text.runs[0].x,t->bounds.x,d.native_fields[3].text.runs[0].width);CHECK(t && d.native_fields[3].text.runs[0].x<t->bounds.x);
    end.type=SDL_EVENT_KEY_UP;end.key.down=false;frame_event(&d,&end);
    SDL_Event preedit={0};preedit.type=SDL_EVENT_TEXT_EDITING;preedit.edit.windowID=SDL_GetWindowID(d.ui.window);preedit.edit.text="漢字";preedit.edit.start=0;preedit.edit.length=2;
    frame_event(&d,&preedit);CHECK(sb_ui_composition_active(&d.ui) && !d.native_fields[3].text.count);CHECK(strlen(d.repository)==600);CHECK(d.native_fields[1].text.count && d.native_fields[2].text.count);
    sb_ui_composition_cancel(&d.ui);frame(&d);CHECK(!geometry(&d,3,"form-repo",d.repository));
    d.form=SB_FORM_NONE;frame(&d);OK(sb_app_new_note(&d.model,"knowledge","editor","Editor"));d.next_edit=true;d.card=true;d.focus_editor=true;strcpy(d.focus,"editor");frame(&d);frame(&d);
    CHECK(d.native_editor.count && d.ui.edit_native_cache);SBEditNativeCache *memo=d.ui.edit_native_cache;frame(&d);CHECK(d.ui.edit_native_cache==memo && d.native_fields[0].text.count);
    char picture[SB_PATH_CAP];OK(sb_path_join(picture,sizeof(picture),root,"fields.bmp"));OK(sb_ui_capture(&d.ui,picture));
    sb_desktop_free(&d);printf("%u production search/form/source/geometry/scale/provider assertions passed.\n",checks);return 0;
}
