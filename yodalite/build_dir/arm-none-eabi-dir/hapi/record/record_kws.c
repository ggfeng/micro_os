
#include <yodalite_autoconf.h>

#include <osal/unistd.h>
#include <osal/pthread.h>
#include <osal/semaphore.h>
#include "freertos/FreeRTOS.h"
#include "flora_record.h"
#include "record_kws.h"


static void record_kws(void *argv)
{
    vTaskDelay(100/portTICK_PERIOD_MS);
    flora_record_init();
    vTaskDelay(800/portTICK_PERIOD_MS);
    record_init();
}

int record_kws_init(void)
{

  pthread_attr_t attr;
  struct sched_param  param; 
  pthread_t pthread_record_kws;

  if(pthread_attr_init(&attr) != 0)
  {
    printf("error:pthread_attr_init\n");
	return -1;
  }

  memset(&param,0,sizeof(struct sched_param));
  param.sched_priority = 5;
  pthread_attr_setschedparam(&attr,&param);
  //pthread_attr_setstacksize(&attr,4096);

  printf("%s create record_kws thread\n",__func__);
  (void) pthread_create(&pthread_record_kws,&attr,record_kws,NULL);

  pthread_detach(pthread_record_kws);
  pthread_attr_destroy(&attr);

  return 0;
}

