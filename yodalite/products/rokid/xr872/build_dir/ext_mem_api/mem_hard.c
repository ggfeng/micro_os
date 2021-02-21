#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/mem_ext/mem_ext.h>
#include <yodalite_autoconf.h>

#if CONFIG_MEM_EXT_ENABLE == 1

#define PSRAM_TOTAL_HEAP_SIZE (((uint32_t)(0x1400000) + (uint32_t)__PSRAM_LENGTH)- (size_t)__psram_end__ )

extern uint8_t __PSRAM_LENGTH[];
extern uint8_t __psram_end__[]; 
extern void *psram_malloc( size_t xWantedSize );
extern void psram_free( void *pv );
extern size_t psram_GetFreeHeapSize( void );
extern void heap_get_space(uint8_t **start, uint8_t **end, uint8_t **current);
extern int platform_mem_init(platform_mem_t *pmem);

static void *ext_mem_malloc(size_t size)
{
    void *data =  NULL;

    data = psram_malloc(size); 

    #if (CONFIG_MEM_EXT_DEBUG == 1)
      printf("malloc_ext at %p %x@%x\n",data,size,psram_GetFreeHeapSize());
    #endif
    return data;
}

static void ext_mem_free(void *ptr)
{
    psram_free(ptr);

    #if (CONFIG_MEM_EXT_DEBUG == 1)
      printf("free_ext at %p@%x\n",ptr,psram_GetFreeHeapSize());
    #endif
}

static void *ext_mem_calloc(size_t nmemb, size_t size)
{
    void *data =  NULL;

    data = psram_malloc(nmemb*size);

	memset(data,0,nmemb*size);
    #if (CONFIG_MEM_EXT_DEBUG == 1)
      printf("calloc_ext at %x@%x\n",data,nmemb*size,psram_GetFreeHeapSize());
    #endif

    return data;
}

static void *ext_mem_realloc(void *ptr, size_t size)
{
   uint8_t * psize; 
   void *p = NULL;
   size_t s;

   if(!ptr)
     return malloc_ext(size);

   if(size ==0){
	 free_ext(ptr);
	 return NULL;
   }

   psize = (uint8_t*)ptr -12;
   s = (*((size_t *)psize) &0x7FFFFFFF) - 0x10;
  
   if((p = malloc_ext(size)) == NULL){
		free_ext(ptr);
		return NULL; 
	 }
   
    memset(p,0,size);
    memcpy(p,ptr,(s<size)?s:size);

    free_ext(ptr);

    #if (CONFIG_MEM_EXT_DEBUG == 1)
     printf("realloc_ext at %p %x@%x\n",p,size,get_externel_free_heap_size());
    #endif

    return p;
}

static void ext_mem_print(void)
{
        uint8_t *start, *end, *current;
		size_t psram_size; 

        heap_get_space(&start, &end, &current);
        printf("heap total %u (%u KB), use %u (%u KB), free %u (%u KB)\n",
                          end - start,(end - start)/1024,current - start,(current - start)/1024,end - current,(end - current)/1024);

        psram_size = psram_GetFreeHeapSize();
        printf("psram total %u (%u KB), use %u (%u KB), free %u (%u KB)\n",PSRAM_TOTAL_HEAP_SIZE,(PSRAM_TOTAL_HEAP_SIZE)/1024,PSRAM_TOTAL_HEAP_SIZE-psram_size,(PSRAM_TOTAL_HEAP_SIZE-psram_size)/1024,psram_size,psram_size/1024);

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
