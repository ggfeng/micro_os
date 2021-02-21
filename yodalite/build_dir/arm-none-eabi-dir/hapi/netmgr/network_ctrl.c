#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <lib/property/properties.h>
#include <lib/cjson/cJSON.h>
#include <lib/eloop/eloop.h>
#include "common.h"
#include "wifi_event.h"
#include "ipc.h"
#include "network_switch.h"
#include "wifi_hal.h"
#include "network_mode.h"
#include "network_monitor.h"
#include "wifi_event.h"
#include "flora_upload.h"
//#include <resolv.h>


#define CMD_DEVICE_KEY  "device"
#define CMD_COMMAND_KEY "command"

#define NET_WIFI_S      ("wifi")
#define NET_WIFI_AP_S   ("wifi_ap")

#define WIFI_SSID_KEY          ("SSID")
#define WIFI_PASSWD_KEY        ("PASSWD")
#define WIFI_AP_IP_KEY         ("IP") 
#define WIFI_AP_TIMEOUT        ("TIMEOUT")

#define NET_PARMS_KEY   "params"
#define NET_MAX_CAPACITY       (4)

#define CMD_MAX_LEN (64)

#define AP_DEFAULT_TIMEOUT (5000)

tNETWORK_MODE_INFO  g_ap_info = {0};

typedef struct ctrl_cmd_format
{
   char *device;
   char *command;
   int (*respond_func)(flora_call_reply_t reply,void *data);
}tCTRL_CMD_FMT;

typedef struct ctr_cmd_string
{
   char device[CMD_MAX_LEN];
   char command[CMD_MAX_LEN];
}tCTRL_CMD_STR;

static int g_ap_timeout_flag = 0;

static const char *cmd_fail_reason[] = 
{
  "NONE",
  "WIFI_NOT_CONNECT",
  "WIFI_NOT_FOUND",
  "WIFI_WRONG_KEY",
  "WIFI_TIMEOUT",
  "WIFI_CFGNET_TIMEOUT",
  "NETWORK_OUT_OF_CAPACITY",
  "WIFI_WPA_SUPLICANT_ENABLE_FAIL",
  "WIFI_HOSTAPD_ENABLE_FAIL",
  "COMMAND_FORMAT_ERROR",
  "COMMAND_RESPOND_ERROR",
  "WIFI_MONITOR_ENABLE_FAIL",
  "WIFI_SSID_PSK_SET_FAIL"
};

extern void dns_init(void);

static int wifi_enable_all_network(void)
{
   tNETWORK_MODE_INFO info,*p_info = &info;
   memset(p_info,0,sizeof(tNETWORK_MODE_INFO));

   property_get(NETWORK_WIFI_STATION_SSID,p_info->ssid,NETWORK_WIFI_DEFAULT_SSID);
   property_get(NETWORK_WIFI_STATION_PSK,p_info->psk,NETWORK_WIFI_DEFAULT_PSK);
  
   wifi_hal_connect_station(p_info->ssid,p_info->psk);

   return 0;
}

static int wifi_disable_all_network(void)
{
   wifi_hal_disconnect_station();
   return 0;
}

int wifi_save_network(tNETWORK_MODE_INFO *pinfo)
{
  property_set(NETWORK_WIFI_STATION_SSID,pinfo->ssid);
  property_set(NETWORK_WIFI_STATION_PSK,pinfo->psk);
  return 0;
}

static int wifi_remove_network(tNETWORK_MODE_INFO *pinfo)
{
  property_remove(NETWORK_WIFI_STATION_SSID);
  property_remove(NETWORK_WIFI_STATION_PSK);
  return 0;
}

static int reset_dns(void)
{
  dns_init();
  return 0;
}

static int wifi_get_config_network(int *num)
{
  int iret1,iret2;
  char psk[64];
  char ssid[128];

  memset(psk,0,64);
  memset(ssid,0,128);

  iret1 = property_get(NETWORK_WIFI_STATION_SSID,ssid,NETWORK_WIFI_DEFAULT_SSID);
  iret2 = property_get(NETWORK_WIFI_STATION_PSK,psk,NETWORK_WIFI_DEFAULT_PSK);
   
  if(iret1 == 1 && iret2 == 1)
    *num=1;
  else
    *num=0;

  return 0;
}

