#include <stdlib.h>
#include <string.h>
#include "yodalite_autoconf.h"

#include "Qlink_API.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/event_groups.h"
// #include "freertos/semphr.h"
// #include "freertos/timers.h"
#include "lib/cjson/cJSON.h"

#include <hapi/flora_agent.h>
#include <unistd.h>

#ifdef YODALITE
#include <osal/pthread.h>
#else
#include <pthread.h>
#endif

#include "network.h"
#include "login.h"
#include "common.h"

static flora_agent_t agent;

pthread_mutex_t _network_mutex;
pthread_cond_t  _network_cond;

#define ANDLINK_MEG_LENTH  512

static int wifi_state = WIFI_STA_MODE;
static int net_status = NETWORK_NONE;

static void flora_recv_post(const char *name, uint32_t msgtype, caps_t msg, void *arg)
{
    const char* msg_string;
    cJSON *monitor_json;
    cJSON *network = NULL;


    caps_read_string(msg, &msg_string);
    monitor_json = cJSON_Parse(msg_string);
    if (monitor_json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            printf("Error before: %s\n", error_ptr);
        }
        goto end;
    }

    network = cJSON_GetObjectItemCaseSensitive(monitor_json, "network");
    if(network) {
       if((net_status == NETWORK_DISCONNECTED || net_status == NETWORK_NONE) && memcmp(cJSON_GetObjectItem(network, "state")->valuestring,"CONNECTED",strlen("CONNECTED")) == 0)
       {
         /* pthread_t pthread_login;
          (void) pthread_create(&pthread_login, NULL,login,NULL);
          pthread_detach(pthread_login);*/
          if(change_to_login_mode(NULL))
          {
              change_to_ap_mode();
              printf("error login failed\n");
          }
          net_status = NETWORK_CONNECTED;  
          printf("network state:%s\n", cJSON_GetObjectItem(network, "state")->valuestring); 
       }
       if((net_status == NETWORK_CONNECTED || net_status == NETWORK_NONE) &&
                memcmp(cJSON_GetObjectItem(network, "state")->valuestring,"DISCONNECTED",strlen("DISCONNECTED")) == 0)
       {
          net_status = NETWORK_DISCONNECTED;
          printf("network state disconnect :%s\n", cJSON_GetObjectItem(network, "state")->valuestring); 
          change_to_ap_mode();
       }
    }

   end:
    cJSON_Delete(monitor_json);
}

int get_network_status()
{
    return net_status;
}


static int network_flora_int(void) 
{
   flora_agent_config_param_t param;
   memset(&param,0,sizeof(flora_agent_config_param_t));
   agent = flora_agent_create();
   param.sock_path  = "unix:/var/run/flora.sock";
   flora_agent_config(agent, FLORA_AGENT_CONFIG_URI, &param);
   param.value  = 1024;
   flora_agent_config(agent, FLORA_AGENT_CONFIG_BUFSIZE,&param);
   flora_agent_subscribe(agent,"network.status", flora_recv_post, NULL);
   flora_agent_start(agent, 0);
   return 0;
}

int network_int(void) 
{
    pthread_mutex_init(&_network_mutex,NULL);
    pthread_cond_init(&_network_cond,NULL);
    network_flora_int();
}

// add new
void network__flora_ext(void)
{
  flora_agent_close(agent);
  flora_agent_delete(agent);
}

void network_ext(void)
{
    network__flora_ext();
}

static int floar_xfer(char *path,caps_t send_msg,char *recv,uint32_t len)
{
  int iret = -1;

 do{
  flora_call_result result;
  //if ((iret = flora_agent_call(agent,path,send_msg, "net_manager", &result,50000)) == FLORA_CLI_SUCCESS) 
  if ((iret = flora_agent_call(agent,path,send_msg, "net_manager", &result,0)) == FLORA_CLI_SUCCESS) 
  {
     printf("read string:%s,ret_code:%d\n",result.data,result.ret_code);

  }
  else
  printf("%s->%d,ret:%d\n",__func__,__LINE__,iret);

  flora_call_reply_end(&result);

  }while(0);

 return iret;
}

static int get_wifi_info()
{
    const char *net_command = NULL;
    caps_t msg; 
    cJSON *root =cJSON_CreateObject();

    cJSON_AddStringToObject(root,"device","WIFI");
    cJSON_AddStringToObject(root,"command","GET_INFO");

    net_command = cJSON_Print(root);
    printf("send:%s\n",net_command);

    msg = caps_create();
    caps_write_string(msg, net_command);

    int iret = -1;

    do{
        flora_call_result result;
        //if ((iret = flora_agent_call(agent,path,send_msg, "net_manager", &result,50000)) == FLORA_CLI_SUCCESS) 
        if ((iret = flora_agent_call(agent,"network.command",msg, "net_manager", &result,0)) == FLORA_CLI_SUCCESS) 
        {
            printf("read string:%s,ret_code:%d\n",result.data,result.ret_code);
        }
        else
        printf("%s->%d,ret:%d\n",__func__,__LINE__,iret);

        flora_call_reply_end(&result);

    }while(0);
    if(iret < 0) {
        printf("get_wifi_info error\n");
    }
    yodalite_free(net_command);
    cJSON_Delete(root);
    caps_destroy(msg);

    return 0 ;
}

