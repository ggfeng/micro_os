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
#include <hapi/flora_cli.h>
#include <lib/mem_ext/mem_ext.h>

#ifdef CONFIG_MEM_EXT_ENABLE
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

static flora_cli_sock_t* gflora_sock_head = NULL;
static flora_thread_arg_queue_t gflora_thread_arg_queue = { \
    .head = NULL,   \
    .tail = NULL,   \
};
static flora_thread_arg_queue_t gflora_thread_arg_recycle = { \
    .head = NULL,   \
    .tail = NULL,   \
};
static flora_mutex_queue_t gflora_mutex_recycle = { \
    .head = NULL,   \
    .tail = NULL,   \
};
static flora_channel_queue_t gflora_channel_recycle = { \
    .head = NULL,   \
    .tail = NULL,   \
};

static void flora_mutex_recycle(flora_mutex_t* channel_mutex)
{
    flora_mutex_t *pMutex = channel_mutex;
    if(!gflora_mutex_recycle.head) {
        gflora_mutex_recycle.head = pMutex;
        gflora_mutex_recycle.tail = pMutex;
    } else {
        pMutex->next = gflora_mutex_recycle.head;
        gflora_mutex_recycle.head->prev = pMutex;
        gflora_mutex_recycle.head = pMutex;
    }
    pMutex->flag = FLORA_MUTEX_RECYCLE;
}

static int flora_mutex_is_recycle(flora_mutex_t* channel_mutex)
{
    if(channel_mutex->flag == FLORA_MUTEX_RECYCLE)
        return 1;
    else
        return 0;
}
static flora_mutex_t* flora_mutex_create(const char* channel_id)
{
    flora_mutex_t* pMutex;
    pMutex = (flora_mutex_t*)yodalite_malloc(sizeof(flora_mutex_t));
    if(!pMutex) {
        FLORA_LOG("flora_mutex_create error:no enough memory\n");
        return NULL;
    }
    memset(pMutex, 0, sizeof(flora_mutex_t));
    pMutex->channel_id = channel_id;
    pMutex->mutex = (pthread_mutex_t*)yodalite_malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(pMutex->mutex, NULL);
    return pMutex;
}

static void flora_mutex_destroy(flora_mutex_t* channel_mutex)
{
    flora_mutex_t *trace;
    flora_mutex_t *prev;
    flora_mutex_t *next;
    flora_mutex_t *destroy;
    pthread_mutex_destroy(channel_mutex->mutex);
    yodalite_free(channel_mutex->mutex);
    yodalite_free(channel_mutex);
    trace = gflora_mutex_recycle.head;
    while(trace) {
        if(!trace->mutex_cnt && (trace->flag == FLORA_MUTEX_UNLOCK||trace->flag == FLORA_MUTEX_DEL)) {
            prev = trace->prev;
            next = trace->next;
            if(prev) {
                prev->next = next;
            } else {
                gflora_mutex_recycle.head = next;
            }
            if(next) {
                next->prev = prev;
            } else {
                gflora_mutex_recycle.tail = prev;
            }
            destroy = trace;
            trace = trace->next;
            pthread_mutex_destroy(destroy->mutex);
            yodalite_free(destroy->mutex);
            yodalite_free(destroy);
        } else {
            trace = trace->next;
        }
    }
}

static int flora_mutex_lock(flora_mutex_t *channel_mutex)
{
    channel_mutex->mutex_cnt++;
    if(channel_mutex->mutex_cnt == 1) {
        channel_mutex->flag = FLORA_MUTEX_LOCK;
        pthread_mutex_lock(channel_mutex->mutex);
        if(flora_mutex_is_recycle(channel_mutex)) {
            FLORA_LOG("flora_mutex_lock recycled\n");
            return FLORA_CLI_ENEXISTS;
        }
    }
    return FLORA_CLI_SUCCESS;
}

static void flora_mutex_unlock(flora_mutex_t *channel_mutex)
{
    channel_mutex->mutex_cnt--;
    if(channel_mutex->mutex_cnt < 0) {
        FLORA_ASSERT("flora_mutex_unlock(%s) mutex_cnt(%d) error\n",channel_mutex->channel_id,channel_mutex->mutex_cnt);
        return;
    }
    if(!channel_mutex->mutex_cnt) {
        pthread_mutex_unlock(channel_mutex->mutex);
        channel_mutex->flag = FLORA_MUTEX_UNLOCK;
    }
}

