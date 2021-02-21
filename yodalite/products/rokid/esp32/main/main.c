/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include  "yodalite_autoconf.h"
#include "lib/shell/shell.h"

extern int yodalite_init_all(void);
extern int yodalite_deinit_all(void);
extern int yodalite_register_all_cmd(void);

#if CONFIG_SHELL_ENABLE == 1

static int esp32_shell_hook(void)
{
     vTaskDelay(10/portTICK_PERIOD_MS);
     return 0;
}

static int console_loop(void)
{

    printf("\t\r\n==============================\r\n");
    printf("\t\r\n    Yodalite Console V1.0     \r\n");
    printf("\t\r\n==============================\r\n");
    printf("\n\n");

    yodalite_cmd_init();

    yodalite_register_shell_hook(esp32_shell_hook);

    yodalite_register_all_cmd();

    yodalite_run_cmd();

    yodalite_unregister_shell_hook();

    return 0;
}
#endif


void app_main()
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

  yodalite_init_all();

  #if CONFIG_SHELL_ENABLE == 1
   console_loop();
   #endif

   yodalite_deinit_all();

}
