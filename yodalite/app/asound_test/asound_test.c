/* Examples of speech recognition with multiple keywords.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

#include "hardware/platform.h"
#include <hardware/pal_asound.h>
#include "kws.h"
#include "yodalite_autoconf.h"
#include "lib/shell/shell.h"
//#include "freertos/queue.h"
//#include <osal/unistd.h>
//#include <osal/pthread.h>
//#include <osal/semaphore.h>
//#include "activation/activation.h"
//#include "player.h"

extern struct asound_lapi xr872_asound_lapi;

int mp3_write = -1;
int http_write = -1;

int asound_flag =0;


int us_pcm_read = 0;


void rec_app(void)
{

    struct yodalite_pcm_config *config;
    unsigned int card, device, flags,num_flag;
	 void *pcm_data;
    printf("start to rec app\n");
    config = (struct yodalite_pcm_config *)malloc(sizeof(struct yodalite_pcm_config));
   // memset(config, 0x0, sizeof(struct pcm_config));
    pal_asound_init(&xr872_asound_lapi);
    asound_flag = 1;
    num_flag = 0;
    card = 1;
    device = 1;
    flags = YODALITE_PCM_IN;
    config->channels = 1;
    config->rate = 48000;
    config->format = 16;
    config->period_count = sizeof(short);
    config->source = RAW_ASR;
    //config->source = RAW_SD;
    config->period_size = 480;
    us_pcm_read = pcm_open(card, device, flags, config);
    printf("us_pcm_read+++%d\n",us_pcm_read);
    pcm_data = malloc(320 * 2);
    float thresholdSetting[3] = {0.6, 0.7, 0.6};
    unsigned char channel_num = 1;
    unsigned char min_gap = 5;
    kws_task task = kws_create("123", channel_num, thresholdSetting, min_gap);
    static float *in_ptr[8];
    static float in_buffer[320];//in_buffer[480];
    in_ptr[0] = in_buffer; 
    int  pcm_buff_count;
    int i;
    static wake_num;
    while (asound_flag) {
     pcm_buff_count =  pcm_read(us_pcm_read,  (void *)pcm_data,   320 * sizeof(short));
      for ( i = 0; i < pcm_buff_count/2; i++) {
           in_buffer[i]= (float)(((short *)pcm_data)[i]*20);
     }
     int active_status = kws_status(task, (const float ** )in_ptr, pcm_buff_count/2,NULL);
     if (active_status == 1) {
	 	printf("wake up ++++++++++++%d\n",wake_num++);
    }
   }
    free(pcm_data);
   // player_destory(us_pcm_read);
    //vTaskDelete(NULL);
}

#if 0

void app_play(void *parameters)
{
    asound_flag =1;

    xQueue1 = xQueueCreate(10, sizeof(uint32_t));
    pal_asound_init(&esp32_asound_lapi);
    mp3_write = play_local_init(START_UP);

    http_write = play_url_init("http://dl.espressif.com/dl/audio/adf_music.mp3");
 //   play_pcm_data(http_write);
 //   player_stop(http_write);

    int element;

      //if(xQueueReceive(xQueue1,&element,portMAX_DELAY))
    mp3_write = play_local_init(PLAY_ERR);
    play_pcm_data(mp3_write);
    player_stop(mp3_write);
    mp3_write = play_local_init(AWEAK);
    play_pcm_data(mp3_write);
    player_stop(mp3_write);
    mp3_write = play_local_init(START_UP);
    play_pcm_data(mp3_write);
    player_stop(mp3_write);
   // http_write = play_url_init("http://dl.espressif.com/dl/audio/adf_music.mp3");
   while(asound_flag)
    {
     play_pcm_data(http_write);
     player_stop(http_write);
     player_destory(http_write);
     http_write = play_url_init("http://dl.espressif.com/dl/audio/adf_music.mp3");
    }
    play_pcm_data(http_write);
    player_stop(http_write);
    player_destory(mp3_write);
    player_destory(http_write);
    vTaskDelete(NULL);
}
void *asound_task(void *argv)
{
    xTaskCreatePinnedToCore(app_play, "app_play", 3 * 1024, NULL,6, NULL, 1);//6
    xTaskCreatePinnedToCore(rec_app, "rec_app", 3 * 1024, NULL, 5, NULL, 1);  // 1  支持http的播放和唤醒；
    //xTaskCreatePinnedToCore(test_app,"test_play",3*1024,NULL,5,NULL,1);
    return NULL;
}

#endif

static int asound_start_cmd(int argc,int8_t * const argv[])
{
//pthread_t pthread; 	
//(void) pthread_create(&pthread, NULL,asound_task,NULL);
//	activation_init();
    rec_app();
    printf("start to asound test \n");
    return 0;
}
static int asound_stop_cmd(int argc,int8_t * const argv[])
{
   // printf("asound_flag %d\n",asound_flag);
   // asound_flag  = 0;
   // printf("asound_flag %d\n",asound_flag);
    printf("stop to asound test\n");
    return 0;
}


#define max_args      (1)
#define asound_start_help  "asound start test"

int cmd_asound_start(void)
{
  YODALITE_REG_CMD(asound_start,max_args,asound_start_cmd,asound_start_help);
  return 0;
}

#define asound_stop_help  "asound stop test"


int cmd_asound_stop(void)
{
  YODALITE_REG_CMD(asound_stop,max_args,asound_stop_cmd,asound_stop_help);
  return 0;
}

