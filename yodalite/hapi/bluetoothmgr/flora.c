#include "common.h"
#include <hapi/btmgr.h>
//#include <hapi/flora_agent.h>

static flora_agent_t agent;

void bt_flora_send_msg(int element, uint8_t *buf, int len) {
    caps_t msg = caps_create();
    caps_write_string(msg, (const char *)buf);

    switch (element) {
    case BLUETOOTH_FUNCTION_COMMON: {
        flora_agent_post(agent, FLORA_BT_COMMON_EVT, msg, FLORA_MSGTYPE_INSTANT);
        break;
    }
    case BLUETOOTH_FUNCTION_BLE: {
        flora_agent_post(agent, FLORA_BT_BLE_EVT, msg, FLORA_MSGTYPE_INSTANT);
        break;
    }

    case BLUETOOTH_FUNCTION_A2DPSINK: {
        flora_agent_post(agent, FLORA_BT_A2DPSINK_EVT, msg, FLORA_MSGTYPE_INSTANT);
        break;
    }

    case BLUETOOTH_FUNCTION_A2DPSOURCE: {
        flora_agent_post(agent, FLORA_BT_A2DPSOURCE_EVT, msg, FLORA_MSGTYPE_INSTANT);
        break;
    }

    case BLUETOOTH_FUNCTION_HFP : {
        flora_agent_post(agent, FLORA_BT_HFP_EVT, msg, FLORA_MSGTYPE_INSTANT);
        break;
    }
    default:
        break;
    }

    caps_destroy(msg);
}
static void bt_service_command_callback(const char *name, uint32_t msgtype, caps_t msg, void *arg){

    const char *buf = NULL;
    cJSON *bt_command = NULL;

    BT_LOGV("name :: %s\n", name);

    if (strcmp(name, FLORA_BT_COMMON_CMD) == 0) {
        caps_read_string(msg, &buf);
        bt_command = cJSON_Parse(buf);
        handle_common_handle(bt_command, g_bt_handle, NULL);
    } else if (strcmp(name, FLORA_BT_BLE_CMD) == 0) {
        caps_read_string(msg, &buf);
        bt_command = cJSON_Parse(buf);
        handle_ble_handle(bt_command, g_bt_handle, NULL);
    } else if (strcmp(name, FLORA_BT_A2DPSINK_CMD) == 0) {
        caps_read_string(msg, &buf);
        bt_command = cJSON_Parse(buf);
        handle_a2dpsink_handle(bt_command, g_bt_handle, NULL);
    } else if (strcmp(name, FLORA_VOICE_COMING) == 0) {
//#if defined(ROKID_YODAOS)
        bt_a2dpsink_set_mute();
//#endif
    } else if (strcmp(name, FLORA_MODULE_SLEEP) == 0) {
        caps_read_string(msg, &buf);
        bt_command = cJSON_Parse(buf);
        handle_module_sleep(bt_command, g_bt_handle);
    } else if (strcmp(name, FLORA_POWER_SLEEP) == 0) {
        caps_read_string(msg, &buf);
        bt_command = cJSON_Parse(buf);
        handle_power_sleep(bt_command, g_bt_handle);
    } else if (strcmp(name, FLORA_BT_A2DPSOURCE_CMD) == 0) {
        caps_read_string(msg, &buf);
        bt_command = cJSON_Parse(buf);
        handle_a2dpsource_handle(bt_command, g_bt_handle, NULL);
    }

#if defined(BT_SERVICE_HAVE_HFP)
    else if (strcmp(name, FLORA_BT_HFP_CMD) == 0) {
        caps_read_string(msg, &buf);
        bt_command = cJSON_Parse(buf);
        handle_hfp_handle(bt_command, g_bt_handle, NULL);
    }
#endif
    cJSON_Delete(bt_command);
}

void bt_flora_report_reply(uint32_t msgtype, void *data, char *buf) {
    flora_call_reply_t reply = (flora_call_reply_t)data;
    if (reply && buf) {
        caps_t msg = caps_create();
        caps_write_string(msg, buf);
      
        flora_call_reply_write_code(reply, FLORA_CLI_SUCCESS);
        flora_call_reply_write_data(reply, msg);
      //flora_call_reply_end(reply);
        caps_destroy(msg);
    }
}