static int32_t flora_add_channel(flora_cli_sock_t *pSock, flora_cli_callback_t *cb, const char* channel_id, flora_cli_callback_arg_t *arg, uint32_t msg_buf_size)
{
    flora_cli_channel_t *pChannel;
    if(!pSock) {
        FLORA_LOG("flora_add_channel error:invalid param\n");
        return FLORA_CLI_EINVAL;
    }
    pChannel = (flora_cli_channel_t*)yodalite_malloc(sizeof(flora_cli_channel_t));
    if(!pChannel) {
        FLORA_LOG("flora_add_channel error:no enough memory\n");
        return FLORA_CLI_EINVAL;
    }
    memset(pChannel, 0, sizeof(flora_cli_channel_t));
    pChannel->channel_epoll = flora_epoll_create(FLORA_CLI_EPOLL_SIZE);
    if(!pChannel->channel_epoll) {
        FLORA_LOG("flora_add_channel error: flora_epoll_create fail\n");
        return FLORA_CLI_EINVAL;
    }
    if(cb) {
        pChannel->callback.recv_post = cb->recv_post;
        pChannel->callback.recv_call = cb->recv_call;
        pChannel->callback.disconnected = cb->disconnected;
    }
    if(arg) {
        pChannel->callback_arg.recv_post = arg->recv_post;
        pChannel->callback_arg.recv_call = arg->recv_call;
        pChannel->callback_arg.disconnected = arg->disconnected;
    }
    pChannel->pSock = pSock;
    if(channel_id)
        pChannel->channel_id = channel_id;
    else
        pChannel->channel_id = DEFAULT_ID;
    if(pSock->pChannel_head) {
        pSock->pChannel_tail->next = pChannel;
        pChannel->prev = pSock->pChannel_tail;
        pSock->pChannel_tail = pChannel;
    } else {
        pSock->pChannel_head = pChannel;
        pSock->pChannel_tail = pChannel;
    }
    pChannel->msg_buf_size = msg_buf_size;
    pChannel->channel_mutex = flora_mutex_create(pChannel->channel_id);
    return FLORA_CLI_SUCCESS;
}

static void flora_free_channel(flora_cli_t handle)
{
    flora_cli_t trace = gflora_channel_recycle.head;
    flora_cli_t prev;
    flora_cli_t next;
    flora_cli_t destroy;
    if(handle) {
        yodalite_free(handle);
    }
    while(trace) {
        if(trace->flag == FLORA_CHANNEL_IDLE) {
            prev = trace->prev;
            next = trace->next;
            if(prev) {
                prev->next = next;
            } else {
                gflora_channel_recycle.head = next;
            }
            if(next) {
                next->prev = prev;
            } else {
                gflora_channel_recycle.tail = prev;
            }
            destroy = trace;
            trace = trace->next;
            yodalite_free(destroy);
        } else {
            trace = trace->next;
	}
    }
}

static void flora_recycle_channel(flora_cli_t handle)
{
    if(handle) {
        handle->prev = NULL;
        handle->next = NULL;
        if(!gflora_channel_recycle.head) {
            gflora_channel_recycle.head = handle;
            gflora_channel_recycle.tail = handle;
        } else {
            handle->next = gflora_channel_recycle.head;
            gflora_channel_recycle.head->prev = handle;
            gflora_channel_recycle.head = handle;
        }
    }
}

static flora_message_t* flora_asyncmsg_hit(flora_cli_t handle, const char *name)
{
    flora_message_t *pMsg;
    if(!handle) {
        FLORA_LOG("flora_asyncmsg_hit error:invalid param\n");
        return NULL;
    }
    pMsg = handle->async_msg_head;
    while(pMsg) {
        if(!strcmp(pMsg->msg_name, name))
            return pMsg;
        pMsg = pMsg->next;
    }
    return NULL;
}

static int32_t flora_asyncmsg_add(flora_cli_t handle, const char *name)
{
    flora_message_t *pMsg;
    if(!handle) {
        FLORA_LOG("flora_asyncmsg_add error:invalid param\n");
        return FLORA_CLI_EINVAL;
    }
    pMsg = (flora_message_t*)yodalite_malloc(sizeof(flora_message_t));
    if(!pMsg) {
        FLORA_LOG("flora_asyncmsg_add error:no enough memory\n");
        return FLORA_CLI_EINVAL;
    }
    memset(pMsg, 0, sizeof(flora_message_t));
    pMsg->msg_name = name;
    if(handle->msg_buf_size) {
        pMsg->persist_msg = (void*)yodalite_malloc(handle->msg_buf_size);
        if(!pMsg->persist_msg) {
            FLORA_LOG("flora_asyncmsg_add error:no enough memory\n");
            return FLORA_CLI_EINVAL;
        }
        memset(pMsg->persist_msg, 0, handle->msg_buf_size);
    }
    if(!handle->async_msg_head) {
        handle->async_msg_head = pMsg;
        handle->async_msg_tail = pMsg;
    } else {
        handle->async_msg_tail->next = pMsg;
        pMsg->prev = handle->async_msg_tail;
        handle->async_msg_tail = pMsg;
    }
    return FLORA_CLI_SUCCESS;
}

static int32_t flora_asyncmsg_del(flora_cli_t handle, flora_message_t *pMsg)
{
    flora_message_t *prev_msg, *next_msg;
    if(!handle) {
        FLORA_LOG("flora_asyncmsg_del error:invalid param\n");
        return FLORA_CLI_EINVAL;
    }
    if(!pMsg) {
        FLORA_LOG("flora_asyncmsg_del error::invalid param\n");
        return FLORA_CLI_EINVAL;
    }
    prev_msg = pMsg->prev;
    next_msg = pMsg->next;
    if(handle->async_msg_head == pMsg)
        handle->async_msg_head = next_msg;
    if(handle->async_msg_tail == pMsg)
        handle->async_msg_tail = prev_msg;
    if(prev_msg)
        prev_msg->next = next_msg;
    if(next_msg)
        next_msg->prev = prev_msg;
    if(pMsg->persist_msg)
        yodalite_free(pMsg->persist_msg);
    yodalite_free(pMsg);
    return FLORA_CLI_SUCCESS;
}

