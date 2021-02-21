#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <osal/time.h>
#include <lib/cjson/cJSON.h>
#include <hapi/flora_agent.h>
#include <lib/shell/shell.h>


static flora_agent_t agent;

static void bt_service_command_callback(const char *name, uint32_t msgtype, caps_t msg, void *arg)
{
    const char *buf = NULL;
    printf("name :: %s\n", name);
    caps_read_string(msg, &buf);
    printf("data :: %s\n", buf);
}

static int bt_flora_init(void) {

   flora_agent_config_param_t param;

   memset(&param,0,sizeof(flora_agent_config_param_t));
   agent = flora_agent_create();

   param.sock_path  = "unix:/var/run/flora.sock";
   flora_agent_config(agent, FLORA_AGENT_CONFIG_URI, &param);
   param.value  = 1024;
   flora_agent_config(agent, FLORA_AGENT_CONFIG_BUFSIZE,&param);

   flora_agent_subscribe(agent, "bluetooth.common.command", bt_service_command_callback, NULL);
   flora_agent_subscribe(agent, "bluetooth.ble.command", bt_service_command_callback, NULL);
   flora_agent_subscribe(agent, "bluetooth.a2dpsink.command", bt_service_command_callback, NULL);
   flora_agent_subscribe(agent, "bluetooth.a2dpsource.command", bt_service_command_callback, NULL);
   flora_agent_subscribe(agent, "bluetooth.hfp.command", bt_service_command_callback, NULL);

   flora_agent_subscribe(agent, "bluetooth.common.event", bt_service_command_callback, NULL);
   flora_agent_subscribe(agent, "bluetooth.ble.event", bt_service_command_callback, NULL);
   flora_agent_subscribe(agent, "bluetooth.a2dpsink.event", bt_service_command_callback, NULL);
   flora_agent_subscribe(agent, "bluetooth.a2dpsource.event", bt_service_command_callback, NULL);
   flora_agent_subscribe(agent, "bluetooth.hfp.event", bt_service_command_callback, NULL);

   flora_agent_start(agent, 0);

   return 0;
}


static int bt_monitor_start(int argc,int8_t * const argv[])
{
  bt_flora_init();

  return 0;
}

static int bt_monitor_stop(int argc,int8_t * const argv[])
{
  flora_agent_close(agent);
  flora_agent_delete(agent);

  return 0;
}

#define max_start_args    (1)
#define start_help  "bt_moniter_start"

int cmd_bt_monitor_start(void)
{

  YODALITE_REG_CMD(bt_monitor_start,max_start_args,bt_monitor_start,start_help);

  return 0;
}

#define max_stop_args    (1)
#define stop_help  "bt_moniter_stop"

int cmd_bt_monitor_stop(void)
{

  YODALITE_REG_CMD(bt_monitor_stop,max_stop_args,bt_monitor_stop,stop_help);

  return 0;
}

