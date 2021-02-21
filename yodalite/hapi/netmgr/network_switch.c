#include <stdio.h>
#include <string.h>
#include "wifi_event.h"
#include "network_ctrl.h"
#include "network_switch.h"
#include "wifi_hal.h"
#include "common.h"

#define PTHREAD_WAIT_DELAY (20000)

extern int wifi_save_network(tNETWORK_MODE_INFO *pinfo);

static g_swith_flag = 0;
static pthread_mutex_t switch_mutex;

static int network_start_wifi_ap(tNETWORK_MODE_INFO * info) 
{
   int iret  = 0;

   info->reason = eREASON_NONE;

   if(wifi_hal_start_ap(info->ssid,info->psk,info->ip)){
      info->reason = eREASON_HOSTAPD_ENABLE_FAIL;
      iret = -1;
   }
    
   NM_LOGI("wifi ap start\n");

   return iret;
}

static int network_stop_wifi_ap(void)
{
  wifi_hal_stop_ap();
  return 0;
}


static int network_start_wifi(tNETWORK_MODE_INFO * info) 
{
    int iret = 0;
    tRESULT result;
    struct network_service *p_net  = get_network_service();

    info->reason = eREASON_NONE;

    if(wifi_hal_start_station()){
       NM_LOGE("wifi_hal_start_station fail\n");
       info->reason = eREASON_WPA_SUPLICANT_ENABLE_FAIL;
       return -1;
    }

    if(wifi_start_monitor()){
       NM_LOGE("wifi_start_monitor fail\n");
       info->reason = eREASON_WIFI_MONITOR_ENABLE_FAIL;
       return -1;
    }

   if(wifi_hal_connect_station(info->ssid,info->psk)){
        info->reason = eREASON_WIFI_NOT_CONNECT;
        NM_LOGW("wifi connect station fail\n");
        iret  = -1;
   }
   else
     wifi_save_network(info);

   return iret;
}

static int network_stop_wifi(void) 
{
    wifi_stop_monitor();
    wifi_hal_disconnect_station();
    wifi_hal_stop_station();

    return 0;
}

int is_mode_in_capacity(int mode)
{
    int iret  = -1;
    int capacity = get_network_capacity();

    switch(mode){
      case WIFI:
      case WIFI_AP:
       if(capacity & NET_CAPACITY_WIFI)
         iret  = 0;
        break;
      default:
        break; 
    }
     
    return iret; 
}
 
int  network_configure_net(int mode,int start,tNETWORK_MODE_INFO*info)
{
    int iret = 0;

    if(g_swith_flag == 0)
	{
       pthread_mutex_init(&switch_mutex,NULL);     	
	   g_swith_flag = 1;
	}
    if(info)
      info->reason  = 0;

    if(is_mode_in_capacity(mode)){
       NM_LOGE("mode(%d) out of capacity\n",mode);
       info->reason = eREASON_OUT_OF_CAPACITY;
       return -1;
    }

    pthread_mutex_lock(&switch_mutex);

     switch (mode)  {
        case WIFI:
            if(start == START)
            iret =  network_start_wifi(info); 
            else 
            iret = network_stop_wifi();
            break;
        case WIFI_AP:
          if(start == START)
            iret = network_start_wifi_ap(info);
          else  
            iret = network_stop_wifi_ap();
          break;
        default:
            break;
     }

   pthread_mutex_unlock(&switch_mutex);

   return iret;
}