static int32_t flora_asyncmsg_paste(flora_cli_t handle, flora_message_t *pMsg)
{
    if(!handle) {
        FLORA_LOG("flora_asyncmsg_paste error:invalid handle\n");
        return FLORA_CLI_EINVAL;
    }
    if(!pMsg) {
        FLORA_LOG("flora_asyncmsg_paste error:invalid pMsg\n");
        return FLORA_CLI_EINVAL;
    }
    if(!handle->async_msg_head) {
        handle->async_msg_head = pMsg;
        handle->async_msg_tail = pMsg;
        pMsg->prev = NULL;
        pMsg->next = NULL;
    } else {
        handle->async_msg_tail->next = pMsg;
        pMsg->prev = handle->async_msg_tail;
        pMsg->next = NULL;
        handle->async_msg_tail = pMsg;
    }
    return FLORA_CLI_SUCCESS;
}

static flora_message_t* flora_persistmsg_hit(flora_cli_t handle, const char *name)
{
    flora_message_t *pMsg;
    if(!handle) {
        FLORA_LOG("flora_persistmsg_hit error:invalid param\n");
        return NULL;
    }
    pMsg = handle->persist_msg_head;
    while(pMsg) {
        if(!strcmp(pMsg->msg_name, name))
            return pMsg;
        pMsg = pMsg->next;
    }
    return NULL;
}

static int32_t flora_persistmsg_create(flora_cli_t handle, flora_message_t *pMsg, caps_t msg)
{
    if(!handle) {
        FLORA_LOG("flora_persistmsg_create error:invalid handle\n");
        return FLORA_CLI_EINVAL;
    }
    if(!pMsg) {
        FLORA_LOG("flora_persistmsg_create error:invalid pMsg\n");
        return FLORA_CLI_EINVAL;
    }
    if(!msg) {
        FLORA_LOG("flora_persistmsg_create error:invalid msg\n");
        return FLORA_CLI_EINVAL;
    }
    if(pMsg->persist_msg) {
        FLORA_LOG("flora_persistmsg_create error:invalid pMsg->persist_msg\n");
        return FLORA_CLI_EINVAL;
    }
    if(handle->msg_buf_size) {
        pMsg->persist_msg = (void*)yodalite_malloc(handle->msg_buf_size);
        if(!pMsg->persist_msg) {
            FLORA_LOG("flora_persistmsg_create error:no enough memory\n");
            return FLORA_CLI_EINVAL;
        }
        memset(pMsg->persist_msg, 0, handle->msg_buf_size);
        pMsg->persistmsg_size = caps_serialize(msg, pMsg->persist_msg, handle->msg_buf_size);
        if(handle->msg_buf_size < pMsg->persistmsg_size) {
            FLORA_LOG("flora_persistmsg_create error:(%s)msg_buf_size(%d)<persistmsg_size(%d)\n",handle->channel_id,handle->msg_buf_size,pMsg->persistmsg_size);
            pMsg->persistmsg_size = 0;
            yodalite_free(pMsg->persist_msg);
            pMsg->persist_msg = NULL;
            return FLORA_CLI_EINVAL;
        }
        return FLORA_CLI_SUCCESS;
    }
    return FLORA_CLI_EINVAL;
}

static int32_t flora_persistmsg_add(flora_cli_t handle, const char *name, caps_t msg, uint32_t msgtype)
{
    flora_message_t *pMsg;
    if(!handle) {
        FLORA_LOG("flora_persistmsg_add error:invalid param\n");
        return FLORA_CLI_EINVAL;
    }
    if(!msg) {
        FLORA_LOG("flora_persistmsg_add warning:invalid msg\n");
        return FLORA_CLI_EINVAL;
    }
    if(msgtype != FLORA_MSGTYPE_PERSIST) {
        FLORA_LOG("flora_persistmsg_add error:invalid msgtype\n");
        return FLORA_CLI_EINVAL;
    }
    pMsg = (flora_message_t*)yodalite_malloc(sizeof(flora_message_t));
    if(!pMsg) {
        FLORA_LOG("flora_persistmsg_add error:no enough memory\n");
        return FLORA_CLI_EINVAL;
    }
    memset(pMsg, 0, sizeof(flora_message_t));
    if(flora_persistmsg_create(handle, pMsg, msg) != FLORA_CLI_SUCCESS) {
        yodalite_free(pMsg);
        return FLORA_CLI_EINVAL;
    }
    pMsg->msg_name = name;
    pMsg->msgtype = msgtype;
    if(!handle->persist_msg_head) {
        handle->persist_msg_head = pMsg;
        handle->persist_msg_tail = pMsg;
    } else {
        handle->persist_msg_tail->next = pMsg;
        pMsg->prev = handle->persist_msg_tail;
        handle->persist_msg_tail = pMsg;
    }
    return FLORA_CLI_SUCCESS;
}

