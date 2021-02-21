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
#include <hapi/app_mgr.h>
#include <lib/mem_ext/mem_ext.h>

#ifdef CONFIG_MEM_EXT_ENABLE
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

static app_queue_t gApp_queue = {
    .head = NULL,
    .tail = NULL,
};
static app_stack_t gApp_stack = {
    .top = NULL,
    .bot = NULL,
    .depth = 0,
};
static app_t* gApp = NULL;
static pthread_t gApp_thread = 0;
static flora_epoll_t* gApp_epoll = NULL;
static app_status_t gApp_status = IDLE;    /*0: idle, 1:running, 2:suspend, 3:resume*/


int app_register(app_t *pApp)
{
    flora_agent_config_param_t app_flora_config;
    int i;
    if(!pApp)
        return -1;
    if(!gApp_queue.head) {
        gApp_queue.head = pApp;
        gApp_queue.tail = pApp;
        pApp->next = NULL;
        pApp->prev = NULL;
    } else {
        gApp_queue.tail->next = pApp;
        pApp->prev = gApp_queue.tail;
        pApp->next = NULL;
        gApp_queue.tail = pApp;
    }
    pApp->app_itc = flora_agent_create();
    if(!pApp->app_itc) {
        APPMGR_LOG("app_register error: flora_agent_create fail\n");
        return -1;
    }
    app_flora_config.sock_path = APPMGR_SOCK;
    flora_agent_config(pApp->app_itc, FLORA_AGENT_CONFIG_URI, &app_flora_config);
    app_flora_config.channel_id = pApp->name;
    flora_agent_config(pApp->app_itc, FLOAR_AGENT_CONFIG_CHANNELID, &app_flora_config);

    /*Some specific setting need be done in onPrepare, such as un-common flora_agent_confign etc.*/
    if(pApp->onPrepare)
        pApp->onPrepare(pApp);

    for(i=0; i<pApp->intent_cnt; i++) {
        flora_agent_subscribe(pApp->app_itc, pApp->intentDeal[i].task_name, pApp->intent_callback, pApp->intent_argument);
    }
    for(i=0; i<pApp->syscall_cnt; i++) {
        flora_agent_declare_method(pApp->app_itc, pApp->syscallDeal[i].task_name, pApp->syscall_callback, pApp->syscall_argument);
    }
    return 0;
}

app_t* app_pickup(const char* app_name)
{
    app_t *pApp = gApp_queue.head;
    while(pApp) {
        if(!strcmp(pApp->name, app_name))
            return pApp;
        pApp = pApp->next;
    }
    return pApp;
}

app_t* app_select(const char* app_id)
{
    app_t *pApp = gApp_queue.head;
    while(pApp) {
        if(!strcmp(pApp->appId, app_id))
            return pApp;
        pApp = pApp->next;
    }
    return pApp;
}

static void app_stack_push(app_t *pApp)
{
    app_index_t *pApp_index = (app_index_t*)yodalite_malloc(sizeof(app_index_t));
    if(!pApp_index) {
        APPMGR_LOG("app_stack_push error: no enough memory\n");
        return;
    }
    memset(pApp_index, 0, sizeof(app_index_t));
    pApp_index->app_name = pApp->name;
    APPMGR_LOG("app_stack_push %s\n",pApp_index->app_name);
    if(!gApp_stack.depth) {
        gApp_stack.top = pApp_index;
        gApp_stack.bot = pApp_index;
        gApp_stack.depth = 1;
    } else {
        gApp_stack.top->prev = pApp_index;
        pApp_index->next = gApp_stack.top;
        gApp_stack.top = pApp_index;
        gApp_stack.depth++;
    }
    if(MAX_TASK_STACK_DEPTH >0) {
        while(gApp_stack.depth > MAX_TASK_STACK_DEPTH) {
            pApp_index = gApp_stack.bot;
            gApp_stack.bot = pApp_index->prev;
            if(gApp_stack.bot)
                gApp_stack.bot->next = NULL;
            yodalite_free(pApp_index);
            gApp_stack.depth--;
        }
    }
}

