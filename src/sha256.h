#ifndef SB_SHA256_H
#define SB_SHA256_H
#include <stddef.h>
#include <stdint.h>
typedef struct { uint32_t state[8]; uint64_t bytes; unsigned char block[64]; size_t used; } SBSha256;
void sb_sha256_init(SBSha256 *hash);
void sb_sha256_update(SBSha256 *hash,const void *data,size_t length);
void sb_sha256_finish(SBSha256 *hash,unsigned char digest[32]);
#endif