static int32_t flora_persistmsg_cut(flora_cli_t handle, flora_message_t *pMsg)
{
    flora_message_t *prev_msg, *next_msg;
    if(!handle) {
        FLORA_LOG("flora_persistmsg_cut error:invalid handle\n");
        return FLORA_CLI_EINVAL;
    }
    if(!pMsg) {
        FLORA_LOG("flora_persistmsg_cut error::invalid pMsg\n");
        return FLORA_CLI_EINVAL;
    }
    prev_msg = pMsg->prev;
    next_msg = pMsg->next;
    if(handle->persist_msg_head == pMsg)
        handle->persist_msg_head = next_msg;
    if(handle->persist_msg_tail == pMsg)
        handle->persist_msg_tail = prev_msg;
    if(prev_msg)
        prev_msg->next = next_msg;
    if(next_msg)
        next_msg->prev = prev_msg;
    return FLORA_CLI_SUCCESS;
}

static void flora_persistmsg_copy(flora_cli_t handle, flora_message_t *pMsg, caps_t msg)
{
    if(!pMsg || !msg)
        return;
    if(!pMsg->persist_msg)
        return;
    memset(pMsg->persist_msg, 0, handle->msg_buf_size);
    pMsg->persistmsg_size = caps_serialize(msg, pMsg->persist_msg, handle->msg_buf_size);
    if(pMsg->persistmsg_size > handle->msg_buf_size) {
        pMsg->persistmsg_size = 0;
    }
}

static flora_thread_arg_t* flora_thread_arg_create(flora_cli_channel_t *pChannel, flora_message_t *pMsg, caps_t msg)
{
    flora_thread_arg_t *pArg;
    flora_cli_channel_t *thread_pChannel;
    flora_message_t *thread_pMsg;
    int *pTimeout;
    caps_t thread_msg;

    if(!pChannel || !pMsg || !msg)
        return NULL;
    pArg = (flora_thread_arg_t*)yodalite_malloc(sizeof(flora_thread_arg_t));
    if(!pArg) {
        FLORA_LOG("flora_thread_arg_push pArg error: no enough memory\n");
        return NULL;
    }
    memset(pArg, 0, sizeof(flora_thread_arg_t));
    thread_pChannel = (flora_cli_channel_t*)yodalite_malloc(sizeof(flora_cli_channel_t));
    if(!thread_pChannel) {
        yodalite_free(pArg);
        FLORA_LOG("flora_thread_arg_push thread_pChannel error: no enough memory\n");
        return NULL;
    }
    memcpy(thread_pChannel, pChannel, sizeof(flora_cli_channel_t));
    thread_pMsg = (flora_message_t*)yodalite_malloc(sizeof(flora_message_t));
    if(!thread_pMsg) {
        yodalite_free(thread_pChannel);
        yodalite_free(pArg);
        FLORA_LOG("flora_thread_arg_push thread_pMsg error: no enough memory\n");
        return NULL;
    }
    memcpy(thread_pMsg, pMsg, sizeof(flora_message_t));
    pTimeout = (int*)yodalite_malloc(sizeof(int));
    if(!pTimeout) {
        yodalite_free(thread_pMsg);
        yodalite_free(thread_pChannel);
        yodalite_free(pArg);
        FLORA_LOG("flora_thread_arg_push pTimeout error: no enough memory\n");
        return NULL;
    }
    *pTimeout = 0;
    thread_msg = caps_create();
    caps_write_object(thread_msg, msg);
    pArg->pChannel = thread_pChannel;
    pArg->pMsg = thread_pMsg;
    pArg->msg = thread_msg;
    pArg->pTimeout = pTimeout;
    return pArg;
}
#if 0
static flora_thread_arg_t* flora_thread_arg_push(flora_cli_channel_t *pChannel, flora_message_t *pMsg, caps_t msg)
{
    flora_thread_arg_t *pArg;
    flora_cli_channel_t *thread_pChannel;
    flora_message_t *thread_pMsg;
    int *pTimeout;
    caps_t thread_msg;

    if(!pChannel || !pMsg || !msg)
        return NULL;
    pArg = (flora_thread_arg_t*)yodalite_malloc(sizeof(flora_thread_arg_t));
    if(!pArg) {
        FLORA_LOG("flora_thread_arg_push pArg error: no enough memory\n");
        return NULL;
    }
    memset(pArg, 0, sizeof(flora_thread_arg_t));
    thread_pChannel = (flora_cli_channel_t*)yodalite_malloc(sizeof(flora_cli_channel_t));
    if(!thread_pChannel) {
        yodalite_free(pArg);
        FLORA_LOG("flora_thread_arg_push thread_pChannel error: no enough memory\n");
        return NULL;
    }
    memcpy(thread_pChannel, pChannel, sizeof(flora_cli_channel_t));
    thread_pMsg = (flora_message_t*)yodalite_malloc(sizeof(flora_message_t));
    if(!thread_pMsg) {
        yodalite_free(thread_pChannel);
        yodalite_free(pArg);
        FLORA_LOG("flora_thread_arg_push thread_pMsg error: no enough memory\n");
        return NULL;
    }
    memcpy(thread_pMsg, pMsg, sizeof(flora_message_t));
    pTimeout = (int*)yodalite_malloc(sizeof(int));
    if(!pTimeout) {
        yodalite_free(thread_pMsg);
        yodalite_free(thread_pChannel);
        yodalite_free(pArg);
        FLORA_LOG("flora_thread_arg_push pTimeout error: no enough memory\n");
        return NULL;
    }
    *pTimeout = 0;
    thread_msg = caps_create();
    caps_write_object(thread_msg, msg);
    pArg->pChannel = thread_pChannel;
    pArg->pMsg = thread_pMsg;
    pArg->msg = thread_msg;
    pArg->pTimeout = pTimeout;
    if(!gflora_thread_arg_queue.head) {
        gflora_thread_arg_queue.head = pArg;
        gflora_thread_arg_queue.tail = pArg;
    } else {
        pArg->next = gflora_thread_arg_queue.head;
        gflora_thread_arg_queue.head->prev = pArg;
        gflora_thread_arg_queue.head = pArg;
    }
    return pArg;
}

