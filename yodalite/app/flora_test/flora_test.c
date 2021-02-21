/*
 * (C) Copyright 2019 Rokid Corp.
 * Zhu Bin <bin.zhu@rokid.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <yodalite_autoconf.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#include <osal/semaphore.h>
#include <osal/pthread.h>
#include <osal/time.h>
#include <lib/shell/shell.h>
#include <hapi/flora_agent.h>

#ifdef CONFIG_OSAL_UNISTD
#include <osal/unistd.h>
#endif

#ifdef CONFIG_MEM_EXT_ENABLE
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

static pthread_t flora_cli_thread;
static pthread_t flora_server_thread;
static pthread_t flora_monitor_thread;

static void flora_recv_post_func(const char *name, uint32_t msgtype, caps_t msg, void *arg)
{
    const char* msg_string;
    const void* data;
    uint32_t length;
    caps_t obj;
    caps_t subobj;
    if(!strcmp(name, "flora serial test")) {
        int32_t i;
        const char* string;
        float f;
        caps_read_binary(msg, &data, &length);
        caps_parse(data, length, &obj);
        caps_read_integer(obj, &i);
        FLORA_LOG("flora_recv_post_func:%d\n", i);
        caps_read_string(obj, &string);
        FLORA_LOG("flora_recv_post_func:%s\n", string);
        caps_read_object(obj, &subobj);
        caps_read_float(subobj, &f);
        FLORA_LOG("flora_recv_post_func:%f\n", f);
        caps_destroy(subobj);
        caps_destroy(obj);
    } else {
        caps_read_string(msg, &msg_string);
        FLORA_LOG("post callback: msg_name(%s), msgtype(%d), msg(%s)\n", name, msgtype, msg_string);
    }
}

static void flora_recv_call_func(const char *name, caps_t msg, void *arg, flora_call_reply_t reply)
{
    const char* msg_string;
    const char* reply_string;
    caps_read_string(msg, &msg_string);
    FLORA_LOG("call callback: msg_name(%s), msg(%s)\n", name, msg_string);
    sleep(2);
    flora_call_reply_write_code(reply, FLORA_CLI_SUCCESS);
    FLORA_LOG("%s callback: code(%d)\n", msg_string, reply->ret_code);
    flora_call_reply_write_data(reply, msg);
    //caps_read_string(reply->data, &reply_string);
    //FLORA_LOG("%s callback: data(%s)\n", msg_string, reply_string);
}

static void* flora_cli_thread_func(void* arg)
{
    flora_agent_t flora_client;
    flora_agent_config_param_t flora_cli_config;
    flora_call_result flora_reply;
    int32_t ret;
    caps_t cli_msg;
    caps_t obj;
    caps_t subobj;
    char odata[256];
    FLORA_LOG("flora_cli_thread_func start\n");
    flora_client = flora_agent_create();
    flora_cli_config.sock_path = "unix:/var/run/flora.sock#client";
    flora_agent_config(flora_client, FLORA_AGENT_CONFIG_URI, &flora_cli_config);
    flora_agent_start(flora_client, 0);
    FLORA_LOG("flora_client channel_id:%s\n", flora_client->pChannel->channel_id);
    FLORA_LOG("flora_client sock_path:%s\n", flora_client->pChannel->pSock->uri);
    memset(&flora_reply, 0, sizeof(flora_call_result));
    while(1) {
        cli_msg = caps_create();
        caps_write_string(cli_msg, "persist post+++");
        FLORA_LOG("flora_agent_post persist start\n");
        flora_agent_post(flora_client, "flora persist test", cli_msg, FLORA_MSGTYPE_PERSIST);
        FLORA_LOG("flora_agent_post persist end\n");
        caps_destroy(cli_msg);
        cli_msg = caps_create();
        caps_write_string(cli_msg, "call");
        FLORA_LOG("flora test start\n");
        ret = flora_agent_call(flora_client, "flora_lite test", cli_msg, "server", &flora_reply, 0);
        if(ret != FLORA_CLI_SUCCESS) {
            FLORA_LOG("flora test call error:%d\n", ret);
        } else {
            const char* reply_string;
            caps_read_string(flora_reply.data, &reply_string);
            FLORA_LOG("flora test call reply: ret_code(%d), data(%s)\n", flora_reply.ret_code, reply_string);
            //FLORA_LOG("flora_client call reply: ret_code(%d)\n", flora_reply.ret_code);
            flora_call_reply_end(&flora_reply);
        }
        caps_destroy(cli_msg);
        cli_msg = caps_create();
        caps_write_string(cli_msg, "call11");
        FLORA_LOG("flora test1 start\n");
        ret = flora_agent_call(flora_client, "flora_lite test1", cli_msg, "server", &flora_reply, 1000);
        if(ret != FLORA_CLI_SUCCESS) {
            FLORA_LOG("flora test1 call1 error:%d\n", ret);
        } else {
            const char* reply_string;
            caps_read_string(flora_reply.data, &reply_string);
            FLORA_LOG("flora test1 call1 reply: ret_code(%d), data(%s)\n", flora_reply.ret_code, reply_string);
            //FLORA_LOG("flora_client call1 reply: ret_code(%d)\n", flora_reply.ret_code);
            flora_call_reply_end(&flora_reply);
        }
        caps_destroy(cli_msg);
        cli_msg = caps_create();
        caps_write_string(cli_msg, "call222");
        FLORA_LOG("flora test2 start\n");
        ret = flora_agent_call(flora_client, "flora_lite test2", cli_msg, "server", &flora_reply, 5000);
        if(ret != FLORA_CLI_SUCCESS) {
            FLORA_LOG("flora test2 call2 error:%d\n", ret);
        } else {
            const char* reply_string;
            caps_read_string(flora_reply.data, &reply_string);
            FLORA_LOG("flora test2 call2 reply: ret_code(%d), data(%s)\n", flora_reply.ret_code, reply_string);
            //FLORA_LOG("flora_client call2 reply: ret_code(%d)\n", flora_reply.ret_code);
            flora_call_reply_end(&flora_reply);
        }
        caps_destroy(cli_msg);
        cli_msg = caps_create();
        caps_write_string(cli_msg, "flora_lite instant post");
        FLORA_LOG("flora test3 start\n");
        flora_agent_post(flora_client, "flora_lite test3", cli_msg, FLORA_MSGTYPE_INSTANT);
        FLORA_LOG("flora test3 end\n");
        caps_destroy(cli_msg);
        obj = caps_create();
        caps_write_integer(obj, 100);
        caps_write_string(obj, "hello world");
        subobj = caps_create();
        caps_write_float(subobj, 0.1);
        caps_write_object(obj, subobj);
        ret = caps_serialize(obj, odata, 256);
        FLORA_LOG("caps_serialize:%d\n",ret);
        caps_destroy(subobj);
        caps_destroy(obj);
        cli_msg = caps_create();
        caps_write_binary(cli_msg, odata, ret);
        FLORA_LOG("flora serial test start\n");
        flora_agent_post(flora_client, "flora serial test", cli_msg, FLORA_MSGTYPE_INSTANT);
        FLORA_LOG("flora serial test end\n");
        caps_destroy(cli_msg);
        sleep(1);
    }
}

static void* flora_server_thread_func(void* arg)
{
    flora_agent_t flora_server;
    flora_agent_config_param_t flora_server_config;
    flora_message_t *pMsg;
    FLORA_LOG("flora_server_thread_func start\n");
    flora_server = flora_agent_create();
    //flora_server_config.sock_path = "unix:/var/run/flora.sock#server";
    flora_server_config.sock_path = "unix:/var/run/flora.sock";
    flora_agent_config(flora_server, FLORA_AGENT_CONFIG_URI, &flora_server_config);
    flora_server_config.value = 128;
    flora_agent_config(flora_server, FLORA_AGENT_CONFIG_BUFSIZE, &flora_server_config);
    flora_server_config.channel_id = "server";
    flora_agent_config(flora_server, FLOAR_AGENT_CONFIG_CHANNELID, &flora_server_config);
    flora_agent_subscribe(flora_server, "flora_lite test", flora_recv_post_func, NULL);
    flora_agent_subscribe(flora_server, "flora_lite test1", flora_recv_post_func, NULL);
    flora_agent_subscribe(flora_server, "flora_lite test2", flora_recv_post_func, NULL);
    flora_agent_subscribe(flora_server, "flora_lite test3", flora_recv_post_func, NULL);
    flora_agent_subscribe(flora_server, "flora serial test", flora_recv_post_func, NULL);
    flora_agent_declare_method(flora_server, "flora_lite test", flora_recv_call_func, NULL);
    flora_agent_declare_method(flora_server, "flora_lite test1", flora_recv_call_func, NULL);
    flora_agent_declare_method(flora_server, "flora_lite test2", flora_recv_call_func, NULL);
    flora_agent_declare_method(flora_server, "flora_lite test3", flora_recv_call_func, NULL);
    //flora_agent_start(flora_server, 0);
    //FLORA_LOG("flora_server_thread_func->flora_agent_start\n");
    while(1) {
        FLORA_LOG("server->flora_agent_start+++\n");
        flora_agent_start(flora_server, 0);
        FLORA_LOG("server->flora_agent_start---\n");
        sleep(1);
        FLORA_LOG("server->flora_cli_subscribe+++\n");
        flora_cli_subscribe(flora_server->pChannel, "flora persist test");
        FLORA_LOG("server->flora_cli_subscribe---\n");
        sleep(10);
        FLORA_LOG("server->flora_agent_close+++\n");
        flora_agent_close(flora_server);
        FLORA_LOG("server->flora_agent_close---\n");
        sleep(1);
    }
    //flora_agent_close(flora_server);
    //FLORA_LOG("flora_server_thread_func->flora_agent_close\n");
}

static void flora_monitor_func(const char *name, uint32_t msgtype, caps_t msg, void *arg)
{
    const char* msg_string;
    const void* data;
    uint32_t length;
    caps_t obj;
    caps_t subobj;
    if(!strcmp(name, "flora serial test")) {
        int32_t i;
        const char* string;
        float f;
        caps_read_binary(msg, &data, &length);
        caps_parse(data, length, &obj);
        caps_read_integer(obj, &i);
        FLORA_LOG("flora_monitor_func:%d\n", i);
        caps_read_string(obj, &string);
        FLORA_LOG("flora_monitor_func:%s\n", string);
        caps_read_object(obj, &subobj);
        caps_read_float(subobj, &f);
        FLORA_LOG("flora_monitor_func:%f\n", f);
        caps_destroy(subobj);
        caps_destroy(obj);
    } else {
        caps_read_string(msg, &msg_string);
        FLORA_LOG("monitor: msg_name(%s), msgtype(%d), msg(%s)\n", name, msgtype, msg_string);
    }
}

static void* flora_monitor_thread_func(void* arg)
{
    flora_agent_t flora_monitor;
    flora_agent_config_param_t flora_monitor_config;
    FLORA_LOG("flora_monitor_thread_func start\n");
    flora_monitor = flora_agent_create();
    flora_monitor_config.sock_path = "unix:/var/run/flora.sock#monitor";
    flora_agent_config(flora_monitor, FLORA_AGENT_CONFIG_URI, &flora_monitor_config);
    flora_monitor_config.value = 128;
    flora_agent_config(flora_monitor, FLORA_AGENT_CONFIG_BUFSIZE, &flora_monitor_config);
    flora_agent_subscribe(flora_monitor, "flora_lite test", flora_monitor_func, NULL);
    flora_agent_subscribe(flora_monitor, "flora_lite test1", flora_monitor_func, NULL);
    flora_agent_subscribe(flora_monitor, "flora_lite test2", flora_monitor_func, NULL);
    flora_agent_subscribe(flora_monitor, "flora_lite test3", flora_monitor_func, NULL);
    flora_agent_subscribe(flora_monitor, "flora serial test", flora_monitor_func, NULL);
    flora_agent_start(flora_monitor, 0);
    FLORA_LOG("flora_monitor channel_id:%s\n", flora_monitor->pChannel->channel_id);
    FLORA_LOG("flora_monitor_thread_func->flora_agent_start\n");
    while(1){
        sleep(10);
    }
    flora_agent_close(flora_monitor);
    FLORA_LOG("flora_monitor_thread_func->flora_agent_close\n");

	return NULL;
}

static int flora_test_pthread(int argc,int8_t * const argv[])
{
    printf("flora_test_pthread start\n");
    pthread_create(&flora_server_thread, NULL, flora_server_thread_func, NULL);
    pthread_create(&flora_cli_thread, NULL, flora_cli_thread_func, NULL);
    pthread_create(&flora_monitor_thread, NULL, flora_monitor_thread_func, NULL);
    printf("flora_test_pthread end\n");
    return 0;
}

#define max_args      (2)
#define flora_test_help  "flora_test"

int cmd_flora_test(void)
{
  YODALITE_REG_CMD(flora_test,max_args,flora_test_pthread,flora_test_help);
  return 0;
}

