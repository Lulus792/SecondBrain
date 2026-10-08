#include "native_text.h"
#include "accessibility.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"NATIVE TEXT %d: %s\n",__LINE__,#x);return 1;}}while(0)
#define OK(x) CHECK((x).code==SB_OK)
static char *tree(SBAccessibility *a){accesskit_tree_update *t=sb_accessibility_tree(a);char *s=accesskit_tree_update_debug(t);accesskit_tree_update_free(t);return s;}
static size_t children(const char *s,accesskit_node_id *nodes,size_t capacity){const char *p=strstr(s,"role: MultilineTextInput");if(!p)return 0;p=strstr(p,"children: [");if(!p)return 0;p+=11;size_t n=0;while(*p!=']' && *p){if(*p=='#'){CHECK(n<capacity);nodes[n++]=strtoull(p+1,(char **)&p,10);}else ++p;}return n;}
int main(int argc,char **argv){CHECK(argc==2);char source[2500];memset(source,'a',300);strcpy(source+300," é 👩‍👩‍👧‍👦 can't 3,456.7 τέλος\r\nabc");
 SBTextSpan styles[]={{0,300,SB_TEXT_BOLD},{300,strlen(source)-300,SB_TEXT_ITALIC}};
 SBNativeText text={0};OK(sb_native_text(source,strlen(source),styles,2,&text));CHECK(text.count>=4 && !text.scalar_fallback);
 size_t bytes=0,scalars=0;bool accent=false,emoji=false,crlf=false;
 for(size_t i=0;i<text.count;++i){SBNativeTextRun *r=&text.runs[i];CHECK(r->characters<=255 && r->span.offset==bytes && r->scalars[0]==scalars);size_t n=0;
  for(size_t j=0;j<r->characters;++j){CHECK(r->lengths[j]>0 && r->scalars[j+1]>r->scalars[j]);accent|=r->lengths[j]==3 && r->scalars[j+1]-r->scalars[j]==2;emoji|=r->lengths[j]==25 && r->scalars[j+1]-r->scalars[j]==7;crlf|=r->lengths[j]==2 && source[bytes+n]=='\r';n+=r->lengths[j];}
  CHECK(n==r->span.length);for(size_t j=0;j<r->word_count;++j)CHECK(r->words[j]<r->characters && (!j || r->words[j]>r->words[j-1]));bytes+=n;scalars=r->scalars[r->characters];
 }CHECK(bytes==strlen(source) && accent && emoji && crlf);CHECK(text.runs[0].characters==255 && text.runs[0].word_count==1 && text.runs[0].words[0]==0);CHECK(text.runs[1].word_count==0);
 SBNativeText plain={0};OK(sb_native_text(source,strlen(source),NULL,0,&plain));CHECK(plain.runs[0].characters==255);
 SBNativeText empty={0};OK(sb_native_text(NULL,0,NULL,0,&empty));CHECK(empty.count==1 && !empty.runs[0].characters);sb_native_text_free(&empty);
 CHECK(sb_native_text("\xc0\x80",2,NULL,0,&empty).code==SB_INVALID);
 OK(sb_native_text("\r\n",2,NULL,0,&empty));CHECK(empty.count==2 && empty.runs[0].characters==1 && empty.runs[0].lengths[0]==2 && !empty.runs[1].characters && empty.runs[1].scalars[0]==2);sb_native_text_free(&empty);
 char *long_cluster=malloc(1001);CHECK(long_cluster);long_cluster[0]='a';for(unsigned i=1;i<1001;i+=2){long_cluster[i]=(char)0xcc;long_cluster[i+1]=(char)0x88;}
 OK(sb_native_text(long_cluster,1001,NULL,0,&empty));CHECK(empty.scalar_fallback && empty.count==2);size_t total=0;for(size_t i=0;i<empty.count;++i){CHECK(empty.runs[i].characters<=255);total+=empty.runs[i].span.length;}CHECK(total==1001);free(long_cluster);sb_native_text_free(&empty);
 SBUi ui;OK(sb_ui_init(&ui,argv[1],800,600,true));SBAccessibility *a=sb_accessibility_new(ui.window);CHECK(a);
 SBAccessibleItem item={.id="editor",.label="Native Wörter",.value=source,.bounds={0,0,700,400},.role=ACCESSKIT_ROLE_MULTILINE_TEXT_INPUT,.editable=true,.anchor=290,.caret=314};
 sb_accessibility_update(a,"Native Probe","editor",&item,1,"",false,7);char *snapshot=tree(a);CHECK(strstr(snapshot,"word_starts: [0]") && strstr(snapshot,"character_lengths: ["));
 const char *focus=strstr(snapshot,"focus: #");CHECK(focus);accesskit_node_id parent=strtoull(focus+8,NULL,10),nodes[32];size_t count=children(snapshot,nodes,32);CHECK(count==plain.count);accesskit_string_free(snapshot);
 /* Selection spans different runs; indices count whole selectable graphemes. */
 size_t run=0,index=0;for(size_t i=0;i<plain.count;++i)for(size_t j=0;j<=plain.runs[i].characters;++j)if(plain.runs[i].scalars[j]==311){run=i;index=j;}
 CHECK(run>0);accesskit_text_selection selection={{nodes[0],250},{nodes[run],index}};CHECK(sb_accessibility_select(a,parent,selection));SBAccessibleAction action;CHECK(sb_accessibility_next_action(a,&action));CHECK(action.anchor==250 && action.caret==311 && action.action==ACCESSKIT_ACTION_SET_TEXT_SELECTION);sb_accessibility_action_free(&action);
 selection.anchor.node=12345;CHECK(!sb_accessibility_select(a,parent,selection));selection.anchor.node=nodes[0];selection.anchor.character_index=256;CHECK(!sb_accessibility_select(a,parent,selection));
 selection.anchor.character_index=250;CHECK(sb_accessibility_select(a,parent,selection));CHECK(sb_accessibility_next_action(a,&action));
 item.value="Changed";sb_accessibility_update(a,"Native Probe","editor",&item,1,"",false,7);CHECK(!sb_accessibility_current(a,&action,7));sb_accessibility_action_free(&action);
 item.value=source;item.anchor=290;item.caret=314;sb_accessibility_update(a,"Native Probe","editor",&item,1,"",false,7);snapshot=tree(a);accesskit_node_id again[32];CHECK(children(snapshot,again,32)==count);CHECK(!memcmp(nodes,again,count*sizeof(*nodes)));accesskit_string_free(snapshot);
 item.bounds.y=30;sb_accessibility_update(a,"Native Probe","editor",&item,1,"",false,7);snapshot=tree(a);CHECK(children(snapshot,again,32)==count && !memcmp(nodes,again,count*sizeof(*nodes)));accesskit_string_free(snapshot);
 sb_accessibility_free(a);sb_ui_shutdown(&ui);sb_native_text_free(&plain);sb_native_text_free(&text);printf("%u native grapheme/run/word/selection/cache assertions passed.\n",checks);return 0;}
