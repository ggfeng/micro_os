#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "fatfs.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"
#include "wifi_api.h"
#include "hardware/wifi_hal.h"
#include "hardware/pal_flash.h"
#include "yodalite_flash.h"
#include "hardware/bt/bluetooth.h"
#include "hardware/pal_asound.h"
//#include "asound.h"
#include "mqtt_client.h"
#include "hapi/mqtt_login.h"
#include "hapi/audio_out.h"

#include "yodalite_autoconf.h"
#if (CONFIG_BT_ENABLED && CONFIG_BLUETOOTH_HAL_ENABLE)
#include "bluetooth_common.h"
#endif
#ifdef CONFIG_MEM_EXT_ENABLE
#include "lib/mem_ext/mem_ext.h"
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

#define DEFAULT_FATFS_LABEL "fatfs"
#define FATFS_BUF_SIZE (4096)

extern struct asound_lapi esp32_asound_lapi;
static  char *g_topic= NULL;
static const char *TAG = "MQTT_EXAMPLE";
static mqtt_hook_t yodalite_mqtt_hook = NULL;

#if CONFIG_FATFS_ENABLE == 1
/*
#include "yodalite_autoconf.h"
#ifdef CONFIG_MEM_EXT_ENABLE
#include "lib/mem_ext/mem_ext.h"
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif
*/

static FATFS fs;  


int fatfs_init(void)
{
   int iret=0;
   BYTE *buf;
   FRESULT fr; 
   char * label;
   char drv[3] = {(char)('0' + 0), ':', 0};

   label = DEFAULT_FATFS_LABEL;

   printf("label:%s register as fatfs\n",label);
   
   if((iret= esp_spiflash_fatfs_init(label))== 0){
      printf("label:%s register ok\n",label);
   }else{
      printf("label:%s unregister fail\n",label);
   }

   fr  = f_mount(&fs,drv,1);

   if(fr == 0xd){
     printf("f_mkfs...\n");
 
     buf = yodalite_malloc(FATFS_BUF_SIZE);
     if(buf== NULL){
       printf("yodalite_malloc %d fail\n",FATFS_BUF_SIZE);
       return -1;
     }

     fr = f_mkfs(drv,FM_ANY,0,buf,FATFS_BUF_SIZE);
     yodalite_free(buf);
      if(fr) {
         printf("#Err:f_mkfs()= %d\n",fr);
        return (int)fr;
      }
    }
   
   printf("f_mount() = %d\n",fr);

   return 0;
}


int fatfs_deinit(void)
{
   int iret=0;
   char * label;
   char drv[3] = {(char)('0' + 0), ':', 0};

   label = DEFAULT_FATFS_LABEL;

   printf("label:%s unregister as fatfs\n",label);
   
  if((iret= esp_spiflash_fatfs_uninit(label))== 0){
    printf("label:%s unregister ok\n",label);
  }else{
    printf("label:%s unregister fail\n",label);
  }

  f_mount(0,drv, 0);

  return iret;
}
#endif



#if CONFIG_WIFI_HAL_ENABLE == 1
int wifi_init(void)
{

      printf("wifi_init\n");
      esp_err_t ret = nvs_flash_init();

      if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
       ESP_ERROR_CHECK(nvs_flash_erase());
       ret = nvs_flash_init();
      }

      ESP_ERROR_CHECK(ret);
    
  if (wifi_device_init(&wifi_dev) != WIFI_OK) {
         printf("Wifi device init failed!\n");
         return -1;
   }

   return 0;
}

int wifi_deinit(void)
{
  printf("wifi_deinit\n");
  nvs_flash_deinit();
    printf("warning:wifi component is not select \n");

  return 0;
}
#endif

#if CONFIG_PAL_FLASH_ENABLE == 1
int flash_init(void)
{
  printf("flash_init\n");
  return pal_flash_init(&yodalite_flash);
}

int flash_deinit(void)
{
  printf("flash_deinit\n");
  return 0;
}

#endif

#if (CONFIG_BT_ENABLED && CONFIG_BLUETOOTH_HAL_ENABLE)
int bt_init(void)
{
    printf("bt_init\n");
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
       ESP_ERROR_CHECK(nvs_flash_erase());
       ret = nvs_flash_init();
	}

	ESP_ERROR_CHECK(ret);

	rk_bluetooth_device_init(&rk_bt_dev);
	return 0;
}

#endif

#if CONFIG_ACTIVATION_ENABLE == 1

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
  //  int msg_id;
    esp_mqtt_client_handle_t client = event->client;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            printf("subscribe topic:%s\n",g_topic);
            esp_mqtt_client_subscribe(client,g_topic,0);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
         //   msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
          //  ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);

            if(yodalite_mqtt_hook){
               char *topic=NULL;
               char *msg=NULL;
               if((topic=yodalite_malloc(event->topic_len+event->data_len+2)) == NULL)
               {
                 printf("error:yodalite_malloc %d fail\n",event->topic_len+event->data_len+2);
                 yodalite_free(topic);
                 break;
               }
               memset(topic,0,event->topic_len+event->data_len+2);
               msg = topic + event->topic_len +1;
               strncpy(topic,event->topic,event->topic_len);
               strncpy(msg,event->data,event->data_len);
               yodalite_mqtt_hook(topic,msg);
               yodalite_free(topic);
             }

            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

#define  YODALITE_MQTT_URI "mqtts://wormhole.rokid.com:8885"

int esp32_mqtt_init(char *username,char *passwd,char *topic,mqtt_hook_t func){

    esp_mqtt_client_handle_t client;

    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = YODALITE_MQTT_URI,
        .event_handle = mqtt_event_handler,
    };

    mqtt_cfg.username = username;
    mqtt_cfg.password = passwd;

    printf("topic:%s\n",topic);

    g_topic= topic;
    yodalite_mqtt_hook = func;

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);

   return 0;
}
#endif

#if CONFIG_AUDIO_OUT_ENABLE == 1
int audio_init(void){
  pal_asound_init(&esp32_asound_lapi); 
  return audio_out_init();
}
#endif