static app_t* app_stack_pop(void)
{
    const char *app_name;
    app_t *pApp;
    app_index_t *pApp_index;
    if(!gApp_stack.depth) {
        APPMGR_LOG("app_stack_pop wanning: empty\n");
        return NULL;
    }
    pApp_index = gApp_stack.top;
    app_name = pApp_index->app_name;
    APPMGR_LOG("app_stack_pop %s\n",app_name);
    gApp_stack.top = pApp_index->next;
    if(gApp_stack.top)
        gApp_stack.top->prev = NULL;
    gApp_stack.depth--;
    yodalite_free(pApp_index);
    pApp = app_pickup(app_name);
    return pApp;
}

static void* app_thread_func(void* arg)
{
    flora_epoll_event_t event;
    app_t *pApp;
    pthread_detach(pthread_self());
    gApp = arg;
    gApp_status = RUNNING;
    while(1) {
        switch(gApp_status) {
            case IDLE:
                flora_epoll_wait(gApp_epoll, &event, -1);
                pApp = event.data.ptr;
                gApp = pApp;
                gApp_status = RUNNING;
                break;
            case RUNNING:
                flora_agent_start(gApp->app_itc, 0);
                if(gApp->onStart)
                    gApp->onStart(gApp);
                if(gApp_status == RUNNING) {
                    if(!strcmp(gApp->form, APP_CUT))
                        gApp_status = RESUME;
                    else
                        gApp_status = IDLE;
                    if(gApp->onStop)
                        gApp->onStop(gApp);
                    flora_agent_close(gApp->app_itc);
                } else {
                    flora_agent_close(gApp->app_itc);
                }
                break;
            case SUSPEND:
                flora_epoll_wait(gApp_epoll, &event, -1);
                pApp = event.data.ptr;
                if(!strcmp(gApp->form, APP_SCENE)) {
                    app_stack_push(gApp);
                    gApp = pApp;
                    gApp_status = RUNNING;
                } else if(!strcmp(gApp->form, APP_CUT)) {
                    gApp = pApp;
                    gApp_status = RUNNING;
                } else {
                    APPMGR_LOG("app %s warning: unknown app_form %s\n", gApp->name, gApp->form);
                }
                break;
            case RESUME:
                gApp = app_stack_pop();
                if(!gApp) {
                    gApp_status = IDLE;
                } else {
                    gApp_status = RUNNING;
                }
                break;
            default:
                APPMGR_LOG("app %s warning: unknown app_status %d\n", gApp->name, gApp_status);
                break;
        }
    }
}

static void app_thread_create(app_t *pApp)
{
    gApp_epoll = flora_epoll_create(FLORA_CLI_EPOLL_SIZE);
    pthread_create(&gApp_thread, NULL, app_thread_func, pApp);
}

int app_start(app_t *pApp)
{
    flora_epoll_event_t event;
    if(!pApp)
        return -1;
    if(!pApp->app_itc) {
        APPMGR_LOG("app_start error: no flora_agent\n");
        return -1;
    }
    if(!gApp_thread) {
        app_thread_create(pApp);
    } else {
        if(gApp_status == RUNNING) {
            if(gApp == pApp)
                return 0;
            if(!strcmp(gApp->form, APP_UNINTERRUPTABLE))
                return -1;
            gApp_status = SUSPEND;
            if(gApp->onStop)
                gApp->onStop(gApp);
        }
        event.events = EPOLLWAKEUP;
        event.data.ptr = pApp;
        flora_epoll_post(gApp_epoll, &event);
    }
    return 0;
}

int app_stop(void)
{
    flora_epoll_event_t event;
    if(!gApp)
        return -1;
    if(!gApp->app_itc) {
        APPMGR_LOG("app_stop error: no flora_agent\n");
        return -1;
    }
    gApp_status = IDLE;
    if(gApp->onStop)
        gApp->onStop(gApp);
    return 0;
}