static int netmanger_response_command(flora_call_reply_t reply, int state,int reason) 
{
    char *result = NULL;
    char *respond = NULL;
    cJSON* obj =  cJSON_CreateObject(); 

    switch (state){
      case COMMAND_OK:
        result = "OK";
        break;
      case COMMAND_NOK:
        result = "NOK";
        break;
      default:
        break;
    }

    cJSON_AddStringToObject(obj,"result",result);

   if(state == COMMAND_NOK)
      cJSON_AddStringToObject(obj,"reason",cmd_fail_reason[reason]);

    respond = cJSON_Print(obj);
    network_ipc_return(reply,(uint8_t *)respond);
    yodalite_free((void*)respond);
    cJSON_Delete(obj);

    return 0;
}


int get_wifi_capacity(void)
{
  int capacity = 0;
  if(wifi_hal_get_capacity())
     capacity |= NET_CAPACITY_WIFI;
 
   return capacity;
}

static int netmanager_get_net_capacity(flora_call_reply_t reply,void *data)
{
    const char * respond;
    uint32_t capacity;

    cJSON*root = cJSON_CreateObject();
    cJSON*capacity_array  =cJSON_CreateArray();

    NM_LOGI("%s->%d\n",__func__,__LINE__);

    capacity = get_network_capacity();

    cJSON_AddStringToObject(root,"result","OK");

    if(capacity& NET_CAPACITY_WIFI){
       //cJSON_AddArrayToObject(capacity_array,NET_WIFI_S);
       cJSON_AddItemToArray(capacity_array,cJSON_CreateString(NET_WIFI_S));
    }

    cJSON_AddItemToObject(root,"net_capacities",capacity_array);
     
    respond = cJSON_Print(root);

    network_ipc_return(reply,(uint8_t *)respond);

    yodalite_free((void*)respond);
  //  cJSON_Delete(capacity_array);
    cJSON_Delete(root);

    return 0;
}

static int netmanager_get_net_mode(flora_call_reply_t reply,void *data)
{
    const char * respond;
    uint32_t mode;

    cJSON*root = cJSON_CreateObject();
    cJSON*mode_array  =cJSON_CreateArray();

    NM_LOGI("%s->%d\n",__func__,__LINE__);

    mode = get_network_mode();

    cJSON_AddStringToObject(root,"result","OK");

    if(mode & WIFI){
   //    cJSON_AddArrayToObject(mode_array,NET_WIFI_S);
       cJSON_AddItemToArray(mode_array,cJSON_CreateString(NET_WIFI_S));
    }

    if(mode & WIFI_AP){
   //  cJSON_AddArrayToObject(mode_array,NET_WIFI_AP_S);
       cJSON_AddItemToArray(mode_array,cJSON_CreateString(NET_WIFI_AP_S));
    }

    cJSON_AddItemToObject(root,"net_modes",mode_array);
     
    respond = cJSON_Print(root);

    network_ipc_return(reply,(uint8_t *)respond);

    yodalite_free((void*)respond);
  //cJSON_Delete(mode_array);
    cJSON_Delete(root);

    return 0;
}

int get_wifi_station_info(cJSON*base,tNETWORK_MODE_INFO *mode)
{
   const char *str;

   cJSON*obj = cJSON_CreateObject();
   cJSON*elem = cJSON_CreateObject();;

   NM_LOGI("%s->%d\n",__func__,__LINE__);

   if ((obj=cJSON_GetObjectItem(base,NET_PARMS_KEY)) == NULL){
     NM_LOGE("%s error\n",NET_PARMS_KEY);
     return -1;
   }

   if ((elem=cJSON_GetObjectItem(obj,WIFI_SSID_KEY)) == NULL){
      NM_LOGE("%s error\n",WIFI_SSID_KEY);
      return -1;
   }

   if((str= cJSON_GetStringValue(elem)) == NULL){
      NM_LOGE("SSID:%s error\n",WIFI_SSID_KEY);
     return -1;
   }

   strncpy(mode->ssid,str,MAX_SIZE-1);

   if ((elem=cJSON_GetObjectItem(obj,WIFI_PASSWD_KEY)) == NULL){
      NM_LOGE("%s error\n",WIFI_PASSWD_KEY);
      return -1;
   }

   if((str= cJSON_GetStringValue(elem)) == NULL)
       strncpy(mode->psk,"",MAX_SIZE-1);
   else
     strncpy(mode->psk,str,MAX_SIZE-1);

   return 0;
}

