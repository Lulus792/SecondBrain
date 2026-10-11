#include "desktop.h"
#include "text.h"
#include "platform.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"TITLE GEOMETRY %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
#define TAG(a,b,c,d) (((uint32_t)(a)<<24)|((uint32_t)(b)<<16)|((uint32_t)(c)<<8)|(uint32_t)(d))
static bool native_box(const SBNativeText *native,size_t byte,SBNativeCharBox *out){
    for(size_t i=0;i<native->count;++i){const SBNativeTextRun *r=&native->runs[i];size_t at=r->span.offset;
        for(size_t j=0;j<r->characters;++j){if(at==byte && r->geometry){*out=(SBNativeCharBox){at,r->lengths[j],r->level&1 ? r->x+r->width-r->positions[j]-r->widths[j] : r->x+r->positions[j],r->y,r->widths[j],r->height,r->level};return true;}at+=r->lengths[j];}}
    return false;
}
static int command_case(SBUi *ui,const char *source,uint32_t script){
    const struct nk_user_font *font=&ui->normal->handle;size_t length=strlen(source);
    struct nk_command_text *command=calloc(1,sizeof(*command)+length+1);CHECK(command);command->header.type=NK_COMMAND_TEXT;command->font=font;command->foreground=nk_rgb(240,240,240);command->x=31.25f;command->y=-73.5f;command->w=1000;command->h=100;command->length=(int)length;memcpy(command->string,source,length+1);
    TTF_Font *face=sb_ui_cluster_font(font,source,length,NULL);CHECK(face);SBTextParagraph *bidi=NULL;SBShapeParagraph *paragraph=NULL;SBShapedLine line={0};SBCaretPlan carets={0};SBShapeFontSpan span={0,length,face,script};
    OK(sb_bidi_paragraph_create(source,length,SB_BIDI_AUTO_LTR,&bidi));OK(sb_shape_line(bidi,0,length,&span,1,&line));OK(sb_shape_paragraph_create(bidi,&span,1,&paragraph));OK(sb_caret_plan(paragraph,&line,&carets));
    SBNativeText first={0},second={0};CHECK(sb_ui_text_command_native(ui,command,&first));
    for(size_t i=0;i<carets.cluster_count;++i){SBCaretCluster c=carets.clusters[i];SBNativeCharBox box;CHECK(native_box(&first,c.byte,&box));
        CHECK(fabsf(box.x-command->x-c.left/ui->density)<0.01f && fabsf(box.width-(c.right-c.left)/ui->density)<0.01f);
        CHECK(box.y==command->y && box.level==c.level && fabsf(box.height-fmaxf(font->height,(line.ascent+line.descent)/ui->density))<0.01f);}
    command->x+=17.75f;command->y+=112.25f;CHECK(sb_ui_text_command_native(ui,command,&second));CHECK(first.count==second.count);
    for(size_t i=0;i<first.count;++i)CHECK(fabsf(second.runs[i].x-first.runs[i].x-17.75f)<0.01f && fabsf(second.runs[i].y-first.runs[i].y-112.25f)<0.01f);
    first.runs[0].x=999;sb_native_text_free(&first);sb_native_text_free(&second);CHECK(sb_ui_text_command_native(ui,command,&first) && first.runs[0].x!=999);
    command->foreground.a=0;CHECK(!sb_ui_text_command_native(ui,command,&second) && !second.count);
    sb_native_text_free(&first);free(command);sb_caret_plan_free(&carets);sb_shape_line_free(&line);sb_shape_paragraph_free(paragraph);sb_bidi_paragraph_free(bidi);return 0;
}
static void frame(SBDesktop *d){nk_input_begin(d->ui.ctx);SDL_Event e;while(SDL_PollEvent(&e))sb_desktop_event(d,&e);sb_desktop_tick(d,1.0f/60);nk_input_end(d->ui.ctx);sb_desktop_frame(d);sb_ui_draw(&d->ui);SDL_RenderPresent(d->ui.renderer);sb_desktop_apply(d);}
static SBPassiveText *find(SBDesktop *d,const char *id){for(size_t i=0;i<d->passive_count;++i)if(!strcmp(d->passive[i].id,id))return &d->passive[i];return NULL;}
int main(int argc,char **argv){
    CHECK(argc==3);char root[SB_PATH_CAP],suffix[80];snprintf(suffix,sizeof(suffix),"title-geometry-%lu",sb_process_id());OK(sb_path_join(root,sizeof(root),argv[2],suffix));SBDesktop d;OK(sb_desktop_init(&d,root,argv[1],true));
    for(unsigned scale=0;scale<3;++scale){OK(sb_ui_fonts(&d.ui,1+0.5f*scale));CHECK(!command_case(&d.ui,"AV office é",TAG('L','a','t','n')));CHECK(!command_case(&d.ui,"אבגד",TAG('H','e','b','r')));CHECK(!command_case(&d.ui,"ببب",TAG('A','r','a','b')));CHECK(!command_case(&d.ui,"👩‍💻",TAG('Z','y','y','y')));}
    OK(sb_ui_fonts(&d.ui,1));OK(sb_app_new_project(&d.model,"titles","Titles",NULL));OK(sb_app_new_note(&d.model,"knowledge","note","Title"));
    char title[SB_NAME_CAP];memset(title,'W',sizeof(title)-1);title[sizeof(title)-1]=0;snprintf(d.model.editor,SB_TEXT_LIMIT,"# %s\n\n1. First é\n2. Second אבג\n\n- Bullet\n",title);OK(sb_app_save(&d.model));
    d.reduced_motion=true;d.card=true;d.browser=false;d.keyboard=true;strcpy(d.focus,"reader");frame(&d);frame(&d);
    SBPassiveText *heading=find(&d,"reader:block:0");CHECK(heading && heading->native.count && heading->display && !strcmp(heading->text,title));CHECK(strlen(heading->display)<strlen(title) && strstr(heading->display,"…"));
    size_t covered=0;for(size_t i=0;i<heading->native.count;++i)covered+=heading->native.runs[i].span.length;CHECK(covered==strlen(heading->display));
    bool ordered=false,bullet=false;for(size_t i=0;i<d.passive_count;++i){SBPassiveText *p=&d.passive[i];if(p->role!=ACCESSKIT_ROLE_LIST_MARKER)continue;CHECK(p->native.count && p->native.runs[0].geometry);
        if(!strcmp(p->text,"•")){bullet=true;CHECK(p->native.runs[0].width==4*d.ui.scale && p->native.runs[0].height==4*d.ui.scale);}else{ordered=true;CHECK(p->display && !strcmp(p->display,p->text));}}
    CHECK(ordered && bullet);
    strcpy(d.model.editor,"Plain title\n\nBody without a heading.\n");OK(sb_app_save(&d.model));frame(&d);heading=find(&d,"reader:title");char expected[SB_NAME_CAP+32];snprintf(expected,sizeof(expected),"Dokumenttitel: %s",d.model.title);CHECK(heading && heading->native.count && !strcmp(heading->text,expected) && !strcmp(heading->display,d.model.title));
    for(size_t i=0;i<d.passive_count;++i)if(d.passive[i].role==ACCESSKIT_ROLE_PARAGRAPH)CHECK(heading->order<d.passive[i].order);
    d.form=SB_FORM_HELP;frame(&d);heading=find(&d,"modal-title");CHECK(heading && heading->native.count && heading->display);
    char picture[SB_PATH_CAP];OK(sb_path_join(picture,sizeof(picture),root,"title-geometry.bmp"));OK(sb_ui_capture(&d.ui,picture));sb_desktop_free(&d);printf("%u native command/title/name/truncation/marker/heading assertions passed.\n",checks);return 0;
}
