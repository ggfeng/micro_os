/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "yodalite_autoconf.h"
#include "lib/shell/shell.h"
#include <osal/unistd.h>
#include <osal/pthread.h>
#include <osal/semaphore.h>


/* Can run 'make menuconfig' to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#if (CONFIG_BLINK_TEST_ENABLE == 1)

#define BLINK_GPIO 22
int stop_flag = 0;

static void * blink_task(void * arg )
{


	    stop_flag = 1;
		gpio_pad_select_gpio(BLINK_GPIO);
		gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
		while(stop_flag) {
			
          
		    printf("Turning off the LED\n");
			gpio_set_level(BLINK_GPIO, 0);
			vTaskDelay(1000 / portTICK_PERIOD_MS);

		    printf("Turning on the LED\n");
			gpio_set_level(BLINK_GPIO, 1);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			
		}
		return NULL;

}


static int blink_cmd(int argc,int8_t * const argv[])
{
   
	 pthread_t pthread; 
	
	( void ) pthread_create( &pthread, NULL,blink_task,NULL);

    return 0;
}

static int blink_stop_cmd(int argc,int8_t * const argv[])
{
   
	 //pthread_t pthread; 
	 stop_flag  = 0;
	//( void ) pthread_create( &pthread, NULL,blink_stop_task,NULL);

    return 0;
}

#define max_args      (1)
#define blink_help  "blink start test"

int cmd_blink_start_test(void)
{
  YODALITE_REG_CMD(blink_start,max_args,blink_cmd,blink_help);

  return 0;
}


//#define max_args      (1)
#define blink_stop_help  "blink stop test"

int cmd_blink_stop_test(void)
{
  YODALITE_REG_CMD(blink_stop,max_args,blink_stop_cmd,blink_stop_help);

  return 0;
}


#endif