int  get_wifi_ap_info(cJSON*base,tNETWORK_MODE_INFO*mode)
{
   const char *str;
   cJSON*obj = cJSON_CreateObject();
   cJSON*elem = cJSON_CreateObject();;
   
   if ((obj=cJSON_GetObjectItem(base,NET_PARMS_KEY)) == NULL){
     NM_LOGE("%s error\n",NET_PARMS_KEY);
     return -1;
   }

   if ((elem=cJSON_GetObjectItem(obj,WIFI_SSID_KEY)) == NULL){
      NM_LOGE("%s error\n",WIFI_SSID_KEY);
      return -1;
   }

   if((str= cJSON_GetStringValue(elem)) == NULL){
      NM_LOGE("SSID:%s error\n",WIFI_SSID_KEY);
     return -1;
   }

    strncpy(mode->ssid,str,MAX_SIZE-1);


   if ((elem=cJSON_GetObjectItem(obj,WIFI_PASSWD_KEY)) == NULL){
      NM_LOGE("%s error\n",WIFI_PASSWD_KEY);
      return -1;
   }

   if((str= cJSON_GetStringValue(elem)) == NULL)
       strncpy(mode->psk,"",MAX_SIZE-1);
   else
     strncpy(mode->psk,str,MAX_SIZE-1);


   if ((elem=cJSON_GetObjectItem(obj,WIFI_AP_IP_KEY)) == NULL){
      NM_LOGE("%s error\n",WIFI_AP_IP_KEY);
      return -1;
   }

   if((str= cJSON_GetStringValue(elem)) == NULL){
      NM_LOGE("%s error\n",WIFI_AP_IP_KEY);
      return -1;
   }

   strncpy(mode->ip,str,MAX_SIZE-1);

   if ((elem=cJSON_GetObjectItem(obj,WIFI_AP_TIMEOUT)) == NULL){
      NM_LOGE("%s error\n",WIFI_AP_TIMEOUT);
      return -1;
   }

   if((str= cJSON_GetStringValue(elem)) == NULL){
      mode->timeout = AP_DEFAULT_TIMEOUT; /** 5 s */
   }

   mode->timeout = strtol(cJSON_GetStringValue(elem),NULL,0);

   return 0;
}

static int netmanager_start_wifi_scan(flora_call_reply_t reply,void *data)
{

  int reason = eREASON_NONE;
  int state = COMMAND_OK;

  struct network_service *p_net = get_network_service();

  if(get_network_mode() & WIFI_AP){
    network_configure_net(WIFI_AP,STOP,NULL);
    network_set_mode(WIFI_AP,STOP);
    p_net->wifi_state = WIFI_AP_CLOSE; 
  }

  wifi_hal_start_station(); 

  if(wifi_hal_start_scan()){
     state = COMMAND_NOK;
     reason = eREASON_COMMAND_RESPOND_ERROR;
  }

  p_net->wifi_state = WIFI_STATION_PRE_JOIN;

  netmanger_response_command(reply,state,reason); 
  return 0 ;
}

static int netmanager_stop_wifi_scan(flora_call_reply_t reply,void *data)
{

  int reason = eREASON_NONE;
  int state = COMMAND_OK;
  struct network_service *p_net = get_network_service();

  NM_LOGI("%s->%d\n",__func__,__LINE__);

  if(wifi_hal_stop_scan()){
     state = COMMAND_NOK;
     reason = eREASON_COMMAND_RESPOND_ERROR;
   }

   netmanger_response_command(reply,COMMAND_OK,eREASON_NONE); 
   return 0;
}


