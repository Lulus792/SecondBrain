#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "runtime_probe.h"
#include "platform.h"
#include "version.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#include <wchar.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <dlfcn.h>
#include <stdatomic.h>
#else
#include <link.h>
#endif

typedef struct {char *text; size_t length,capacity,count; bool failed;} Probe;
static void append(Probe *p,const char *text) {
    if(p->failed)return;
    size_t n=strlen(text);
    if(n>8*1024*1024 || p->length>8*1024*1024-n){p->failed=true;return;}
    if(p->length+n+1>p->capacity){
        size_t capacity=(p->length+n+1)*2; char *next=realloc(p->text,capacity);
        if(!next){p->failed=true;return;} p->text=next;p->capacity=capacity;
    }
    memcpy(p->text+p->length,text,n+1);p->length+=n;
}
static void quoted(Probe *p,const char *text) {
    if(!text || !sb_utf8_valid(text,strlen(text))){p->failed=true;return;}
    append(p,"\"");
    for(const unsigned char *c=(const unsigned char *)text;*c;++c){
        char escaped[7];
        if(*c=='"' || *c=='\\'){escaped[0]='\\';escaped[1]=(char)*c;escaped[2]=0;}
        else if(*c<32)snprintf(escaped,sizeof(escaped),"\\u%04x",(unsigned)*c);
        else {escaped[0]=(char)*c;escaped[1]=0;}
        append(p,escaped);
    }
    append(p,"\"");
}
static void module(Probe *p,const char *name) {
    if(!name || !*name)return; /* Linux describes the main executable with an empty name. */
    if(p->count++)append(p,",");
    quoted(p,name);
}
#ifdef __APPLE__
/* dyld invokes add callbacks for existing images and serializes loader callbacks.
   Keep the callback registered but inactive after this one diagnostic snapshot. */
static atomic_flag probe_lock=ATOMIC_FLAG_INIT;
static Probe *active_probe;
static void image_added(const struct mach_header *header,intptr_t slide) {
    (void)slide;
    while(atomic_flag_test_and_set_explicit(&probe_lock,memory_order_acquire)){}
    if(active_probe){Dl_info info;if(dladdr(header,&info))module(active_probe,info.dli_fname);else active_probe->failed=true;}
    atomic_flag_clear_explicit(&probe_lock,memory_order_release);
}
#elif !defined(_WIN32)
static int image_loaded(struct dl_phdr_info *info,size_t size,void *data) {
    (void)size;module(data,info->dlpi_name);return 0;
}
#endif

SBStatus sb_runtime_probe(const SBUi *ui,const char *path) {
    Probe p={0};append(&p,"{\"format\":1,\"build\":");quoted(&p,sb_build_info());
    append(&p,",\"renderer\":");quoted(&p,SDL_GetRendererName(ui->renderer));
    append(&p,",\"video_driver\":");quoted(&p,SDL_GetCurrentVideoDriver());
    append(&p,",\"rendered_frames\":4,\"loaded_modules\":[");
#ifdef _WIN32
    HANDLE snapshot=INVALID_HANDLE_VALUE;
    for(unsigned retry=0;retry<8;++retry){
        snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,GetCurrentProcessId());
        if(snapshot!=INVALID_HANDLE_VALUE || GetLastError()!=ERROR_BAD_LENGTH)break;
    }
    if(snapshot==INVALID_HANDLE_VALUE)p.failed=true;
    else {
        MODULEENTRY32W entry;memset(&entry,0,sizeof(entry));entry.dwSize=sizeof(entry);
        if(!Module32FirstW(snapshot,&entry))p.failed=true;
        else do {
            char name[SB_PATH_CAP];
            if(wcslen(entry.szExePath)>=MAX_PATH-1 || !WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,entry.szExePath,-1,name,sizeof(name),NULL,NULL))p.failed=true;
            else module(&p,name);
        }while(Module32NextW(snapshot,&entry));
        if(GetLastError()!=ERROR_NO_MORE_FILES)p.failed=true;
        CloseHandle(snapshot);
    }
#elif defined(__APPLE__)
    while(atomic_flag_test_and_set_explicit(&probe_lock,memory_order_acquire)){}
    active_probe=&p;atomic_flag_clear_explicit(&probe_lock,memory_order_release);
    _dyld_register_func_for_add_image(image_added);
    while(atomic_flag_test_and_set_explicit(&probe_lock,memory_order_acquire)){}
    active_probe=NULL;atomic_flag_clear_explicit(&probe_lock,memory_order_release);
#else
    if(dl_iterate_phdr(image_loaded,&p)!=0)p.failed=true;
#endif
    append(&p,"],\"scope\":\"normal-renderer-start-after-four-frames\"}\n");
    SBStatus result=p.failed || !p.count ? sb_error(SB_IO,"Laufzeitdiagnose konnte nicht vollständig erfasst werden.") : sb_fs_write_new(path,p.text,p.length);
    free(p.text);return result;
}
