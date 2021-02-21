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
#include<osal/pthread.h>
#include <osal/time.h>
#include <hardware/platform.h>
#include <hapi/flora_epoll.h>

#ifdef CONFIG_MEM_EXT_ENABLE
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

static flora_epoll_t *input_epoll;

int pal_input_init(void)
{
    input_epoll = flora_epoll_create(1);
    if(!input_epoll) {
        INPUT_LOG("pal_input_init fail: flora_epoll_create\n");
        return STATUS_EINVAL;
    }
    return STATUS_OK;
}

int pal_input_get_event(input_event_t* event)
{
    flora_epoll_event_t epoll_event;
    input_event_t* pInput;
    if(!event) {
        INPUT_LOG("pal_input_get_event fail for invalid event\n");
        return STATUS_EINVAL;
    }
    flora_epoll_wait(input_epoll, &epoll_event, -1);
    pInput = (input_event_t*)epoll_event.data.ptr;
    memcpy(event, pInput, sizeof(input_event_t));
    yodalite_free(pInput);
    return STATUS_OK;
}

int pal_input_put_event(struct input_event *event)
{
    input_event_t *pInput;
    flora_epoll_event_t epoll_event;
    if(!event) {
        INPUT_LOG("pal_input_put_event fail for invalid event\n");
        return STATUS_EINVAL;
    }
    pInput = (input_event_t*)yodalite_malloc(sizeof(input_event_t));
    if(!pInput) {
        INPUT_LOG("pal_input_put_event error: no enough memory\n");
        return STATUS_EINVAL;
    }
    /* Calculate relative interval as current time plus
       number of seconds given argv[2] */
    if (clock_gettime(CLOCK_REALTIME, &(pInput->ts)) == -1) {
        INPUT_LOG("pal_input_put_event fail:clock_gettime\n");
        return STATUS_EINVAL;
    }
    memcpy(&(pInput->event), event, sizeof(struct input_event));
    epoll_event.events = EPOLLONESHOT;
    epoll_event.data.ptr = pInput;
    if(flora_epoll_post(input_epoll, &epoll_event) != FLORA_EPOLL_SUCCESS)
        return STATUS_EINVAL;
    else
        return STATUS_OK;
}
