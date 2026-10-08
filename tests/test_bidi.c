#include "bidi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define CAP 1024
static unsigned long cases,checks,line_number;
static const char *fixture;
#define CHECK(x) do {++checks;if(!(x)){fprintf(stderr,"BIDI %s:%lu C:%d: %s\n",fixture ? fixture : "own",line_number,__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
static size_t encode(uint32_t cp,char *text) {
    if(cp<0x80){text[0]=(char)cp;return 1;}
    if(cp<0x800){text[0]=(char)(0xc0|(cp>>6));text[1]=(char)(0x80|(cp&63));return 2;}
    if(cp<0x10000){text[0]=(char)(0xe0|(cp>>12));text[1]=(char)(0x80|((cp>>6)&63));text[2]=(char)(0x80|(cp&63));return 3;}
    text[0]=(char)(0xf0|(cp>>18));text[1]=(char)(0x80|((cp>>12)&63));text[2]=(char)(0x80|((cp>>6)&63));text[3]=(char)(0x80|(cp&63));return 4;
}
static bool number(const char *text,int base,int *value) {
    char *end;long n=strtol(text,&end,base);
    if(!*text || *end || n<0 || n>INT_MAX)return false;
    *value=(int)n;return true;
}
static int tokens_limit(char *text,int base,int *values,int capacity) {
    int count=0;
    for(char *t=strtok(text," \t\r\n");t;t=strtok(NULL," \t\r\n")) {
        if(count==capacity)return -1;
        if(!strcmp(t,"x"))values[count++]=-1;
        else if(!number(t,base,&values[count++]))return -1;
    }
    return count;
}
#define tokens(text,base,values) tokens_limit(text,base,values,CAP)
static int run_case(const int *cp,int count,const int *levels,const int *order,int ordered,int base,SBTextDirection direction) {
    char text[CAP*4],original[CAP*4];size_t bytes[CAP+1],length=0;
    CHECK(count>0 && count<=CAP && ordered>=0 && ordered<=count);
    for(int i=0;i<count;++i){CHECK(cp[i]>=0 && cp[i]<=0x10ffff && !(cp[i]>=0xd800 && cp[i]<=0xdfff));bytes[i]=length;length+=encode((uint32_t)cp[i],text+length);}
    bytes[count]=length;memcpy(original,text,length);
    SBTextParagraph *p=NULL;OK(sb_bidi_paragraph_create(text,length,direction,&p));
    CHECK(sb_bidi_paragraph_length(p)==length && !memcmp(sb_bidi_paragraph_text(p),original,length));
    CHECK(base<0 || sb_bidi_paragraph_level(p)==base);
    SBVisualLine line={0};OK(sb_bidi_line(p,0,length,&line));
    unsigned char seen[CAP]={0};int actual[CAP],used=0;
    for(size_t r=0;r<line.count;++r) {
        const SBVisualRun *run=&line.runs[r];
        CHECK(run->length && run->byte<length && run->length<=length-run->byte);
        int start=-1,end=-1;
        for(int i=0;i<=count;++i){if(bytes[i]==run->byte)start=i;if(bytes[i]==run->byte+run->length)end=i;}
        if(start<0 || end<=start)fprintf(stderr,"run %zu/%zu bytes=%zu+%zu level=%u; source length=%zu\n",r,line.count,run->byte,run->length,run->level,length);
        CHECK(start>=0 && end>start);
        for(int i=start;i<end;++i){CHECK(!seen[i]);seen[i]=1;CHECK(levels[i]<0 || run->level==levels[i]);}
        for(int i=0;i<end-start;++i){int logical=(run->level&1) ? end-1-i : start+i;if(levels[logical]>=0){CHECK(used<CAP);actual[used++]=logical;}}
    }
    for(int i=0;i<count;++i)CHECK(seen[i]);
    CHECK(used==ordered);
    for(int i=0;i<ordered;++i)CHECK(actual[i]==order[i]);
    CHECK(!memcmp(text,original,length) && !memcmp(sb_bidi_paragraph_text(p),original,length));
    sb_bidi_line_free(&line);sb_bidi_paragraph_free(p);++cases;return 0;
}
static FILE *open_fixture(const char *root,const char *name){char path[4096];int n=snprintf(path,sizeof(path),"%s/%s",root,name);return n>0 && (size_t)n<sizeof(path) ? fopen(path,"rb") : NULL;}
static int character_tests(const char *root) {
    fixture="BidiCharacterTest.txt";line_number=0;FILE *file=open_fixture(root,fixture);CHECK(file);
    char raw[16384];unsigned long before=cases;
    while(fgets(raw,sizeof(raw),file)) {
        ++line_number;CHECK(strchr(raw,'\n') || feof(file));
        char *comment=strchr(raw,'#');if(comment)*comment=0;
        if(!strchr(raw,';'))continue;
        char *fields[5];fields[0]=raw;
        for(unsigned i=1;i<5;++i){char *split=strchr(fields[i-1],';');CHECK(split);*split=0;fields[i]=split+1;}
        int cp[CAP],levels[CAP],order[CAP],direction,base;
        int count=tokens(fields[0],16,cp);CHECK(count>0);
        CHECK(tokens_limit(fields[1],10,&direction,1)==1 && direction>=0 && direction<=2);
        CHECK(tokens_limit(fields[2],10,&base,1)==1 && base>=0 && base<=1);
        CHECK(tokens(fields[3],10,levels)==count);
        int ordered=tokens(fields[4],10,order);CHECK(ordered>=0);
        CHECK(!run_case(cp,count,levels,order,ordered,base,direction==0 ? SB_BIDI_LTR : direction==1 ? SB_BIDI_RTL : SB_BIDI_AUTO_LTR));
    }
    CHECK(!ferror(file));fclose(file);CHECK(cases-before==91707);
    printf("%lu Unicode character/direction cases passed.\n",cases-before);return 0;
}
static int class_tests(const char *root) {
    static const struct {const char *name;int cp;} classes[]={
        {"AL",0x627},{"AN",0x600},{"B",0x85},{"BN",0x1b},{"CS",0x2e},{"EN",0x30},{"ES",0x2b},{"ET",0x25},
        {"FSI",0x2068},{"L",0x41},{"LRE",0x202a},{"LRI",0x2066},{"LRO",0x202d},{"NSM",0x614},{"ON",0x28},
        {"PDF",0x202c},{"PDI",0x2069},{"R",0x5d0},{"RLE",0x202b},{"RLI",0x2067},{"RLO",0x202e},{"S",9},{"WS",0x20}};
    fixture="BidiTest.txt";line_number=0;FILE *file=open_fixture(root,fixture);CHECK(file);
    char raw[16384];int levels[CAP],order[CAP],level_count=-1,ordered=-1;unsigned long before=cases,data=0;
    while(fgets(raw,sizeof(raw),file)) {
        ++line_number;CHECK(strchr(raw,'\n') || feof(file));char *comment=strchr(raw,'#');if(comment)*comment=0;
        if(!strncmp(raw,"@Levels:",8)){level_count=tokens(raw+8,10,levels);CHECK(level_count>=0);continue;}
        if(!strncmp(raw,"@Reorder:",9)){ordered=tokens(raw+9,10,order);CHECK(ordered>=0);continue;}
        char *split=strchr(raw,';');if(!split)continue;*split=0;
        int mask;CHECK(tokens_limit(split+1,16,&mask,1)==1 && mask>0 && mask<=7);
        int cp[CAP],count=0;
        for(char *t=strtok(raw," \t\r\n");t;t=strtok(NULL," \t\r\n")) {
            int code=-1;for(size_t i=0;i<sizeof(classes)/sizeof(*classes);++i)if(!strcmp(t,classes[i].name))code=classes[i].cp;
            CHECK(code>=0 && count<CAP);cp[count++]=code;
        }
        CHECK(count>0 && level_count==count && ordered>=0);
        if(mask&1)CHECK(!run_case(cp,count,levels,order,ordered,-1,SB_BIDI_AUTO_LTR));
        if(mask&2)CHECK(!run_case(cp,count,levels,order,ordered,0,SB_BIDI_LTR));
        if(mask&4)CHECK(!run_case(cp,count,levels,order,ordered,1,SB_BIDI_RTL));
        ++data;
    }
    CHECK(!ferror(file));fclose(file);CHECK(data==490846);
    printf("%lu Unicode class sequences / %lu direction cases passed.\n",data,cases-before);return 0;
}
static int mirrors(const char *root) {
    fixture="BidiMirroring.txt";line_number=0;FILE *file=open_fixture(root,fixture);CHECK(file);
    uint32_t *expected=calloc(0x110000,sizeof(*expected));CHECK(expected);char raw[2048];size_t pairs=0;
    while(fgets(raw,sizeof(raw),file)){
        ++line_number;char *comment=strchr(raw,'#');if(comment)*comment=0;char *split=strchr(raw,';');if(!split)continue;*split=0;
        int cp,mirror;CHECK(tokens_limit(raw,16,&cp,1)==1 && cp>=0 && cp<0x110000);CHECK(tokens_limit(split+1,16,&mirror,1)==1 && mirror>=0 && mirror<0x110000);
        expected[cp]=(uint32_t)mirror;++pairs;
    }
    CHECK(!ferror(file));fclose(file);CHECK(pairs>400);
    for(uint32_t cp=0;cp<0x110000;++cp){uint32_t actual=sb_bidi_mirror(cp);if(actual!=expected[cp])fprintf(stderr,"mirror U+%04X: %04X expected %04X\n",cp,actual,expected[cp]);CHECK(actual==expected[cp]);}
    free(expected);CHECK(!sb_bidi_mirror(UINT32_MAX));printf("%zu mirror pairs; all Unicode scalar positions checked.\n",pairs);return 0;
}
static int own_tests(void) {
    fixture=NULL;line_number=0;SBTextParagraph *p=NULL;SBVisualLine line={0};
    CHECK(sb_bidi_paragraph_create("\xc0\xaf",2,SB_BIDI_LTR,&p).code==SB_INVALID && !p);
    CHECK(sb_bidi_paragraph_create(NULL,1,SB_BIDI_LTR,&p).code==SB_INVALID && !p);
    CHECK(sb_bidi_paragraph_create("x",1,(SBTextDirection)77,&p).code==SB_INVALID && !p);
    CHECK(sb_bidi_paragraph_create("x",1,SB_BIDI_LTR,NULL).code==SB_INVALID);
    OK(sb_bidi_paragraph_create(NULL,0,SB_BIDI_AUTO_RTL,&p));CHECK(!sb_bidi_paragraph_length(p) && sb_bidi_paragraph_level(p)==1);
    CHECK(sb_bidi_line(p,0,0,&line).code==SB_INVALID);sb_bidi_paragraph_free(p);
    char text[]="אבג abc  \r\nNext";char original[sizeof(text)];memcpy(original,text,sizeof(text));
    OK(sb_bidi_paragraph_create(text,strlen(text),SB_BIDI_AUTO_LTR,&p));
    CHECK(sb_bidi_paragraph_length(p)==14 && sb_bidi_paragraph_level(p)==1);
    memset(text,'!',strlen(text));CHECK(!memcmp(sb_bidi_paragraph_text(p),original,sizeof(original)));
    CHECK(sb_bidi_line(p,1,2,&line).code==SB_INVALID);
    CHECK(sb_bidi_line(p,0,1,&line).code==SB_INVALID);
    CHECK(sb_bidi_line(p,SIZE_MAX,1,&line).code==SB_INVALID);
    CHECK(sb_bidi_line(p,0,SIZE_MAX,&line).code==SB_INVALID);
    OK(sb_bidi_line(p,0,6,&line));CHECK(line.count==1 && line.runs[0].level==1);
    CHECK(sb_bidi_line(p,0,6,&line).code==SB_INVALID);sb_bidi_line_free(&line);
    /* L1 belongs to the wrapped line; resolving one line never changes the
       stored paragraph levels used by another line. */
    unsigned char saved[14];memcpy(saved,sb_bidi_paragraph_levels(p),14);
    OK(sb_bidi_line(p,7,4,&line));CHECK(line.runs[0].byte==10 && line.runs[0].level==1);
    CHECK(!memcmp(saved,sb_bidi_paragraph_levels(p),14));sb_bidi_line_free(&line);sb_bidi_paragraph_free(p);
    return 0;
}
int main(int argc,char **argv) {
    CHECK(argc==2);if(own_tests() || character_tests(argv[1]) || class_tests(argv[1]) || mirrors(argv[1]))return 1;
    printf("%lu bidi assertions passed; %lu normative direction cases.\n",checks,cases);return 0;
}
