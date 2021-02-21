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
#include <hardware/platform.h>

static pthread_t input_event_recv_thread;
static pthread_t input_event_send1_thread;
static pthread_t input_event_send2_thread;

static void* input_event_recv_thread_func(void* arg)
{
    input_event_t input;
    INPUT_LOG("input_event_recv_thread_func START\n");
    while(1) {
        pal_input_get_event(&input);
        INPUT_LOG("recv input_event:tv_sec(%ld), tv_nsec(%ld)\n", input.ts.tv_sec, input.ts.tv_nsec);
        INPUT_LOG("recv input_evnet source:%s\n", input.event.input_source);
        INPUT_LOG("recv input_event:type(%d), code(%d), value(%d)\n", input.event.type, input.event.code, input.event.value);
    }
    return NULL;
}

static void* input_event_send1_thread_func(void* arg)
{
    struct input_event event;
    INPUT_LOG("input_event_send1_thread_func START\n");
    event.input_source = "input_event send1";
    event.type = EV_KEY;
    event.code = KEY_ESC;
    event.value = 1;
    while(1) {
        INPUT_LOG("input_event_send1\n");
        pal_input_put_event(&event);
        sleep(1);
    }
    return NULL;
}

static void* input_event_send2_thread_func(void* arg)
{
    struct input_event event;
    INPUT_LOG("input_event_send2_thread_func START\n");
    event.input_source = "input_event send2";
    event.type = EV_REL;
    event.code = KEY_1;
    event.value = 2;
    while(1) {
        INPUT_LOG("input_event_send2\n");
        pal_input_put_event(&event);
        sleep(1);
    }
    return NULL;
}

static int input_event_test_pthread(int argc,int8_t * const argv[])
{
    printf("input_event_test_pthread start\n");
    pal_input_init();
    pthread_create(&input_event_recv_thread, NULL, input_event_recv_thread_func, NULL);
    pthread_create(&input_event_send1_thread, NULL, input_event_send1_thread_func, NULL);
    pthread_create(&input_event_send2_thread, NULL, input_event_send2_thread_func, NULL);
    while(1) {
        INPUT_LOG("input_event_test_pthread running\n");
        sleep(3);
    }
    printf("input_event_test_pthread end\n");
    return 0;
}

#define max_args      (2)
#define input_event_test_help  "flora_test"

int cmd_input_event_test(void)
{
  YODALITE_REG_CMD(input_event_test, max_args, input_event_test_pthread, input_event_test_help);
  return 0;
}

