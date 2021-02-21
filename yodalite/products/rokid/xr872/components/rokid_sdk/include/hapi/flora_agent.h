#ifndef _FLORA_AGENT_H_
#define _FLORA_AGENT_H_

#include <yodalite_autoconf.h>
#include <inttypes.h>

#include <lib/bm/SH_BM_api.h>
#include <hapi/flora_cli.h>

// config(KEY, string uri)
#define FLORA_AGENT_CONFIG_URI 0
// config(KEY, uint32_t bufsize)
#define FLORA_AGENT_CONFIG_BUFSIZE 1
// config(KEY, uint32_t interval)
#define FLORA_AGENT_CONFIG_RECONN_INTERVAL 2
// config(KEY, uint32_t flags, MonitorCallback *cb)
//   flags: FLORA_CLI_FLAG_MONITOR_*  (see flora-cli.h)
#define FLORA_AGENT_CONFIG_MONITOR 3
// config(KEY, uint32_t interval, uint32_t timeout)
//   interval: interval of send beep packet
//   timeout: timeout of flora service no response
#define FLORA_AGENT_CONFIG_KEEPALIVE 4
// config(KEY, string channel_id)
#define FLOAR_AGENT_CONFIG_CHANNELID    5

struct flora_agent {
    flora_cli_t pChannel;
    const char *uri;
    const char *channel_id;
    uint32_t msg_buf_size;
    flora_cli_recv_post_func_t recv_post;
    void* recv_post_arg;
    flora_cli_recv_call_func_t recv_call;
    void* recv_call_arg;
    flora_message_t* msg_head;
    flora_message_t* msg_tail;
    int agent_start;
};

typedef union {
    const char *sock_path;
    const char *channel_id;
    uint32_t value;
} flora_agent_config_param_t;

typedef struct flora_agent* flora_agent_t;

extern flora_agent_t flora_agent_create(void);
extern void flora_agent_config(flora_agent_t agent, uint32_t key, flora_agent_config_param_t *param);
extern int32_t flora_agent_call(flora_agent_t agent, const char *name, caps_t msg, const char *target, flora_call_result *result, uint32_t timeout);
extern int32_t flora_agent_post(flora_agent_t agent, const char *name, caps_t msg, uint32_t msgtype);
extern void flora_agent_subscribe(flora_agent_t agent, const char *name, flora_cli_recv_post_func_t cb, void *arg);
extern void flora_agent_unsubscribe(flora_agent_t agent, const char *name);
extern void flora_agent_declare_method(flora_agent_t agent, const char *name, flora_cli_recv_call_func_t cb, void *arg);
extern void flora_agent_remove_method(flora_agent_t agent, const char *name);
extern void flora_agent_start(flora_agent_t agent, int32_t block);
extern void flora_agent_close(flora_agent_t agent);
extern void flora_agent_delete(flora_agent_t agent);

#endif  /*_FLORA_AGENT_H_*/
