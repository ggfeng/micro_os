#include  <stdio.h>
#include  "yodalite_autoconf.h"
#include  "hapi/netmgr.h"
#include  "hapi/btmgr.h"
#include  "activation/activation.h"
#include  "lib/property/properties.h"
#include  "hapi/nv.h"
#include  "hapi/audio_out.h"
#include  "hapi/record_kws.h"

typedef int (*init_entry_t)(void);
typedef int (*deinit_entry_t)(void);

#if CONFIG_MEM_EXT_ENABLE == 1
extern int ext_mem_init(void);
#endif

#if CONFIG_FATFS_ENABLE == 1
extern int fatfs_init(void);
extern int fatfs_deinit(void);
#endif

#if CONFIG_PAL_FLASH_ENABLE == 1
extern int flash_init(void);
extern int flash_deinit(void);
#endif

#if CONFIG_WIFI_HAL_ENABLE ==1
extern int wifi_init(void);
extern int wifi_deinit(void);
#endif

#if (CONFIG_BT_ENABLED && CONFIG_BLUETOOTH_HAL_ENABLE)
extern int bt_init(void);
#endif

#if CONFIG_AUDIO_OUT_ENABLE == 1
extern int audio_init(void);
#endif

static init_entry_t init_tbl[]= 
{
  #if CONFIG_MEM_EXT_ENABLE == 1
    ext_mem_init,
  #endif

  #if CONFIG_PAL_FLASH_ENABLE == 1
    flash_init,
  #endif
  #if CONFIG_FATFS_ENABLE == 1
    fatfs_init,
  #endif

  #if CONFIG_WIFI_HAL_ENABLE ==1
    wifi_init,
   #if CONFIG_NETMGR_LITE_ENABLE == 1
   hapi_netmgr_init,

  #if CONFIG_FACTORY_ENABLE == 1
    nv_prop_init,
   #endif
  #endif
  #endif

  #if CONFIG_ACTIVATION_ENABLE ==1
	activation_init,
  #endif


#if (CONFIG_BT_ENABLED && CONFIG_BLUETOOTH_HAL_ENABLE)
	bt_init,
#if CONFIG_BLUETOOTHMGR_ENABLE == 1
    btmgr_init,
#endif
#endif

#if CONFIG_AUDIO_OUT_ENABLE == 1
   audio_init,
#endif

#if CONFIG_RECORD_KWS_ENABLE ==1
  record_kws_init,
#endif

   NULL
};


static deinit_entry_t deinit_tbl[]= 
{
  #if CONFIG_BLUETOOTHMGR_ENABLE == 1
    btmgr_deinit,
  #endif

  #if CONFIG_FACTORY_ENABLE == 1
   nv_prop_deinit,
  #endif
   #if CONFIG_NETMGR_LITE_ENABLE == 1
    hapi_netmgr_exit,
   #endif

  #if CONFIG_WIFI_HAL_ENABLE ==1
   wifi_deinit,
  #endif

  #if CONFIG_FATFS_ENABLE == 1
   fatfs_deinit,
  #endif

  #if CONFIG_PAL_FLASH_ENABLE == 1
   flash_deinit,
  #endif
   NULL
};

int yodalite_init_all(void)
{
   init_entry_t *init = &init_tbl[0]; 

   while(*init != NULL)
   {
      (*init)();
      init ++;
   }

   return 0;
}

int yodalite_deinit_all(void)
{
   deinit_entry_t *deinit = &deinit_tbl[0]; 

   while(*deinit != NULL)
   {
      (*deinit)();
      deinit ++;
   }

   return 0;
}

