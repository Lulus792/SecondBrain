#include "desktop.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"FIELD ACTIONS %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
static void frame(SBDesktop *d){
    nk_input_begin(d->ui.ctx);SDL_Event event;while(SDL_PollEvent(&event))sb_desktop_event(d,&event);
    sb_desktop_tick(d,1.0f/60);nk_input_end(d->ui.ctx);sb_desktop_frame(d);sb_ui_draw(&d->ui);SDL_RenderPresent(d->ui.renderer);sb_desktop_apply(d);
}
static accesskit_node_id focused_node(SBDesktop *d){
    accesskit_tree_update *tree=sb_accessibility_tree(d->accessibility);char *dump=accesskit_tree_update_debug(tree);accesskit_tree_update_free(tree);
    const char *p=dump ? strstr(dump,"focus: #") : NULL;accesskit_node_id node=p ? strtoull(p+8,NULL,10) : 0;accesskit_string_free(dump);return node;
}
static int exercise(SBDesktop *d,size_t slot,const char *id,char *source,size_t capacity){
    strcpy(d->focus,id);d->keyboard=true;frame(d);accesskit_node_id node=focused_node(d);CHECK(node>4);
    CHECK(sb_accessibility_submit(d->accessibility,node,ACCESSKIT_ACTION_SET_VALUE,"A é 👩‍👩‍👧‍👦 אב Z",0,0));frame(d);
    CHECK(!strcmp(source,"A é 👩‍👩‍👧‍👦 אב Z") && !strcmp(d->focus,id));
    CHECK(sb_accessibility_submit(d->accessibility,node,ACCESSKIT_ACTION_SET_TEXT_SELECTION,NULL,3,10));frame(d);
    CHECK(d->field_selections[slot].anchor==2 && d->field_selections[slot].caret==12);
    CHECK(d->native_fields[slot].anchor==2 && d->native_fields[slot].caret==12);
    CHECK(sb_accessibility_submit(d->accessibility,node,ACCESSKIT_ACTION_REPLACE_SELECTED_TEXT,"Ω",0,0));frame(d);
    CHECK(!strcmp(source,"A Ω אב Z") && d->field_selections[slot].caret==3 && d->field_selections[slot].anchor==3);
    /* Reversed selection preserves direction; empty replacement deletes. */
    CHECK(sb_accessibility_submit(d->accessibility,node,ACCESSKIT_ACTION_SET_TEXT_SELECTION,NULL,5,4));frame(d);
    CHECK(d->field_selections[slot].anchor==5 && d->field_selections[slot].caret==4);
    CHECK(sb_accessibility_submit(d->accessibility,node,ACCESSKIT_ACTION_REPLACE_SELECTED_TEXT,"",0,0));frame(d);CHECK(!strcmp(source,"A Ω ב Z"));
    char before[SB_PATH_CAP];strcpy(before,source);
    CHECK(sb_accessibility_submit(d->accessibility,node,ACCESSKIT_ACTION_REPLACE_SELECTED_TEXT,"bad\nline",0,0));frame(d);CHECK(!strcmp(source,before) && d->message.code==SB_INVALID);
    char *long_value=malloc(capacity+1);CHECK(long_value);memset(long_value,'x',capacity);long_value[capacity]=0;
    CHECK(sb_accessibility_submit(d->accessibility,node,ACCESSKIT_ACTION_REPLACE_SELECTED_TEXT,long_value,0,0));frame(d);CHECK(!strcmp(source,before) && d->message.code==SB_LIMIT);free(long_value);
    CHECK(!sb_accessibility_submit(d->accessibility,node,ACCESSKIT_ACTION_SET_TEXT_SELECTION,NULL,0,10000));
    /* Selection and replacement may arrive in the same native queue drain. */
    CHECK(sb_accessibility_submit(d->accessibility,node,ACCESSKIT_ACTION_SET_TEXT_SELECTION,NULL,0,1));
    CHECK(sb_accessibility_submit(d->accessibility,node,ACCESSKIT_ACTION_REPLACE_SELECTED_TEXT,"Q",0,0));frame(d);CHECK(!strcmp(source,"Q Ω ב Z"));
    /* A queued callback cannot overwrite a field whose context has closed. */
    CHECK(sb_accessibility_submit(d->accessibility,node,ACCESSKIT_ACTION_SET_VALUE,"Stale callback",0,0));d->form=SB_FORM_HELP;frame(d);CHECK(!strstr(source,"Stale callback"));
    return 0;
}
int main(int argc,char **argv){
    CHECK(argc==3);char root[SB_PATH_CAP],suffix[80];snprintf(suffix,sizeof(suffix),"field-actions-%lu",sb_process_id());OK(sb_path_join(root,sizeof(root),argv[2],suffix));
    SBDesktop d;OK(sb_desktop_init(&d,root,argv[1],true));OK(sb_app_new_project(&d.model,"actions","Actions",NULL));d.reduced_motion=true;frame(&d);
    d.form=SB_FORM_NONE;strcpy(d.focus,"search");d.focus_search=true;frame(&d);CHECK(!exercise(&d,0,"search",d.search,sizeof(d.search)));
    const char *ids[]={"form-name","form-id","form-repo"};char *sources[]={d.name,d.id,d.repository};size_t capacities[]={sizeof(d.name),sizeof(d.id),sizeof(d.repository)};
    for(size_t i=0;i<3;++i){d.form=SB_FORM_PROJECT;d.id_manual=true;d.form_focus=(int)i+1;strcpy(d.focus,ids[i]);frame(&d);CHECK(!exercise(&d,i+1,ids[i],sources[i],capacities[i]));}
    d.form=SB_FORM_WORKSPACE;d.form_focus=1;strcpy(d.focus,"form-folder");frame(&d);CHECK(!exercise(&d,4,"form-folder",d.folder,sizeof(d.folder)));
    d.form=SB_FORM_BACKUP;d.form_focus=1;strcpy(d.focus,"backup-path");frame(&d);CHECK(!exercise(&d,5,"backup-path",d.backup_path,sizeof(d.backup_path)));
    d.form=SB_FORM_RESTORE;d.restore_checked=true;strcpy(d.checked_backup,d.backup_path);d.form_focus=2;strcpy(d.focus,"restore-id");frame(&d);CHECK(!exercise(&d,6,"restore-id",d.restore_id,sizeof(d.restore_id)));
    /* A range inside an accent collapses to its leading grapheme boundary. */
    d.form=SB_FORM_PROJECT;d.id_manual=true;strcpy(d.name,"éX");strcpy(d.focus,"form-name");d.form_focus=1;frame(&d);frame(&d);accesskit_node_id node=focused_node(&d);
    CHECK(sb_accessibility_submit(d.accessibility,node,ACCESSKIT_ACTION_SET_TEXT_SELECTION,NULL,1,1));frame(&d);CHECK(d.field_selections[1].anchor==0 && d.field_selections[1].caret==0);
    CHECK(sb_accessibility_submit(d.accessibility,node,ACCESSKIT_ACTION_SET_TEXT_SELECTION,NULL,0,2));frame(&d);strcpy(d.focus,"form-id");frame(&d);
    CHECK(sb_accessibility_submit(d.accessibility,node,ACCESSKIT_ACTION_FOCUS,NULL,0,0));frame(&d);CHECK(d.native_fields[1].anchor==0 && d.native_fields[1].caret==2);
    CHECK(sb_accessibility_submit(d.accessibility,node,ACCESSKIT_ACTION_REPLACE_SELECTED_TEXT,"Q",0,0));strcpy(d.name,"External edit");frame(&d);CHECK(!strcmp(d.name,"External edit") && d.message.code==SB_CONFLICT);
    CHECK(sb_accessibility_submit(d.accessibility,node,ACCESSKIT_ACTION_SET_VALUE,"",0,0));frame(&d);
    CHECK(sb_accessibility_submit(d.accessibility,node,ACCESSKIT_ACTION_REPLACE_SELECTED_TEXT,"👩‍💻",0,0));frame(&d);CHECK(!strcmp(d.name,"👩‍💻") && d.field_selections[1].caret==3);
    CHECK(sb_accessibility_submit(d.accessibility,node,ACCESSKIT_ACTION_SET_VALUE,"💻",0,0));frame(&d);
    CHECK(sb_accessibility_submit(d.accessibility,node,ACCESSKIT_ACTION_SET_TEXT_SELECTION,NULL,0,0));frame(&d);
    CHECK(sb_accessibility_submit(d.accessibility,node,ACCESSKIT_ACTION_REPLACE_SELECTED_TEXT,"👩‍",0,0));frame(&d);CHECK(!strcmp(d.name,"👩‍💻") && d.field_selections[1].caret==3);
    char exact[SB_NAME_CAP];memset(exact,'x',sizeof(exact)-1);exact[sizeof(exact)-1]=0;
    CHECK(sb_accessibility_submit(d.accessibility,node,ACCESSKIT_ACTION_SET_VALUE,exact,0,0));frame(&d);CHECK(!strcmp(d.name,exact));
    CHECK(sb_accessibility_submit(d.accessibility,node,ACCESSKIT_ACTION_REPLACE_SELECTED_TEXT,"y",0,0));frame(&d);CHECK(!strcmp(d.name,exact) && d.message.code==SB_LIMIT);
    sb_desktop_free(&d);printf("%u production native field selection/replacement/grapheme/capacity/context assertions passed.\n",checks);return 0;
}
