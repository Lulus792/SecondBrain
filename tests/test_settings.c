#include "settings.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
int main(int argc,char **argv) {
    unsigned checks=0; SBSettings s,loaded; SBRevision revision={0},saved={0};
    char root[SB_PATH_CAP],path[SB_PATH_CAP],other[SB_PATH_CAP],suffix[80],*text=NULL; size_t length;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"SETTINGS FAIL %d: %s\n",__LINE__,#x); return 1; } } while (0)
#define OK(x) CHECK((x).code==SB_OK)
    CHECK(argc==2);
    snprintf(suffix,sizeof(suffix),"run-%lu-%lu",sb_process_id(),(unsigned long)time(NULL));
    OK(sb_path_join(root,sizeof(root),argv[1],suffix)); OK(sb_fs_mkdirs(root));
    OK(sb_path_join(path,sizeof(path),root,"settings ü.conf"));
    OK(sb_settings_load(path,&s,&revision));
    CHECK(!revision.exists && s.font_percent==100 && s.dark && s.width==1336);
    strcpy(s.workspace,"/Pfad ü/100%=Projekt\nweitere Zeile"); strcpy(s.project,"projekt-ue"); strcpy(s.note,"knowledge/überblick.md");
    s.font_percent=200; s.dark=false; s.solid=true; s.reduced_motion=true; s.width=780; s.height=520;
    OK(sb_settings_save(path,&s,revision,&saved)); OK(sb_settings_load(path,&loaded,&revision));
    CHECK(!strcmp(s.workspace,loaded.workspace) && !strcmp(s.note,loaded.note) && !strcmp(s.project,loaded.project));
    CHECK(loaded.font_percent==200 && !loaded.dark && loaded.solid && loaded.reduced_motion && loaded.width==780);
    CHECK(revision.hash==saved.hash && revision.length==saved.length);
    SBRevision stale=revision;
    s.font_percent=150; OK(sb_settings_save(path,&s,revision,&saved));
    CHECK(sb_settings_save(path,&loaded,stale,NULL).code==SB_CONFLICT);
    OK(sb_settings_load(path,&loaded,&revision)); CHECK(loaded.font_percent==150);
    s.font_percent=99; CHECK(sb_settings_save(path,&s,revision,NULL).code==SB_INVALID);
    OK(sb_fs_read(path,&text,&length)); CHECK(length<32768 && !strstr(text,"Pfad ü"));
    char *changed=malloc(length+64); CHECK(changed!=NULL); memcpy(changed,text,length); strcpy(changed+length,"font=100\n");
    OK(sb_path_join(other,sizeof(other),root,"change.tmp")); OK(sb_fs_write_new(other,changed,length+9)); OK(sb_fs_replace(other,path));
    SBSettings unchanged=loaded;
    CHECK(sb_settings_load(path,&loaded,&revision).code==SB_INVALID);
    CHECK(!memcmp(&loaded,&unchanged,sizeof(loaded)));
    s.font_percent=100; CHECK(sb_settings_save(path,&s,revision,NULL).code==SB_INVALID);
    char *preserved=NULL; size_t preserved_length;
    OK(sb_fs_read(path,&preserved,&preserved_length)); CHECK(preserved_length==length+9 && !memcmp(preserved,changed,preserved_length));
    free(preserved); free(changed); free(text);
    OK(sb_fs_remove(path));
    const char *bad="SecondBrain settings 2\n";
    OK(sb_fs_write_new(path,bad,strlen(bad))); CHECK(sb_settings_load(path,&loaded,&revision).code==SB_INVALID);
    OK(sb_fs_remove(path));
    bad="SecondBrain settings 1\nworkspace=%00\n";
    OK(sb_fs_write_new(path,bad,strlen(bad))); CHECK(sb_settings_load(path,&loaded,&revision).code==SB_INVALID);
    printf("%u settings assertions passed: persistence, UTF-8, malformed input, conflicts and preservation.\n",checks);
    return 0;
}
