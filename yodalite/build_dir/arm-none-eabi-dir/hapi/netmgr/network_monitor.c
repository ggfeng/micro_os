#include <yodalite_autoconf.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <osal/pthread.h>
#include <lib/nettool/ping.h>
#include "common.h"
#include "wifi_event.h"
#include "network_switch.h"
#include "flora_upload.h"
#include "network_report.h"
#include "network_switch.h"
#include "network_mode.h"
#ifdef CONFIG_OSAL_UNISTD
#include <osal/unistd.h>
#endif

static pthread_t  g_pthread_network;
static int g_pthread_network_flag  = 0;
static int g_network_monitor_flag = 0;

static tNETWORK_STATUS g_network_status = {0};
static pthread_mutex_t report_mutex;

int monitor_get_status(tNETWORK_STATUS *status)
{
    pthread_mutex_lock(&report_mutex);
    memcpy(status,&g_network_status,sizeof(tNETWORK_STATUS));
    pthread_mutex_unlock(&report_mutex);

    return 0;
}

static int monitor_set_status(tNETWORK_STATUS *status)
{
    pthread_mutex_lock(&report_mutex);
    memcpy(&g_network_status,status,sizeof(tNETWORK_STATUS));
    pthread_mutex_unlock(&report_mutex);
    return 0;
}

void monitor_report_status(void)
{
   tNETWORK_STATUS status;

   memset(&status,0,sizeof(tNETWORK_STATUS));
   monitor_get_status(&status);
   report_network_status(&status);
}

int monitor_respond_status(flora_call_reply_t reply)
{
   tNETWORK_STATUS status;

   memset(&status,0,sizeof(tNETWORK_STATUS));
   monitor_get_status(&status);

   respond_network_status(reply,&status);
   return 0;
} 

#define SERVER_ADDRESS_PUB_DNS  "180.76.76.76"
#define SERVER_ADDRESS  "www.taobo.com" /** tmp for test **/

int ping_net_address(char* ip)
{
  int count = 3;
  int timeout = 3000;

  return ping(ip,timeout,count);
}
/*
  pthread_t pthread; 
 ( void ) pthread_create( &pthread, NULL,pthread_func,NULL);
*/

void*  network_monitor_link(void *data){
    int ret = 0;
    tNETWORK_STATUS status;
    static int unconnect_times = 0;

   g_pthread_network_flag  = 1;

 //  pthread_detach(pthread_self()); 

   while(g_pthread_network_flag)
   {

    if(get_network_mode() == WIFI){
      ret = ping_net_address(SERVER_ADDRESS_PUB_DNS);
      if (ret) {
        NM_LOGI("network unconnect %s %d\n", SERVER_ADDRESS_PUB_DNS, ret);
        ret = ping_net_address(SERVER_ADDRESS);
        if (ret) {
            NM_LOGI("network unconnect %s %d\n", SERVER_ADDRESS, ret);
            unconnect_times++;
            if (unconnect_times >= 3) {

                monitor_get_status(&status);
                status.state = REPORT_UNCONNECT;
                monitor_set_status(&status);

                report_network_status(&status);
            }
        } else {
//            NM_LOGI("network connect\n");
            unconnect_times = 0;

           monitor_get_status(&status);
           status.state = REPORT_CONNECT;
           monitor_set_status(&status);

            report_network_status(&status);
        }
     } else {
 //       NM_LOGI("network connect\n");
        unconnect_times = 0;

        monitor_get_status(&status);
        status.state = REPORT_CONNECT;
        monitor_set_status(&status);


        report_network_status(&status);
     }
   }
    sleep(3);
  }
}


int network_start_monitor(void)
{

 if(g_network_monitor_flag==0){

    pthread_attr_t attr;
  
	pthread_mutex_init(&report_mutex,NULL); 

  if (pthread_create(&g_pthread_network,NULL,network_monitor_link, NULL) != 0){
        NM_LOGE("Can't create thread wifi roam\n");
        return -1;
  }

    g_network_monitor_flag = 1;
 }
    return 0;
}

void network_stop_monitor(void)
{
  if(g_network_monitor_flag == 1){
    g_network_monitor_flag = 0;
    g_pthread_network_flag  = 0;
    pthread_join(g_pthread_network,NULL);
	pthread_mutex_destroy(&report_mutex);
  }
}
 

