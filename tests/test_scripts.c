#include "scripts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned long checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"SCRIPTS %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define TAG(a,b,c,d) (((uint32_t)(a)<<24)|((uint32_t)(b)<<16)|((uint32_t)(c)<<8)|(uint32_t)(d))
static uint32_t tag(const char *s){return TAG(s[0],s[1],s[2],s[3]);}
static FILE *open_data(const char *root,const char *name){char path[4096];snprintf(path,sizeof(path),"%s/%s",root,name);return fopen(path,"rb");}
static int expected_plan(const char *text,const uint32_t *scripts,size_t count) {
    SBScriptPlan p={0};CHECK(sb_script_plan(text,strlen(text),&p).code==SB_OK);CHECK(p.count==count);size_t covered=0;
    for(size_t i=0;i<count;++i){CHECK(p.spans[i].byte==covered && p.spans[i].length && p.spans[i].script==scripts[i]);covered+=p.spans[i].length;}
    CHECK(covered==strlen(text));sb_script_plan_free(&p);return 0;
}
int main(int argc,char **argv) {
    CHECK(argc==2);
    const uint32_t latin=TAG('L','a','t','n'),greek=TAG('G','r','e','k'),hira=TAG('H','i','r','a'),kana=TAG('K','a','n','a');
    CHECK(!expected_plan("gamma (γ) is",(uint32_t[]){latin,greek,latin},3));
    CHECK(!expected_plan("aー",(uint32_t[]){latin,hira},2));
    CHECK(!expected_plan("カーテン",(uint32_t[]){kana},1));
    CHECK(!expected_plan("*é*",(uint32_t[]){latin},1));
    CHECK(!expected_plan("Á",(uint32_t[]){latin},1));
    CHECK(!expected_plan("abc〈γ〉xyz",(uint32_t[]){latin,greek,latin},3));
    SBScriptPlan bad={0};CHECK(sb_script_plan("\xc3\x28",2,&bad).code==SB_INVALID && !bad.spans);
    char *stress=malloc(40001);CHECK(stress);memset(stress,'(',20000);memset(stress+20000,']',20000);stress[40000]=0;
    CHECK(sb_script_plan(stress,40000,&bad).code==SB_OK);CHECK(bad.count==1);sb_script_plan_free(&bad);free(stress);
    /* Independently parse aliases and every original property assignment. */
    struct Alias {char name[80];uint32_t tag;} aliases[256];size_t alias_count=0;char line[2048];
    FILE *f=open_data(argv[1],"PropertyValueAliases.txt");CHECK(f);
    while(fgets(line,sizeof(line),f)){char short_name[8],long_name[80];if(sscanf(line,"sc ; %7s ; %79s",short_name,long_name)==2){CHECK(strlen(short_name)==4 && alias_count<256);strcpy(aliases[alias_count].name,long_name);aliases[alias_count++].tag=tag(short_name);}}
    CHECK(!ferror(f));fclose(f);CHECK(alias_count>170);
    uint32_t *primary=malloc(0x110000*sizeof(*primary));CHECK(primary);for(uint32_t cp=0;cp<0x110000;++cp)primary[cp]=SB_SCRIPT_UNKNOWN;
    f=open_data(argv[1],"Scripts.txt");CHECK(f);
    while(fgets(line,sizeof(line),f)){char range[40],name[80];if(sscanf(line,"%39s ; %79s",range,name)!=2 || line[0]=='#')continue;unsigned lo,hi;CHECK(sscanf(range,"%x..%x",&lo,&hi)>=1);if(!strstr(range,".."))hi=lo;CHECK(lo<=hi && hi<0x110000);uint32_t value=0;for(size_t i=0;i<alias_count;++i)if(!strcmp(name,aliases[i].name))value=aliases[i].tag;CHECK(value);for(uint32_t cp=lo;cp<=hi;++cp)primary[cp]=value;}
    CHECK(!ferror(f));fclose(f);
    for(uint32_t cp=0;cp<0x110000;++cp)CHECK(sb_script_property(cp).primary==primary[cp]);free(primary);
    f=open_data(argv[1],"ScriptExtensions.txt");CHECK(f);size_t extension_ranges=0;
    while(fgets(line,sizeof(line),f)){char *comment=strchr(line,'#');if(comment)*comment=0;char *split=strchr(line,';');if(!split)continue;*split++=0;unsigned lo,hi;CHECK(sscanf(line,"%x..%x",&lo,&hi)>=1);if(!strstr(line,".."))hi=lo;uint32_t members[32];size_t count=0;for(char *t=strtok(split," \t\r\n");t;t=strtok(NULL," \t\r\n")){CHECK(strlen(t)==4 && count<32);members[count++]=tag(t);}CHECK(count>0);
        for(uint32_t cp=lo;cp<=hi;++cp){SBScriptProperty p=sb_script_property(cp);CHECK(p.count==count);for(size_t i=0;i<count;++i){bool found=false;for(size_t j=0;j<p.count;++j)found|=p.extensions[j]==members[i];CHECK(found);}}++extension_ranges;}
    CHECK(!ferror(f));fclose(f);CHECK(extension_ranges>150);
    f=open_data(argv[1],"BidiBrackets.txt");CHECK(f);size_t pairs=0;while(fgets(line,sizeof(line),f)){unsigned cp,pair;char kind;if(sscanf(line,"%x ; %x ; %c",&cp,&pair,&kind)==3){SBScriptProperty p=sb_script_property(cp);CHECK(p.paired==pair && p.bracket==(kind=='o' ? 1 : 2));++pairs;}}
    CHECK(!ferror(f));fclose(f);CHECK(pairs>120);printf("%lu script assertions passed; all Unicode primary positions, %zu extension ranges, %zu brackets.\n",checks,extension_ranges,pairs);return 0;
}
