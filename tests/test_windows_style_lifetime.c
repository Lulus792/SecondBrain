/* Exercise the current adapter with allocator-owned Win32 scheme names.
   Calls do not mutate OS settings; only three query replies are intercepted. */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system_style.h"
static unsigned allocated,released,queries,failures;static HLOCAL held;
static BOOL WINAPI query(UINT action,UINT size,PVOID value,UINT flags){
    (void)flags;++queries;
    if(action==SPI_GETCLIENTAREAANIMATION){*(BOOL *)value=TRUE;return TRUE;}
    if(action==SPI_GETDISABLEOVERLAPPEDCONTENT){*(BOOL *)value=FALSE;return TRUE;}
    if(action!=SPI_GETHIGHCONTRAST || size!=sizeof(HIGHCONTRASTW) || held){++failures;return FALSE;}
    HIGHCONTRASTW *contrast=value;if(contrast->cbSize!=sizeof(*contrast)){++failures;return FALSE;}
    const WCHAR name[]=L"Owned scheme fixture";held=LocalAlloc(LMEM_FIXED,sizeof(name));if(!held){++failures;return FALSE;}
    memcpy(held,name,sizeof(name));++allocated;contrast->dwFlags=HCF_AVAILABLE;contrast->lpszDefaultScheme=held;return TRUE;
}
static HLOCAL WINAPI release(HLOCAL value){
    if(!held || value!=held){++failures;return value;}
    HLOCAL result=LocalFree(value);if(!result){held=NULL;++released;}else ++failures;return result;
}
#define SystemParametersInfoW query
#define LocalFree release
#include "../app/system_style.c"
int main(void){
    for(unsigned i=0;i<1024;++i){SBSystemStyle s=sb_system_style_read_native();
        const unsigned queried=SB_SYS_MOTION|SB_SYS_TRANSPARENCY|SB_SYS_CONTRAST;
        if((s.known&queried)!=queried || s.contrast || s.motion || s.solid || held || allocated!=released || failures){
            fprintf(stderr,"Win32 scheme lifetime failed at query %u: allocated=%u released=%u held=%d failures=%u\n",i,allocated,released,held!=NULL,failures);
            if(held)LocalFree(held);return 1;}}
    if(queries!=3072 || allocated!=1024 || released!=1024)return 1;
    puts("1024 allocator-owned Win32 scheme names released; 3072 read-only query replies checked.");return 0;
}
