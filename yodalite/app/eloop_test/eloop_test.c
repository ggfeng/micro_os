#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <yodalite_autoconf.h>
#include <lib/eloop/eloop.h>
#include <lib/shell/shell.h>

static int cnts = 0;
static eloop_id_t id;

void timer_timeout_hander(void *ctx)
{
   printf("timer(%d) timeout %d times\n",(int)ctx,++cnts);
}


static int eloop_init_cmd(int argc,char * const argv[])
{
    static int ctx = 1;
    int expires_ms = 1000;
    int period_ms = 2000;

    if(argc >=2)
       expires_ms  = atoi(argv[1]);

    if(argc >= 3)
       period_ms = atoi(argv[2]);

    printf("expires_ms:%d,period_ms:%d\n",expires_ms,period_ms);
    
    id = eloop_timer_add(timer_timeout_hander,(void*)ctx,(unsigned int)expires_ms,(unsigned int)period_ms);

    if(id == NULL){
       printf("eloop_timer_add fail\n");
       return 0;
    }

   if(eloop_timer_start(id)){
       printf("eloop_timer_start fail\n");
       return 0;
    }

    ctx ++; 

   return 0;
}


#define max_init_args      (3)
#define init_help          "eloop_init"

int cmd_eloop_init(void)
{
  YODALITE_REG_CMD(eloop_init,max_init_args,eloop_init_cmd,init_help);
  return 0;
}

static int eloop_deinit_cmd(int argc,char * const argv[])
{
   if(eloop_timer_stop(id)){
     printf("eloop_timer_stop fail\n");
   }

   eloop_timer_delete(id);
   cnts = 0;

   return 0;
}

#define max_deinit_args       (1)
#define deinit_help          "eloop_deinit"

int cmd_eloop_deinit(void)
{
  YODALITE_REG_CMD(eloop_deinit,max_deinit_args,eloop_deinit_cmd,deinit_help);
  return 0;
}
