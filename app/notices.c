#include "notices.h"
#include "platform.h"
#include <stdlib.h>
#include <string.h>
static const struct { const char *name,*file; } notices[]={
    {"SecondBrain · MIT","LICENSE"},
    {"SDL3 · zlib","SDL3.txt"},
    {"Nuklear · MIT","Nuklear-LICENSE"},
    {"Noto-Schriften · SIL Open Font License","OFL.txt"},
    {"Noto CJK · SIL Open Font License","OFL-CJK.txt"},
    {"SDL_ttf · zlib","SDL_ttf.txt"},
    {"FreeType · Lizenzübersicht","FreeType-LICENSE.txt"},
    {"FreeType · FTL","FreeType-FTL.txt"},
    {"FreeType · BDF","FreeType-BDF.txt"},
    {"FreeType · PCF","FreeType-PCF.txt"},
    {"FreeType · zlib-Anteil","FreeType-zlib.txt"},
    {"HarfBuzz · Old MIT","HarfBuzz.txt"},
    {"HarfBuzz · Microsoft USE","HarfBuzz-MS-USE.txt"},
    {"AccessKit · MIT","AccessKit-LICENSE-MIT.txt"},
    {"AccessKit · Apache 2.0","AccessKit-LICENSE-APACHE.txt"},
    {"AccessKit · Chromium BSD","AccessKit-LICENSE.chromium.txt"},
    {"AccessKit · Autoren","AccessKit-AUTHORS.txt"},
    {"Unicode-Daten · Unicode License V3","Unicode.txt"},
    {"Noto Emoji · SIL Open Font License","OFL-Emoji.txt"},
    {"WHATWG-Zeichenreferenzen · CC BY / BSD","WHATWG.txt"},
    {"Noto Math · SIL Open Font License","OFL-Math.txt"}
};
size_t sb_notice_count(void) { return sizeof(notices)/sizeof(*notices); }
const char *sb_notice_name(size_t index) { return index<sb_notice_count() ? notices[index].name : "Lizenzen"; }
SBStatus sb_notice_read(const char *font_path,size_t index,char **text) {
    if (text) *text=NULL;
    if (!text || !font_path || index>=sb_notice_count()) return sb_error(SB_INVALID,"Unbekannter Lizenztext.");
    char root[SB_PATH_CAP],directory[SB_PATH_CAP],path[SB_PATH_CAP];
    if (strlen(font_path)>=sizeof(root)) return sb_error(SB_LIMIT,"Ressourcenpfad ist zu lang.");
    strcpy(root,font_path);
#ifdef _WIN32
    for (char *part=root;*part;++part) if (*part=='\\') *part='/';
#endif
    char *slash=strrchr(root,'/');
    if (!slash) return sb_error(SB_INVALID,"Ressourcenordner fehlt.");
    *slash=0; slash=strrchr(root,'/');
    if (!slash) return sb_error(SB_INVALID,"Ressourcenordner fehlt.");
    *slash=0;
    SBStatus status=sb_path_join(directory,sizeof(directory),root,"licenses");
    if (status.code==SB_OK) status=sb_path_join(path,sizeof(path),directory,notices[index].file);
    size_t length=0;
    if (status.code==SB_OK) status=sb_fs_read(path,text,&length);
    if (status.code==SB_OK && !sb_text_valid(*text,length)) status=sb_error(SB_INVALID,"Lizenztext enthält ungültige Textdaten.");
    if (status.code!=SB_OK) { free(*text); *text=NULL; }
    return status;
}