static int json_parse(char *msg)
{
    cJSON *monitor_json = cJSON_Parse(msg);
    cJSON *network = NULL;
    cJSON *state = NULL;

    if (monitor_json)
    {
        network = cJSON_GetObjectItemCaseSensitive(monitor_json, "network");
        if(network) {
            state = cJSON_GetObjectItemCaseSensitive(network, "state");
            if(state) {
                if(memcmp(state->valuestring,"CONNECTED",strlen(state->valuestring)) == 0) {
                    goto state_connected;
                }
            }
        }
    }
    cJSON_Delete(monitor_json);
    return NETWORK_DISCONNECTED;

state_connected:
    cJSON_Delete(monitor_json);
    return NETWORK_CONNECTED;
}

static int response (unsigned char* body, int len)
{
    printf("now andlink response body:%s,len:%d\n", body, len);

    const cJSON *psk = NULL;
    const cJSON *ssid = NULL;

    if(len<= 0 || len >= ANDLINK_MEG_LENTH) {
      printf("andlink meg lenth :%d error\n",len);
      return -1;
    }
    unsigned char *tmp_str = (unsigned char *)yodalite_malloc(ANDLINK_MEG_LENTH);
    memset(tmp_str,0,ANDLINK_MEG_LENTH);
    memcpy(tmp_str,body,len);
    cJSON *monitor_json = cJSON_Parse((char *)tmp_str);

    if (monitor_json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            printf("Error before: %s\n", error_ptr);
        }
        goto end;
    }
    psk = cJSON_GetObjectItemCaseSensitive(monitor_json, "password");
    if (cJSON_IsString(psk) && (psk->valuestring != NULL))
    {
        printf("Checking password %s\n", psk->valuestring);
    }

    ssid = cJSON_GetObjectItemCaseSensitive(monitor_json, "SSID");
    if (cJSON_IsString(ssid) && (ssid->valuestring != NULL))
    {
        printf("Checking SSID %s\n", ssid->valuestring);
    }

    change_to_sta_mode(ssid->valuestring, psk->valuestring);

end:
    yodalite_free(tmp_str);
    cJSON_Delete(monitor_json);
  return 0;
}

static int wifi_ap_send()
{
    char ssid[36] = {0};
    char psk[64] =  {0};
    char ip[64] =  {0};
    char timeout[64] =  {0};
    const char *net_command = NULL;
    caps_t msg;
    cJSON *root =cJSON_CreateObject();
    cJSON *wifi_info = cJSON_CreateObject();

    strcpy(ssid,"CMQLINK-30671-1234");
    strcpy(psk,"");
    strcpy(ip,"192.168.188.1");
    snprintf(timeout,64,"0");

    printf("input :: ssid :: %s\n", ssid);
    printf("input :: psk :: %s\n", psk);
    printf("input :: ip :: %s\n", ip);
    printf("input :: timeout :: %s\n",timeout);

    cJSON_AddStringToObject(wifi_info,"SSID",ssid);
    cJSON_AddStringToObject(wifi_info,"PASSWD",psk);
    cJSON_AddStringToObject(wifi_info,"IP",ip);
    cJSON_AddStringToObject(wifi_info,"TIMEOUT",timeout);

    cJSON_AddStringToObject(root,"device","WIFI_AP");
    cJSON_AddStringToObject(root,"command","CONNECT");
    cJSON_AddItemToObject(root,"params",wifi_info);

    net_command = cJSON_Print(root);
    printf("send:%s\n",net_command);

    msg = caps_create();
    caps_write_string(msg, net_command);

    if(floar_xfer("network.command",msg,NULL,0))
    {
      fprintf(stderr,"floar xfer fail\n");
            
    }
    yodalite_free(net_command);
    cJSON_Delete(root);
    caps_destroy(msg);
    return 0;
}

static int wifi_send(char *qlink_ssid,char *qlink_psk)
{
    char ssid[36] = {0};
    char psk[64] =  {0};
    const char *net_command = NULL;
    caps_t msg;
    cJSON *root =cJSON_CreateObject();
    cJSON *wifi_info = cJSON_CreateObject();

    strcpy(ssid,qlink_ssid);
    strcpy(psk,qlink_psk);

    printf("input :: ssid :: %s\n", ssid);
    printf("input :: psk :: %s\n", psk);

    cJSON_AddStringToObject(wifi_info,"SSID",ssid);
    cJSON_AddStringToObject(wifi_info,"PASSWD",psk);

    cJSON_AddStringToObject(root,"device","WIFI");
    cJSON_AddStringToObject(root,"command","CONNECT");
    cJSON_AddItemToObject(root,"params",wifi_info);

    net_command = cJSON_Print(root);
    printf("send:%s\n",net_command);

    msg = caps_create();
    caps_write_string(msg, net_command);

    if(floar_xfer("network.command",msg,NULL,0))
    {
      fprintf(stderr,"floar xfer fail\n");
            
    }
    yodalite_free(net_command);
    cJSON_Delete(root);
    caps_destroy(msg);

    return 0;
}

int change_to_ap_mode()
{
    printf("change to ap mode\n");
    if( wifi_state == WIFI_AP_MODE)
    {
        printf("now wifi in ap mode\n");
        return 0;
    }
    wifi_state == WIFI_AP_MODE;

    wifi_ap_send();

    pthread_t pthread_qlink;
    pthread_create(&pthread_qlink, NULL,Qlink_StartCoapServer,NULL);
    pthread_detach(pthread_qlink);
    Qlink_setReciveInternetChannelCallback(response);
    return 0;
}

int change_to_sta_mode(char *qlink_ssid,char *qlink_psk)
{
    wifi_state == WIFI_STA_MODE;
    wifi_send(qlink_ssid,qlink_psk);
    return 0;
}
