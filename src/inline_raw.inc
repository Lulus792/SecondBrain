/* Opaque angle constructs keep attributes and URLs outside delimiter matching. */
static bool ascii_letter(char c) { return (c>='a' && c<='z') || (c>='A' && c<='Z'); }
static bool raw_space(char c) { return whitespace(c) || c=='\f'; }
static bool name_start(char c) { return ascii_letter(c) || c=='_' || c==':'; }
static bool name_rest(char c) { return name_start(c) || (c>='0' && c<='9') || c=='.' || c=='-'; }
static bool email_alnum(char c) { return ascii_letter(c) || (c>='0' && c<='9'); }
static bool email_local(char c) {
    return email_alnum(c) || (c && strchr(".!#$%&'*+/=?^_`{|}~-",c));
}
static size_t email_end(const SBInline *r,size_t p,size_t *budget) {
    const char *t=r->text; size_t n=r->length,start=p;
    while (p<n && email_local(t[p])) { if (!spend(budget)) return 0; ++p; }
    if (p==start || p==n || t[p++]!='@') return 0;
    for (;;) {
        start=p;
        if (p==n || !email_alnum(t[p])) return 0;
        while (p<n && (email_alnum(t[p]) || t[p]=='-')) {
            if (!spend(budget) || p-start==63) return 0;
            ++p;
        }
        if (!email_alnum(t[p-1]) || p==n) return 0;
        if (t[p]=='>') return p+1;
        if (t[p++]!='.') return 0;
    }
}
static size_t angle_end(const SBInline *r,size_t p,size_t *budget,SBInlineKind *kind) {
    const char *t=r->text; size_t n=r->length; ++p;
    if (p>=n) return 0;
    size_t q=p;
    if (ascii_letter(t[q])) {
        ++q;
        while (q<n && q-p<32 && (ascii_letter(t[q]) || (t[q]>='0' && t[q]<='9') || t[q]=='+' || t[q]=='.' || t[q]=='-')) ++q;
        if (q-p>=2 && q<n && t[q]==':') {
            ++q;
            while (q<n && t[q]!='>') { if (!spend(budget) || (unsigned char)t[q]<=32 || t[q]==127 || t[q]=='<') break; ++q; }
            if (q<n && t[q]=='>') { *kind=SB_INLINE_AUTOLINK; return q+1; }
        }
    }
    size_t email=email_end(r,p,budget);
    if (email) { *kind=SB_INLINE_AUTOLINK; return email; }
    if (t[p]=='!' || t[p]=='?') {
        const char *ending=NULL; size_t ending_length=0;
        if (n-p>=3 && !memcmp(t+p,"!--",3)) { p+=3; ending="-->"; ending_length=3; }
        else if (n-p>=8 && !memcmp(t+p,"![CDATA[",8)) { p+=8; ending="]]>"; ending_length=3; }
        else if (t[p]=='?') { ++p; ending="?>"; ending_length=2; }
        else if (p+1<n && t[p+1]>='A' && t[p+1]<='Z') { p+=2; ending=">"; ending_length=1; }
        if (!ending) return 0;
        while (p<n && ending_length<=n-p) {
            if (!spend(budget)) return 0;
            if (!memcmp(t+p,ending,ending_length)) { *kind=SB_INLINE_RAW; return p+ending_length; }
            ++p;
        }
        return 0;
    }
    bool closing=t[p]=='/'; if (closing) ++p;
    if (p>=n || !ascii_letter(t[p])) return 0;
    ++p;
    while (p<n && (ascii_letter(t[p]) || (t[p]>='0' && t[p]<='9') || t[p]=='-')) ++p;
    for (;;) {
        size_t before=p;
        while (p<n && raw_space(t[p])) { if (!spend(budget)) return 0; ++p; }
        if (p<n && t[p]=='>') { *kind=SB_INLINE_RAW; return p+1; }
        if (!closing && p+1<n && t[p]=='/' && t[p+1]=='>') { *kind=SB_INLINE_RAW; return p+2; }
        if (closing || p==before || p>=n || !name_start(t[p])) return 0;
        ++p; while (p<n && name_rest(t[p])) { if (!spend(budget)) return 0; ++p; }
        q=p; while (q<n && raw_space(t[q])) { if (!spend(budget)) return 0; ++q; }
        if (q>=n || t[q]!='=') continue;
        p=q+1; while (p<n && raw_space(t[p])) { if (!spend(budget)) return 0; ++p; }
        if (p>=n) return 0;
        if (t[p]=='\'' || t[p]=='"') {
            char quote=t[p++];
            while (p<n && t[p]!=quote) { if (!spend(budget)) return 0; ++p; }
            if (p==n) return 0; ++p;
        } else {
            before=p;
            while (p<n && !raw_space(t[p]) && t[p]!='>') {
                if (!spend(budget) || t[p]=='\'' || t[p]=='"' || t[p]=='=' || t[p]=='<' || t[p]=='`') return 0;
                ++p;
            }
            if (p==before) return 0;
        }
    }
}
static SBInlineToken *opaque_at(const SBInline *r,size_t p) {
    size_t lo=0,hi=r->opaque_count;
    while (lo<hi) { size_t mid=lo+(hi-lo)/2; if (r->opaque[mid].offset<p) lo=mid+1; else hi=mid; }
    return lo<r->opaque_count && r->opaque[lo].offset==p ? &r->opaque[lo] : NULL;
}
static SBStatus opaque_init(SBInline *r) {
    size_t budget=r->length*32+64;
    for (size_t p=0;p<r->length;) {
        if (r->text[p]=='\\' && p+1<r->length && punctuation((unsigned char)r->text[p+1])) { p+=2; continue; }
        size_t width=0,close=SIZE_MAX;
        if (r->text[p]=='`' && code_at(r,p,&width,&close)) { p=close!=SIZE_MAX ? close+width : p+width; continue; }
        if (r->text[p]!='<') { ++p; continue; }
        SBInlineKind kind=SB_INLINE_RAW; size_t end=angle_end(r,p,&budget,&kind);
        if (!budget) return sb_error(SB_LIMIT,"Markdown-Winkelstruktur ist zu komplex.");
        if (!end) { ++p; continue; }
        void *storage=r->opaque; SBStatus status=reserve(&storage,&r->opaque_capacity,r->opaque_count,sizeof(SBInlineToken)); r->opaque=storage;
        if (status.code!=SB_OK) return status;
        r->opaque[r->opaque_count++]=(SBInlineToken){.kind=kind,.offset=p,.length=end-p,
            .content=kind==SB_INLINE_RAW ? p : p+1,.content_length=kind==SB_INLINE_RAW ? end-p : end-p-2,
            .destination=kind==SB_INLINE_AUTOLINK ? p+1 : 0,.destination_length=kind==SB_INLINE_AUTOLINK ? end-p-2 : 0};
        if (kind==SB_INLINE_AUTOLINK) {
            /* URI schemes contain ':'. The email grammar permits no colon. */
            r->opaque[r->opaque_count-1].email=memchr(r->text+p+1,':',end-p-2)==NULL;
        }
        p=end;
    }
    return sb_ok();
}
