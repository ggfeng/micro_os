#include <stdlib.h>
#include <string.h>
#include <yodalite_autoconf.h>
#include <hardware/wifi_hal.h>
#include <lib/shell/shell.h>
#ifndef SMART_IN_HAL
#include <hapi/smart_config/smart_config.h>
#include <hapi/flora_agent.h>
#endif

#ifdef CONFIG_MEM_EXT_ENABLE
#include <lib/mem_ext/mem_ext.h>
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

#if CONFIG_ESP32
#define DATA_LEN_OFFSET_NOENCRYPT 64
#define DATA_LEN_OFFSET_ENCRYPT 80
#elif CONFIG_XR871
#define DATA_LEN_OFFSET_NOENCRYPT 60
#define DATA_LEN_OFFSET_ENCRYPT 76
#endif

#define FLORA_AGET_SCONFIG_URI "unix:/var/run/flora.sock"
#define FLORA_SCONFIG_REPORT "rokid.sconfig"

static struct hw_module_t *module = NULL;
static struct wifi_device_t *wifi = NULL;
static  int wifi_init_flag = 0;
static int ssid_found = 0;
#ifndef SMART_IN_HAL
#if CONFIG_WIFI_SMART_SNIFFER_ENABLE
static smart_config_t *sconfig = NULL;

#if CONFIG_FLORA_LITE_ENABLE
static flora_agent_t sconfig_agent;

static void *sconfig_send_ssid(void *data)
{
	sconfig_ssid_t *ssid = (sconfig_ssid_t *)data;
    caps_t sconfig_msg;

    pthread_detach(pthread_self());
	sconfig_msg = caps_create();
	caps_write_binary(sconfig_msg,(void *)ssid, sizeof(sconfig_ssid_t));
	flora_agent_post(sconfig_agent, FLORA_SCONFIG_REPORT, sconfig_msg, CAPS_MEMBER_TYPE_BINARY);
	caps_destroy(sconfig_msg);
	
}
#endif

static void smart_sniffer_listener(sniffer_packet_into_t *packet_info)
{
//	char *ssid = sconfig_ssid.ssid;
//	char *passwd = sconfig_ssid.passwd;
	sconfig_ssid_t sconfig_ssid;
#if CONFIG_FLORA_LITE_ENABLE
	pthread_t pthread_sconfig;
#endif
	sniffer_chan_set_t cb = NULL;

	if (!wifi) {
		printf("wifi not init, please init it first!\n");
		return;
	}
	bzero(&sconfig_ssid, sizeof(sconfig_ssid));
	cb = wifi->sniffer_channel_set;
	if (ssid_found == 0) {
		if (smart_analize_packets(packet_info, sconfig, cb, &sconfig_ssid) == SMART_ANALIZE_SUCCESS) {
			printf("Smart get ssid success, ssid = %s, passwd = %s\n", sconfig_ssid.ssid, sconfig_ssid.passwd);
			ssid_found = 1;

#if CONFIG_FLORA_LITE_ENABLE
			pthread_create(&pthread_sconfig, NULL, sconfig_send_ssid, (void*)&sconfig_ssid);
#endif
			/* stop sniffer */
			if (!wifi) {
				printf("wifi not init, please init it first!\n");
				return;
			}

			wifi->smart_sniffer_stop();

		}
	}
}

#if CONFIG_FLORA_LITE_ENABLE
/*************************************************************************************
 * Smart config client code,register flora callback to wait for the smart config finish,
 * then reply configuration success to the configurator by send bc packet with special 
 * port.
 * */
static void sconfig_callback(const char *name, uint32_t msgtype, caps_t msg, void *arg)
{
	if (strcmp(name, FLORA_SCONFIG_REPORT) == 0) {
		sconfig_ssid_t *ssid;
		int length = 0;

		caps_read_binary(msg, (const void**)&ssid, &length);
		printf("Receive ssid, ssid = %s passwd = %s\n", ssid->ssid, ssid->passwd);
	}
}

