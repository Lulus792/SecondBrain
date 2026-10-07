#include "notices.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"NOTICES FAIL %d: %s\n",__LINE__,#x); return 1; } } while (0)
int main(int argc,char **argv) {
    CHECK(argc==3); char *text=NULL;
    CHECK(sb_notice_count()==19);
    for (size_t i=0;i<sb_notice_count();++i) {
        CHECK(sb_notice_read(argv[1],i,&text).code==SB_OK);
        CHECK(text && strlen(text)>20 && sb_text_valid(text,strlen(text)));
        free(text); text=NULL;
    }
    CHECK(sb_notice_read(argv[1],sb_notice_count(),&text).code==SB_INVALID && !text);
    CHECK(sb_notice_read(NULL,0,&text).code==SB_INVALID && !text);
    CHECK(sb_notice_read(argv[1],0,NULL).code==SB_INVALID);
    CHECK(sb_notice_read("font.ttf",0,&text).code==SB_INVALID && !text);
    char root[SB_PATH_CAP],font[SB_PATH_CAP],dir[SB_PATH_CAP],file[SB_PATH_CAP],suffix[80];
    snprintf(suffix,sizeof(suffix),"run-%lu-%lu",sb_process_id(),(unsigned long)time(NULL));
#ifndef _WIN32
    strcat(suffix,"\\literal");
#endif
    CHECK(sb_path_join(root,sizeof(root),argv[2],suffix).code==SB_OK);
    CHECK(sb_path_join(font,sizeof(font),root,"fonts/NotoSans-Regular.ttf").code==SB_OK);
    CHECK(sb_notice_read(font,0,&text).code!=SB_OK && !text);
    CHECK(sb_path_join(dir,sizeof(dir),root,"licenses").code==SB_OK);
    CHECK(sb_fs_mkdirs(dir).code==SB_OK);
    CHECK(sb_path_join(file,sizeof(file),dir,"LICENSE").code==SB_OK);
    const char invalid[]={ 'a',0,'b' };
    CHECK(sb_fs_write_new(file,invalid,sizeof(invalid)).code==SB_OK);
    CHECK(sb_notice_read(font,0,&text).code==SB_INVALID && !text);
    CHECK(sb_fs_remove(file).code==SB_OK);
    const char malformed[]={ (char)0xc3, (char)0x28 };
    CHECK(sb_fs_write_new(file,malformed,sizeof(malformed)).code==SB_OK);
    CHECK(sb_notice_read(font,0,&text).code==SB_INVALID && !text);
    CHECK(sb_fs_remove(file).code==SB_OK);
    const char original[]="MIT\r\n* ** [literal](text) `code`\r\nCopyright ü\r\n";
    CHECK(sb_fs_write_new(file,original,strlen(original)).code==SB_OK);
    CHECK(sb_notice_read(font,0,&text).code==SB_OK && !strcmp(text,original));
    free(text); text=NULL;
#ifdef _WIN32
    for (char *part=font;*part;++part) if (*part=='/') *part='\\';
    CHECK(sb_notice_read(font,0,&text).code==SB_OK && !strcmp(text,original));
    free(text);
#endif
    printf("%u license-resource assertions passed.\n",checks); return 0;
}
