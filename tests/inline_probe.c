#include "platform.h"
#include "inline.h"
#include "markdown.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc,char **argv) {
 if(argc!=2)return 2;
 char *source=NULL;size_t length=0;
 if(sb_fs_read(argv[1],&source,&length).code!=SB_OK)return 3;
 SBMarkdown md;SBMarkdownBlock b;sb_markdown_init(&md,source,length,false);
 while(sb_markdown_next(&md,&b)) {
  if(b.kind==SB_MD_BLANK||b.kind==SB_MD_FENCE||b.kind==SB_MD_RULE)continue;
  SBInline r;SBStyledText out={0};
  if(b.kind==SB_MD_CODE){
   out.text=malloc(b.length+1);out.spans=malloc(sizeof(SBTextSpan));
   if(!out.text||!out.spans){sb_styled_free(&out);free(source);return 4;}
   memcpy(out.text,source+b.content,b.length);out.text[b.length]=0;
   out.count=b.length ? 1 : 0;out.spans[0]=(SBTextSpan){0,b.length,SB_TEXT_CODE};
  } else {
   if(sb_inline_init(&r,source+b.content,b.length).code!=SB_OK){sb_inline_free(&r);free(source);return 4;}
   SBStatus status=sb_inline_styled(&r,0,b.length,&out);sb_inline_free(&r);
   if(status.code!=SB_OK){free(source);return 5;}
  }
  size_t text_length=strlen(out.text);
  printf("B ");for(size_t i=0;i<text_length;++i)printf("%02x",(unsigned char)out.text[i]);puts("");
  for(size_t i=0;i<out.count;++i)printf("S %zu %zu %u\n",out.spans[i].offset,out.spans[i].length,out.spans[i].style);
  sb_styled_free(&out);
 }free(source);return 0;
}
