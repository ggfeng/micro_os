#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <hapi/netmgr.h>
#include "flora_upload.h"
#include "network_ctrl.h"
#include "ipc.h"
#include "common.h"


static flora_agent_t agent;

void network_flora_send_msg(uint8_t *buf)
{
     caps_t msg = caps_create();
     caps_write_string(msg, buf);
     NM_LOGI("%s:%s\n",__func__,buf);
     flora_agent_post(agent,NETWORK_STATUS_NAME,msg, FLORA_MSGTYPE_INSTANT);
     caps_destroy(msg);
}

void network_flora_return_msg(flora_call_reply_t reply, uint8_t *buf)
{
  caps_t data = caps_create();
  caps_write_string(data, buf);
  printf("cgl %s:%s\n",__func__,buf);
  flora_call_reply_write_code(reply, FLORA_CLI_SUCCESS);
  flora_call_reply_write_data(reply, data);
// flora_call_reply_end(reply);
  caps_destroy(data);
}

static void net_manager_command_callback(const char *name ,caps_t msg,void* arg,flora_call_result* reply)
{
    const char *buf;
    caps_read_string(msg, &buf);
    NM_LOGW("name:%s:command:%s\n",name, buf);
    handle_net_command(reply,buf);
}

/*
typedef union {
    const char *sock_path;
    uint32_t value;
} flora_agent_config_param_t;
*/

int network_flora_init(void)
{ 
    flora_agent_config_param_t param;

    agent = flora_agent_create();

    memset(&param,0,sizeof(flora_agent_config_param_t));
    param.sock_path  = FLORA_AGET_NET_MANAGER_URI;
    flora_agent_config(agent,FLORA_AGENT_CONFIG_URI,&param);
    param.value = 1024;
    flora_agent_config(agent, FLORA_AGENT_CONFIG_BUFSIZE,&param);
//  flora_agent_config(agent, FLORA_AGENT_CONFIG_RECONN_INTERVAL, 5000);
    flora_agent_declare_method(agent, NETWORK_COMMAND_NAME,net_manager_command_callback,NULL);

    flora_agent_start(agent, 0);

    return 0;
}

void network_flora_exit(void)
{
  flora_agent_close(agent);
  flora_agent_delete(agent);
}
