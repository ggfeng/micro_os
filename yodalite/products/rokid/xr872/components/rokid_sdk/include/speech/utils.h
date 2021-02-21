#ifndef _COMMON_UTILS_H_
#define _COMMON_UTILS_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Chunk {
    void *ptr;
    uint32_t size;
    uint32_t capacity;
} Chunk;

int  chunk_check_space(Chunk *c, int size);
void chunk_set_data(Chunk *c, const void *ptr, uint32_t size);
void chunk_append(Chunk *c, const void *ptr, uint32_t size);

void chunk_set_str(Chunk *c, const char *ptr);

void chunk_release(Chunk *c);

#ifdef __cplusplus
}
#endif

#endif