static int netmanager_start_wifi_station(flora_call_reply_t reply,void *data)
{
    int iret = -1;
    int reason = eREASON_NONE;
    int state = COMMAND_OK;
    cJSON *base  = (cJSON*)data;
    tNETWORK_MODE_INFO info,*p_info = &info;
    struct network_service *p_net = get_network_service();

    NM_LOGI("%s->%d\n",__func__,__LINE__);
    memset(p_info,0,sizeof(tNETWORK_MODE_INFO));

   if(get_network_mode() & WIFI_AP){
     if(g_ap_timeout_flag == 1){
         eloop_timer_stop(p_net->wifi_ap_timeout);
         eloop_timer_delete(p_net->wifi_ap_timeout);
         g_ap_timeout_flag  =  0;
      }

     network_configure_net(WIFI_AP,STOP,NULL);
     network_set_mode(WIFI_AP,STOP);
     p_net->wifi_state = WIFI_AP_CLOSE;
   }

   memset(p_info,0,sizeof(tNETWORK_MODE_INFO));

    p_net->wifi_state = WIFI_STATION_JOINING;

    if(get_wifi_station_info(base,p_info)){
       state = COMMAND_NOK;
       reason = eREASON_COMMAND_FORMAT_ERROR;
       NM_LOGE("get_wifi_station_info() fail\n");
       iret =-1;
       goto ret;
    }

    if(network_configure_net(WIFI,START,p_info)){
       reason  = p_info->reason;
       state  = COMMAND_NOK;
       wifi_enable_all_network();
       p_net->wifi_state = WIFI_STATION_UNCONNECTED;
    }
    else{
      network_set_mode(WIFI,START);
      p_net->wifi_state = WIFI_STATION_CONNECTED;
      strncpy(p_net->ssid,p_info->ssid,128);
      strncpy(p_net->psk,p_info->psk,64);
      wifi_save_network(p_info);
      NM_LOGW("----ssid:%s connect----\n",p_net->ssid);
    }
ret:
   netmanger_response_command(reply,state,reason);
   return iret;
}


static int netmanager_stop_wifi_station(flora_call_reply_t  reply,void *data)
{
    struct network_service *p_net = get_network_service();

    NM_LOGI("%s->%d\n",__func__,__LINE__);
    p_net->wifi_state = WIFI_STATE_CLOSE;
    network_configure_net(WIFI,STOP,NULL);
    network_set_mode(WIFI,STOP);
    netmanger_response_command(reply,COMMAND_OK,eREASON_NONE); 

    return 0;
}

static int netmanager_get_wifi_scan_result(flora_call_reply_t reply,void *data) 
{
    int num;
    int i = 0;
    cJSON*item;
    ssid_t*list;
    char *buf = NULL;
    cJSON*root = cJSON_CreateObject();
    cJSON*wifi_list  = cJSON_CreateArray();;
    
    NM_LOGI("%s->%d\n",__func__,__LINE__);

    cJSON_AddStringToObject(root,"result","OK");

   if(wifi_hal_get_scan_result(&list,&num)){
      NM_LOGE("error:wifi_hal_get_scan_result()\n");
     cJSON_Delete(wifi_list);
     cJSON_Delete(root);
      return -1;
    }

   for (i = 0; i < num; i++){
        item = cJSON_CreateObject();
        cJSON_AddStringToObject(item,"SSID",list[i].ssid);
        cJSON_AddNumberToObject(item,"SIGNAL",list[i].signal);
        cJSON_AddItemToArray(wifi_list,item);
    }
    
    wifi_hal_clean_scan_result(list);

    cJSON_AddItemToObject(root,"wifilist", wifi_list);

    buf = cJSON_Print(root);

    network_ipc_return(reply,(uint8_t *)buf);

    yodalite_free((void*)buf);
  //cJSON_Delete(wifi_list);
    cJSON_Delete(root);

    return 0;
}

static void wifi_change_to_station(void *data) 
{
   struct network_service *p_net = get_network_service();

    NM_LOGW("wifi change to station");

    p_net->wifi_state = WIFI_AP_CLOSE;
    network_configure_net(WIFI_AP,STOP,NULL);
    network_set_mode(WIFI_AP,STOP);
/*
    wifi_hal_start_station();
    wifi_start_monitor();
*/
    g_ap_timeout_flag  =  0;
    wifi_init_start();
}

