#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <lib/cjson/cJSON.h>
#include <hapi/flora_agent.h>
#include <lib/shell/shell.h>
#include <yodalite_autoconf.h>
#include "network_handler.h"
#include <lib/property/properties.h>

#ifdef CONFIG_MEM_EXT_ENABLE
#include <lib/mem_ext/mem_ext.h>
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

static int connecting = 0;
static flora_agent_t agent;

static ble_conn_func ble_conn_handler;
static ble_data_func ble_get_data_handler;

static int ble_operation(int comandid, char* send_data)
{
    int ret = 0;
    char name[32] = {0};
    char *proto = "ROKID_BLE";
    char *command = NULL;
    char *data = NULL;
    char *bt_command = NULL;
    int unique = 1;
    int flora_call = 0;
    cJSON *root = NULL;
    caps_t msg;

    char serialno[PROPERTY_VALUE_MAX] = {0};
    property_get("ro.boot.serialno",serialno,"0");
    if(strlen(serialno) <= 6) {
        printf("get serialno failed\n");
        return -1;
    }
    strcat(name,"Rokid-Board-");
    strcat(name, serialno + (strlen(serialno) - 6));

    root = cJSON_CreateObject();
    switch(comandid) {
        case BLE_ON:
            command = "ON";
            cJSON_AddStringToObject(root,"name",name);
            cJSON_AddStringToObject(root,"command",command);
            cJSON_AddNumberToObject(root,"unique",unique);
            break;
        case BLE_OFF:
            command = "OFF";
            cJSON_AddStringToObject(root,"command",command);
            break;
        case BLE_DATA:
            data = send_data;
            cJSON_AddStringToObject(root,"data",data);
            break;
        case BLE_STATE:
            flora_call = 1;
            command = "GETSTATE";
            cJSON_AddStringToObject(root,"command",command);
            break;
        default:
            cJSON_Delete(root);
            break;
    }
    cJSON_AddStringToObject(root,"proto",proto);
    bt_command = cJSON_Print(root);
    msg = caps_create();
    caps_write_string(msg, (const char *)bt_command);
    if (flora_call) {
        int iret = -1;
        flora_call = 0;
        flora_call_result result = {0};
        if ((iret=flora_agent_call(agent, "bluetooth.ble.command", msg, "bluetoothservice-agent", &result,0)) == FLORA_CLI_SUCCESS) {
                const char *buf = NULL;
                caps_read_string(result.data, &buf);
                printf("data :: %s\n", buf);
        } else {
            printf("failed to flora_agent_call,iret %d\n",iret);
        }
        flora_call_reply_end(&result);
    } else {
        printf("post ble command:%s\n",bt_command);
        ret = flora_agent_post(agent, "bluetooth.ble.command", msg, FLORA_MSGTYPE_INSTANT);
    }
    yodalite_free(bt_command);
    caps_destroy(msg);
    cJSON_Delete(root);

    return 0;
}

static void bt_service_event_callback(const char *name, uint32_t msgtype, caps_t msg, void *arg)
{
    const char *buf = NULL;
    cJSON *state = NULL;
    cJSON *bt_event = NULL;
    cJSON *data = NULL;

    caps_read_string(msg, &buf); 
    bt_event=cJSON_Parse(buf);

    if (name && strcmp(name, "bluetooth.ble.event") == 0) {
        state = cJSON_GetObjectItemCaseSensitive(bt_event, "state");
        if(state) {
            if(ble_conn_handler)
            {
                ble_conn_handler(state->valuestring);
            }
            goto exit;      
        }
        data = cJSON_GetObjectItemCaseSensitive(bt_event, "data");
        if(data)
        {
            if(ble_get_data_handler)
            {
                ble_get_data_handler(buf);
            }
        }
    }
exit:
    cJSON_Delete(bt_event); 
}

static int bt_flora_init(void) 
{
    flora_agent_config_param_t param;

    memset(&param,0,sizeof(flora_agent_config_param_t));
    agent = flora_agent_create();

    param.sock_path  = "unix:/var/run/flora.sock";
    flora_agent_config(agent, FLORA_AGENT_CONFIG_URI, &param);
    param.value  = 1024;
    flora_agent_config(agent, FLORA_AGENT_CONFIG_BUFSIZE,&param);

    flora_agent_subscribe(agent, "bluetooth.ble.event", bt_service_event_callback, NULL);
    flora_agent_start(agent, 0);
    return 0;
}

static void bt_flora_exit(void)
{
  flora_agent_close(agent);
  flora_agent_delete(agent);
}

void ble_enable(ble_conn_func ble_conn,ble_data_func ble_get_data)
{
    bt_flora_init();
    ble_conn_handler = ble_conn;
    ble_get_data_handler = ble_get_data;

    ble_operation(BLE_ON, NULL);
}

void ble_disable()
{
    ble_operation(BLE_OFF, NULL);
}

void ble_close()
{
    bt_flora_exit();
    ble_conn_handler = NULL;
    ble_get_data_handler = NULL;
}

void send_wifi_status(char* code, char* msg)
{

    cJSON *root = cJSON_CreateObject();
    char *data = NULL;

    cJSON_AddStringToObject(root,"topic","bind");
    cJSON_AddStringToObject(root,"sCode",code);
    cJSON_AddStringToObject(root,"sMsg",msg);
        
    data = cJSON_Print(root);
    ble_operation(BLE_DATA,data);

    yodalite_free(data);
    cJSON_Delete(root);
}

void send_sn(char* deviceid)
{

    cJSON *root = cJSON_CreateObject();
    char *data = NULL;

    cJSON_AddStringToObject(root,"topic","getSn");
    cJSON_AddStringToObject(root,"data",deviceid);
        
    data = cJSON_Print(root);
    ble_operation(BLE_DATA,data);

    yodalite_free(data);
    cJSON_Delete(root);
}