void flora_sconfig_init(void)
{
    flora_agent_config_param_t param;
    sconfig_agent = flora_agent_create();
    memset(&param,0,sizeof(flora_agent_config_param_t));
    param.sock_path  = FLORA_AGET_SCONFIG_URI;

    flora_agent_config(sconfig_agent, FLORA_AGENT_CONFIG_URI,&param);
    param.value  = 1024;
    flora_agent_config(sconfig_agent, FLORA_AGENT_CONFIG_BUFSIZE,&param);
    flora_agent_subscribe(sconfig_agent, FLORA_SCONFIG_REPORT, sconfig_callback, NULL);
    flora_agent_start(sconfig_agent, 0);
}

void flora_sconfig_deinit(void)
{
   flora_agent_close(sconfig_agent);
   flora_agent_delete(sconfig_agent);
}
#endif
/* ******************************************************************************** */

static int smart_config_init(struct wifi_device_t *wifi)
{
	if (!wifi) {
		printf("wifi not init, please init it first!\n");
		return -1;
	}

	sconfig = smart_config_create();
	if (!sconfig) {
		printf("create sconfig failed!\n");
		return -1;
	}
	if (wifi->smart_set_listener) {
		wifi->smart_set_listener(smart_sniffer_listener);
	}
#if CONFIG_FLORA_LITE_ENABLE
	flora_sconfig_init();
#endif
	return 0;
}

static int smart_config_deinit(smart_config_t *smart_config)
{
	if (smart_config) {
		smart_config_destroy(smart_config);
	}
#if CONFIG_FLORA_LITE_ENABLE
	flora_sconfig_deinit();
#endif
	return 0;
}
#endif
#endif

static int wifi_init(int argc,int8_t *const argv[])
{
   if(wifi_init_flag == 0){
      hw_get_module(WIFI_HAL_HW_ID, module);
      module->methods->open(module, WIFI_HARDWARE_MODULE_ID, (struct hw_device_t**)&wifi);
      wifi->wifi_init();
      wifi_init_flag  = 1;
#if CONFIG_WIFI_SMART_SNIFFER_ENABLE
#ifndef SMART_IN_HAL
		if (smart_config_init(wifi)) {
			printf("smart config init failed!\n");
			return -1;
		}
#endif
#endif
	}
	return 0;
}

static int wifi_deinit(int argc,int8_t * const argv[])
{
   if(wifi_init_flag == 1){
 
     printf("wifi_deinit\n");
	if (!wifi) {
		printf("wifi not init, please init it first!\n");
		return -1;
	}
#ifndef SMART_IN_HAL
#if CONFIG_WIFI_SMART_SNIFFER_ENABLE
	smart_config_deinit(sconfig);
#endif
#endif

	if (wifi) {
		 wifi->wifi_deinit();
		 wifi_init_flag  = 0;
		 wifi->common.close((hw_device_t*)wifi); 
	}
  }

  return 0;
}

static int wifi_start_station(int argc,int8_t *const argv[])
{
	if (!wifi) {
		printf("wifi not init, please init it first!\n");
		return -1;
	}

	wifi->start_station();
	return 0;
}

static int wifi_sta_connect_ap(int argc,int8_t *const argv[])
{
	char *ssid = "ROKID.TC";
	char *passwd = "rokidguys";

	if(wifi_init_flag == 0){
		wifi_init(0,NULL);  
		wifi_init_flag = 1;
	}

	if(argc >= 3){
		ssid = argv[1];
		passwd = argv[2];
	}

	if(argc == 2) {
		ssid = argv[1];
		passwd = NULL;
	}

	printf("wifi ssid:%s passwd:%s connecting ...\n",ssid,passwd);
	if (!wifi) {
		printf("wifi not init, please init it first!\n");
		return -1;
	}
	wifi->sta_connect(ssid, passwd);
	return 0;
}

static int wifi_sta_disconnect(int argc,int8_t * const argv[])
{
	if (!wifi) {
		printf("wifi not init, please init it first!\n");
		return -1;
	}
	wifi->sta_disconnect();
	return 0;
}

static int wifi_stop_station(int argc,int8_t *const argv[])
{
	if (!wifi) {
		printf("wifi not init, please init it first!\n");
		return -1;
	}

  wifi->stop_station();

  return 0;
}

