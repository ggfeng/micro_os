#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yodalite_autoconf.h>
#include <hardware/wifi_hal.h>
#include "common.h"


static int g_start_ap_flag = 0;
static int g_start_station_flag = 0;
static struct hw_module_t *module = NULL;
static struct wifi_device_t *wifi_device = NULL;

static inline int wifi_device_open(const hw_module_t *module, struct wifi_device_t **device) 
{
  return module->methods->open(module, WIFI_HARDWARE_MODULE_ID,(struct hw_device_t **)device);
}

int wifi_hal_init(void)
{
     hw_get_module(WIFI_HAL_HW_ID, module);

    if(wifi_device_open(module,&wifi_device)){
       NM_LOGE("modem_dev_open fail\n");
       return -1;
    }

    return wifi_device->wifi_init();
}

void wifi_hal_exit(void)
{
   wifi_device->wifi_deinit();
   wifi_device->common.close(&wifi_device->common);
}

int wifi_hal_get_capacity(void)
{
  return 1;
}

int wifi_hal_start_station(void) 
{
   int iret = 0;

   if(g_start_station_flag == 0)
   {
     g_start_station_flag  =1;
     iret = wifi_device->start_station();
   }

   return iret;
}

void wifi_hal_stop_station(void) 
{
  if(g_start_station_flag)
  {
     wifi_device->stop_station();
     g_start_station_flag = 0;
  }
}

int wifi_hal_connect_station(char *ssid,char * passwd)
{
  int iret = 0;
  if(g_start_station_flag  ==1){
     iret = wifi_device->sta_connect(ssid,passwd);     
  }
  return iret;
}

int wifi_hal_disconnect_station(void)
{
  int iret = 0;
  if(g_start_station_flag  ==1){
     iret = wifi_device->sta_disconnect();     
  }

  return iret;
}

int wifi_hal_start_scan(void)
{
   return wifi_device->scan_start();
}

int wifi_hal_stop_scan(void)
{
   return wifi_device->scan_stop();
}

// need free by own
int wifi_hal_get_scan_result(ssid_t** list,int*num)
{
  int iret = -1;
  ssid_t *ssid_array = NULL;
  uint16_t ssid_cnt = 0;

  wifi_device->get_scan_list_cnt(&ssid_cnt);

  ssid_array = (ssid_t*)yodalite_malloc(sizeof(ssid_t) * ssid_cnt);

  if (!ssid_array) {
          printf("Malloc ssid_array failed!\n");
          return -1;
  }

  memset(ssid_array,0,sizeof(ssid_t) * ssid_cnt);

  if(wifi_device->get_scan_list(ssid_array, ssid_cnt) == 0){
     *list = ssid_array;
     *num = (int)ssid_cnt;
     iret = 0; 
  }

  return iret;
}

int wifi_hal_clean_scan_result(ssid_t *list)
{
  yodalite_free(list);
  return 0;
}

int wifi_get_status(wifi_hal_status_t *wifi_status)
{
  wifi_device->wifi_get_status(wifi_status);

  return 0;

//  return wifi_device->wifi_get_status(wifi_status);
}

int wifi_hal_start_ap(char *ssid,char *psk,char *ip)
{
  int iret = 0;

  if(g_start_ap_flag == 0){
     g_start_ap_flag = 1;
     iret =  wifi_device->start_ap(ssid,psk,ip);
  }

  return iret;
}

void wifi_hal_stop_ap(void)
{
  if(g_start_ap_flag){ 
    g_start_ap_flag = 0;
    wifi_device->stop_ap();
  }
}
