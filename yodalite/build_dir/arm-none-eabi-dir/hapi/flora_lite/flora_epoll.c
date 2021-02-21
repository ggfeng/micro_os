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
#include <hapi/flora_epoll.h>
#include <lib/mem_ext/mem_ext.h>

#ifdef CONFIG_MEM_EXT_ENABLE
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

static flora_epoll_event_t* flora_epoll_event_pop(flora_epoll_t* pEpoll)
{
    flora_epoll_event_t *ret, *temp;
    if(!pEpoll) {
        EPOLL_LOG("flora_epoll_event_pop error:invalid param\n");
        return NULL;
    }
    if(!pEpoll->event_head)
        return NULL;
    ret = pEpoll->event_head;
    temp = ret->next;
    if(!temp) {
        pEpoll->event_head = NULL;
        pEpoll->event_tail = NULL;
        return ret;
    }
    pEpoll->event_head = temp;
    temp->prev = NULL;
    ret->next = NULL;
    ret->prev = NULL;
    return ret;
}

static void flora_epoll_event_push(flora_epoll_t* pEpoll, flora_epoll_event_t* pEvent)
{
    if(!pEpoll || !pEvent) {
        EPOLL_LOG("flora_epoll_event_push error:invalid param\n");
        return;
    }
    if(!pEpoll->event_head) {
        pEpoll->event_head = pEvent;
        pEpoll->event_tail = pEvent;
        pEvent->next = NULL;
        pEvent->prev = NULL;
        pEpoll->event_avail_cb(pEpoll);
    } else {
        pEpoll->event_tail->next = pEvent;
        pEvent->prev = pEpoll->event_tail;
        pEpoll->event_tail = pEvent;
        pEvent->next = NULL;
    }
    return;
}

static void flora_epoll_event_avail(void* param)
{
    flora_epoll_t *pEpoll = (flora_epoll_t*)param;
    if(!pEpoll) {
        EPOLL_LOG("flora_epoll_event_avail error: invalid param\n");
        return;
    }
    pthread_cond_signal(&(pEpoll->epoll_cond));
}

flora_epoll_t* flora_epoll_create(int size)
{
    flora_epoll_t *pEpoll;
    int ret;
    pEpoll = (flora_epoll_t*)yodalite_malloc(sizeof(flora_epoll_t));
    if(!pEpoll) {
        EPOLL_LOG("flora_epoll_create error: no enough memory\n");
        return NULL;
    }
    memset(pEpoll, 0, sizeof(flora_epoll_t));
    pthread_cond_init(&(pEpoll->epoll_cond), NULL);
    pthread_mutex_init(&(pEpoll->epoll_mutex), NULL);
    pEpoll->event_avail_cb = flora_epoll_event_avail;
    return pEpoll;
}

void flora_epoll_delete(flora_epoll_t* pEpoll)
{
    flora_epoll_event_t* pEvent;
    if(!pEpoll) {
        EPOLL_LOG("flora_epoll_delete error: invalid param\n");
        return;
    }
    pthread_mutex_destroy(&(pEpoll->epoll_mutex));
    pthread_cond_destroy(&(pEpoll->epoll_cond));
    pEvent = pEpoll->event_head;
    while(pEvent) {
        flora_epoll_event_t* temp = pEvent;
        pEvent = pEvent->next;
        yodalite_free(temp);
    }
    yodalite_free(pEpoll);
}

int flora_epoll_wait(flora_epoll_t* epfd, flora_epoll_event_t *event, int timeout_ms)
{
    flora_epoll_event_t *pEvent;
    struct timespec ts;
    int timeout_s;
    int timeout_nsec;

    if(!epfd || !event) {
        EPOLL_LOG("flora_epoll_wait error: invalid param epfd\n");
        return FLORA_EPOLL_EINVAL;
    }
    pthread_mutex_lock(&(epfd->epoll_mutex));
    if((epfd->event_pop = flora_epoll_event_pop(epfd)) == NULL) {
        if(timeout_ms<0) {
            pthread_cond_wait(&(epfd->epoll_cond), &(epfd->epoll_mutex));
        } else if(timeout_ms>0) {
            /* Calculate relative interval as current time plus
               number of seconds given argv[2] */
            if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                pthread_mutex_unlock(&(epfd->epoll_mutex));
                EPOLL_LOG("flora_epoll_wait fail:clock_gettime\n");
                return FLORA_EPOLL_EINVAL;
            }
            timeout_s = timeout_ms/1000;
            ts.tv_sec += timeout_s;
            timeout_nsec = (timeout_ms - timeout_s*1000)*1000000;
            ts.tv_nsec += timeout_nsec;
            pthread_cond_timedwait(&(epfd->epoll_cond), &(epfd->epoll_mutex), &ts);
        }
        epfd->event_pop = flora_epoll_event_pop(epfd);
    }
    pthread_mutex_unlock(&(epfd->epoll_mutex));
    if(epfd->event_pop) {
        pEvent = epfd->event_pop;
        memcpy(event, pEvent, sizeof(flora_epoll_event_t));
        yodalite_free(pEvent);
        return FLORA_EPOLL_SUCCESS;
    }
    return FLORA_EPOLL_ETIMEOUT;
}

int flora_epoll_post(flora_epoll_t* epfd, flora_epoll_event_t *event)
{
    flora_epoll_event_t *pEvent;
    int ret;
    if(!epfd) {
        EPOLL_LOG("flora_epoll_post error: invalid param epfd\n");
        return FLORA_EPOLL_EINVAL;
    }
    if(!event) {
        EPOLL_LOG("flora_epoll_post error: invalid param events\n");
        return FLORA_EPOLL_EINVAL;
    }
    pthread_mutex_lock(&(epfd->epoll_mutex));
    pEvent = (flora_epoll_event_t*)yodalite_malloc(sizeof(flora_epoll_event_t));
    if(!pEvent) {
        EPOLL_LOG("flora_epoll_post error: no enough memory\n");
        pthread_mutex_unlock(&(epfd->epoll_mutex));
        return FLORA_EPOLL_EINVAL;
    }
    memcpy(pEvent, event, sizeof(flora_epoll_event_t));
    flora_epoll_event_push(epfd, pEvent);
    pthread_mutex_unlock(&(epfd->epoll_mutex));
    return FLORA_EPOLL_SUCCESS;
}

