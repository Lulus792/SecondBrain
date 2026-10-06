from pathlib import Path
import math
# Original SecondBrain line icons on a 24-unit grid. Generated C is shipped, no Python at runtime.
icons={
'close':[(6,6,18,18),(18,6,6,18)],
'add':[(12,5,12,19),(5,12,19,12)],
'minus':[(5,12,19,12)],
'left':[(14,5,7,12),(7,12,14,19)],
'right':[(10,5,17,12),(17,12,10,19)],
'home':[(4,11,12,4),(12,4,20,11),(6,10,6,20),(6,20,18,20),(18,20,18,10),(10,20,10,14),(10,14,14,14),(14,14,14,20)],
'expand':[(4,9,4,4),(4,4,9,4),(15,4,20,4),(20,4,20,9),(20,15,20,20),(20,20,15,20),(9,20,4,20),(4,20,4,15)],
'collapse':[(4,9,9,9),(9,9,9,4),(15,4,15,9),(15,9,20,9),(20,15,15,15),(15,15,15,20),(9,20,9,15),(9,15,4,15)],
'list':[(9,6,20,6),(9,12,20,12),(9,18,20,18),(4,6,5,6),(4,12,5,12),(4,18,5,18)],
'note':[(5,3,16,3),(16,3,20,7),(20,7,20,21),(20,21,5,21),(5,21,5,3),(16,3,16,7),(16,7,20,7),(9,11,16,11),(9,15,16,15)],
'project':[(3,7,10,7),(10,7,12,10),(12,10,21,10),(21,10,21,20),(21,20,3,20),(3,20,3,7),(3,5,11,5),(11,5,14,8),(14,8,21,8)],
'edit':[(4,20,5,15),(5,15,16,4),(16,4,20,8),(20,8,9,19),(9,19,4,20),(13,7,17,11)],
'save':[(4,4,17,4),(17,4,20,7),(20,7,20,20),(20,20,4,20),(4,20,4,4),(8,4,8,10),(8,10,16,10),(16,10,16,4),(8,20,8,15),(8,15,16,15),(16,15,16,20)],
'archive':[(3,4,21,4),(21,4,21,9),(21,9,3,9),(3,9,3,4),(5,9,5,20),(5,20,19,20),(19,20,19,9),(10,13,14,13)],
'copy':[(8,8,20,8),(20,8,20,20),(20,20,8,20),(8,20,8,8),(4,16,4,4),(4,4,16,4)],
'filter':[(4,6,20,6),(7,12,17,12),(10,18,14,18)],
'reload':[(19,8,19,3),(19,8,14,8),(5,16,5,21),(5,16,10,16)]+[(12+8*math.cos(a),12+8*math.sin(a),12+8*math.cos(b),12+8*math.sin(b)) for a,b in [(x*math.pi/12,(x+1)*math.pi/12) for x in range(3,14)]],
'eye':[(3,12,7,7),(7,7,17,7),(17,7,21,12),(21,12,17,17),(17,17,7,17),(7,17,3,12)]+[(12+3*math.cos(a),12+3*math.sin(a),12+3*math.cos(b),12+3*math.sin(b)) for a,b in [(x*math.pi/8,(x+1)*math.pi/8) for x in range(16)]],
'actions':[(5,12,5.01,12),(12,12,12.01,12),(19,12,19.01,12)],
'search':[(16,16,21,21)]+[(10+7*math.cos(a),10+7*math.sin(a),10+7*math.cos(b),10+7*math.sin(b)) for a,b in [(x*math.pi/12,(x+1)*math.pi/12) for x in range(24)]],
'settings':[(5,6,19,6),(5,12,19,12),(5,18,19,18),(9,4,9,8),(15,10,15,14),(10,16,10,20)],
'help':[(11,15,11,16),(11,20,11,20.1),(7,7,8,4),(8,4,14,4),(14,4,17,7),(17,7,16,11),(16,11,11,15)],
'link':[(9,7,7,7),(7,7,3,11),(3,11,3,15),(3,15,6,18),(6,18,10,18),(10,18,13,15),(11,9,14,6),(14,6,18,6),(18,6,21,9),(21,9,21,13),(21,13,17,17),(17,17,15,17),(8,15,16,9)],
'context':[(7,6,3,12),(3,12,7,18),(17,6,21,12),(21,12,17,18),(14,4,10,20)],
'palette':[(4,4,20,4),(20,4,20,20),(20,20,4,20),(4,20,4,4),(8,8,9,8),(14,8,16,8),(8,14,9,14),(14,14,16,14)],
'motion':[(3,8,9,8),(3,12,7,12),(3,16,9,16),(12,6,20,12),(20,12,12,18),(12,18,12,6)],
'font':[(5,20,12,4),(12,4,19,20),(8,14,16,14)],
'glass':[(7,4,17,4),(17,4,20,10),(20,10,20,18),(20,18,4,18),(4,18,4,10),(4,10,7,4),(7,10,17,10)],
}
root=Path('assets/icons');root.mkdir(exist_ok=True)
h='''#ifndef SB_ICONS_H\n#define SB_ICONS_H\n#include "ui.h"\ntypedef enum { SB_ICON_NONE, '''+', '.join('SB_ICON_'+n.upper() for n in icons)+''', SB_ICON_COUNT } SBIcon;\nSBIcon sb_icon_for(const char *id);\nvoid sb_icon_draw(struct nk_command_buffer *canvas, SBIcon icon, struct nk_rect bounds, struct nk_color color);\n#endif\n'''
Path('app/icons.h').write_text(h)
c='#include "icons.h"\n#include <string.h>\n/* Original icons: uniform strokes on a 24-unit grid. */\ntypedef struct { float x1,y1,x2,y2; } Stroke;\n'
for n,lines in icons.items():
 c+='static const Stroke '+n+'[] = {'+','.join('{'+','.join(f'{v:.4f}f' for v in row)+'}' for row in lines)+'};\n'
 svg='<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.7" stroke-linecap="square" stroke-linejoin="round">'+''.join(f'<path d="M{x1:.4f} {y1:.4f}L{x2:.4f} {y2:.4f}"/>' for x1,y1,x2,y2 in lines)+'</svg>\n'
 (root/(n+'.svg')).write_text(svg)
