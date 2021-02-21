#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yodalite_autoconf.h>
#include <osal/sched.h>
#include <osal/pthread.h>
#ifdef CONFIG_OSAL_UNISTD
#include <osal/unistd.h>
#endif
#include <osal/errno.h>
#include <lib/shell/shell.h>

static int total=0;
static pthread_mutex_t test_mutex;

#if (CONFIG_PTHREAD_TEST_ENABLE ==1) 
// int freertos_pthread_attr_init( pthread_attr_t * attr )

static void * pthread_func( void * arg)
{
  int i = 10;

  pthread_detach(pthread_self()); 

  printf("pthread total run %d times\n",i);

  while(i >=0)
  {
	 pthread_mutex_lock(&test_mutex);
     printf("%p:pthread run %d times,all total:%d\n",pthread_self(),i--,++total);
	 pthread_mutex_unlock(&test_mutex);

     sleep(1);
  }
  return NULL;
}

static int pthread_test_cmd(int argc,int8_t * const argv[])
{
  pthread_t pthread; 
  pthread_attr_t attr;
  struct sched_param  param; 

  pthread_mutex_init(&test_mutex,NULL);

  printf("%s->%d:mutex:%p\n",__func__,__LINE__,(int *)test_mutex);

  if(pthread_attr_init(&attr) != 0)
  {
    printf("error:pthread_attr_init\n");
	return -1;
  }

  memset(&param,0,sizeof(struct sched_param));

  param.sched_priority = 6;

  pthread_attr_setschedparam(&attr,&param);
  pthread_attr_setstacksize(&attr,4096);


  (void) pthread_create(&pthread,&attr,pthread_func,NULL);

  pthread_attr_destroy(&attr);

  if(pthread_attr_init(&attr) != 0)
  {
    printf("error:pthread_attr_init\n");
	return -1;
  }

  memset(&param,0,sizeof(struct sched_param));
  param.sched_priority = 5;
  pthread_attr_setschedparam(&attr,&param);
  pthread_attr_setstacksize(&attr,2048);
  (void) pthread_create(&pthread,&attr,pthread_func,NULL);
  pthread_attr_destroy(&attr);

   return 0;
}

#define max_args      (2)
#define pthread_help  "pthread test"

int cmd_pthread_test(void)
{
  YODALITE_REG_CMD(pthread,max_args,pthread_test_cmd,pthread_help);

  return 0;
}

#endif
