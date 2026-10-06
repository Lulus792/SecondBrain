#include "settings.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void sb_settings_defaults(SBSettings *s) {
    memset(s,0,sizeof(*s)); s->font_percent=100; s->width=1336; s->height=840; s->dark=true;
}
static bool valid(const SBSettings *s) {
    return s->font_percent>=100 && s->font_percent<=200 && s->font_percent%25==0 &&
        s->width>=780 && s->width<=8192 && s->height>=520 && s->height<=8192 &&
        (!s->project[0] || sb_id_valid(s->project)) &&
        sb_utf8_valid(s->workspace,strlen(s->workspace)) && sb_utf8_valid(s->note,strlen(s->note));
}
static int hex(unsigned char c) {
    if (c>='0' && c<='9') return c-'0';
    if (c>='a' && c<='f') return c-'a'+10;
    if (c>='A' && c<='F') return c-'A'+10;
    return -1;
}
static bool decode(const char *in, char *out, size_t cap) {
    size_t n=0;
    while (*in) {
        unsigned char c=(unsigned char)*in++;
        if (c=='%') {
            if (!in[0] || !in[1] || hex((unsigned char)in[0])<0 || hex((unsigned char)in[1])<0) return false;
            c=(unsigned char)(hex((unsigned char)in[0])*16+hex((unsigned char)in[1])); in+=2;
            if (!c) return false;
        }
        if (n+1>=cap) return false;
        out[n++]=(char)c;
    }
    out[n]=0; return sb_utf8_valid(out,n);
}
static void encode(const char *in, char *out) {
    const char *digits="0123456789ABCDEF";
    while (*in) {
        unsigned char c=(unsigned char)*in++;
        if (c>=33 && c<=126 && c!='%' && c!='=') *out++=(char)c;
        else { *out++='%'; *out++=digits[c>>4]; *out++=digits[c&15]; }
    }
    *out=0;
}
static bool number(const char *text, unsigned *out) {
    unsigned value=0;
    if (!*text) return false;
    for (;*text;++text) { if (*text<'0' || *text>'9' || value>8192) return false; value=value*10+(unsigned)(*text-'0'); }
    *out=value; return true;
}
SBStatus sb_settings_load(const char *path, SBSettings *out, SBRevision *revision) {
    char *text=NULL; size_t length=0; SBSettings s; unsigned seen=0;
    sb_settings_defaults(&s); memset(revision,0,sizeof(*revision));
    if (sb_fs_kind(path)!=0 && sb_fs_kind(path)!=1) return sb_error(SB_INVALID,"Einstellungen sind keine reguläre Datei.");
    SBStatus status=sb_fs_read(path,&text,&length);
    if (status.code==SB_NOT_FOUND) { *out=s; return sb_ok(); }
    if (status.code!=SB_OK) return status;
    revision->exists=true; revision->length=length; revision->hash=sb_hash(text,length);
    bool version2=length>=23 && !strncmp(text,"SecondBrain settings 2\n",23);
    if (length>32768 || strlen(text)!=length || (!version2 && strncmp(text,"SecondBrain settings 1\n",23))) {
        free(text); return sb_error(SB_INVALID,"Einstellungsdatei ist beschädigt oder hat eine unbekannte Version.");
    }
    char *cursor=text+23;
    while (*cursor) {
        char *end=strchr(cursor,'\n'), *equal=strchr(cursor,'=');
        if (!end || !equal || equal>=end) goto invalid;
        *end=0; *equal=0;
        unsigned bit=0, value=0; char *data=equal+1;
        if (!strcmp(cursor,"workspace")) { bit=1; if (!decode(data,s.workspace,sizeof(s.workspace))) goto invalid; }
        else if (!strcmp(cursor,"project")) { bit=2; if (!decode(data,s.project,sizeof(s.project))) goto invalid; }
        else if (!strcmp(cursor,"note")) { bit=4; if (!decode(data,s.note,sizeof(s.note))) goto invalid; }
        else {
            if (!number(data,&value)) goto invalid;
            if (!strcmp(cursor,"font")) { bit=8; s.font_percent=value; }
            else if (!strcmp(cursor,"width")) { bit=16; s.width=value; }
            else if (!strcmp(cursor,"height")) { bit=32; s.height=value; }
            else if (!strcmp(cursor,"dark")) { bit=64; if (value>1) goto invalid; s.dark=value!=0; }
            else if (!strcmp(cursor,"solid")) { bit=128; if (value>1) goto invalid; s.solid=value!=0; }
            else if (!strcmp(cursor,"motion")) { bit=256; if (value>1) goto invalid; s.reduced_motion=value!=0; }
            else if (version2 && !strcmp(cursor,"follow-theme")) { bit=512; if (value>1) goto invalid; s.follow_theme=value!=0; }
            else if (version2 && !strcmp(cursor,"contrast")) { bit=1024; if (value>1) goto invalid; s.contrast=value!=0; }
            else goto invalid;
        }
        if (seen&bit) goto invalid;
        seen|=bit; cursor=end+1;
    }
    if (seen!=(version2 ? 2047u : 511u) || !valid(&s)) goto invalid;
    free(text); *out=s; return sb_ok();
invalid:
    free(text); return sb_error(SB_INVALID,"Einstellungsdatei enthält ungültige oder doppelte Werte.");
}
SBStatus sb_settings_save(const char *path, const SBSettings *s, SBRevision expected, SBRevision *saved) {
    char workspace[SB_PATH_CAP*3], project[65*3], note[SB_PATH_CAP*3], text[32768], temporary[SB_PATH_CAP];
    SBSettings current; SBRevision actual; SBStatus status;
    if (!valid(s)) return sb_error(SB_INVALID,"Ungültige Einstellungen.");
    /* Never silently replace a malformed file or a concurrent change. */
    status=sb_settings_load(path,&current,&actual);
    if (status.code!=SB_OK) return status;
    if (actual.exists!=expected.exists || actual.hash!=expected.hash || actual.length!=expected.length)
        return sb_error(SB_CONFLICT,"Einstellungen wurden von einer anderen Instanz geändert.");
    encode(s->workspace,workspace); encode(s->project,project); encode(s->note,note);
    int length=snprintf(text,sizeof(text),"SecondBrain settings 2\nworkspace=%s\nproject=%s\nnote=%s\nfont=%u\nwidth=%u\nheight=%u\ndark=%u\nsolid=%u\nmotion=%u\nfollow-theme=%u\ncontrast=%u\n",workspace,project,note,s->font_percent,s->width,s->height,(unsigned)s->dark,(unsigned)s->solid,(unsigned)s->reduced_motion,(unsigned)s->follow_theme,(unsigned)s->contrast);
    if (length<0 || (size_t)length>=sizeof(text)) return sb_error(SB_LIMIT,"Einstellungen sind zu lang.");
    int count=snprintf(temporary,sizeof(temporary),"%s.tmp-%lu",path,sb_process_id());
    if (count<0 || (size_t)count>=sizeof(temporary)) return sb_error(SB_LIMIT,"Einstellungspfad ist zu lang.");
    status=sb_fs_write_new(temporary,text,(size_t)length);
    if (status.code!=SB_OK) return status;
    status=sb_settings_load(path,&current,&actual);
    if (status.code==SB_OK && (actual.exists!=expected.exists || actual.hash!=expected.hash || actual.length!=expected.length))
        status=sb_error(SB_CONFLICT,"Einstellungen wurden zwischenzeitlich geändert.");
    if (status.code==SB_OK) status=expected.exists ? sb_fs_replace(temporary,path) : sb_fs_move_new(temporary,path);
    if (status.code!=SB_OK) { sb_fs_remove(temporary); return status; }
    if (saved) { saved->exists=true; saved->length=(size_t)length; saved->hash=sb_hash(text,(size_t)length); }
    return sb_ok();
}