c+='''void sb_icon_draw(struct nk_command_buffer *canvas, SBIcon icon, struct nk_rect b, struct nk_color color) {
    const Stroke *lines=NULL; size_t count=0;
    switch(icon) {\n'''
for n in icons:c+=f'    case SB_ICON_{n.upper()}: lines={n}; count=sizeof({n})/sizeof(*{n}); break;\n'
c+='''    default: return;
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
'''
mapping={'close':['close-card','cancel','clear-search'],'add':['zoom-in','new-note','new-project','new-project-settings','new-project-detail'],'minus':['zoom-out'],'left':['rotate-left','source-back','page-prev','project-prev'],'right':['rotate-right','page-next','project-next'],'home':['camera-home'],'expand':['expand'],'list':['list'],'project':['project-picker','project-settings','workspace-settings','workspace-detail'],'edit':['edit'],'save':['save','guard-save'],'archive':['archive'],'copy':['save-copy','copy-context','guard-copy'],'filter':['filter','list-filter'],'reload':['reload'],'eye':['read'],'actions':['actions'],'settings':['settings','settings-actions'],'help':['help','help-actions'],'context':['context'],'palette':['theme'],'motion':['motion'],'font':['font-minus','font-plus'],'glass':['transparency']}
for n,ids in mapping.items():c+='    if ('+' || '.join(f'!strcmp(id,"{x}")' for x in ids)+f') return SB_ICON_{n.upper()};\n'
c+='    if (!strncmp(id,"link:",5)) return SB_ICON_LINK;\n    return SB_ICON_NONE;\n}\n';Path('app/icons.c').write_text(c)
