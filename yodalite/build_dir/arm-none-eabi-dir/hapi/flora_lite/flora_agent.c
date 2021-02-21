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
#include <hapi/flora_agent.h>
#include <lib/mem_ext/mem_ext.h>

#ifdef CONFIG_MEM_EXT_ENABLE
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

static flora_message_t* flora_agent_msg_hit(flora_agent_t agent, const char *name)
{
    flora_message_t *pMsg;
    if(!agent) {
        FLORA_LOG("flora_asyncmsg_hit error:invalid param\n");
        return NULL;
    }
    pMsg = agent->msg_head;
    while(pMsg) {
        if(!strcmp(pMsg->msg_name, name))
            return pMsg;
        pMsg = pMsg->next;
    }
    return NULL;
}

static void flora_agent_msg_add(flora_agent_t agent, const char *name)
{
    if(agent->msg_head) {
        flora_message_t *pMsg;
        pMsg = (flora_message_t*)yodalite_malloc(sizeof(flora_message_t));
        if(!pMsg) {
            FLORA_LOG("flora_agent_msg_add error: no enough memory");
            return;
        }
        memset(pMsg, 0, sizeof(flora_message_t));
        pMsg->msg_name = name;
        agent->msg_tail->next = pMsg;
        pMsg->prev = agent->msg_tail;
        agent->msg_tail = pMsg;
    } else {
        agent->msg_head = (flora_message_t*)yodalite_malloc(sizeof(flora_message_t));
        if(!agent->msg_head) {
            FLORA_LOG("flora_agent_msg_add error: no enough memory");
            return;
        }
        memset(agent->msg_head, 0, sizeof(flora_message_t));
        agent->msg_head->msg_name = name;
        agent->msg_tail = agent->msg_head;
    }
}

static void flora_agent_msg_del(flora_agent_t agent, const char *name)
{
    flora_message_t *pMsg = agent->msg_head;
    while(pMsg) {
        if(!strcmp(pMsg->msg_name, name)) {
            flora_message_t *prev_msg, *next_msg;
            prev_msg = pMsg->prev;
            next_msg = pMsg->next;
            if(agent->msg_head == pMsg)
                agent->msg_head = next_msg;
            if(agent->msg_tail == pMsg)
                agent->msg_tail = prev_msg;
            if(prev_msg)
                prev_msg->next = next_msg;
            if(next_msg)
                next_msg->prev = prev_msg;
            yodalite_free(pMsg);
            break;
        }
        pMsg = pMsg->next;
    }
}


flora_agent_t flora_agent_create(void)
{
    flora_agent_t pAgent;
    pAgent = (flora_agent_t)yodalite_malloc(sizeof(struct flora_agent));
    if(!pAgent) {
        FLORA_LOG("flora_agent_create error:no enough memory\n");
        return NULL;
    }
    memset(pAgent, 0, sizeof(struct flora_agent));
    return pAgent;
}

void flora_agent_config(flora_agent_t agent, uint32_t key, flora_agent_config_param_t *param)
{
    if(!agent) {
        FLORA_LOG("flora_agent_config error: invalid agent\n");
        return;
    }
    if(key == FLORA_AGENT_CONFIG_URI)
        agent->uri = param->sock_path;
    else if(key == FLORA_AGENT_CONFIG_BUFSIZE)
        agent->msg_buf_size = param->value;
    else if(key == FLOAR_AGENT_CONFIG_CHANNELID)
        agent->channel_id = param->channel_id;
    else
        FLORA_LOG("flora_agent_config WARN: not support config\n");
}

int32_t flora_agent_call(flora_agent_t agent, const char *name, caps_t msg, const char *target, flora_call_result *result, uint32_t timeout)
{
    if(!agent) {
        FLORA_LOG("flora_agent_call error: invalid agent\n");
        return FLORA_CLI_EINVAL;
    }
    if(!agent->agent_start)
        return FLORA_CLI_ECLOSED;
    return flora_cli_call(agent->pChannel, name, msg, target, result, timeout);
}

int32_t flora_agent_post(flora_agent_t agent, const char *name, caps_t msg, uint32_t msgtype)
{
    if(!agent) {
        FLORA_LOG("flora_agent_post error: invalid agent\n");
        return FLORA_CLI_EINVAL;
    }
    if(!agent->agent_start)
        return FLORA_CLI_ECLOSED;
    return flora_cli_post(agent->pChannel, name, msg, msgtype);
}

void flora_agent_subscribe(flora_agent_t agent, const char *name, flora_cli_recv_post_func_t cb, void *arg)
{
    if(!agent) {
        FLORA_LOG("flora_agent_subscribe error: invalid agent\n");
        return;
    }
    agent->recv_post = cb;
    agent->recv_post_arg = arg;
    if(!flora_agent_msg_hit(agent, name)) {
        flora_agent_msg_add(agent, name);
    }
}

void flora_agent_unsubscribe(flora_agent_t agent, const char *name)
{
    flora_agent_msg_del(agent, name);
    if(!agent->msg_head) {
        agent->recv_post = NULL;
        agent->recv_post_arg = NULL;
    }
}

void flora_agent_declare_method(flora_agent_t agent, const char *name, flora_cli_recv_call_func_t cb, void *arg)
{
    if(!agent) {
        FLORA_LOG("flora_agent_declare_method error: invalid agent\n");
        return;
    }
    agent->recv_call = cb;
    agent->recv_call_arg = arg;
    if(!flora_agent_msg_hit(agent, name)) {
        flora_agent_msg_add(agent, name);
    }
}

void flora_agent_remove_method(flora_agent_t agent, const char *name)
{
    flora_agent_msg_del(agent, name);
    if(!agent->msg_head) {
        agent->recv_call = NULL;
        agent->recv_call_arg = NULL;
    }
}

void flora_agent_start(flora_agent_t agent, int32_t block)
{
    flora_cli_callback_t cb;
    flora_cli_callback_arg_t arg;
    flora_message_t *pMsg;
    if(!agent) {
        FLORA_LOG("flora_agent_start error: invalid agent\n");
        return;
    }
    if(agent->agent_start)
        return;
    cb.recv_post = agent->recv_post;
    cb.recv_call = agent->recv_call;
    cb.disconnected = NULL;
    arg.recv_post = agent->recv_post_arg;
    arg.recv_call = agent->recv_call_arg;
    arg.disconnected = NULL;
    flora_cli_connect(agent->uri, &cb, &arg, agent->msg_buf_size, &(agent->pChannel));
    if(agent->channel_id)
        flora_cli_set_channel_id(agent->pChannel, agent->channel_id);
    pMsg = agent->msg_head;
    while(pMsg) {
        flora_cli_subscribe(agent->pChannel, pMsg->msg_name);
        pMsg = pMsg->next;
    }
    agent->agent_start = 1;
}

void flora_agent_close(flora_agent_t agent)
{
    agent->agent_start = 0;
    if(agent->pChannel) {
        flora_cli_delete(agent->pChannel);
        agent->pChannel = NULL;
    }
}

void flora_agent_delete(flora_agent_t agent)
{
    flora_message_t *pMsg;
    if(!agent) {
        FLORA_LOG("flora_agent_delete error: invalid agent\n");
        return;
    }
    if(agent->pChannel)
        flora_cli_delete(agent->pChannel);
    pMsg = agent->msg_head;
    while(pMsg) {
        flora_message_t *temp;
        temp = pMsg;
        pMsg = pMsg->next;
        yodalite_free(temp);
    }
    yodalite_free(agent);
}