static int netmanager_start_wifi_ap(flora_call_reply_t reply,void *data)
{
    int iret = 0;
    int reason = eREASON_NONE;
    int state = COMMAND_OK;
    struct network_service *p_net = get_network_service();
    uint32_t mode = get_network_mode(); 

    cJSON *base  = (cJSON*)data;
    tNETWORK_MODE_INFO info, *p_info = &info;

    network_configure_net(WIFI,STOP,NULL);
    network_set_mode(WIFI,STOP);
    p_net->wifi_state = WIFI_STATE_CLOSE;
  
    if(mode & WIFI_AP){
      network_configure_net(WIFI_AP,STOP,NULL);
      network_set_mode(WIFI_AP,STOP);
      p_net->wifi_state = WIFI_AP_CLOSE;
    }

    memset(p_info,0,sizeof(tNETWORK_MODE_INFO));

    if(get_wifi_ap_info(base,p_info)){
       state = COMMAND_NOK;
       reason = eREASON_COMMAND_FORMAT_ERROR;
       NM_LOGE("get_wifi_ap_info()\n");
       iret =-1;
       goto ret;
    }

   if(network_configure_net(WIFI_AP,START,p_info)){
     reason  = p_info->reason;
     state  = COMMAND_NOK;
     p_net->wifi_state = WIFI_AP_UNCONNECTED;
   }
   else{
     p_net->wifi_state = WIFI_AP_CONNECTED;
     memcpy(&g_ap_info,p_info,sizeof(tNETWORK_MODE_INFO));
     network_set_mode(WIFI_AP,START);
   }

  if(p_info->timeout > 0 && g_ap_timeout_flag  ==0)
  {
    g_ap_timeout_flag = 1;
    p_net->wifi_ap_timeout = eloop_timer_add(wifi_change_to_station,NULL,p_info->timeout,0);
    eloop_timer_start(p_net->wifi_ap_timeout);
  }
   
ret:
   netmanger_response_command(reply,state,reason); 
   return iret;
}

static int netmanager_stop_wifi_ap(flora_call_reply_t reply,void *data)
{
   int reason = eREASON_NONE;
   int state = COMMAND_OK;
   struct network_service *p_net = get_network_service();

    NM_LOGI("%s->%d\n",__func__,__LINE__);

    p_net->wifi_state = WIFI_AP_CLOSE;

    network_configure_net(WIFI_AP,STOP,NULL);
    network_set_mode(WIFI_AP,STOP);

    netmanger_response_command(reply,state,reason); 

    return  0;
}


static int netmanager_get_network_status(flora_call_reply_t reply,void* data)
{
   return  monitor_respond_status(reply);
}

static int netmanager_get_wifi_status(flora_call_reply_t reply,void* data)
{
   return  wifi_respond_status(reply);
}

static int netmanager_get_config_network(flora_call_reply_t reply,void* data)
{
    int num =0;
    uint32_t mode;
    uint32_t capacity;
    
    cJSON*root = cJSON_CreateObject();

    char *buf = NULL;
    
    NM_LOGI("%s->%d\n",__func__,__LINE__);

    capacity = get_network_capacity();
    mode =  get_network_mode();

    cJSON_AddStringToObject(root,"result","OK");

    if((capacity& NET_CAPACITY_WIFI) &&  !(mode & WIFI_AP)){
       wifi_hal_start_station(); 
      if(wifi_get_config_network(&num)){
        num =0;
        NM_LOGW("wifi get_config_num fail\n");
      }
    }
   
    NM_LOGI("wifi get_config_num:%d\n",num);

    cJSON_AddItemToObject(root, "wifi_config",cJSON_CreateNumber(num));
    buf = cJSON_Print(root);
    network_ipc_return(reply,(uint8_t *)buf);
    yodalite_free((void*)buf);
    cJSON_Delete(root);
  
    return 0;
}

static int netmanager_save_config_network(flora_call_reply_t reply,void* data)
{
  int iret = 0;
  int reason = eREASON_NONE;
  int state = COMMAND_OK;

  struct network_service *p_net = get_network_service();
  tNETWORK_MODE_INFO info,*p_info = &info;

  NM_LOGI("%s->%d\n",__func__,__LINE__);

  strncpy(p_info->ssid,p_net->ssid,128);
  strncpy(p_info->psk,p_net->psk,36);

  if(wifi_save_network(p_info)){
     state = COMMAND_NOK;
     reason = eREASON_COMMAND_RESPOND_ERROR;
  }

/*ret: */
  netmanger_response_command(reply,state,reason); 

  return iret;
}