static flora_thread_arg_t* flora_thread_arg_pop(void)
{
    flora_thread_arg_t *prev;
    flora_thread_arg_t *pop;

    pop = gflora_thread_arg_queue.tail;
    if(!pop)
        return pop;
    prev = pop->prev;
    if(prev) {
        prev->next = NULL;
        gflora_thread_arg_queue.tail = prev;
    }
    else {
        gflora_thread_arg_queue.head = NULL;
        gflora_thread_arg_queue.tail = NULL;
    }
    pop->prev = NULL;
    pop->next = NULL;
    return pop;
}
#endif

static void flora_thread_arg_recycle(flora_thread_arg_t *pArg)
{
    if(!gflora_thread_arg_recycle.head) {
        gflora_thread_arg_recycle.head = pArg;
        gflora_thread_arg_recycle.tail = pArg;
        pArg->prev = NULL;
        pArg->next = NULL;
    } else {
        pArg->next = gflora_thread_arg_recycle.head;
        gflora_thread_arg_recycle.head->prev = pArg;
        gflora_thread_arg_recycle.head = pArg;
        pArg->prev = NULL;
    }
}

static void flora_thread_arg_release(void)
{
    flora_thread_arg_t *prev;
    flora_thread_arg_t *release;

    release = gflora_thread_arg_recycle.tail;
    if(!release)
        return;
    prev = release->prev;
    if(prev) {
        prev->next = NULL;
        gflora_thread_arg_recycle.tail = prev;
    }
    else {
        gflora_thread_arg_recycle.head = NULL;
        gflora_thread_arg_recycle.tail = NULL;
    }
    caps_destroy(release->msg);
    yodalite_free(release->pTimeout);
    yodalite_free(release->pMsg);
    yodalite_free(release->pChannel);
    yodalite_free(release);
}

static int flora_thread_arg_exist(flora_thread_arg_t *pArg)
{
    flora_thread_arg_t *temp;
    temp = gflora_thread_arg_recycle.head;
    while(temp) {
        if(temp == pArg)
            return 1;
        temp = temp->next;
    }
#if 0
    temp = gflora_thread_arg_queue.head;
    while(temp) {
        if(temp == pArg)
            return 1;
        temp = temp->next;
    }
#endif
    return 0;
}

static void* flora_channel_thread_func(void* arg)
{
    flora_thread_arg_t *pArg;
    flora_cli_channel_t* pChannel;
    flora_message_t* pMsg;
    int* pTimeout;
    flora_epoll_event_t event;
    pthread_detach(pthread_self());
    pArg = arg;
    if(!pArg)
        return NULL;
    flora_thread_arg_recycle(pArg);
    pChannel = pArg->pChannel;
    pMsg = pArg->pMsg;
    pTimeout = pArg->pTimeout;
    if(pChannel->callback.recv_call)
        pChannel->callback.recv_call(pMsg->msg_name, pArg->msg, pChannel->callback_arg.recv_call, pMsg->msg_reply);
    event.events = EPOLLWAKEUP;
    event.data.ptr = NULL;
    if(!(*pTimeout)) {
        flora_epoll_post(pChannel->channel_epoll, &event);
        //FLORA_LOG("flora_channel_thread_func post\n");
    } else {
        flora_call_reply_end(pMsg->msg_reply);
        //FLORA_LOG("flora_channel_thread_func timeout\n");
    }
    flora_thread_arg_release();
    return NULL;
}

void flora_call_reply_write_code(flora_call_reply_t reply, int32_t code)
{
    if(reply)
        reply->ret_code = code;
}

void flora_call_reply_write_data(flora_call_reply_t reply, caps_t data)
{
    if(reply && data) {
        reply->data = caps_create();
        if(!reply->data) {
            FLORA_LOG("flora_call_reply_write_data ERROR: no enough memory\n");
            return;
        }
        caps_write_object(reply->data, data);
        caps_write_void(reply->data);
        caps_read_void(reply->data);
        reply->extra = NULL;
    }
}