static void bt_service_command_method_callback(const char *name ,caps_t msg,void* arg,flora_call_result* reply)
{
    const char *buf = NULL;
    cJSON *bt_command = NULL;

    BT_LOGV("method_callback::name :: %s\n", name);

    if (strcmp(name, FLORA_BT_COMMON_CMD) == 0) {
        caps_read_string(msg, &buf);
        bt_command = cJSON_Parse(buf);
        handle_common_handle(bt_command, g_bt_handle, (void*)reply);
    } else if (strcmp(name, FLORA_BT_BLE_CMD) == 0) {
        caps_read_string(msg, &buf);
        bt_command = cJSON_Parse(buf);
        handle_ble_handle(bt_command, g_bt_handle, (void*)reply);
    } else if (strcmp(name, FLORA_BT_A2DPSINK_CMD) == 0) {
        caps_read_string(msg, &buf);
        bt_command = cJSON_Parse(buf);
        handle_a2dpsink_handle(bt_command, g_bt_handle, (void*)reply);
    } else if (strcmp(name, FLORA_BT_A2DPSOURCE_CMD) == 0) {
        caps_read_string(msg, &buf);
        bt_command = cJSON_Parse(buf);
        handle_a2dpsource_handle(bt_command, g_bt_handle, (void*)reply);
    } else if (strcmp(name, FLORA_BT_HFP_CMD) == 0) {
    #if defined(BT_SERVICE_HAVE_HFP)
        caps_read_string(msg, &buf);
        bt_command = cJSON_Parse(buf);
        handle_hfp_handle(bt_command, g_bt_handle, (void*)reply);
    #endif
    }

    cJSON_Delete(bt_command);
}

void bt_flora_init(void) 
{

    flora_agent_config_param_t param;

    agent = flora_agent_create();

    memset(&param,0,sizeof(flora_agent_config_param_t));

    param.sock_path  = FLORA_AGET_BTMGR_URI;
    flora_agent_config(agent, FLORA_AGENT_CONFIG_URI,&param );

    param.value  = 1024;
    flora_agent_config(agent, FLORA_AGENT_CONFIG_BUFSIZE,&param);

    flora_agent_subscribe(agent, FLORA_BT_COMMON_CMD, bt_service_command_callback, NULL);
    flora_agent_subscribe(agent, FLORA_BT_BLE_CMD, bt_service_command_callback, NULL);
    flora_agent_subscribe(agent, FLORA_BT_A2DPSINK_CMD, bt_service_command_callback, NULL);
    flora_agent_subscribe(agent, FLORA_BT_A2DPSOURCE_CMD, bt_service_command_callback, NULL);
#if defined(BT_SERVICE_HAVE_HFP)
    flora_agent_subscribe(agent, FLORA_BT_HFP_CMD, bt_service_command_callback, NULL);
#endif

    flora_agent_subscribe(agent, FLORA_VOICE_COMING, bt_service_command_callback, NULL);
    flora_agent_subscribe(agent, FLORA_POWER_SLEEP, bt_service_command_callback, NULL);
    flora_agent_subscribe(agent, FLORA_MODULE_SLEEP, bt_service_command_callback, NULL);

    flora_agent_declare_method(agent, FLORA_BT_COMMON_CMD, bt_service_command_method_callback, NULL);
    flora_agent_declare_method(agent, FLORA_BT_BLE_CMD, bt_service_command_method_callback, NULL);
    flora_agent_declare_method(agent, FLORA_BT_A2DPSINK_CMD, bt_service_command_method_callback, NULL);
    flora_agent_declare_method(agent, FLORA_BT_A2DPSOURCE_CMD, bt_service_command_method_callback, NULL);
#if defined(BT_SERVICE_HAVE_HFP)
    flora_agent_declare_method(agent, FLORA_BT_HFP_CMD, bt_service_command_method_callback, NULL);
#endif

    flora_agent_start(agent, 0);
    return NULL;
}

void bt_flora_deinit(void) 
{
  flora_agent_close(agent);
  flora_agent_delete(agent);
}
