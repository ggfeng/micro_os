#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wifi_api.h"
#include "asound.h"
#include "hardware/wifi_hal.h"
#include "hardware/pal_flash.h"
#include "hardware/bt/bluetooth.h"
#include "hapi/audio_out.h"
#include "hardware/pal_asound.h"
#include "yodalite_flash.h"

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

#if CONFIG_FATFS_ENABLE == 1
#include "lib/fatfs/ff.h"
#define DEFAULT_FATFS_LABEL "fatfs"
#define FATFS_BUF_SIZE (4096)
#include "fatfs.h"
static FATFS fs;  

int fatfs_init(void)
{
   int iret=0;
   BYTE *buf;
   FRESULT fr; 
   char drv[3] = {(char)('0' + 0), ':', 0};

   printf("register as fatfs\n");
   
   if((iret= spiflash_fatfs_init())== 0){
      printf("register ok\n");
   }else{
      printf("unregister fail\n");
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
   char drv[3] = {(char)('0' + 0), ':', 0};

   printf("unregister as fatfs\n");

  if((iret= spiflash_fatfs_uninit())== 0){
    printf("unregister ok\n");
  }else{
    printf("unregister fail\n");
  }

  f_mount(0,drv, 0);

  return iret;
}

#endif

#if CONFIG_WIFI_HAL_ENABLE == 1
int wifi_init(void)
{
  printf("wifi_init\n");
    
  if (wifi_device_init(&wifi_dev) != WIFI_OK) {
         printf("Wifi device init failed!\n");
         return -1;
   }

   return 0;
}

int wifi_deinit(void)
{
  printf("wifi_deinit\n");
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

#if CONFIG_AUDIO_OUT_ENABLE == 1

int audio_init(void){
  pal_asound_init(&xr872_asound_lapi);
  return audio_out_init();
}

#endif


