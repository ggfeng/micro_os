

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "yodalite_autoconf.h"
#include "lib/property/properties.h"
#include "lib/shell/shell.h"

#ifdef CONFIG_MEM_EXT_ENABLE
#include <lib/mem_ext/mem_ext.h>
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

#if (CONFIG_MQTT_TEST_ENABLE == 1)

static  char *g_topic= NULL;
static const char *TAG = "MQTT_EXAMPLE";

//static EventGroupHandle_t wifi_event_group;
const static int CONNECTED_BIT = BIT0;


static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
            ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
            //msg_id =esp_mqtt_client_subscribe(client,"u/86787B89C01C4CA9B35F746F20923C4F/deviceType/060F941561F24278B8ED71733D7B9507/deviceId/0602041832000020/rc",0);

            printf("subscribe topic:%s\n",g_topic);
            msg_id =esp_mqtt_client_subscribe(client,g_topic,0);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
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

static void mqtt_app_start(void)
{
    char * buf;
    char *master_id;
    char *device_type_id;
    char *device_id;
    char *user_name;
    char *passwd;;
    char *topic;

    uint32_t buf_size = 2048;
    esp_mqtt_client_handle_t client;

    esp_mqtt_client_config_t mqtt_cfg = {
       // .uri = "mqtts:\/\/wormhole.rokid.com:8885",
        .uri = "mqtts://wormhole.rokid.com:8885",
        .event_handle = mqtt_event_handler,
    //  .username = "b03e058912d4054cad4fa97d041f0a5d",
    //  .password = "fcd789d2-7752-42f3-88d3-a30faba576e6",
   //   .password = "95c18544-b59d-44b2-afee-79449e9a38e8",
    };

    if((buf = yodalite_malloc(buf_size)) == NULL){
      printf("error:yodalite_malloc %d fail\n",buf_size);
      //return -1;
   }

    memset(buf,0,buf_size);

    master_id = buf;
    device_type_id = buf+256;
    device_id = device_type_id + 256;
    user_name = device_id+256;
    passwd = user_name +256;
    topic = passwd +256;
    g_topic = topic;

   if(property_get("ro.master_id",master_id,"0") <=0 ||
     property_get("ro.boot.devicetypeid",device_type_id,"0") <= 0 ||
     property_get("ro.boot.serialno",device_id,"0") <=0  ||
     property_get("ro.username",user_name,"0") <=0  ||
     property_get("ro.passwd",passwd,"0") <=0){

     printf("error:%s->%d parm is not valid\n",__func__,__LINE__);
     return ;
    }

    mqtt_cfg.username = user_name;
    mqtt_cfg.password = passwd;


    snprintf(topic,256,"u/%s/deviceType/%s/deviceid/%s/rc",master_id,device_type_id,device_id);

    printf("username:%s,password:%s\n",mqtt_cfg.username,mqtt_cfg.password);
    printf("topic:%s\n",topic);

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);

//  msg_id =esp_mqtt_client_subscribe(client,topic,0);
}


static int mqtt_cmd(int argc,char *const argv[])
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    mqtt_app_start();

   return 0;
}

#define max_args   (1)
#define mqtt_help  "mqtt"

int cmd_mqtt(void)
{
    YODALITE_REG_CMD(mqtt,max_args,mqtt_cmd,mqtt_help);

    return 0;
}

#endif
