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

#include "lib/nettool/sntp_timer.h"
#if CONFIG_SNTP_TIMER == 1 
static int sntp_cmd(int argc,int8_t * const argv[])
{
   struct timeval tv;

   gettimeofday(&tv,NULL);
   return 0;
}


#define max_args      (1)
#define sntp_help     "sntp cmd help"

int cmd_net_sntp(void)
{
  YODALITE_REG_CMD(sntp,max_args,sntp_cmd,sntp_help);

  return 0;
}
#endif
