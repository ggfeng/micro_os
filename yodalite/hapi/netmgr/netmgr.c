#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <lib/property/properties.h>
#include "common.h"
#include "wifi_event.h"
#include "network_switch.h"
#include "network_ctrl.h"
#include "ipc.h"
#include "wifi_hal.h"
#include "network_monitor.h"
#include "network_mode.h"

static int wifi_hal_flag = 0;
static uint32_t g_capacity = NET_DEFAULT_CAPACITY; 
static struct network_service g_network_service;
int nm_log_level = LEVEL_OVER_LOGN;

uint32_t get_network_capacity(void)
{
   return g_capacity; 
}

struct network_service *get_network_service(void)
{
   return &g_network_service;
}

static int nm_set_bug_level(void)
{
  char value[PROPERTY_VALUE_MAX] = {0};
  memset(value,0,PROPERTY_VALUE_MAX);
  property_get("persist.rokid.debug.netmanager", value, "0");
  if (atoi(value) > 0){
    nm_log_level = atoi(value);
  }

  return 0;
}

static int network_parm_init(void) 
{
    struct network_service *p_net = get_network_service();

    memset(p_net,0,sizeof(struct network_service));
    p_net->wifi_state = WIFI_STATION_UNCONNECTED;

    return 0;
}

int hapi_netmgr_init(void)
{
   if(property_init()){
      NM_LOGE("property_init() fail\n");
      return -1;
   }

   nm_set_bug_level();
   
   if (network_ipc_init() < 0){
        NM_LOGE("ipc init fail\n");
        return -1;
    }

   if(wifi_hal_init() == 0){
      wifi_hal_flag = 1;
      g_capacity |= get_wifi_capacity();
   }
   else
    NM_LOGW("wifi_hal_init fail\n");
   
   NM_LOGI("Network Capacity is:%d \n",g_capacity);

   if (network_parm_init()< 0){
        NM_LOGE("network timer init fail\n");
        return -1;
   }

   if(network_set_initmode()){
     NM_LOGE("network_set_initmode fail\n");
   }

   network_start_monitor();

   return 0;
}

int hapi_netmgr_exit(void){

   wifi_stop_monitor();
   network_stop_monitor();

   if(wifi_hal_flag)
      wifi_hal_exit();

    network_ipc_exit();
    property_deinit();
}
