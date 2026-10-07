#include "grapheme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks,cases;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"GRAPHEME FAIL %u line%d: %s\n",cases,__LINE__,#x); exit(1); } } while (0)
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
        char text[4096],original[4096]; SBGraphemeBoundary expected[1024]; size_t bytes=0,count=0,characters=0;
        while (token) {
            if (!strcmp(token,"\xc3\xb7")) { CHECK(count<1024); expected[count++]=(SBGraphemeBoundary){bytes,characters}; }
            else if (strcmp(token,"\xc3\x97")) { char *end; unsigned cp=(unsigned)strtoul(token,&end,16); CHECK(!*end && cp<=0x10ffff && bytes+4<=sizeof(text)); bytes+=encode(text+bytes,cp); ++characters; }
            token=strtok(NULL," \t\r\n");
        }
        CHECK(count>=2); memcpy(original,text,bytes); SBGrapheme reader; CHECK(sb_grapheme_init(&reader,text,bytes));
        size_t at=0; SBGraphemeBoundary b;
        while (sb_grapheme_next(&reader,&b)) { CHECK(at<count && b.byte==expected[at].byte && b.characters==expected[at].characters); ++at; }
        CHECK(at==count && !memcmp(text,original,bytes));
        for (size_t position=0;position<=characters;++position) {
            size_t index=0; while (index+1<count && expected[index+1].characters<=position) ++index;
            bool boundary=expected[index].characters==position;
            size_t upper=index+1<count ? expected[index+1].characters : characters;
            SBGraphemePosition p; CHECK(sb_grapheme_position(text,bytes,position,&p));
            CHECK(p.boundary==boundary && p.floor==expected[index].characters && p.ceil==(boundary ? position : upper) && p.next==upper);
            CHECK(p.previous==(boundary && index ? expected[index-1].characters : expected[index].characters));
        }
        ++cases;
    }
    CHECK(!ferror(file)); fclose(file); CHECK(cases>700);
    SBGrapheme r; SBGraphemeBoundary b;
    CHECK(sb_grapheme_init(&r,NULL,0)); CHECK(sb_grapheme_next(&r,&b) && b.byte==0); CHECK(!sb_grapheme_next(&r,&b));
    CHECK(!sb_grapheme_init(&r,"\xf0\x80\x80\x80",4)); CHECK(!sb_grapheme_init(&r,NULL,1));
    char *long_cluster=malloc(100001); CHECK(long_cluster); long_cluster[0]='a';
    for (size_t i=1;i<100001;i+=2) { long_cluster[i]=(char)0xcc;long_cluster[i+1]=(char)0x88; }
    CHECK(sb_grapheme_init(&r,long_cluster,100001)); CHECK(sb_grapheme_next(&r,&b) && b.byte==0);
    CHECK(sb_grapheme_next(&r,&b) && b.byte==100001 && b.characters==50001); CHECK(!sb_grapheme_next(&r,&b)); free(long_cluster);
    printf("%u assertions passed against %u Unicode 18.0 grapheme test cases, bounded invalid input and a long cluster.\n",checks,cases); return 0;
}
