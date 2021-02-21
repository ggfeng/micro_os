#include <stdio.h>
#include "app/fatfs_test.h"
#include "app/wifi_test.h"
#include "app/property_test.h"
#include "app/ftm.h"

#include  "yodalite_autoconf.h"

typedef int (*cmd_init_t)(void);

#if (CONFIG_SHELL_ENABLE == 1) 

#if (CONFIG_ANDLINK_TEST_ENABLE ==1 )
extern int cmd_andlink_start(void);
#endif

#if (CONFIG_CJSON_TEST_ENABLE == 1)
extern int cmd_json_shell(void);
#endif

#if (CONFIG_PRTTN_INFO_SHOW_ENABLE ==1)
extern int cmd_show_prttn_info(void);
#endif

#if (CONFIG_LIB_TEST_ENABLE ==1)
extern int cmd_lib_test(void);
#endif

#if (CONFIG_POSIX_TEST_ENABLE == 1)
  #if (CONFIG_PTHREAD_TEST_ENABLE == 1)
   extern int cmd_pthread_test(void);
  #endif
  #if (CONFIG_COND_TEST_ENABLE ==1)
   extern int cmd_pthread_wait(void);
   extern int cmd_pthread_wake(void);
  #endif

  #if (CONFIG_MQ_TEST_ENABLE == 1)
   extern int cmd_pthread_mq(void);
  #endif
#endif

#if (CONFIG_BLINK_TEST_ENABLE == 1)
extern int cmd_blink_start_test(void);
extern int cmd_blink_stop_test(void);
#endif

#if(CONFIG_ASOUND_TEST_ENABLE == 1)
extern int cmd_asound_start(void);
extern int cmd_asound_stop(void);
#endif
#if (CONFIG_NETTOOL_TEST_ENABLE == 1)
extern int cmd_net_ping(void);
#endif
#if (CONFIG_ELOOP_TEST_ENABLE ==1)
extern int cmd_eloop_init(void);
extern int cmd_eloop_deinit(void);
#endif
#if (CONFIG_FLORA_TEST_ENABLE == 1)
extern int cmd_flora_test(void);
#endif
#if (CONFIG_INPUT_TEST_ENABLE == 1)
extern int cmd_input_event_test(void);
#endif
#if (CONFIG_NETMGR_TEST_ENABLE ==1)
extern int cmd_netmgr_test(void);
extern int cmd_netmgr_monitor(void);
#endif
#if (CONFIG_AES_TEST_ENABLE ==1)
extern int cmd_aes_test(void);
#endif
#if (CONFIG_SOCKET_TEST_ENABLE ==1)
extern int cmd_socket_test(void);
#endif
#if (CONFIG_MD5_TEST_ENABLE ==1)
extern int cmd_md5_test(void);
#endif
#if (CONFIG_BASE64_TEST_ENABLE ==1)
extern int cmd_base64_test(void);
#endif
#if (CONFIG_AUTH_TEST_ENABLE ==1)
extern int cmd_auth_aes_test(void);
extern int cmd_auth_rsa_test(void);
#endif

#if (CONFIG_MEMEXT_TEST_ENABLE == 1)
extern int cmd_mem_ext(void);
extern int cmd_mem_free(void);
#endif

#if (CONFIG_BT_TEST_ENABLE ==1)
extern int add_bt_cmd(void);
#endif
#if (CONFIG_BTMGR_TEST_ENABLE == 1)
extern int cmd_btmgr_test(void);
extern int cmd_bt_monitor_start(void);
extern int cmd_bt_monitor_stop(void);
#endif

#if (CONFIG_MQTT_TEST_ENABLE == 1)
extern int cmd_mqtt(void);
extern int cmd_mqtt_deinit(void);
extern int cmd_mqtt_init(void);
extern int cmd_login(void);
#endif

#if (CONFIG_APPMGR_TEST_ENABLE == 1)
extern int cmd_appmgr_test(void);
#endif

