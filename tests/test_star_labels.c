#include "desktop.h"
#include "platform.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"STAR LABEL %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
static void layout(SBDesktop *d){
    nk_input_begin(d->ui.ctx);SDL_Event e;while(SDL_PollEvent(&e)){}nk_input_end(d->ui.ctx);
    d->keyboard=true;strcpy(d->focus,"galaxy");sb_desktop_frame(d);
}
static void finish(SBDesktop *d){sb_ui_draw(&d->ui);SDL_RenderPresent(d->ui.renderer);}
int main(int argc,char **argv){
    CHECK(argc==3);char root[SB_PATH_CAP],suffix[80];snprintf(suffix,sizeof(suffix),"star-labels-%lu",sb_process_id());OK(sb_path_join(root,sizeof(root),argv[2],suffix));
    SBDesktop d;OK(sb_desktop_init(&d,root,argv[1],true));OK(sb_app_new_project(&d.model,"review","Sternbeschriftung",NULL));
    OK(sb_app_new_note(&d.model,"knowledge","label","MMMMMMMMMMMMMMMMMMMMMMMMMMMM"));d.card=false;d.browser=false;d.reduced_motion=true;layout(&d);finish(&d);
    size_t star=0;while(star<d.graph.count && strcmp(d.graph_notes.items[star].path,d.model.path))++star;CHECK(star<d.graph.count);
    for(size_t i=0;i<d.graph.count;++i){d.graph.stars[i].x=100000;d.graph.stars[i].y=d.graph.stars[i].z=0;}
    d.graph.stars[star].x=0;d.star=star;d.follow_star=false;d.flight=1;d.focus_x=d.focus_y=d.focus_z=0;d.view_yaw=d.view_pitch=0;
    for(unsigned scale=0;scale<3;++scale)for(unsigned side=0;side<2;++side)for(unsigned contrast=0;contrast<2;++contrast){
        OK(sb_ui_fonts(&d.ui,1+scale*0.5f));d.ui.contrast=contrast;sb_ui_theme(&d.ui,true);d.card=side;d.view_pan_x=0;layout(&d);finish(&d);
        if(side)d.view_pan_x=d.ui.card_bounds.x-30-d.ui.space.points[star].x;
        layout(&d);SBPoint p=d.ui.space.points[star];CHECK(p.visible && p.focused && p.selected);
        unsigned count=0;struct nk_rect label={0};const struct nk_command *command;
        nk_foreach(command,d.ui.ctx)if(command->type==NK_COMMAND_TEXT){const struct nk_command_text *t=(const struct nk_command_text *)command;
            if(t->length>=4 && !memcmp(t->string,"MMMM",4) && fabsf(t->y-p.y)<60){label=nk_rect(t->x,t->y,t->w,t->h);++count;}
        }
        CHECK(count>0);float gap=side ? p.x-(label.x+label.w) : label.x-p.x;
        CHECK(label.h>=d.ui.normal->handle.height-1 && label.h<24*d.ui.scale);
        finish(&d);char picture[SB_PATH_CAP],name[80];snprintf(name,sizeof(name),"label-%u-%s-%u.bmp",100+scale*50,side ? "left" : "right",contrast);OK(sb_path_join(picture,sizeof(picture),root,name));OK(sb_ui_capture(&d.ui,picture));
        if(gap<18 || fabsf(label.y+label.h/2-p.y)>1.5f)fprintf(stderr,"scale=%.1f side=%u gap=%.2f center=%.2f picture=%s\n",d.ui.scale,side,gap,label.y+label.h/2-p.y,picture);
        CHECK(gap>=18);CHECK(fabsf(label.y+label.h/2-p.y)<=1.5f);
        if(side)CHECK(label.x+label.w<d.ui.card_bounds.x);
    }
    sb_desktop_free(&d);printf("%u star-label spacing/centering/card assertions passed.\n",checks);return 0;
}