void flora_call_reply_end(flora_call_reply_t reply)
{
    if(!reply)
        return;
    if(reply->data)
        caps_destroy(reply->data);
    if(reply->extra)
        yodalite_free(reply->extra);
}

int32_t flora_cli_subscribe(flora_cli_t handle, const char *name)
{
    flora_message_t *pMsg;
    int32_t ret = FLORA_CLI_ENEXISTS;

    if(!handle) {
        FLORA_LOG("flora_cli_subscribe error:invalid param\n");
        return FLORA_CLI_EINVAL;
    }
    handle->channel_mutex->flag = FLORA_MUTEX_LOCK;
    pthread_mutex_lock(handle->channel_mutex->mutex);
    pMsg = flora_persistmsg_hit(handle, name);
    if(pMsg) {
        flora_persistmsg_cut(handle, pMsg);
        flora_asyncmsg_paste(handle, pMsg);
        if(handle->callback.recv_post) {
            caps_t msg;
            if(pMsg->persistmsg_size) {
                caps_parse(pMsg->persist_msg, pMsg->persistmsg_size, &msg);
                handle->callback.recv_post(name, FLORA_MSGTYPE_PERSIST, msg, handle->callback_arg.recv_post);
                caps_destroy(msg);
            }
        }
        pthread_mutex_unlock(handle->channel_mutex->mutex);
        handle->channel_mutex->flag = FLORA_MUTEX_UNLOCK;
        return FLORA_CLI_SUCCESS;
    }
    pMsg = flora_asyncmsg_hit(handle, name);
    if(!pMsg) {
        ret = flora_asyncmsg_add(handle, name);
    }
    pthread_mutex_unlock(handle->channel_mutex->mutex);
    handle->channel_mutex->flag = FLORA_MUTEX_UNLOCK;
    return ret;
}

int32_t flora_cli_unsubscribe(flora_cli_t handle, const char *name)
{
    flora_message_t *pMsg;
    int32_t ret = FLORA_CLI_ENEXISTS;

    if(!handle) {
        FLORA_LOG("flora_cli_unsubscribe error:invalid param\n");
        return FLORA_CLI_EINVAL;
    }
    handle->channel_mutex->flag = FLORA_MUTEX_LOCK;
    pthread_mutex_lock(handle->channel_mutex->mutex);
    pMsg = flora_asyncmsg_hit(handle, name);
    if(pMsg) {
        ret = flora_asyncmsg_del(handle, pMsg);
    }
    pthread_mutex_unlock(handle->channel_mutex->mutex);
    handle->channel_mutex->flag = FLORA_MUTEX_UNLOCK;
    return ret;
}

int32_t flora_cli_declare_method(flora_cli_t handle, const char *name)
{
    return flora_cli_subscribe(handle, name);
}

int32_t flora_cli_remove_method(flora_cli_t handle, const char *name)
{
    return flora_cli_unsubscribe(handle, name);
}

int32_t flora_cli_post(flora_cli_t handle, const char *name, caps_t msg, uint32_t msgtype) 
{
    flora_cli_channel_t *pChannel;
    flora_message_t* pMsg;
    flora_mutex_t* channel_mutex;
    int ret = 0;
    if(!handle) {
        FLORA_LOG("flora_cli_post error:invalid param\n");
        return FLORA_CLI_EINVAL;
    }
    pChannel = handle->pSock->pChannel_head;
    while(pChannel) {
        pChannel->flag = FLORA_CHANNEL_SCAN;
        channel_mutex = pChannel->channel_mutex;
        if(channel_mutex->flag == FLORA_MUTEX_DEL) {
            pChannel->flag = FLORA_CHANNEL_IDLE;
            pChannel = pChannel->next;
            continue;
        }
        pMsg = flora_asyncmsg_hit(pChannel, name);
        if(pMsg) {
            ret = flora_mutex_lock(channel_mutex);
            if(ret != FLORA_CLI_SUCCESS) {
                flora_mutex_unlock(channel_mutex);
                pChannel->flag = FLORA_CHANNEL_IDLE;
                pChannel = pChannel->next;
                continue;
            }
            if(msgtype == FLORA_MSGTYPE_PERSIST) {
                if(msg) {
                    if(pMsg->persist_msg) {
                        flora_persistmsg_copy(pChannel, pMsg->persist_msg, msg);
                    } else {
                        flora_persistmsg_create(pChannel, pMsg, msg);
                    }
                }
            } else {
                if(pMsg->persist_msg) {
                    memset(pMsg->persist_msg, 0, handle->msg_buf_size);
                    pMsg->persistmsg_size = 0;
                }
            }
            pMsg->msgtype = msgtype;
            if(pChannel->callback.recv_post) {
                caps_t post_msg;
                post_msg = caps_create();
                caps_write_object(post_msg, msg);
                pChannel->callback.recv_post(name, msgtype, post_msg, pChannel->callback_arg.recv_post);
                caps_destroy(post_msg);
            }
            flora_mutex_unlock(channel_mutex);
        } else {
            if(msgtype == FLORA_MSGTYPE_PERSIST) {
                ret = flora_mutex_lock(channel_mutex);
                if(ret != FLORA_CLI_SUCCESS) {
                    flora_mutex_unlock(channel_mutex);
                    pChannel->flag = FLORA_CHANNEL_IDLE;
                    pChannel = pChannel->next;
                    continue;
                }
                pMsg = flora_persistmsg_hit(pChannel, name);
                if(!pMsg) {
                    flora_persistmsg_add(pChannel, name, msg, msgtype);
                } else {
                    if(pMsg->persist_msg && msg) {
                        flora_persistmsg_copy(pChannel, pMsg->persist_msg, msg);
                    }
                    pMsg->msgtype = msgtype;
                }
                flora_mutex_unlock(channel_mutex);
            }
        }
        pChannel->flag = FLORA_CHANNEL_IDLE;
        pChannel = pChannel->next;
    }
    return FLORA_CLI_SUCCESS;
}

