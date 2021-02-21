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
#include <hapi/app_mgr.h>
#include <hapi/netmgr.h>
#include <hapi/btmgr.h>

#ifdef CONFIG_MEM_EXT_ENABLE
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

#define chat_id "E33FCE60E7294A61B84C43C1A171DFD8"
#define music_id "RBA66C902A6347DD86CA8D419B0BB974"
#define phone_id "R1F6514A8E244E77942E1F1D5F24595B"

static pthread_t activation_demo_thread;

static void onChat(uint32_t msgtype, caps_t msg, void *arg)
{
    APPMGR_LOG("onChat\n");
}
static void onNetStatus(uint32_t msgtype, caps_t msg, void *arg)
{
    APPMGR_LOG("onNetStatus\n");
}
static app_intent_t chat_intent[] = {
    {"chat", onChat},
    {NETWORK_STATUS_NAME, onNetStatus},
};

static void onModuleSleep(caps_t msg, void *arg, flora_call_result* reply)
{
    APPMGR_LOG("onModuleSleep\n");
}
static void onPowerSleep(caps_t msg, void *arg, flora_call_result* reply)
{
    APPMGR_LOG("onModuleSleep\n");
}
static app_syscall_t chat_syscall[] = {
    {FLORA_MODULE_SLEEP, onModuleSleep},
    {FLORA_POWER_SLEEP, onPowerSleep},
};

static void* chat_prepare(void* param)
{
    APPMGR_LOG("chat_prepare\n");
    return NULL;
}

static int chat_running;
static void* chat_start(void* param)
{
    app_t *pApp = param;
    int i;
    APPMGR_LOG("chat_start\n");
    chat_running = 1;
    for(i=0; i<10; i++) {
        if(!chat_running){
	    APPMGR_LOG("chat quit\n");
            break;
	}
        sleep(1);
        APPMGR_LOG("chat running\n");
    }
    return NULL;
}

static void* chat_stop(void* param)
{
    APPMGR_LOG("chat_stop\n");
    chat_running = 0;
    return NULL;
}

app_create(chat, E33FCE60E7294A61B84C43C1A171DFD8, cut, chat_prepare, chat_start, chat_stop, NULL, chat_intent, NULL, chat_syscall);

static void onPlaySinger(uint32_t msgtype, caps_t msg, void *arg)
{
    APPMGR_LOG("onPlaySinger\n");
}
static void onMusicNetStatus(uint32_t msgtype, caps_t msg, void *arg)
{
    APPMGR_LOG("onMusicNetStatus\n");
}
static app_intent_t music_intent[] = {
    {"play_singer", onPlaySinger},
    {NETWORK_STATUS_NAME, onMusicNetStatus},
};

static void onMusicModuleSleep(caps_t msg, void *arg, flora_call_result* reply)
{
    APPMGR_LOG("onMusicModuleSleep\n");
}
static void onMusicPowerSleep(caps_t msg, void *arg, flora_call_result* reply)
{
    APPMGR_LOG("onMusicPowerSleep\n");
}
static app_syscall_t music_syscall[] = {
    {FLORA_MODULE_SLEEP, onMusicModuleSleep},
    {FLORA_POWER_SLEEP, onMusicPowerSleep},
};

void* music_prepare(void* param)
{
    APPMGR_LOG("music_prepare\n");
    return NULL;
}

static int music_running;
void* music_start(void* param)
{
    app_t *pApp = param;
    APPMGR_LOG("music_start\n");
    music_running = 1;
    while(music_running) {
        sleep(1);
        APPMGR_LOG("music running\n");
    }
    APPMGR_LOG("music quit\n");
    return NULL;
}

void* music_stop(void* param)
{
    APPMGR_LOG("music_stop\n");
    music_running = 0;
    return NULL;
}

app_create(music, RBA66C902A6347DD86CA8D419B0BB974, scene, music_prepare, music_start, music_stop, NULL, music_intent, NULL, music_syscall);

static void* activation_demo_func(void* arg)
{
    app_t *pApp;
    int ret;
    APPMGR_LOG("activation_demo_func start\n");
    app_register(&app_chat);
    app_register(&app_music);
    while(1) {
        pApp = app_pickup("chat");
        APPMGR_LOG("%s app_start+++\n", pApp->name);
        ret = app_start(pApp);
        APPMGR_LOG("%s app_start---:%d\n", pApp->name, ret);
        sleep(3);
        APPMGR_LOG("%s app_stop+++\n", pApp->name);
        ret = app_stop();
        APPMGR_LOG("%s app_stop---:%d\n", pApp->name, ret);
        pApp = app_pickup("music");
        APPMGR_LOG("%s app_start+++\n", pApp->name);
        ret = app_start(pApp);
        APPMGR_LOG("%s app_start---:%d\n", pApp->name, ret);
        sleep(3);
        pApp = app_select(chat_id);  //chat_id
        APPMGR_LOG("%s app_start+++\n", pApp->name);
        ret = app_start(pApp);
        APPMGR_LOG("%s app_start---:%d\n", pApp->name, ret);
        sleep(20);
        pApp = app_select(music_id);  //music_id
        APPMGR_LOG("%s app_start+++\n", pApp->name);
        ret = app_start(pApp);
        APPMGR_LOG("%s app_start---:%d\n", pApp->name, ret);
        sleep(3);
    }
}

static int appmgr_test_pthread(int argc,int8_t * const argv[])
{
    printf("appmgr_test_pthread start\n");
    pthread_create(&activation_demo_thread, NULL, activation_demo_func, NULL);
    printf("appmgr_test_pthread end\n");
    return 0;
}

#define max_args      (2)
#define appmgr_test_help  "appmgr_test"

int cmd_appmgr_test(void)
{
  YODALITE_REG_CMD(appmgr_test,max_args,appmgr_test_pthread,appmgr_test_help);
  return 0;
}


