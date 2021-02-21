#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "yodalite_autoconf.h"
#include "lib/mem_ext/mem_ext.h"
#include "esp_heap_caps.h"


#if CONFIG_MEM_EXT_ENABLE == 1

extern uint32_t esp_get_free_heap_size( void );
extern int platform_mem_init(platform_mem_t *pmem);

static void *ext_mem_malloc(size_t size)
{
    void *data =  NULL;
    data = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    #if (CONFIG_MEM_EXT_DEBUG == 1)
      printf("malloc_ext at %p %x@%x\n",data,size,heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    #endif

    return data;
}

static void ext_mem_free(void *ptr)
{
    free(ptr);
}

static void *ext_mem_calloc(size_t nmemb, size_t size)
{
    void *data =  NULL;

    #if (CONFIG_MEM_EXT_DEBUG == 1)
      printf("calloc_ext %x@%x\n",size, heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    #endif

    data = heap_caps_malloc(nmemb * size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (data) {
        memset(data, 0, nmemb * size);
    }

    return data;
}

static void *ext_mem_realloc(void *ptr, size_t size)
{
    void *p = NULL;

    #if (CONFIG_MEM_EXT_DEBUG == 1)
      printf("realloc_ext %x@%x\n",size,heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    #endif

    p = heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    return p;
}

static void ext_mem_print(void)
{
    size_t ts,is,ds,es;
    ts =  esp_get_free_heap_size();
    is = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    ds = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    es = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    printf("MEM Total:%d K (0x%x) Bytes, Inter:%d K (0x%x) Bytes, Dram:%d K (0x%x) Bytes,SPIRAM:%d K (0x%x)Bytes\r\n",ts>>10,ts,is>>10,is,ds>>10,ds,es>>10,es);
}


int ext_mem_init(void){

   platform_mem_t mem,*pmem = &mem;

   memset(&mem,0,sizeof(platform_mem_t));
  
   pmem->malloc = ext_mem_malloc;
   pmem->free =   ext_mem_free;
   pmem->calloc  = ext_mem_calloc;
   pmem->realloc = ext_mem_realloc;
   pmem->mem_print = ext_mem_print;

   platform_mem_init(pmem);

   return 0;
}

#endif