#if (CONFIG_TASK_TEST_ENABLE == 1)
extern int cmd_task_test(void);
#endif
static cmd_init_t cmd_tbl[]= 
{

#if (CONFIG_CJSON_TEST_ENABLE == 1)
   cmd_json_shell,
#endif

#if (CONFIG_PRTTN_INFO_SHOW_ENABLE ==1)
   cmd_show_prttn_info,
#endif
#if (CONFIG_FATFS_TEST_ENABLE == 1)
   cmd_fatfs_access,
   cmd_fatfs_read,
   cmd_fatfs_format,
#endif
#if (CONFIG_LIB_TEST_ENABLE ==1)
   cmd_lib_test,
#endif

#if (CONFIG_POSIX_TEST_ENABLE == 1)
  #if (CONFIG_PTHREAD_TEST_ENABLE == 1)
   cmd_pthread_test,
  #endif
  #if (CONFIG_COND_TEST_ENABLE ==1)
   cmd_pthread_wait,
   cmd_pthread_wake,
  #endif
  #if (CONFIG_MQ_TEST_ENABLE == 1)
   cmd_pthread_mq,
  #endif
#endif

#if (CONFIG_BLINK_TEST_ENABLE == 1)
   cmd_blink_start_test,
   cmd_blink_stop_test,
#endif

#if(CONFIG_ASOUND_TEST_ENABLE == 1)
   cmd_asound_start,
   cmd_asound_stop,
#endif
#if(CONFIG_WIFI_TEST_ENABLE == 1)
   cmd_wifi_init,
   cmd_wifi_deinit,
   cmd_wifi_ap_start,
   cmd_wifi_ap_stop,
   cmd_wifi_station_start,
   cmd_wifi_station_stop,
   cmd_wifi_sta_connect_ap,
   cmd_wifi_sta_disconnect,
   cmd_wifi_get_status,
   cmd_wifi_scan_start,
   cmd_wifi_scan_stop,
   cmd_wifi_get_scan_list,

#if CONFIG_WIFI_SMART_SNIFFER_ENABLE
	cmd_smart_preamble_set,
	cmd_smart_format_set,
	cmd_smart_start,
	cmd_smart_stop,
#endif
#endif
#if (CONFIG_NETTOOL_TEST_ENABLE == 1)
   cmd_net_ping,
#endif
#if (CONFIG_PROPERTY_TEST_ENABLE ==1)
   cmd_property_get,
   cmd_property_set,
   cmd_property_init,
   cmd_property_deinit,
   cmd_property_print,
   cmd_property_remove,
#endif
#if (CONFIG_ELOOP_TEST_ENABLE ==1)
   cmd_eloop_init,
   cmd_eloop_deinit,
#endif
#if (CONFIG_FLORA_TEST_ENABLE == 1)
   cmd_flora_test,
#endif
#if (CONFIG_INPUT_TEST_ENABLE == 1)
   cmd_input_event_test,
#endif
#if (CONFIG_NETMGR_TEST_ENABLE ==1)
   cmd_netmgr_test,
   cmd_netmgr_monitor,
#endif
#if (CONFIG_AES_TEST_ENABLE ==1)
   cmd_aes_test,
#endif
#if (CONFIG_MD5_TEST_ENABLE ==1)
   cmd_md5_test,
#endif
#if (CONFIG_BASE64_TEST_ENABLE ==1)
   cmd_base64_test,
#endif
#if (CONFIG_AUTH_TEST_ENABLE ==1)
   cmd_auth_aes_test,
   cmd_auth_rsa_test,
#endif

#if (CONFIG_SOCKET_TEST_ENABLE ==1)
   cmd_socket_test,
#endif

#if (CONFIG_ANDLINK_TEST_ENABLE ==1)
   cmd_andlink_start,
#endif
 
#if (CONFIG_MEMEXT_TEST_ENABLE == 1)
   cmd_mem_ext,
   cmd_mem_free,
#endif

#if (CONFIG_FTM_ENABLE ==1)
   cmd_ftm,
#endif

#if (CONFIG_BTMGR_TEST_ENABLE == 1)
 cmd_btmgr_test,
 cmd_bt_monitor_start,
 cmd_bt_monitor_stop,
#endif

#if (CONFIG_MQTT_TEST_ENABLE == 1)
 cmd_mqtt,
 cmd_mqtt_deinit,
 cmd_mqtt_init,
 cmd_login,
#endif

#if (CONFIG_APPMGR_TEST_ENABLE == 1)
 cmd_appmgr_test,
#endif
#if (CONFIG_TASK_TEST_ENABLE == 1)
 cmd_task_test,
#endif
   NULL
};


int yodalite_register_all_cmd(void)
{
   cmd_init_t *cmd = &cmd_tbl[0];

   while(*cmd != NULL)
   {
      (*cmd)();
      cmd ++;
   }
#if (CONFIG_BT_TEST_ENABLE ==1)
	add_bt_cmd();
#endif
   return 0;
}

#endif
