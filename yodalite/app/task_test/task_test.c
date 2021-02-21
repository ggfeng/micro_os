
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/shell/shell.h>
#define max_args      (1)
#define task_help      "task_test"


static int task_test_cmd(int argc,int8_t * const argv[])
{
  char buf[1024];  
  vTaskGetRunTimeStats(buf);           
  printf("Run Time Stats:\nTask Name    Time    Percent\n%s\n", buf);
  return 0;
}

int cmd_task_test(void)
{
  YODALITE_REG_CMD(p,max_args,task_test_cmd,task_help);
  return 0;
}
