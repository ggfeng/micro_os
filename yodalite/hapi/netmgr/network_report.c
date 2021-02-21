#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <osal/pthread.h>
#include <lib/cjson/cJSON.h>
#include <hapi/flora_agent.h>

#include "ipc.h"
#include "common.h"
#include "network_report.h"

#define REPORT_WIFI_MAIN_KEY     "wifi"
#define REPORT_WIFI_STATE_KEY    "state"
#define REPORT_WIFI_SIGNAL_KEY   "signal"
#define REPORT_NETWORK_MAIN_KEY  "network"
#define REPORT_NETWORK_STATE_KEY "state"

static char *net_status[]= 
{
 "DISCONNECTED",
 "CONNECTED"
};

#define WIFI_DEF_SIGNAL -71

int report_wifi_status(tWIFI_STATUS *p_status)
{
  int signal;
  const char *status = NULL;
  cJSON *root = cJSON_CreateObject();
  cJSON *elem = cJSON_CreateObject();

  if(p_status->state >= sizeof(net_status) /*|| 
   p_status->signal >= sizeof(signal_strength)*/){
     NM_LOGE("wifi report parm error\n");
     cJSON_Delete(elem);
     cJSON_Delete(root);
     return -1;
  }

  cJSON_AddStringToObject(elem,REPORT_WIFI_STATE_KEY,net_status[p_status->state]);

  if(p_status->signal)
    signal = p_status->signal;
  else
    signal = WIFI_DEF_SIGNAL;

  cJSON_AddNumberToObject(elem,REPORT_WIFI_SIGNAL_KEY,signal);
  cJSON_AddItemToObject(root,REPORT_WIFI_MAIN_KEY,elem);

  status = cJSON_Print(root);

 //NM_LOGI("report network status:%s\n",status);

  network_ipc_upload((uint8_t *)status); 

  yodalite_free((void*)status);
//  cJSON_Delete(elem);
  cJSON_Delete(root);

  return 0; 
}

int report_network_status(tNETWORK_STATUS *p_status)
{
  const char *status = NULL;
  cJSON *root = cJSON_CreateObject();
  cJSON *elem = cJSON_CreateObject();

  if(p_status->state >= sizeof(net_status))
  {
     NM_LOGE("wifi report parm error\n");
     cJSON_Delete(elem);
     cJSON_Delete(root);
     return -1;
  }

  cJSON_AddStringToObject(elem,REPORT_NETWORK_STATE_KEY,net_status[p_status->state]);
  cJSON_AddItemToObject(root,REPORT_NETWORK_MAIN_KEY,elem);

  status = cJSON_Print(root);
  
 //NM_LOGI("report network status:%s\n",status);

  network_ipc_upload((uint8_t *)status); 

  yodalite_free((void*)status);
//  cJSON_Delete(elem);
  cJSON_Delete(root);

  return  0;
}


int respond_wifi_status(flora_call_reply_t reply,tWIFI_STATUS *p_status)
{
  int signal;
  char str[64] = {0}; 
  const char *status = NULL;
  cJSON *root = cJSON_CreateObject();
  cJSON *elem = cJSON_CreateObject();

  if(p_status->state >= sizeof(net_status) /*|| 
   p_status->signal >= sizeof(signal_strength)*/){
     NM_LOGE("wifi report parm error\n");
     cJSON_Delete(elem);
     cJSON_Delete(root);
     return -1;
 }

 cJSON_AddStringToObject(root,"result","OK");
 cJSON_AddStringToObject(elem,REPORT_WIFI_STATE_KEY,net_status[p_status->state]);


  if(p_status->signal)
    signal = p_status->signal;
  else
    signal = WIFI_DEF_SIGNAL;

  cJSON_AddNumberToObject(elem,REPORT_WIFI_SIGNAL_KEY,signal);
  cJSON_AddItemToObject(root,REPORT_WIFI_MAIN_KEY,elem);


  status = cJSON_Print(root);

// NM_LOGI("report wifi status:%s\n",status);
  network_ipc_return(reply,(uint8_t *)status); 

  yodalite_free((void*)status);
//cJSON_Delete(elem);
  cJSON_Delete(root);

 return 0; 
}

int respond_network_status(flora_call_reply_t reply,tNETWORK_STATUS *p_status)
{
  const char *status = NULL;
  cJSON *root = cJSON_CreateObject();
  cJSON *elem = cJSON_CreateObject();

  if(p_status->state >= sizeof(net_status)){
     NM_LOGE("wifi report parm error\n");
     cJSON_Delete(elem);
     cJSON_Delete(root);
     return -1;
  }

  cJSON_AddStringToObject(root,"result","OK");
  cJSON_AddStringToObject(elem,REPORT_NETWORK_STATE_KEY,net_status[p_status->state]);
  cJSON_AddItemToObject(root,REPORT_NETWORK_MAIN_KEY,elem);

  status = cJSON_Print(root);
  
 //NM_LOGI("report network status:%s\n",status);

  network_ipc_return(reply,(uint8_t *)status); 

  yodalite_free((void*)status);
//cJSON_Delete(elem);
  cJSON_Delete(root);

 return  0;
}
