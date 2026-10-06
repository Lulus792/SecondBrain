#include "icons.h"
#include <string.h>
/* Original icons: uniform strokes on a 24-unit grid. */
typedef struct { float x1,y1,x2,y2; } Stroke;
static const Stroke close[] = {{6.0000f,6.0000f,18.0000f,18.0000f},{18.0000f,6.0000f,6.0000f,18.0000f}};
static const Stroke add[] = {{12.0000f,5.0000f,12.0000f,19.0000f},{5.0000f,12.0000f,19.0000f,12.0000f}};
static const Stroke minus[] = {{5.0000f,12.0000f,19.0000f,12.0000f}};
static const Stroke left[] = {{14.0000f,5.0000f,7.0000f,12.0000f},{7.0000f,12.0000f,14.0000f,19.0000f}};
static const Stroke right[] = {{10.0000f,5.0000f,17.0000f,12.0000f},{17.0000f,12.0000f,10.0000f,19.0000f}};
static const Stroke home[] = {{4.0000f,11.0000f,12.0000f,4.0000f},{12.0000f,4.0000f,20.0000f,11.0000f},{6.0000f,10.0000f,6.0000f,20.0000f},{6.0000f,20.0000f,18.0000f,20.0000f},{18.0000f,20.0000f,18.0000f,10.0000f},{10.0000f,20.0000f,10.0000f,14.0000f},{10.0000f,14.0000f,14.0000f,14.0000f},{14.0000f,14.0000f,14.0000f,20.0000f}};
static const Stroke expand[] = {{4.0000f,9.0000f,4.0000f,4.0000f},{4.0000f,4.0000f,9.0000f,4.0000f},{15.0000f,4.0000f,20.0000f,4.0000f},{20.0000f,4.0000f,20.0000f,9.0000f},{20.0000f,15.0000f,20.0000f,20.0000f},{20.0000f,20.0000f,15.0000f,20.0000f},{9.0000f,20.0000f,4.0000f,20.0000f},{4.0000f,20.0000f,4.0000f,15.0000f}};
static const Stroke collapse[] = {{4.0000f,9.0000f,9.0000f,9.0000f},{9.0000f,9.0000f,9.0000f,4.0000f},{15.0000f,4.0000f,15.0000f,9.0000f},{15.0000f,9.0000f,20.0000f,9.0000f},{20.0000f,15.0000f,15.0000f,15.0000f},{15.0000f,15.0000f,15.0000f,20.0000f},{9.0000f,20.0000f,9.0000f,15.0000f},{9.0000f,15.0000f,4.0000f,15.0000f}};
static const Stroke list[] = {{9.0000f,6.0000f,20.0000f,6.0000f},{9.0000f,12.0000f,20.0000f,12.0000f},{9.0000f,18.0000f,20.0000f,18.0000f},{4.0000f,6.0000f,5.0000f,6.0000f},{4.0000f,12.0000f,5.0000f,12.0000f},{4.0000f,18.0000f,5.0000f,18.0000f}};
static const Stroke note[] = {{5.0000f,3.0000f,16.0000f,3.0000f},{16.0000f,3.0000f,20.0000f,7.0000f},{20.0000f,7.0000f,20.0000f,21.0000f},{20.0000f,21.0000f,5.0000f,21.0000f},{5.0000f,21.0000f,5.0000f,3.0000f},{16.0000f,3.0000f,16.0000f,7.0000f},{16.0000f,7.0000f,20.0000f,7.0000f},{9.0000f,11.0000f,16.0000f,11.0000f},{9.0000f,15.0000f,16.0000f,15.0000f}};
static const Stroke project[] = {{3.0000f,7.0000f,10.0000f,7.0000f},{10.0000f,7.0000f,12.0000f,10.0000f},{12.0000f,10.0000f,21.0000f,10.0000f},{21.0000f,10.0000f,21.0000f,20.0000f},{21.0000f,20.0000f,3.0000f,20.0000f},{3.0000f,20.0000f,3.0000f,7.0000f},{3.0000f,5.0000f,11.0000f,5.0000f},{11.0000f,5.0000f,14.0000f,8.0000f},{14.0000f,8.0000f,21.0000f,8.0000f}};
static const Stroke edit[] = {{4.0000f,20.0000f,5.0000f,15.0000f},{5.0000f,15.0000f,16.0000f,4.0000f},{16.0000f,4.0000f,20.0000f,8.0000f},{20.0000f,8.0000f,9.0000f,19.0000f},{9.0000f,19.0000f,4.0000f,20.0000f},{13.0000f,7.0000f,17.0000f,11.0000f}};
static const Stroke save[] = {{4.0000f,4.0000f,17.0000f,4.0000f},{17.0000f,4.0000f,20.0000f,7.0000f},{20.0000f,7.0000f,20.0000f,20.0000f},{20.0000f,20.0000f,4.0000f,20.0000f},{4.0000f,20.0000f,4.0000f,4.0000f},{8.0000f,4.0000f,8.0000f,10.0000f},{8.0000f,10.0000f,16.0000f,10.0000f},{16.0000f,10.0000f,16.0000f,4.0000f},{8.0000f,20.0000f,8.0000f,15.0000f},{8.0000f,15.0000f,16.0000f,15.0000f},{16.0000f,15.0000f,16.0000f,20.0000f}};
static const Stroke archive[] = {{3.0000f,4.0000f,21.0000f,4.0000f},{21.0000f,4.0000f,21.0000f,9.0000f},{21.0000f,9.0000f,3.0000f,9.0000f},{3.0000f,9.0000f,3.0000f,4.0000f},{5.0000f,9.0000f,5.0000f,20.0000f},{5.0000f,20.0000f,19.0000f,20.0000f},{19.0000f,20.0000f,19.0000f,9.0000f},{10.0000f,13.0000f,14.0000f,13.0000f}};
static const Stroke copy[] = {{8.0000f,8.0000f,20.0000f,8.0000f},{20.0000f,8.0000f,20.0000f,20.0000f},{20.0000f,20.0000f,8.0000f,20.0000f},{8.0000f,20.0000f,8.0000f,8.0000f},{4.0000f,16.0000f,4.0000f,4.0000f},{4.0000f,4.0000f,16.0000f,4.0000f}};
static const Stroke filter[] = {{4.0000f,6.0000f,20.0000f,6.0000f},{7.0000f,12.0000f,17.0000f,12.0000f},{10.0000f,18.0000f,14.0000f,18.0000f}};
static const Stroke reload[] = {{19.0000f,8.0000f,19.0000f,3.0000f},{19.0000f,8.0000f,14.0000f,8.0000f},{5.0000f,16.0000f,5.0000f,21.0000f},{5.0000f,16.0000f,10.0000f,16.0000f},{17.6569f,17.6569f,16.0000f,18.9282f},{16.0000f,18.9282f,14.0706f,19.7274f},{14.0706f,19.7274f,12.0000f,20.0000f},{12.0000f,20.0000f,9.9294f,19.7274f},{9.9294f,19.7274f,8.0000f,18.9282f},{8.0000f,18.9282f,6.3431f,17.6569f},{6.3431f,17.6569f,5.0718f,16.0000f},{5.0718f,16.0000f,4.2726f,14.0706f},{4.2726f,14.0706f,4.0000f,12.0000f},{4.0000f,12.0000f,4.2726f,9.9294f},{4.2726f,9.9294f,5.0718f,8.0000f}};
static const Stroke eye[] = {{3.0000f,12.0000f,7.0000f,7.0000f},{7.0000f,7.0000f,17.0000f,7.0000f},{17.0000f,7.0000f,21.0000f,12.0000f},{21.0000f,12.0000f,17.0000f,17.0000f},{17.0000f,17.0000f,7.0000f,17.0000f},{7.0000f,17.0000f,3.0000f,12.0000f},{15.0000f,12.0000f,14.7716f,13.1481f},{14.7716f,13.1481f,14.1213f,14.1213f},{14.1213f,14.1213f,13.1481f,14.7716f},{13.1481f,14.7716f,12.0000f,15.0000f},{12.0000f,15.0000f,10.8519f,14.7716f},{10.8519f,14.7716f,9.8787f,14.1213f},{9.8787f,14.1213f,9.2284f,13.1481f},{9.2284f,13.1481f,9.0000f,12.0000f},{9.0000f,12.0000f,9.2284f,10.8519f},{9.2284f,10.8519f,9.8787f,9.8787f},{9.8787f,9.8787f,10.8519f,9.2284f},{10.8519f,9.2284f,12.0000f,9.0000f},{12.0000f,9.0000f,13.1481f,9.2284f},{13.1481f,9.2284f,14.1213f,9.8787f},{14.1213f,9.8787f,14.7716f,10.8519f},{14.7716f,10.8519f,15.0000f,12.0000f}};
static const Stroke actions[] = {{5.0000f,12.0000f,5.0100f,12.0000f},{12.0000f,12.0000f,12.0100f,12.0000f},{19.0000f,12.0000f,19.0100f,12.0000f}};
static const Stroke search[] = {{16.0000f,16.0000f,21.0000f,21.0000f},{17.0000f,10.0000f,16.7615f,11.8117f},{16.7615f,11.8117f,16.0622f,13.5000f},{16.0622f,13.5000f,14.9497f,14.9497f},{14.9497f,14.9497f,13.5000f,16.0622f},{13.5000f,16.0622f,11.8117f,16.7615f},{11.8117f,16.7615f,10.0000f,17.0000f},{10.0000f,17.0000f,8.1883f,16.7615f},{8.1883f,16.7615f,6.5000f,16.0622f},{6.5000f,16.0622f,5.0503f,14.9497f},{5.0503f,14.9497f,3.9378f,13.5000f},{3.9378f,13.5000f,3.2385f,11.8117f},{3.2385f,11.8117f,3.0000f,10.0000f},{3.0000f,10.0000f,3.2385f,8.1883f},{3.2385f,8.1883f,3.9378f,6.5000f},{3.9378f,6.5000f,5.0503f,5.0503f},{5.0503f,5.0503f,6.5000f,3.9378f},{6.5000f,3.9378f,8.1883f,3.2385f},{8.1883f,3.2385f,10.0000f,3.0000f},{10.0000f,3.0000f,11.8117f,3.2385f},{11.8117f,3.2385f,13.5000f,3.9378f},{13.5000f,3.9378f,14.9497f,5.0503f},{14.9497f,5.0503f,16.0622f,6.5000f},{16.0622f,6.5000f,16.7615f,8.1883f},{16.7615f,8.1883f,17.0000f,10.0000f}};
static const Stroke settings[] = {{5.0000f,6.0000f,19.0000f,6.0000f},{5.0000f,12.0000f,19.0000f,12.0000f},{5.0000f,18.0000f,19.0000f,18.0000f},{9.0000f,4.0000f,9.0000f,8.0000f},{15.0000f,10.0000f,15.0000f,14.0000f},{10.0000f,16.0000f,10.0000f,20.0000f}};
static const Stroke help[] = {{11.0000f,15.0000f,11.0000f,16.0000f},{11.0000f,20.0000f,11.0000f,20.1000f},{7.0000f,7.0000f,8.0000f,4.0000f},{8.0000f,4.0000f,14.0000f,4.0000f},{14.0000f,4.0000f,17.0000f,7.0000f},{17.0000f,7.0000f,16.0000f,11.0000f},{16.0000f,11.0000f,11.0000f,15.0000f}};
static const Stroke link[] = {{9.0000f,7.0000f,7.0000f,7.0000f},{7.0000f,7.0000f,3.0000f,11.0000f},{3.0000f,11.0000f,3.0000f,15.0000f},{3.0000f,15.0000f,6.0000f,18.0000f},{6.0000f,18.0000f,10.0000f,18.0000f},{10.0000f,18.0000f,13.0000f,15.0000f},{11.0000f,9.0000f,14.0000f,6.0000f},{14.0000f,6.0000f,18.0000f,6.0000f},{18.0000f,6.0000f,21.0000f,9.0000f},{21.0000f,9.0000f,21.0000f,13.0000f},{21.0000f,13.0000f,17.0000f,17.0000f},{17.0000f,17.0000f,15.0000f,17.0000f},{8.0000f,15.0000f,16.0000f,9.0000f}};
static const Stroke context[] = {{7.0000f,6.0000f,3.0000f,12.0000f},{3.0000f,12.0000f,7.0000f,18.0000f},{17.0000f,6.0000f,21.0000f,12.0000f},{21.0000f,12.0000f,17.0000f,18.0000f},{14.0000f,4.0000f,10.0000f,20.0000f}};
static const Stroke palette[] = {{4.0000f,4.0000f,20.0000f,4.0000f},{20.0000f,4.0000f,20.0000f,20.0000f},{20.0000f,20.0000f,4.0000f,20.0000f},{4.0000f,20.0000f,4.0000f,4.0000f},{8.0000f,8.0000f,9.0000f,8.0000f},{14.0000f,8.0000f,16.0000f,8.0000f},{8.0000f,14.0000f,9.0000f,14.0000f},{14.0000f,14.0000f,16.0000f,14.0000f}};
static const Stroke motion[] = {{3.0000f,8.0000f,9.0000f,8.0000f},{3.0000f,12.0000f,7.0000f,12.0000f},{3.0000f,16.0000f,9.0000f,16.0000f},{12.0000f,6.0000f,20.0000f,12.0000f},{20.0000f,12.0000f,12.0000f,18.0000f},{12.0000f,18.0000f,12.0000f,6.0000f}};
static const Stroke font[] = {{5.0000f,20.0000f,12.0000f,4.0000f},{12.0000f,4.0000f,19.0000f,20.0000f},{8.0000f,14.0000f,16.0000f,14.0000f}};
static const Stroke glass[] = {{7.0000f,4.0000f,17.0000f,4.0000f},{17.0000f,4.0000f,20.0000f,10.0000f},{20.0000f,10.0000f,20.0000f,18.0000f},{20.0000f,18.0000f,4.0000f,18.0000f},{4.0000f,18.0000f,4.0000f,10.0000f},{4.0000f,10.0000f,7.0000f,4.0000f},{7.0000f,10.0000f,17.0000f,10.0000f}};
void sb_icon_draw(struct nk_command_buffer *canvas, SBIcon icon, struct nk_rect b, struct nk_color color) {
    const Stroke *lines=NULL; size_t count=0;
    switch(icon) {
    case SB_ICON_CLOSE: lines=close; count=sizeof(close)/sizeof(*close); break;
    case SB_ICON_ADD: lines=add; count=sizeof(add)/sizeof(*add); break;
    case SB_ICON_MINUS: lines=minus; count=sizeof(minus)/sizeof(*minus); break;
    case SB_ICON_LEFT: lines=left; count=sizeof(left)/sizeof(*left); break;
    case SB_ICON_RIGHT: lines=right; count=sizeof(right)/sizeof(*right); break;
    case SB_ICON_HOME: lines=home; count=sizeof(home)/sizeof(*home); break;
    case SB_ICON_EXPAND: lines=expand; count=sizeof(expand)/sizeof(*expand); break;
    case SB_ICON_COLLAPSE: lines=collapse; count=sizeof(collapse)/sizeof(*collapse); break;
    case SB_ICON_LIST: lines=list; count=sizeof(list)/sizeof(*list); break;
    case SB_ICON_NOTE: lines=note; count=sizeof(note)/sizeof(*note); break;
    case SB_ICON_PROJECT: lines=project; count=sizeof(project)/sizeof(*project); break;
    case SB_ICON_EDIT: lines=edit; count=sizeof(edit)/sizeof(*edit); break;
    case SB_ICON_SAVE: lines=save; count=sizeof(save)/sizeof(*save); break;
    case SB_ICON_ARCHIVE: lines=archive; count=sizeof(archive)/sizeof(*archive); break;
    case SB_ICON_COPY: lines=copy; count=sizeof(copy)/sizeof(*copy); break;
    case SB_ICON_FILTER: lines=filter; count=sizeof(filter)/sizeof(*filter); break;
    case SB_ICON_RELOAD: lines=reload; count=sizeof(reload)/sizeof(*reload); break;
    case SB_ICON_EYE: lines=eye; count=sizeof(eye)/sizeof(*eye); break;
    case SB_ICON_ACTIONS: lines=actions; count=sizeof(actions)/sizeof(*actions); break;
    case SB_ICON_SEARCH: lines=search; count=sizeof(search)/sizeof(*search); break;
    case SB_ICON_SETTINGS: lines=settings; count=sizeof(settings)/sizeof(*settings); break;
    case SB_ICON_HELP: lines=help; count=sizeof(help)/sizeof(*help); break;
    case SB_ICON_LINK: lines=link; count=sizeof(link)/sizeof(*link); break;
    case SB_ICON_CONTEXT: lines=context; count=sizeof(context)/sizeof(*context); break;
    case SB_ICON_PALETTE: lines=palette; count=sizeof(palette)/sizeof(*palette); break;
    case SB_ICON_MOTION: lines=motion; count=sizeof(motion)/sizeof(*motion); break;
    case SB_ICON_FONT: lines=font; count=sizeof(font)/sizeof(*font); break;
    case SB_ICON_GLASS: lines=glass; count=sizeof(glass)/sizeof(*glass); break;
    default: return;
    }
    float scale=b.w/24, weight=1.7f*scale;
    for (size_t i=0;i<count;++i) {
        float x1=b.x+lines[i].x1*scale,y1=b.y+lines[i].y1*scale;
        float x2=b.x+lines[i].x2*scale,y2=b.y+lines[i].y2*scale;
        if (icon==SB_ICON_ACTIONS) nk_fill_circle(canvas,nk_rect(x1-1.2f*scale,y1-1.2f*scale,2.4f*scale,2.4f*scale),color);
        else nk_stroke_line(canvas,x1,y1,x2,y2,weight,color);
    }
}
SBIcon sb_icon_for(const char *id) {
    if (!strcmp(id,"close-card") || !strcmp(id,"cancel") || !strcmp(id,"clear-search")) return SB_ICON_CLOSE;
    if (!strcmp(id,"zoom-in") || !strcmp(id,"new-note") || !strcmp(id,"new-project") || !strcmp(id,"new-project-settings") || !strcmp(id,"new-project-detail")) return SB_ICON_ADD;
    if (!strcmp(id,"zoom-out")) return SB_ICON_MINUS;
    if (!strcmp(id,"rotate-left") || !strcmp(id,"source-back") || !strcmp(id,"page-prev") || !strcmp(id,"project-prev")) return SB_ICON_LEFT;
    if (!strcmp(id,"rotate-right") || !strcmp(id,"page-next") || !strcmp(id,"project-next")) return SB_ICON_RIGHT;
    if (!strcmp(id,"camera-home")) return SB_ICON_HOME;
    if (!strcmp(id,"expand")) return SB_ICON_EXPAND;
    if (!strcmp(id,"list")) return SB_ICON_LIST;
    if (!strcmp(id,"project-picker") || !strcmp(id,"project-settings") || !strcmp(id,"workspace-settings") || !strcmp(id,"workspace-detail")) return SB_ICON_PROJECT;
    if (!strcmp(id,"edit")) return SB_ICON_EDIT;
    if (!strcmp(id,"save") || !strcmp(id,"guard-save")) return SB_ICON_SAVE;
    if (!strcmp(id,"archive")) return SB_ICON_ARCHIVE;
    if (!strcmp(id,"save-copy") || !strcmp(id,"copy-context") || !strcmp(id,"guard-copy")) return SB_ICON_COPY;
    if (!strcmp(id,"filter") || !strcmp(id,"list-filter")) return SB_ICON_FILTER;
    if (!strcmp(id,"reload") || !strcmp(id,"restore-project")) return SB_ICON_RELOAD;
    if (!strcmp(id,"read")) return SB_ICON_EYE;
    if (!strcmp(id,"actions")) return SB_ICON_ACTIONS;
    if (!strcmp(id,"settings") || !strcmp(id,"settings-actions")) return SB_ICON_SETTINGS;
    if (!strcmp(id,"help") || !strcmp(id,"help-actions")) return SB_ICON_HELP;
    if (!strcmp(id,"context")) return SB_ICON_CONTEXT;
    if (!strcmp(id,"theme") || !strcmp(id,"system-theme") || !strcmp(id,"contrast")) return SB_ICON_PALETTE;
    if (!strcmp(id,"motion")) return SB_ICON_MOTION;
    if (!strcmp(id,"font-minus") || !strcmp(id,"font-plus")) return SB_ICON_FONT;
    if (!strcmp(id,"transparency")) return SB_ICON_GLASS;
    if (!strncmp(id,"link:",5)) return SB_ICON_LINK;
    return SB_ICON_NONE;
}
