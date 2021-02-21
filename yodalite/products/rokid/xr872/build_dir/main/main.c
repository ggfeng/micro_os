/*
 * Copyright (C) 2017 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include "common/framework/platform_init.h"
#include "net/wlan/wlan.h"
#include "driver/chip/hal_uart.h"

#include "lib/libtest/lib_test.h"
#include <string.h>
#include  "yodalite_autoconf.h"
#include "lib/shell/shell.h"

#ifdef CONFIG_OSAL_UNISTD
#include <osal/unistd.h>
#endif

#if CONFIG_SHELL_ENABLE == 1
extern int yodalite_register_all_cmd(void);
extern int yodalite_init_all(void);
extern int yodalite_deinit_all(void);

static int shell_hook(void)
{
	 sleep(1);
     return 0;
}

static int console_loop(void)
{

    printf("\t\r\n==============================\r\n");
    printf("\t\r\n    Yodalite Console V1.0     \r\n");
    printf("\t\r\n==============================\r\n");
    printf("\n\n");

    yodalite_cmd_init();

    yodalite_register_shell_hook(shell_hook);

    yodalite_register_all_cmd();

    yodalite_run_cmd();

    yodalite_unregister_shell_hook();

    return 0;
}

int __wrap_getchar(void){

 uint8_t data; 

 if(HAL_UART_Receive_Poll(0,&data,1,1000) >0)
   return data;
 else
   return 0xff;
}


#endif

int main(void)
{
   platform_init();
   yodalite_init_all();

   printf("\nxr872 main entry\r\n");

   #if CONFIG_SHELL_ENABLE == 1
   console_loop();
   #endif

   printf("\nxr872 main exit\n");

   return 0;
}
