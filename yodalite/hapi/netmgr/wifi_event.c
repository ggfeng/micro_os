#include <yodalite_autoconf.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <osal/pthread.h>

#include "wifi_event.h"
#include "flora_upload.h"
#include "ipc.h"
#include "network_switch.h"
#include "flora_upload.h"
#include "network_report.h"
#include "wifi_hal.h"
#ifdef CONFIG_OSAL_UNISTD
#include <osal/unistd.h>
#endif

static tWIFI_STATUS g_wifi_status = {0};
static int g_wifi_monitor_flag = 0;
static int g_pthread_wifi_flag = 0;
static pthread_t  g_pthread_wifi;

static pthread_mutex_t report_mutex;

static int wifi_event_get_status(tWIFI_STATUS *status)
{
    pthread_mutex_lock(&report_mutex);
    memcpy(status,&g_wifi_status,sizeof(tWIFI_STATUS));
    pthread_mutex_unlock(&report_mutex);

    return 0;
}

static int wifi_event_set_status(tWIFI_STATUS *status)
{
    pthread_mutex_lock(&report_mutex);
    memcpy(&g_wifi_status,status,sizeof(tWIFI_STATUS));
    pthread_mutex_unlock(&report_mutex);
    return 0;
}

int wifi_respond_status(flora_call_reply_t reply)
{
     tWIFI_STATUS status;

     memset(&status,0,sizeof(tWIFI_STATUS));
     wifi_event_get_status(&status);
     respond_wifi_status(reply,&status);

     return 0;
}

static void wifi_report_connect_state(int state)
{
     tWIFI_STATUS status;

  //   NM_LOGE("########p_net->wifi_state = %d\n",state);
     memset(&status,0,sizeof(tWIFI_STATUS));
     wifi_event_get_status(&status);
     status.state = state;
     wifi_event_set_status(&status);
     report_wifi_status(&status);
}

static void * wifi_monitor_status(void *arg) {
  wifi_hal_status_t status;
  g_pthread_wifi_flag = 1;

  pthread_detach(pthread_self()); 

  while(g_pthread_wifi_flag){
       memset(&status,0,sizeof(wifi_hal_status_t));
 
      if(wifi_get_status(&status) == 0 && status.mode == STA_MODE){
         if(status.ap_sta_status.sta_status.status == STA_CONNECTED) 
           wifi_report_connect_state(1);
         else
           wifi_report_connect_state(0);
      }
       
     sleep(3);
   }

  return NULL;
}

int wifi_start_monitor(void)
{
#if 0
   if(g_wifi_monitor_flag == 0){  
    pthread_mutex_init(&report_mutex,NULL);
   if(pthread_create(&g_pthread_wifi,NULL,wifi_monitor_status,NULL) != 0){
       NM_LOGE("Can't create thread wifi roam\n");
       return -1;
   }

    g_wifi_monitor_flag  = 1;
 }
#endif
 return 0;
}

void wifi_stop_monitor(void)
{
#if 0
  if(g_wifi_monitor_flag == 1){
    g_wifi_monitor_flag = 0;
    g_pthread_wifi_flag = 0;
	pthread_join(g_pthread_wifi,NULL);
    pthread_mutex_destroy(&report_mutex);
  }
#endif
}
 