static int wifi_start_ap(int argc,int8_t *const argv[])
{
   char *ssid = "shanjiaming";
   char *passwd = "";
   char *ipaddr = "192.168.1.8";
   
	if (!wifi) {
		printf("wifi not init, please init it first!\n");
		return -1;
	}

   if(wifi_init_flag == 0){
      wifi_init(0,NULL);  
      wifi_init_flag = 1;
   }

   if(argc >= 2){
       ipaddr = argv[1];
   }

   if(argc >= 3)
     ssid = argv[2];


   if(argc >= 4)
       passwd = argv[3];

  printf("wifi ssid:%s passwd:%s ipaddr:%s connecting ...\n",ssid,passwd,ipaddr);

  wifi->start_ap(ssid, passwd, ipaddr);

  return 0;
}

static int wifi_stop_ap(int argc,int8_t * const argv[])
{
	if (!wifi) {
		printf("wifi not init, please init it first!\n");
		return -1;
	}
	wifi->stop_ap();
	return 0;
}

static int wifi_get_status(int argc,int8_t *const argv[])
{
   uint32_t ipaddr;
   char mac[6];
   char bssid[6];
   wifi_hal_status_t status = {0};

	if (!wifi) {
		printf("wifi not init, please init it first!\n");
		return -1;
	}

  wifi->wifi_get_status(&status);
  printf("Wifi is in %s mode:\n", status.mode ? "STATION" : "AP");
  if (status.mode == STA_MODE) {
	ipaddr = status.ap_sta_status.sta_status.ipaddr;
	memcpy(mac, status.ap_sta_status.sta_status.mac, 6);
	memcpy(bssid, status.ap_sta_status.sta_status.bssid, 6);
	if (status.ap_sta_status.sta_status.status == STA_CONNECTED) {
		printf("Station has connected to SSID:%s passwd:%s\n", status.ap_sta_status.sta_status.ssid, 
				status.ap_sta_status.sta_status.passwd);
		printf("Station ipaddr: 0x%08x-- %d.%d.%d.%d, mac: %02x:%02x:%02x:%02x:%02x:%02x\n", ipaddr, 
				(uint8_t)(ipaddr & 0xff), (uint8_t)((ipaddr >> 8) &0xff), (uint8_t)((ipaddr >> 16) & 0xff), 
				(uint8_t)((ipaddr >> 24) & 0xff), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		printf("AP bssid: %02x:%02x:%02x:%02x:%02x:%02x channel = %d rssi = %d\n", bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5],
			status.ap_sta_status.sta_status.channel, status.ap_sta_status.sta_status.rssi);
	} else {
		printf("Station has not connect to AP!\n");
	}

  } else if (status.mode == AP_MODE) {

	ipaddr = status.ap_sta_status.ap_status.ipaddr;
	memcpy(mac, status.ap_sta_status.ap_status.mac, 6);
	printf("AP ssid is %s passwd = %s channel is %d\n", status.ap_sta_status.ap_status.ssid, 
			status.ap_sta_status.ap_status.passwd, status.ap_sta_status.ap_status.channel);
	printf("AP ipaddr: 0x%08x-- %d.%d.%d.%d, mac: %02x:%02x:%02x:%02x:%02x:%02x\n", ipaddr, 
			(uint8_t)(ipaddr & 0xff), (uint8_t)((ipaddr >> 8) &0xff), (uint8_t)((ipaddr >> 16) & 0xff), 
			(uint8_t)((ipaddr >> 24) & 0xff), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	
  } else {
	  printf("Wifi is never in AP mode nor in station mode!\n");
  }

  return 0;
}

static int wifi_scan_start(int argc,int8_t *const argv[])
{
	if (!wifi) {
		printf("wifi not init, please init it first!\n");
		return -1;
	}

  wifi->scan_start();
  return 0;
}

static int wifi_scan_stop(int argc,int8_t *const argv[])
{
	if (!wifi) {
		printf("wifi not init, please init it first!\n");
		return -1;
	}
  wifi->scan_stop();
  return 0;
}

static int wifi_get_scan_list(int argc,int8_t *const argv[])
{
   ssid_t *ssid_array = NULL;
   uint16_t ssid_cnt = 0;
   int i = 0;

	if (!wifi) {
		printf("wifi not init, please init it first!\n");
		return -1;
	}

  wifi->get_scan_list_cnt(&ssid_cnt);
  ssid_array = (ssid_t*)yodalite_malloc(sizeof(ssid_t) * ssid_cnt);
  if (!ssid_array) {
	  printf("Malloc ssid_array failed!\n");
	  return -1;
  }
  wifi->get_scan_list(ssid_array, ssid_cnt);
  printf("Get %d AP:\n", ssid_cnt);
  for (i = 0; i < ssid_cnt; i++) {
	  printf("SSID : %s Signal : %d\n", ssid_array[i].ssid, ssid_array[i].signal);
  }
  yodalite_free(ssid_array);
  return 0;
}

#if CONFIG_WIFI_SMART_SNIFFER_ENABLE
static int wifi_smart_preamble_set(int argc, int8_t *const argv[])
{
   uint8_t len;
   int i;
   int buf[8];

	if (!wifi) {
		printf("wifi not init, please init it first!\n");
		return -1;
	}

  if (argc < 4) {
	  printf("Too less argvs!\n");
	  return -1;
  }
  len = atoi(argv[1]);
  for (i = 0; i < len; i++) {
	  buf[i] = atoi(argv[i + 2]);
  }
#ifdef SMART_IN_HAL
  wifi->smart_preamble_set(buf, len);
#else
  smart_preamble_set(sconfig, buf, len);
#endif

  return 0;
}

static int wifi_smart_format_set(int argc, int8_t *const argv[])
{
	smart_format_t format;

	if (!wifi) {
		printf("wifi not init, please init it first!\n");
		return -1;
	}

  if (argc < 6) {
	  printf("Too less argvs!\n");
	  return -1;
  }
  format.ascii_offset = atoi(argv[1]);
  format.ssid_id = atoi(argv[2]);
  format.passwd_id = atoi(argv[3]);
  format.data_len_offset_encrypt = atoi(argv[4]);
  format.data_len_offset_noencrypt = atoi(argv[5]);
#ifdef SMART_IN_HAL
  wifi->smart_format_set(&format);
#else
  smart_format_set(&format, sconfig);
#endif
  return 0;
}

static int wifi_smart_sniffer_start(int argc,int8_t *const argv[])
{
	if (!wifi) {
		printf("wifi not init, please init it first!\n");
		return -1;
	}

	ssid_found = 0;
  wifi->smart_sniffer_start();
  return 0;
}

static int wifi_smart_sniffer_stop(int argc,int8_t *const argv[])
{
	if (!wifi) {
		printf("wifi not init, please init it first!\n");
		return -1;
	}
  wifi->smart_sniffer_stop();
  return 0;
}
#endif

#define max_init_args      (1)
#define wifi_init_help     "wifi_init"

int cmd_wifi_init(void)
{
  YODALITE_REG_CMD(wifi_init,max_init_args,wifi_init,wifi_init_help);

  return 0;
}

#define max_deinit_args      (1)
#define wifi_deinit_help     "wifi_deinit"

int cmd_wifi_deinit(void)
{
  YODALITE_REG_CMD(wifi_deinit,max_deinit_args,wifi_deinit,wifi_deinit_help);

  return 0;
}

#define max_sstart_args      (1)
#define wifi_sstart_help     "wifi_start_station"

int cmd_wifi_station_start(void)
{
  YODALITE_REG_CMD(wifi_start_station,max_sstart_args,wifi_start_station,wifi_sstart_help);

  return 0;
}

#define max_sstop_args        (1)
#define wifi_sstop_help     "wifi_stop_station"

int cmd_wifi_station_stop(void)
{

  YODALITE_REG_CMD(wifi_stop_station,max_sstop_args,wifi_stop_station,wifi_sstop_help);

  return 0;
}

#define max_sconnect_args     (3)
#define wifi_sconnect_help    "wifi_sta_connect ssid psk"

int cmd_wifi_sta_connect_ap(void)
{
  YODALITE_REG_CMD(wifi_sta_connect_ap,max_sconnect_args,wifi_sta_connect_ap,wifi_sconnect_help);

  return 0;
}

#define max_sdisconnect_args     (3)
#define wifi_sdisconnect_help    "wifi_sta_disconnect"

int cmd_wifi_sta_disconnect(void)
{
  YODALITE_REG_CMD(wifi_sta_disconnect,max_sdisconnect_args,wifi_sta_disconnect,wifi_sdisconnect_help);

  return 0;
}

#define max_sstatus_args        (1)
#define wifi_sstatus_help     "wifi get status"

int cmd_wifi_get_status(void)
{

  YODALITE_REG_CMD(wifi_get_status,max_sstatus_args,wifi_get_status,wifi_sstatus_help);

  return 0;
}

#define max_sscan_start_args        (1)
#define wifi_sscan_start_help     "wifi start scanning"

int cmd_wifi_scan_start(void)
{

  YODALITE_REG_CMD(wifi_scan_start,max_sscan_start_args,wifi_scan_start,wifi_sscan_start_help);

  return 0;
}

#define max_sscan_stop_args        (1)
#define wifi_sscan_stop_help     "wifi stop scanning"

int cmd_wifi_scan_stop(void)
{

  YODALITE_REG_CMD(wifi_scan_stop,max_sscan_stop_args,wifi_scan_stop,wifi_sscan_stop_help);

  return 0;
}

#define max_sget_scan_list_args        (1)
#define wifi_sget_scan_list_help     "wifi get scan list"

int cmd_wifi_get_scan_list(void)
{

  YODALITE_REG_CMD(wifi_get_scan_list,max_sget_scan_list_args,wifi_get_scan_list,wifi_sget_scan_list_help);

  return 0;
}


#define max_astart_args      (4)
#define wifi_astart_help     "wifi_start_ap ip ssid psk"

int cmd_wifi_ap_start(void)
{
  YODALITE_REG_CMD(wifi_start_ap,max_astart_args,wifi_start_ap,wifi_astart_help);

  return 0;
}

#define max_astop_args        (1)
#define wifi_astop_help     "wifi stop ap"

int cmd_wifi_ap_stop(void)
{

  YODALITE_REG_CMD(wifi_stop_ap,max_astop_args,wifi_stop_ap,wifi_astop_help);

  return 0;
}

#if CONFIG_WIFI_SMART_SNIFFER_ENABLE
#define max_smart_preamble_set_args        (8)
#define wifi_smart_preamble_help     "smart preamble set: wifi_smart_preamble_set len p0 p1 p2 ..."

int cmd_smart_preamble_set(void)
{

  YODALITE_REG_CMD(wifi_smart_preamble_set,max_smart_preamble_set_args,wifi_smart_preamble_set,wifi_smart_preamble_help);

  return 0;
}

#define max_smart_format_set_args        (6)
#define wifi_smart_format_help     "smart format set: wifi_smart_format_set offset ssid_id passwd_id data_len_offset_encrypt data_len_offset_noencrypt"

int cmd_smart_format_set(void)
{

  YODALITE_REG_CMD(wifi_smart_format_set,max_smart_format_set_args,wifi_smart_format_set,wifi_smart_format_help);

  return 0;
}

#define max_smart_start_args        (1)
#define wifi_smart_start_help     "smart sniffer start"

int cmd_smart_start(void)
{

  YODALITE_REG_CMD(wifi_smart_sniffer_start,max_smart_start_args,wifi_smart_sniffer_start,wifi_smart_start_help);

  return 0;
}

#define max_smart_stop_args        (1)
#define wifi_smart_stop_help     "smart sniffer stop"

int cmd_smart_stop(void)
{

  YODALITE_REG_CMD(wifi_smart_sniffer_stop,max_smart_stop_args,wifi_smart_sniffer_stop,wifi_smart_stop_help);

  return 0;
}
#endif

