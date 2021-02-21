#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yodalite_autoconf.h>
#include <lib/libtest/lib_test.h>
#include <osal/semaphore.h>
#include <osal/pthread.h>
#include <osal/time.h>
#ifdef CONFIG_OSAL_UNISTD
#include <osal/unistd.h>
#endif
#include <osal/errno.h>
#include <lib/shell/shell.h>

#if (CONFIG_COND_TEST_ENABLE == 1)
static int mseconds = 10000;
static pthread_mutex_t test_mutex;
static pthread_cond_t  test_cond;
static int is_pthread_inited = 0;

static void * pthread_wait_func( void * arg)
{
   int iret;
   int ms  = (int)arg;
  struct timespec timeout;

  printf("pthread cond wait %d  ms\n",ms);

  pthread_detach(pthread_self());

  if (clock_gettime(CLOCK_REALTIME, &timeout) == -1) {
        printf("clock_gettime fail\n");
        return NULL;
   }

  timeout.tv_sec += ms/1000;
  timeout.tv_nsec += (ms %1000) *1000000;

  if(is_pthread_inited == 0)
  {
    pthread_cond_init(&test_cond,NULL);
    pthread_mutex_init(&test_mutex,NULL);
    is_pthread_inited = 1;
  }

  pthread_mutex_lock(&test_mutex);

 if((iret = pthread_cond_timedwait(&test_cond,&test_mutex,&timeout)) ==ETIMEDOUT)
 {
    printf("pthread_cond_timedwait timeout\n");
 }else{
    printf("pthread_cond_timedwait return ok\n");
 }

 printf("pthread_cond_timedwait() = %d\n",iret);
 pthread_mutex_unlock(&test_mutex);

 return NULL;
}

static int pthread_wait_cmd(int argc,int8_t * const argv[])
{
    pthread_t pthread;

    if(argc >= 2)
     mseconds = atoi(argv[1]);

   (void ) pthread_create( &pthread, NULL,pthread_wait_func,(void*)mseconds);

    return 0;
}

static int pthread_wakeup_cmd(int argc,int8_t * const argv[])
{
  if(is_pthread_inited == 0)
  {
    pthread_cond_init(&test_cond,NULL);
    pthread_mutex_init(&test_mutex,NULL);
    is_pthread_inited = 1;
  }

   pthread_mutex_lock(&test_mutex);
   pthread_cond_signal(&test_cond);
   pthread_mutex_unlock(&test_mutex);

   return 0;
}

#define wait_args      (2)
#define wait_help      "pthread_wait"

int cmd_pthread_wait(void)
{
  YODALITE_REG_CMD(pthread_wait,wait_args,pthread_wait_cmd,wait_help);

  return 0;
}

#define wake_args      (1)
#define wake_help  "pthread_wake"

int cmd_pthread_wake(void)
{
  YODALITE_REG_CMD(pthread_wake,wake_args,pthread_wakeup_cmd,wake_help);

  return 0;
}

#endif
