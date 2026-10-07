#include "desktop.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
static unsigned checks;
#define CHECK(x) do {++checks;if(!(x)){fprintf(stderr,"GRAPH SNAPSHOT %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
static void frame(SBDesktop *d) {
    nk_input_begin(d->ui.ctx);SDL_Event event;while(SDL_PollEvent(&event))sb_desktop_event(d,&event);
    sb_desktop_tick(d,1.0f/60);nk_input_end(d->ui.ctx);sb_desktop_frame(d);sb_ui_draw(&d->ui);SDL_RenderPresent(d->ui.renderer);sb_desktop_apply(d);
}
static size_t find(const SBNotes *notes,const char *path) {size_t i=0;while(i<notes->count && strcmp(notes->items[i].path,path))++i;return i;}
static accesskit_node_id native_star(SBDesktop *d,const char *label) {
    accesskit_tree_update *tree=sb_accessibility_tree(d->accessibility);char *text=accesskit_tree_update_debug(tree);
    accesskit_node_id result=0;
    for(const char *p=text;(p=strstr(p,"(#"));++p) {
        const char *end=strstr(p,"}),");if(!end)end=p+strlen(p);
        const char *role=strstr(p,"role: ListBoxOption"),*name=strstr(p,label);
        if(role && role<end && name && name<end){result=(accesskit_node_id)strtoull(p+2,NULL,10);break;}
    }
    accesskit_string_free(text);accesskit_tree_update_free(tree);return result;
}
int main(int argc,char **argv) {
    CHECK(argc==3);char root[SB_PATH_CAP],suffix[80],path[SB_PATH_CAP];
    snprintf(suffix,sizeof(suffix),"snapshot-%lu-%lu",sb_process_id(),(unsigned long)time(NULL));OK(sb_path_join(root,sizeof(root),argv[2],suffix));
    SBDesktop d;OK(sb_desktop_init(&d,root,argv[1],true));OK(sb_app_new_project(&d.model,"one","Erstes Projekt",NULL));
    OK(sb_app_new_note(&d.model,"knowledge","a","Snapshot Original A"));strcpy(d.model.editor,"# Snapshot Original A\n\n[Nachbar](b.md)\n");OK(sb_app_save(&d.model));
    OK(sb_app_new_note(&d.model,"knowledge","b","Snapshot Original B"));frame(&d);frame(&d);
    size_t a=find(&d.graph_notes,"knowledge/a.md"),b=find(&d.graph_notes,"knowledge/b.md"),count=d.graph.count;
    CHECK(a<count && b<count && d.graph.edge_count>0);uint64_t identity=d.graph_ids[a];SBStar *stars=d.graph.stars;SBEdge *edges=d.graph.edges;SBNote *metadata=d.graph_notes.items;
    /* An external write invalidates one file while the current inventory has
       changed order and size. A retained graph must never use that inventory. */
    OK(sb_path_join(path,sizeof(path),d.model.project.root,"knowledge/b.md"));OK(sb_fs_remove(path));OK(sb_fs_write_new(path,"\xff",1));
    SBNote only=d.model.notes.items[find(&d.model.notes,"knowledge/b.md")];strcpy(only.title,"WRONG LIVE LABEL");d.model.notes.items[0]=only;d.model.notes.count=1;
    d.graph_dirty=true;frame(&d);CHECK(d.graph_stale && d.graph.count==count && d.graph.stars==stars && d.graph.edges==edges && d.graph_notes.items==metadata);
    CHECK(!strcmp(d.graph_notes.items[a].title,"Snapshot Original A") && d.graph_ids[a]==identity && strstr(d.message.message,"letzter gültiger Stand"));
    accesskit_node_id native=native_star(&d,"Snapshot Original A");CHECK(native);
    CHECK(sb_accessibility_submit(d.accessibility,native,ACCESSKIT_ACTION_CLICK,NULL,0,0));frame(&d);frame(&d);
    CHECK(!strcmp(d.model.path,"knowledge/a.md") && strstr(d.model.editor,"[Nachbar]"));
    d.star=b;strcpy(d.focus,"galaxy");SDL_Event key={0};key.type=SDL_EVENT_KEY_DOWN;key.key.key=SDLK_RETURN;key.key.down=true;
    d.message=sb_ok();sb_desktop_event(&d,&key);sb_desktop_apply(&d);CHECK(!strcmp(d.model.path,"knowledge/a.md") && d.message.code!=SB_OK);
    d.card=false;d.browser=false;d.follow_star=false;d.flight=1;
    memset(d.flight_from,0,sizeof(d.flight_from));memset(d.flight_to,0,sizeof(d.flight_to));frame(&d);
    CHECK(d.ui.space.points[b].visible);SDL_Event mouse={0};mouse.type=SDL_EVENT_MOUSE_BUTTON_DOWN;mouse.button.button=SDL_BUTTON_LEFT;
    mouse.button.windowID=SDL_GetWindowID(d.ui.window);mouse.button.x=d.ui.space.points[b].x;mouse.button.y=d.ui.space.points[b].y;
    sb_desktop_event(&d,&mouse);mouse.type=SDL_EVENT_MOUSE_BUTTON_UP;sb_desktop_event(&d,&mouse);
    CHECK(d.navigation.kind==SB_ACT_NOTE && !strcmp(d.navigation.value,"knowledge/b.md"));d.message=sb_ok();sb_desktop_apply(&d);
    CHECK(d.message.code!=SB_OK && !strcmp(d.model.path,"knowledge/a.md"));frame(&d);
    native=native_star(&d,"Snapshot Original A");CHECK(native);
    /* Recover, reorder, add a new note, then rebuild the entire snapshot. */
    OK(sb_fs_remove(path));OK(sb_fs_write_new(path,"# Recovered B\n",strlen("# Recovered B\n")));
    SBNote new_note;OK(sb_note_create(&d.model.project,"knowledge","c","New C",&new_note));sb_notes_free(&d.model.notes);OK(sb_notes_list(&d.model.project,&d.model.notes));
    SBNote swap=d.model.notes.items[0];d.model.notes.items[0]=d.model.notes.items[d.model.notes.count-1];d.model.notes.items[d.model.notes.count-1]=swap;
    d.message=d.graph_status;d.graph_dirty=true;frame(&d);CHECK(!d.graph_stale && d.graph.count==count+1 && d.message.code==SB_OK);
    a=find(&d.graph_notes,"knowledge/a.md");b=find(&d.graph_notes,"knowledge/b.md");CHECK(d.graph_ids[a]==identity && !strcmp(d.graph_notes.items[b].title,"Recovered B"));
    CHECK(d.graph.stars!=stars && d.graph_notes.items!=metadata);CHECK(native_star(&d,"Snapshot Original A")==native);
    d.star=b;strcpy(d.focus,"galaxy");sb_desktop_event(&d,&key);sb_desktop_apply(&d);CHECK(!strcmp(d.model.path,"knowledge/b.md"));frame(&d);
    /* A project boundary never exposes another project's cached map. */
    OK(sb_app_new_project(&d.model,"two","Zweites Projekt",NULL));OK(sb_path_join(path,sizeof(path),d.model.project.root,"STATE.md"));OK(sb_fs_remove(path));frame(&d);
    CHECK(d.graph_stale && d.graph.count==0 && d.graph_notes.count==0 && !d.ui.space.count);
    sb_desktop_free(&d);printf("%u graph snapshot assertions passed.\n",checks);return 0;
}
