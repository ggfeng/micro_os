#include <stdlib.h>
#include <string.h>
#include "yodalite_autoconf.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "lib/shell/shell.h"
#include "nvs_flash.h"
#include "lib/shell/shell.h"
#include "lib/andlink/Qlink_API.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "lib/cjson/cJSON.h"
#include "wifi_api.h"

#define ANDLINK_MEG_LENTH  512

#if (CONFIG_ANDLINK_TEST_ENABLE ==1)

static TaskHandle_t  andlinkTaskHandle;

static int response (unsigned char* body, int len)
{
    printf("now andlink response body:%s,len:%d\n", body, len);
    const cJSON *psk = NULL;
    const cJSON *ssid = NULL;

    if(len<= 0 || len >= ANDLINK_MEG_LENTH) {
      printf("andlink meg lenth :%d error\n",len);
      return -1;
    }
    unsigned char *tmp_str = (unsigned char *)malloc(ANDLINK_MEG_LENTH);
    memset(tmp_str,0,ANDLINK_MEG_LENTH);
    memcpy(tmp_str,body,len);
    cJSON *monitor_json = cJSON_Parse((char *)tmp_str);

    if (monitor_json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            printf("Error before: %s\n", error_ptr);
        }
        goto end;
    }
    psk = cJSON_GetObjectItemCaseSensitive(monitor_json, "password");
    if (cJSON_IsString(psk) && (psk->valuestring != NULL))
    {
        printf("Checking password %s\n", psk->valuestring);
    }

    ssid = cJSON_GetObjectItemCaseSensitive(monitor_json, "SSID");
    if (cJSON_IsString(ssid) && (ssid->valuestring != NULL))
    {
        printf("Checking SSID %s\n", ssid->valuestring);
    }
end:
    free(tmp_str);
    cJSON_Delete(monitor_json);
    vTaskDelete(andlinkTaskHandle);

  return 0;
}

static int andlink_start(int argc,int8_t *const argv[])
{
  xTaskCreate(Qlink_StartCoapServer, "andlink", 1024 * 5, NULL, 5, &andlinkTaskHandle);
  Qlink_setReciveInternetChannelCallback(response);
  return 0;
}

#define max_start_args      (1)
#define andlink_start_help     "andlink_start"

int cmd_andlink_start(void)
{
  YODALITE_REG_CMD(andlink_start,max_start_args,andlink_start,andlink_start_help);
  return 0;
}

#endif


