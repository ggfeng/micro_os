#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "common.h"
#include "network_ctrl.h"
#include "wifi_hal.h"
#include "wifi_event.h"
#include "network_switch.h"
#include "network_mode.h"
#include <osal/pthread.h>
#include <lib/property/properties.h>
#include <yodalite_autoconf.h>
#ifdef CONFIG_OSAL_UNISTD
#include <osal/unistd.h>
#endif

static volatile uint32_t  g_network_mode=0; 

static pthread_mutex_t mode_mutex; 

uint32_t get_network_mode(void)
{
    uint32_t mode;
   
    pthread_mutex_lock(&mode_mutex);
    mode = g_network_mode;
    pthread_mutex_unlock(&mode_mutex);

   return mode;
}

static void set_wifi_station_mode(int start)
{
  if(start == START)
  {
    g_network_mode |= WIFI;
    g_network_mode &= ~WIFI_AP;
  }
  else
    g_network_mode &= ~WIFI;

}

static void set_wifi_ap_mode(int start)
{
  if(start == START)
  {
    g_network_mode &=~ WIFI;
    g_network_mode |= WIFI_AP;
  }
  else
    g_network_mode &= ~WIFI_AP;

}

int network_set_mode(uint32_t mode,uint32_t start)
{

  pthread_mutex_lock(&mode_mutex);

  switch(mode)
  {
    case WIFI:
         set_wifi_station_mode(start);
         break;
    case WIFI_AP:
         set_wifi_ap_mode(start);
         break;
     default:
        break;
  }

  pthread_mutex_unlock(&mode_mutex);

  return 0;     
}

static int network_get_mode(uint32_t *cur_mode)
{
  uint32_t mode = WIFI;
  char val[PROPERTY_VALUE_MAX]={0};  

  property_get(NETWORK_WIFI_MODE_KEY,val,"1");

  if(atoi(val))
    mode  |= WIFI;

  property_get(NETWORK_WIFI_AP_MODE_KEY,val,"0");

  if(atoi(val))
    mode  |= WIFI_AP;

  *cur_mode = mode;
 
  return 0;
}

static tNETWORK_MODE_INFO info = {0};

int wifi_init_start(void)
{
   int iret = -1;
   int iret1,iret2;

   tNETWORK_MODE_INFO *p_info = &info;

//   printf("%s->%d\n",__func__,__LINE__);
   memset(p_info,0,sizeof(tNETWORK_MODE_INFO));

   iret1 = property_get(NETWORK_WIFI_STATION_SSID,p_info->ssid,NETWORK_WIFI_DEFAULT_SSID);
   iret2 = property_get(NETWORK_WIFI_STATION_PSK,p_info->psk,NETWORK_WIFI_DEFAULT_PSK);

 //  printf("ssid:%s,psk:%s\n",p_info->ssid,p_info->psk);
   if(wifi_hal_start_station() == 0 && wifi_start_monitor()==0){
       NM_LOGW(" wifi station start ok\n");
       network_set_mode(WIFI,START);

       if(iret1 && iret2)
         wifi_hal_connect_station(p_info->ssid,p_info->psk);

         iret = 0;
    }else{
       NM_LOGW(" wifi station start fail\n");
    }

   return iret; 
}

static int wifi_ap_init_start(void)
{
     int iret = -1;
     tNETWORK_MODE_INFO *p_info = &info;
     memset(p_info,0,sizeof(tNETWORK_MODE_INFO));

     property_get(NETWORK_WIFI_AP_SSID,p_info->ssid,NETWORK_AP_DEFAULT_SSID);
     property_get(NETWORK_WIFI_AP_PSK,p_info->psk,NETWORK_AP_DEFAULT_PSK );
     property_get(NETWORK_WIFI_AP_HOSTIP,p_info->ip,NETWORK_AP_DEFAULT_IP);

    if(wifi_hal_start_ap(p_info->ssid,p_info->psk,p_info->ip)==0){
       network_set_mode(WIFI_AP,START);
       NM_LOGW("wifi ap start ok\n");
       iret = 0;
    }else{
       NM_LOGW("wifi ap start fail\n");
    }
    return iret;
}

int network_set_initmode(void)
{
  uint32_t mode;

  pthread_mutex_init(&mode_mutex,NULL);

  network_get_mode(&mode);

  NM_LOGW("network mode:%d\n",mode);

  if((mode&WIFI)  && !(mode &WIFI_AP) && is_mode_in_capacity(WIFI)==0){
    wifi_init_start();
  }
  else if(!(mode&WIFI)  && (mode&WIFI_AP) && is_mode_in_capacity(WIFI_AP)==0){
     wifi_ap_init_start();
  }

  NM_LOGI("current network work mode :%d\n",g_network_mode);

  return 0;
}
