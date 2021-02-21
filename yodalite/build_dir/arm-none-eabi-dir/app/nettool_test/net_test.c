#include <yodalite_autoconf.h>

#ifndef CONFIG_LIBC_ENABLE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/unistd.h>
#else
#include <lib/libc/yodalite_libc.h>
#endif
#include "lib/libtest/lib_test.h"
#include "lib/shell/shell.h"

#ifdef CONFIG_OSAL_UNISTD
#include <osal/unistd.h>
#endif

extern int ping(char*addr,int timeout_ms,int count);

static int ping_cmd(int argc,int8_t * const argv[])
{

   int ms = 3000;
   int n = 0;

   int cnts = 3;
   char *addr = "192.168.4.2";

   if(argc >= 2 )
     addr= argv[1]; 

   if(argc >= 4){
    if(strcmp(argv[2],"-c") == 0)
       cnts = atoi(argv[3]);
   }

   if(argc >= 6){
     if(strcmp(argv[4],"-W") ==0)
        ms=atoi(argv[5]);
   }

   printf("ping %s -c %d -W %d\n",addr,cnts,ms);

   while(cnts)
   {
//     printf("ping %s -c %d -W %d\n",addr,cnts,ms);
     if(ping(addr,ms,3) == 0)
      printf("ping %s %d is alive\n",addr,n);
     else
      printf("ping %s %d is not alive\n",addr,n);

     n++;
     cnts --;

     sleep(1);
   }

   return 0;
}


#define max_args      (8)
#define ping_help  "ping ip -c count -W timeout"

int cmd_net_ping(void)
{
  YODALITE_REG_CMD(ping,max_args,ping_cmd,ping_help);

  return 0;
}
