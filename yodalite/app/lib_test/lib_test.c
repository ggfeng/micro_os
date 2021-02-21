#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yodalite_autoconf.h>
#include <lib/libtest/lib_test.h>
#include <osal/errno.h>
#include <lib/shell/shell.h>

int lib_test(void)
{
   return yodalite_lib_test();
}

#define max_args      (1)
#define lib_help          "lib test"

int cmd_lib_test(void)
{
  YODALITE_REG_CMD(lib_test,max_args,lib_test,lib_help);

  return 0;
}
