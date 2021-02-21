#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "yodalite_autoconf.h"

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
#include "network_handler.h"
#include <lib/property/properties.h>

static flora_agent_t agent;

pthread_mutex_t _network_mutex;
pthread_cond_t  _network_cond;

#define ANDLINK_MEG_LENTH  512

static int wifi_state = WIFI_NONE_MODE;
static int net_status = NETWORK_NONE;
static char *uuid[128];

static int get_wifi_info();

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
       if((net_status == NETWORK_DISCONNECTED || net_status == NETWORK_NONE) &&
            memcmp(cJSON_GetObjectItem(network, "state")->valuestring,"CONNECTED",strlen("CONNECTED")) == 0)
       {
          get_wifi_info();
          if( wifi_state == WIFI_STA_MODE) {
                send_wifi_status("11", "wifi连接成功");
                send_wifi_status("100", "登录中");
                if(change_to_login_mode(uuid))
                {
                    change_to_ble_mode();
                    printf("error login failed\n");
                    send_wifi_status("-101", "登录失败");
                    send_wifi_status("-201", "绑定失败");
                } else {
                    send_wifi_status("100", "登录成功");
                    send_wifi_status("201", "绑定成功"); 
                }
                ble_disable();
                ble_close();
          }
          else
          {
              change_to_login_mode(NULL);
          }
          
          net_status = NETWORK_CONNECTED;  
          printf("network state:%s\n", cJSON_GetObjectItem(network, "state")->valuestring); 
       }
       if((net_status == NETWORK_CONNECTED || net_status == NETWORK_NONE) &&
                memcmp(cJSON_GetObjectItem(network, "state")->valuestring,"DISCONNECTED",strlen("DISCONNECTED")) == 0)
       {
          net_status = NETWORK_DISCONNECTED;
          printf("network state disconnect :%s\n", cJSON_GetObjectItem(network, "state")->valuestring); 
          change_to_ble_mode();
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
     const char* buf;
     caps_read_string(result.data, &buf);
     printf("read string:%s,ret_code:%d\n",buf,result.ret_code);

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
            const char* buf;

            caps_read_string(result.data, &buf);

            printf("read string:%s,ret_code:%d\n",buf,result.ret_code);
            memset(&properties, 0, sizeof(struct properties_t));
            cJSON *monitor_json = cJSON_Parse(buf);
            cJSON *wifi_info = NULL;
            if (monitor_json) {
                wifi_info = cJSON_GetObjectItemCaseSensitive(monitor_json, "WIFI_INFO");
                printf("now get mac\n");
                if (wifi_info) {
                    cJSON *mac = cJSON_GetObjectItemCaseSensitive(wifi_info, "mac");
                    cJSON *ip = cJSON_GetObjectItemCaseSensitive(wifi_info, "ip");
                    printf("now mac:%s,ip:%s\n",mac->valuestring, ip->valuestring);
                    memcpy(properties.mac, mac->valuestring, strlen(mac->valuestring));
                    memcpy(properties.ip, ip->valuestring, strlen(ip->valuestring));
                }
            }
            cJSON_Delete(monitor_json);
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
      printf("floar xfer fail\n");
            
    }
    yodalite_free(net_command);
    cJSON_Delete(root);
    caps_destroy(msg);

    return 0;
}

void ble_connected(unsigned char* event)
{
    printf("get event:%s\n",event);
    if (memcmp(event,"connected",strlen("connected")) == 0)
    {
        printf("ble connected\n");
    } else if (memcmp(event,"disconnected",strlen("disconnected")) == 0) {
        printf("ble disconnected\n");
    } else {   
        printf("ble state:%s\n",event);
    }
}

void get_ble_data(unsigned char* buf)
{
    cJSON *bt_data = NULL;
    cJSON *realdata = NULL;
    cJSON *topic = NULL;
    cJSON *data = NULL;
    char serialno[PROPERTY_VALUE_MAX] = {0};

    data = cJSON_Parse(buf);
    bt_data = cJSON_GetObjectItemCaseSensitive(data, "data");
    if(!bt_data) {
        cJSON_Delete(bt_data);
        return;
    }
    topic = cJSON_GetObjectItemCaseSensitive(bt_data, "topic");
    printf("get ble data:%s\n",buf);

    if(topic) {
        if(memcmp(topic->valuestring, "getSn", strlen(topic->valuestring)) == 0) {

            if(property_get("ro.boot.serialno",serialno,"0") >= 0) {
                send_sn(serialno);
            } else {
                printf("error:property_get sn fail\n");
            }
        }
        else if(memcmp(topic->valuestring, "bind", strlen(topic->valuestring)) == 0) {
            realdata = cJSON_GetObjectItemCaseSensitive(bt_data, "data");
            if(realdata)
            {
                printf("uuid:%s,ssid:%s,pwd:%s\n",cJSON_GetObjectItem(realdata, "U")->valuestring
                ,cJSON_GetObjectItem(realdata, "S")->valuestring, cJSON_GetObjectItem(realdata, "P")->valuestring);
                change_to_sta_mode(cJSON_GetObjectItem(realdata, "S")->valuestring,cJSON_GetObjectItem(realdata, "P")->valuestring);
                memset(uuid, 0, 128);
                strncpy(uuid,cJSON_GetObjectItem(realdata, "U")->valuestring,strlen(cJSON_GetObjectItem(realdata, "U")->valuestring));
            }
        }
        else {
            printf("other topic :%s\n",topic->valuestring);
        }  
    }
    cJSON_Delete(bt_data);
}

int change_to_ble_mode()
{
    printf("change to ble mode\n");
    if( wifi_state == WIFI_BLE_MODE)
    {
        printf("now wifi in ble mode\n");
        return 0;
    }
    wifi_state == WIFI_BLE_MODE;

    ble_enable(ble_connected,get_ble_data);
    return 0;
}

int change_to_sta_mode(char *qlink_ssid,char *qlink_psk)
{
    wifi_state = WIFI_STA_MODE;
    wifi_send(qlink_ssid,qlink_psk);
    send_wifi_status("10", "wifi连接中");
    return 0;
}
