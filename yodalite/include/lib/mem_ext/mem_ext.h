#ifndef _MEM_EXT_H_
#define _MEM_EXT_H_

extern void *malloc_ext(size_t size);
extern void free_ext(void *ptr);
extern void *calloc_ext(size_t nmemb, size_t size);
extern void *realloc_ext(void *ptr, size_t size);
extern void mem_print_ext(void);

typedef struct platform_mem {
 void *(*malloc)(size_t size);
 void *(*calloc)(size_t nmemb, size_t size);
 void *(*realloc)(void *ptr, size_t size);
 void (*free)(void *ptr);
 void (*mem_print)(void);
}platform_mem_t; 

#endif
