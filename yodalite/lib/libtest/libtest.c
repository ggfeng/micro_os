#include <yodalite_autoconf.h>

#ifndef CONFIG_LIBC_ENABLE
#include <stdio.h>
#else
#include <lib/libc/yodalite_libc.h>
#endif

#include  <lib/libtest/lib_test.h>

int yodalite_lib_test(void)
{

   char * buf;

   printf("+++++++yodalite lib test +++++++\r\n");
   printf("####CROSS TOOL=%sgcc####\r\n",CONFIG_TOOLPREFIX);

   return 0xf5;
}

int yodalite_lib_val(void)
{
  return YODALITE_TEST_VAL;
}