static int netmanager_remove_network(flora_call_reply_t reply,void* data)
{
  int i;
  int iret=0;
  int reason = eREASON_NONE;
  int state = COMMAND_OK;

  cJSON* base  = (cJSON*)data;
  tNETWORK_MODE_INFO info,*p_info = &info;

  NM_LOGI("%s->%d\n",__func__,__LINE__);
/*
 if(get_wifi_station_info(base,p_info)){
       state = COMMAND_NOK;
       reason = eREASON_COMMAND_FORMAT_ERROR;
       NM_LOGE("get_wifi_station_info() fail\n");
       iret =-1;
       goto ret;
 }
*/

 if(wifi_remove_network(p_info)){
      NM_LOGE("wpa_sync_send_command error\n"); 
      state = COMMAND_NOK;
      reason = eREASON_COMMAND_FORMAT_ERROR;
  }
//ret:
  netmanger_response_command(reply,state,reason); 
  return iret;
}


static int netmanager_enable_all_network(flora_call_reply_t reply,void* data)
{
  int reason = eREASON_NONE;
  int state = COMMAND_OK;

  NM_LOGI("%s->%d\n",__func__,__LINE__);

  if(wifi_enable_all_network()){
     state = COMMAND_NOK;
     reason = eREASON_COMMAND_RESPOND_ERROR;
  }

  netmanger_response_command(reply,state,reason); 
  return 0;
}

static int netmanager_disable_all_network(flora_call_reply_t reply,void* data)
{
  int reason = eREASON_NONE;
  int state = COMMAND_OK;
  NM_LOGI("%s->%d\n",__func__,__LINE__);

  if(wifi_disable_all_network()){
     state = COMMAND_NOK;
     reason = eREASON_COMMAND_RESPOND_ERROR;
  }

  netmanger_response_command(reply,state,reason); 

  return 0;
}


static int netmanager_reset_dns(flora_call_reply_t reply,void* data)
{
  int reason = eREASON_NONE;
  int state = COMMAND_OK;
  NM_LOGI("%s->%d\n",__func__,__LINE__);

  if(reset_dns()){
     state = COMMAND_NOK;
     reason = eREASON_COMMAND_RESPOND_ERROR;
  }

  netmanger_response_command(reply,state,reason); 

  return 0;
}

static int netmanager_remove_all_network(flora_call_reply_t reply,void* data)
{
  NM_LOGI("%s->%d\n",__func__,__LINE__);
  return  netmanager_remove_network(reply,data);
}

static int wifi_respond_station_info(flora_call_reply_t reply,wifi_hal_status_t *status)
{
    char *str = NULL;
    char buf[32] = {0};

    wifi_sta_status_t *sta = &status->ap_sta_status.sta_status;
 
    cJSON*root = cJSON_CreateObject();
    cJSON*item = cJSON_CreateObject();
    
    NM_LOGI("%s->%d\n",__func__,__LINE__);

    cJSON_AddStringToObject(root,"result","OK");

    cJSON_AddNumberToObject(item,"mode",status->mode);

    memset(buf,0,32);
    snprintf(buf,32,"%x:%x:%x:%x:%x:%x",sta->mac[0],sta->mac[1],sta->mac[2],sta->mac[3],sta->mac[4],sta->mac[5]);
    printf("mac %s\n",buf);
    cJSON_AddStringToObject(item,"mac",buf);
    
    memset(buf,0,32);
    snprintf(buf,32,"%d.%d.%d.%d",(sta->ipaddr& 0xff),(sta->ipaddr >> 8)&0xff,(sta->ipaddr>>16)&0xff,(sta->ipaddr>>24)&0xff);
    printf("ip %s\n",buf);
    cJSON_AddStringToObject(item,"ip",buf);

    cJSON_AddStringToObject(item,"ssid",sta->ssid);
    cJSON_AddStringToObject(item,"passwd",sta->passwd);
    cJSON_AddNumberToObject(item,"channel",sta->channel);

    memset(buf,0,32);
    snprintf(buf,32,"%x:%x:%x:%x:%x:%x",sta->bssid[0],sta->bssid[1],sta->bssid[2],sta->bssid[3],sta->bssid[4],sta->bssid[5]);
    printf("bssid %s\n",buf);
    cJSON_AddStringToObject(item,"bssid",buf);
    cJSON_AddNumberToObject(item,"status",sta->status);
    cJSON_AddItemToObject(root,"WIFI_INFO",item);
    str = cJSON_Print(root);
    printf("str:%s\n",str);
    network_ipc_return(reply,(uint8_t *)str);

    yodalite_free((void*)str);
  //cJSON_Delete(item);
    cJSON_Delete(root);
    return 0;
}