int32_t flora_cli_call(flora_cli_t handle, const char *name, caps_t msg, const char *target, flora_call_result *result, uint32_t timeout)
{
    flora_cli_channel_t *pChannel;
    flora_message_t* pMsg;
    flora_mutex_t* channel_mutex;
    int ret = 0;
    if(!handle) {
        FLORA_LOG("flora_cli_call error:invalid handle\n");
        return FLORA_CLI_EINVAL;
    }
    if(!result) {
        FLORA_LOG("flora_cli_call error:invalid result\n");
        return FLORA_CLI_EINVAL;
    }
    memset(result, 0, sizeof(flora_call_result));
    pChannel = handle->pSock->pChannel_head;
    while(pChannel) {
        pChannel->flag = FLORA_CHANNEL_SCAN;
        channel_mutex = pChannel->channel_mutex;
        if(channel_mutex->flag == FLORA_MUTEX_DEL) {
            pChannel->flag = FLORA_CHANNEL_IDLE;
            pChannel = pChannel->next;
            continue;
        }
        if(!strcmp(pChannel->channel_id, target)) {
            pMsg = flora_asyncmsg_hit(pChannel, name);
            if(pMsg) {
                pMsg->msg_reply = result;
                if(!timeout) {
                    ret = flora_mutex_lock(channel_mutex);
                    if(ret != FLORA_CLI_SUCCESS) {
                        flora_mutex_unlock(channel_mutex);
                        pChannel->flag = FLORA_CHANNEL_IDLE;
                        pChannel = pChannel->next;
                        continue;
                    }
                    if(pChannel->callback.recv_call) {
                        pChannel->callback.recv_call(name, msg, pChannel->callback_arg.recv_call, result);
                    }
                    flora_mutex_unlock(channel_mutex);
                } else {
                    flora_epoll_event_t event;
                    channel_mutex->flag = FLORA_MUTEX_LOCK;
                    pthread_mutex_lock(channel_mutex->mutex);
                    flora_thread_arg_t* pArg = flora_thread_arg_create(pChannel, pMsg, msg);
                    if(!pArg) {
                        pthread_mutex_unlock(channel_mutex->mutex);
                        channel_mutex->flag = FLORA_MUTEX_UNLOCK;
                        pChannel->flag = FLORA_CHANNEL_IDLE;
                        return FLORA_CLI_EINVAL;
                    }
                    pthread_create(&(pChannel->channel_thread), NULL, flora_channel_thread_func, pArg);
                    ret = flora_epoll_wait(pChannel->channel_epoll, &event, timeout);
                    pChannel->channel_thread = 0;
                    if(ret != FLORA_EPOLL_SUCCESS) {
                        if(flora_thread_arg_exist(pArg)) {
                            int *pTimeout = pArg->pTimeout;
                            *pTimeout = 1;
                        }
                        pthread_mutex_unlock(channel_mutex->mutex);
                        channel_mutex->flag = FLORA_MUTEX_UNLOCK;
                        pChannel->flag = FLORA_CHANNEL_IDLE;
                        return FLORA_CLI_ETIMEOUT;
                    }
                    pthread_mutex_unlock(channel_mutex->mutex);
                    channel_mutex->flag = FLORA_MUTEX_UNLOCK;
                }
                pChannel->flag = FLORA_CHANNEL_IDLE;
                return FLORA_CLI_SUCCESS;
            }
            pChannel->flag = FLORA_CHANNEL_IDLE;
            break;
        }
        pChannel->flag = FLORA_CHANNEL_IDLE;
        pChannel = pChannel->next;
    }
    return FLORA_CLI_ENEXISTS;
}

