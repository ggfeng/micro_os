#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/mem_ext/mem_ext.h>
#include <lib/shell/shell.h>

#define MALLOC_TEST_4K  (0x1000)
#define MALLOC_TEST_16K  (0x10000)
#define MALLOC_TEST_1M  (0x100000)

static int mem_ext_cmd(int argc,int8_t * const argv[])
{
   char *ptr;
   size_t ts,is,ds,es;

   printf("#################################################################################\n");
   printf("                                     malloc test                                 \n");
   printf("#################################################################################\n");
   mem_print_ext();
   printf("malloc 0x%x\n",MALLOC_TEST_4K);
   if((ptr=(char*)malloc(MALLOC_TEST_4K)) == NULL){
     printf("malloc 0x%x failed\n",MALLOC_TEST_4K);
     return -1;
   }
   mem_print_ext();
   printf("free 0x%x\n",MALLOC_TEST_4K);
   free(ptr);
   mem_print_ext();
   printf("#################################################################################\n");
   printf("#################################################################################\n");
   printf("                                 malloc_ext test                                 \n");
   printf("#################################################################################\n");
   mem_print_ext();
   printf("malloc_ext 0x%x\n",MALLOC_TEST_4K);
   if((ptr=(char*)malloc_ext(MALLOC_TEST_4K)) == NULL){
     printf("malloc_ext 0x%x failed\n",MALLOC_TEST_4K);
     return -1;
   }
   mem_print_ext();
   printf("free 0x%x\n",MALLOC_TEST_4K);
   free_ext(ptr);
   mem_print_ext();
   printf("#################################################################################\n");
   printf("#################################################################################\n");
   printf("                                 malloc_ext test                                 \n");
   printf("#################################################################################\n");
   mem_print_ext();
   printf("malloc_ext 0x%x\n",MALLOC_TEST_4K);
   if((ptr=(char*)malloc_ext(MALLOC_TEST_4K)) == NULL){
     printf("malloc_ext 0x%x failed\n",MALLOC_TEST_4K);
     return -1;
   }
   mem_print_ext();
   printf("free 0x%x\n",MALLOC_TEST_4K);
   free_ext(ptr);
   mem_print_ext();
   printf("#################################################################################\n");
   printf("#################################################################################\n");
   printf("                                 malloc_ext test                                 \n");
   printf("#################################################################################\n");
   mem_print_ext();
   printf("malloc_ext 0x%x\n",MALLOC_TEST_1M);
   if((ptr=(char*)malloc_ext(MALLOC_TEST_1M)) == NULL){
     printf("malloc_ext 0x%x failed\n",MALLOC_TEST_1M);
     return -1;
   }
   mem_print_ext();
   printf("free 0x%x\n",MALLOC_TEST_1M);
   free_ext(ptr);
   mem_print_ext();
   printf("#################################################################################\n");


   printf("#################################################################################\n");
   printf("                                 realloc_ext test                                 \n");
   printf("#################################################################################\n");

   mem_print_ext();
   printf("malloc_ext 0x%x\n",MALLOC_TEST_4K);
   if((ptr=(char*)malloc_ext(MALLOC_TEST_4K)) == NULL){
     printf("malloc_ext 0x%x failed\n",MALLOC_TEST_4K);
     return -1;
   }

   mem_print_ext();
   
   printf("realloc_ext 0x%x\n",MALLOC_TEST_16K);
   if((ptr=(char*)realloc_ext(ptr,MALLOC_TEST_16K)) == NULL){
     printf("realloc_ext 0x%x failed\n",MALLOC_TEST_16K);
     return -1;
   }
   mem_print_ext();

   printf("free 0x%x\n",MALLOC_TEST_16K);
   free_ext(ptr);
   mem_print_ext();
   printf("#################################################################################\n");

/*   
   ts = get_total_free_heap_size();
   is = get_internel_free_heap_size();
   es = get_externel_free_heap_size();
   ds = get_dram_free_heap_size();
   printf("total:0x%x,is(internel size):0x%x,es(externel size):0x%x,ds(dram size):0x%x\n",ts,is,es,ds);
 */


   return 0;
}

static int mem_free_cmd(int argc,int8_t * const argv[])
{
  mem_print_ext();
  return 0;
}

#define free_max_args      (1)
#define mem_free_help      "mem_free"

int cmd_mem_free(void)
{
  YODALITE_REG_CMD(mem_free,free_max_args,mem_free_cmd,mem_free_help);
  YODALITE_REG_CMD(m,free_max_args,mem_free_cmd,mem_free_help);
  return 0;
}

#define max_args      (1)
#define mem_help      "mem_ext"


int cmd_mem_ext(void)
{
  YODALITE_REG_CMD(mem_ext,max_args,mem_ext_cmd,mem_help);

  return 0;
}