static int wifi_respond_ap_info(flora_call_reply_t reply,wifi_hal_status_t *status)
{
    char *str = NULL;
    char buf[32] = {0};
    wifi_ap_status_t *ap = &status->ap_sta_status.ap_status;
    cJSON*root = cJSON_CreateObject();
    cJSON*item = cJSON_CreateObject();

    
    NM_LOGI("%s->%d\n",__func__,__LINE__);
    cJSON_AddStringToObject(root,"result","OK");
    cJSON_AddNumberToObject(item,"mode",status->mode);

    memset(buf,0,32);
    snprintf(buf,32,"%x:%x:%x:%x:%x:%x",ap->mac[0],ap->mac[1],ap->mac[2],ap->mac[3],ap->mac[4],ap->mac[5]);
    cJSON_AddStringToObject(item,"mac",buf);
    
    memset(buf,0,32);
    snprintf(buf,32,"%d.%d.%d.%d",(ap->ipaddr& 0xff),(ap->ipaddr >> 8)&0xff,(ap->ipaddr>>16)&0xff,(ap->ipaddr>>24)&0xff);
    cJSON_AddStringToObject(item,"ip",buf);
    cJSON_AddStringToObject(item,"ssid",ap->ssid);
    cJSON_AddStringToObject(item,"passwd",ap->passwd);
    cJSON_AddNumberToObject(item,"channel",ap->channel);
    cJSON_AddItemToObject(root,"WIFI_INFO",item);

    str = cJSON_Print(root);
    network_ipc_return(reply,(uint8_t *)str);
    yodalite_free((void*)str);
  //cJSON_Delete(item);
    cJSON_Delete(root);
    return 0;
}

static int wifi_respond_none_mode(flora_call_reply_t reply,wifi_hal_status_t *status)
{

    char *str = NULL;
//`   char buf[32] = {0};
    wifi_ap_status_t *ap = &status->ap_sta_status.ap_status;
    cJSON*root = cJSON_CreateObject();
    cJSON*item = cJSON_CreateObject();

    
    NM_LOGI("%s->%d\n",__func__,__LINE__);
    cJSON_AddStringToObject(root,"result","OK");
    cJSON_AddNumberToObject(item,"mode",status->mode);

//    memset(buf,0,32);
//    snprintf(buf,32,"");
    cJSON_AddStringToObject(item,"mac","");
    
//   memset(buf,0,32);
//    snprintf(buf,32,"");
    cJSON_AddStringToObject(item,"ip","");
    cJSON_AddStringToObject(item,"ssid","");
    cJSON_AddStringToObject(item,"passwd","");
    cJSON_AddNumberToObject(item,"channel",0);
    cJSON_AddItemToObject(root,"WIFI_INFO",item);

    str = cJSON_Print(root);
    network_ipc_return(reply,(uint8_t *)str);
    yodalite_free((void*)str);
  //cJSON_Delete(item);
    cJSON_Delete(root);

}

static int netmanager_get_wifi_info(flora_call_reply_t reply,void* data)
{
  int reason = eREASON_NONE;
  int state = COMMAND_OK;
  wifi_hal_status_t status; 
  
  NM_LOGI("%s->%d\n",__func__,__LINE__);
  memset(&status,0,sizeof(wifi_hal_status_t));

  if(wifi_get_status(&status)){
    state =  COMMAND_NOK;
    reason = eREASON_COMMAND_RESPOND_ERROR;
    netmanger_response_command(reply,state,reason); 
    return 0;
  }

  if(status.mode == STA_MODE){
    wifi_respond_station_info(reply,&status);
  }else if(status.mode == AP_MODE){
    wifi_respond_ap_info(reply,&status);
  }else{
   wifi_respond_none_mode(reply,&status);
 }
 
  return 0; 
}