int32_t flora_cli_connect(const char *uri, flora_cli_callback_t *cb, flora_cli_callback_arg_t *arg, uint32_t msg_buf_size, flora_cli_t *result)
{
    const char* channel_id = strchr(uri, '#');
    if(!gflora_sock_head) {
        gflora_sock_head = (flora_cli_sock_t*)yodalite_malloc(sizeof(flora_cli_sock_t));
        if(!gflora_sock_head) {
            FLORA_LOG("flora_cli_connect error:no enough memory\n");
            return FLORA_CLI_EINVAL;
        }
        memset(gflora_sock_head, 0, sizeof(flora_cli_sock_t));
        gflora_sock_head->uri = uri;
        if(channel_id)
            channel_id++;   /*skip # to get real channel_id start pointer*/
        if(flora_add_channel(gflora_sock_head, cb, channel_id, arg, msg_buf_size) != FLORA_CLI_SUCCESS) {
            FLORA_LOG("flora_cli_connect error: flora_add_channel fail\n");
            return FLORA_CLI_EINVAL;
        }
        *result = gflora_sock_head->pChannel_tail;
        return FLORA_CLI_SUCCESS;
    } else {
        flora_cli_sock_t *pSock = gflora_sock_head;
        flora_cli_sock_t *pSock_prev = NULL;
        while(pSock) {
            int cmp_ret;
            const char* tmp_uri = strchr(pSock->uri, '#');

            if(channel_id)
                cmp_ret = strncmp(pSock->uri, uri, (channel_id - uri));
            else if(tmp_uri)
                cmp_ret = strncmp(pSock->uri, uri, (tmp_uri - pSock->uri));
            else
                cmp_ret = strcmp(pSock->uri, uri);
            if(!cmp_ret) {
                if(channel_id)
                    channel_id++;   /*skip # to get real channel_id start pointer*/
                if(flora_add_channel(pSock, cb, channel_id, arg, msg_buf_size) != FLORA_CLI_SUCCESS) {
                    FLORA_LOG("flora_cli_connect error: flora_add_channel fail\n");
                    return FLORA_CLI_EINVAL;
                }
                *result = pSock->pChannel_tail;
                return FLORA_CLI_SUCCESS;
            } else {
                pSock_prev = pSock;
                pSock = pSock->next;
            }
        }
        pSock = (flora_cli_sock_t*)yodalite_malloc(sizeof(flora_cli_sock_t));
        if(!pSock) {
            FLORA_LOG("flora_cli_connect error:no enough memory\n");
            return FLORA_CLI_EINVAL;
        }
        memset(pSock, 0, sizeof(flora_cli_sock_t));
        pSock->uri = uri;
        if(channel_id)
            channel_id++;   /*skip # to get real channel_id start pointer*/
        if(flora_add_channel(pSock, cb, channel_id, arg, msg_buf_size) != FLORA_CLI_SUCCESS) {
            FLORA_LOG("flora_cli_connect error: flora_add_channel fail\n");
            return FLORA_CLI_EINVAL;
        }
        *result = pSock->pChannel_tail;
        if(pSock_prev)
            pSock_prev->next = pSock;
    }
    return FLORA_CLI_SUCCESS;
}

void flora_cli_delete(flora_cli_t handle)
{
    flora_cli_channel_t *prev_handle;
    flora_cli_channel_t *next_handle;
    flora_cli_sock_t *pSock;
    flora_message_t *pMsg;
    pthread_mutex_t* mutex;

    if(!handle) {
        FLORA_LOG("flora_cli_delete error:invalid param\n");
        return;
    }
    handle->channel_mutex->flag = FLORA_MUTEX_DEL;
    mutex = handle->channel_mutex->mutex;
    pthread_mutex_lock(mutex);
    prev_handle = handle->prev;
    next_handle = handle->next;
    pSock = handle->pSock;
    if(!pSock) {
        pthread_mutex_unlock(mutex);
        FLORA_LOG("flora_cli_delete error:invalid param\n");
        return;
    }
    pMsg = handle->async_msg_head;
    while(pMsg) {
        flora_message_t *temp;
        temp = pMsg;
        if(pMsg->persist_msg)
            yodalite_free(pMsg->persist_msg);
        pMsg = pMsg->next;
        yodalite_free(temp);
    }
    pMsg = handle->persist_msg_head;
    while(pMsg) {
        flora_message_t *temp;
        temp = pMsg;
        if(pMsg->persist_msg)
            yodalite_free(pMsg->persist_msg);
        pMsg = pMsg->next;
        yodalite_free(temp);
    }
    if(prev_handle)
        prev_handle->next = next_handle;
    else
        pSock->pChannel_head = next_handle;
    if(next_handle)
        next_handle->prev = prev_handle;
    else
        pSock->pChannel_tail = prev_handle;
    flora_epoll_delete(handle->channel_epoll);
    pthread_mutex_unlock(mutex);
    if(handle->channel_mutex->flag == FLORA_MUTEX_UNLOCK || handle->channel_mutex->flag == FLORA_MUTEX_DEL)
        flora_mutex_destroy(handle->channel_mutex);
    else
        flora_mutex_recycle(handle->channel_mutex);
    if(handle->flag == FLORA_CHANNEL_IDLE)
        flora_free_channel(handle);
    else
        flora_recycle_channel(handle);
}

void flora_cli_set_channel_id(flora_cli_t handle, const char *channel_id)
{
    if(!handle) {
        FLORA_LOG("flora_cli_set_name error:invalid param\n");
        return;
    }
    handle->channel_id = channel_id;
}

