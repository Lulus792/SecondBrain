#include "word.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks,cases;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"WORD FAIL %u line%d: %s\n",cases,__LINE__,#x); exit(1); } } while (0)
static size_t encode(char *out,unsigned cp) {
    if (cp<0x80) { out[0]=(char)cp; return 1; }
    if (cp<0x800) { out[0]=(char)(0xc0|(cp>>6)); out[1]=(char)(0x80|(cp&63)); return 2; }
    if (cp<0x10000) { out[0]=(char)(0xe0|(cp>>12)); out[1]=(char)(0x80|((cp>>6)&63)); out[2]=(char)(0x80|(cp&63)); return 3; }
    out[0]=(char)(0xf0|(cp>>18)); out[1]=(char)(0x80|((cp>>12)&63)); out[2]=(char)(0x80|((cp>>6)&63)); out[3]=(char)(0x80|(cp&63)); return 4;
}
int main(int argc,char **argv) {
    CHECK(argc==2); FILE *file=fopen(argv[1],"rb"); CHECK(file);
    char line[16384];
    while (fgets(line,sizeof(line),file)) {
        char *comment=strchr(line,'#'); if (comment) *comment=0;
        char *token=strtok(line," \t\r\n"); if (!token) continue;
        char text[4096],original[4096]; SBWordBoundary expected[1024]; size_t bytes=0,count=0,characters=0;
        while (token) {
            if (!strcmp(token,"\xc3\xb7")) { CHECK(count<1024); expected[count++]=(SBWordBoundary){.byte=bytes,.characters=characters}; }
            else if (strcmp(token,"\xc3\x97")) { char *end; unsigned cp=(unsigned)strtoul(token,&end,16); CHECK(!*end && cp<=0x10ffff && bytes+4<=sizeof(text)); bytes+=encode(text+bytes,cp); ++characters; }
            token=strtok(NULL," \t\r\n");
        }
        CHECK(count>=2); memcpy(original,text,bytes); SBWord reader; CHECK(sb_word_init(&reader,text,bytes));
        size_t at=0; SBWordBoundary b;
        while (sb_word_next(&reader,&b)) { CHECK(at<count && b.byte==expected[at].byte && b.characters==expected[at].characters); ++at; }
        CHECK(at==count && !memcmp(text,original,bytes));
        ++cases;
    }
    CHECK(!ferror(file)); fclose(file); CHECK(cases==1944);
    SBWord r; SBWordBoundary b;
    CHECK(sb_word_init(&r,NULL,0)); CHECK(sb_word_next(&r,&b) && b.byte==0); CHECK(!sb_word_next(&r,&b));
    CHECK(!sb_word_init(&r,"\xf0\x80\x80\x80",4)); CHECK(!sb_word_init(&r,NULL,1));
    char *long_cluster=malloc(100001); CHECK(long_cluster); long_cluster[0]='a';
    for (size_t i=1;i<100001;i+=2) { long_cluster[i]=(char)0xcc;long_cluster[i+1]=(char)0x88; }
    CHECK(sb_word_init(&r,long_cluster,100001)); CHECK(sb_word_next(&r,&b) && b.byte==0);
    CHECK(sb_word_next(&r,&b) && b.byte==100001 && b.characters==50001); CHECK(!sb_word_next(&r,&b)); free(long_cluster);
    char *flags=malloc(800000);CHECK(flags);
    for(size_t i=0;i<800000;i+=4)encode(flags+i,0x1f1e6);
    CHECK(sb_word_init(&r,flags,800000));size_t groups=0;
    while(sb_word_next(&r,&b)){CHECK(b.byte==groups*8 && b.characters==groups*2 && (!groups || b.significant));++groups;}
    CHECK(groups==100001);free(flags);
    printf("%u assertions passed against %u Unicode 18.0 word test cases, bounded invalid input and a long cluster.\n",checks,cases); return 0;
}
