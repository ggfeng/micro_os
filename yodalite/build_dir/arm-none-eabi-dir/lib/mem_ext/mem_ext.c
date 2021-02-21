#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/mem_ext/mem_ext.h>
#include <yodalite_autoconf.h>

static platform_mem_t g_platform_mem= {0};

int platform_mem_init(platform_mem_t *pmem)
{
	platform_mem_t *p = &g_platform_mem; 

    if(pmem != NULL){
     p->malloc =  pmem->malloc;
	 p->free   =  pmem->free;
	 p->calloc =  pmem->calloc;
	 p->realloc = pmem->realloc; 
	 p->mem_print = pmem->mem_print;
   }

   return  0;
}

void *malloc_ext(size_t size)
{
    void *data =  NULL;
	platform_mem_t *pmem = &g_platform_mem; 

	if(pmem->malloc != NULL)
		data = pmem->malloc(size);
	else
		data = malloc(size);

    return data;
}

void free_ext(void *ptr)
{
	platform_mem_t *pmem = &g_platform_mem; 

	if(pmem->free != NULL)
		pmem->free(ptr);
	else
		free(ptr);
}

void *calloc_ext(size_t nmemb, size_t size)
{
    void *data =  NULL;
	platform_mem_t *pmem = &g_platform_mem; 

	if(pmem->calloc)
		data = pmem->calloc(nmemb,size); 
	else
		data = calloc(nmemb, size);

    return data;
}

void *realloc_ext(void *ptr, size_t size)
{
    void *data = NULL;
	platform_mem_t *pmem = &g_platform_mem; 

	if(pmem->realloc != NULL)
		data = pmem->realloc(ptr,size);
	else
		data = realloc(ptr,size);

    return data;
}

void mem_print_ext(void)
{
	platform_mem_t *pmem = &g_platform_mem; 

	if(pmem->mem_print != NULL)
       pmem->mem_print();	
	else
	   printf("mem_print current not support\n");
}