static tCTRL_CMD_FMT ctrl_cmd[] = 
{
   {"NETWORK"     ,"GET_CAPACITY" ,netmanager_get_net_capacity},
   {"NETWORK"     ,"GET_MODE"     ,netmanager_get_net_mode},
   {"WIFI"        ,"START_SCAN"  ,netmanager_start_wifi_scan},
   {"WIFI"        ,"STOP_SCAN"   ,netmanager_stop_wifi_scan},
   {"WIFI"        ,"GET_WIFILIST",netmanager_get_wifi_scan_result},
   {"WIFI"        ,"CONNECT"     ,netmanager_start_wifi_station},
   {"WIFI"        ,"DISCONNECT"  ,netmanager_stop_wifi_station},
   {"WIFI_AP"     ,"CONNECT"     ,netmanager_start_wifi_ap},
   {"WIFI_AP"     ,"DISCONNECT"  ,netmanager_stop_wifi_ap}, 
   {"NETWORK"     ,"GET_STATUS"  ,netmanager_get_network_status},  
   {"WIFI"        ,"GET_STATUS"  ,netmanager_get_wifi_status},
   {"WIFI"       ,"GET_CFG_NUM"  ,netmanager_get_config_network},
   {"WIFI"       ,"SAVE_CFG"     ,netmanager_save_config_network},
   {"WIFI"       ,"REMOVE"       ,netmanager_remove_network},
   {"WIFI"       ,"ENABLE"       ,netmanager_enable_all_network},
   {"WIFI"       ,"DISABLE"      ,netmanager_disable_all_network},
   {"NETWORK"    ,"RESET_DNS"    ,netmanager_reset_dns},
   {"WIFI"       ,"REMOVE_ALL"   ,netmanager_remove_all_network},
   {"WIFI"       ,"GET_INFO"     ,netmanager_get_wifi_info},

};

static int get_cmd_str(cJSON*base,tCTRL_CMD_STR *cmd)
{
    int iret = -1;
    cJSON*device = NULL;
    cJSON*command = NULL;

    if(cmd == NULL)
     return iret;

    if ((device = cJSON_GetObjectItem(base,CMD_DEVICE_KEY)) != NULL && 
        ((command = cJSON_GetObjectItem(base,CMD_COMMAND_KEY)) != NULL)){
        strncpy(cmd->device,cJSON_GetStringValue(device),CMD_MAX_LEN-1);
        strncpy(cmd->command,cJSON_GetStringValue(command),CMD_MAX_LEN-1);
        iret  = 0;
    }

    return iret;
}

static int get_ctrl_cmd_idx(tCTRL_CMD_STR*cmd,int *idx)
{

  int iret = 0;
  int i,cnts;
 
  tCTRL_CMD_FMT *p_cmd;

  cnts  = sizeof(ctrl_cmd)/sizeof(ctrl_cmd[0]);

  for(i=0;i<cnts;i++){

     p_cmd = &ctrl_cmd[i];

     if(strcmp(p_cmd->device,cmd->device) == 0 && 
        strcmp(p_cmd->command,cmd->command) == 0){
          *idx  = i;
          break;
      }
  }

  if(i >= cnts)
   iret = -1;

  return iret;
}

void handle_net_command(flora_call_reply_t reply,const char *buf)
{
  int idx;
  uint32_t reason = eREASON_COMMAND_FORMAT_ERROR;

  cJSON*obj = NULL;
  tCTRL_CMD_STR cmd;
  tCTRL_CMD_FMT *p_cmd;

  memset(&cmd,0,sizeof(tCTRL_CMD_STR));  
  obj = cJSON_Parse(buf);
    
  if(get_cmd_str(obj,&cmd)){
    NM_LOGE("get_cmd_str fail:%s\n",buf);
    netmanger_response_command(reply,COMMAND_NOK,reason); 
    goto iret ;
  }

  if(get_ctrl_cmd_idx(&cmd,&idx)){
    NM_LOGE("device:%s,command:%s is not support\n",
       cmd.device,cmd.command);
    netmanger_response_command(reply,COMMAND_NOK,reason); 
    goto iret;
  }

  p_cmd = &ctrl_cmd[idx];
  p_cmd->respond_func(reply,(void *)obj);

iret:
  cJSON_Delete(obj);
}

