/*
 * =====================================================================================
 *
 *       Filename:  wifi_hw_esp.c
 *
 *    Description:  wifi_hw for esp
 *
 *        Version:  1.0
 *        Created:  04/16/2019 08:39:02 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  glhou (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "yodalite_autoconf.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "wifi_api.h"
#include "lwip/inet.h"
#include "hardware/wifi_hal.h"
#if CONFIG_WIFI_SMART_SNIFFER_ENABLE
#include "wifi_smart_sniffer.h"
#endif
/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;

static const char *TAG = "simple wifi";

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
#if 0
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
#endif
	case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;
	case SYSTEM_EVENT_AP_STACONNECTED:
        ESP_LOGI(TAG, "station:"MACSTR" join, AID=%d",
                 MAC2STR(event->event_info.sta_connected.mac),
                 event->event_info.sta_connected.aid);
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        ESP_LOGI(TAG, "station:"MACSTR"leave, AID=%d",
                 MAC2STR(event->event_info.sta_disconnected.mac),
                 event->event_info.sta_disconnected.aid);
        break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;
	default:
        break;
    }
    return ESP_OK;
}

static int wifi_init(void)
{
    wifi_event_group = xEventGroupCreate();
    tcpip_adapter_init();
    if (esp_event_loop_init(event_handler, NULL) != ESP_OK) {
		printf("Esp event loop init failed!\n");
		return -1;
	}
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&cfg) != ESP_OK) {
		printf("Esp wifi init failed!\n");
		return -1;
	}
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) );
    ESP_ERROR_CHECK( esp_wifi_start() );
	return 0;
}

static int wifi_deinit(void)
{
   	if (esp_wifi_stop() != ESP_OK) {
		printf("Stop wifi failed!\n");
		return -1;
	}
	esp_wifi_deinit();
    vEventGroupDelete(wifi_event_group);
	return 0;
}

static int wifi_get_status(wifi_hal_status_t *wifi_status)
{
	EventBits_t e_bits;
    wifi_config_t cfg = {0};
    wifi_mode_t mode;
    tcpip_adapter_ip_info_t ip_info;
    tcpip_adapter_if_t ifx;
	e_bits = xEventGroupGetBits(wifi_event_group);

    if (esp_wifi_get_mode(&mode) != ESP_OK) {
		printf("esp get wifi mode failed!\n");
		return -1;
	}
    if (WIFI_MODE_AP == mode) {
		wifi_status->mode = AP_MODE;
        esp_wifi_get_config(WIFI_IF_AP, &cfg);
		memcpy(wifi_status->ap_sta_status.ap_status.ssid, cfg.ap.ssid, 32);
		memcpy(wifi_status->ap_sta_status.ap_status.passwd, cfg.ap.password, 64);
		wifi_status->ap_sta_status.ap_status.channel = cfg.ap.channel;	
		if (esp_wifi_get_mac(WIFI_IF_AP, wifi_status->ap_sta_status.ap_status.mac) != ESP_OK) {
			printf("esp get wifi mac failed!\n");
			return -1;
		}
		ifx = TCPIP_ADAPTER_IF_AP;
		tcpip_adapter_get_ip_info(ifx, &ip_info);
		wifi_status->ap_sta_status.ap_status.ipaddr = ip_info.ip.addr;
  //      ESP_LOGI(TAG, "AP mode, %s %s", cfg.ap.ssid, cfg.ap.password);
    } else if (WIFI_MODE_STA == mode) {
		wifi_status->mode = STA_MODE;
		wifi_ap_record_t ap_info;
        if (e_bits & BIT0) {
            esp_wifi_get_config(WIFI_IF_STA, &cfg);
			memcpy(wifi_status->ap_sta_status.sta_status.ssid, cfg.sta.ssid, 32);
			memcpy(wifi_status->ap_sta_status.sta_status.passwd, cfg.sta.password, 64);
			memcpy(wifi_status->ap_sta_status.sta_status.bssid, cfg.sta.bssid, 6);
			
			esp_wifi_sta_get_ap_info(&ap_info);
			wifi_status->ap_sta_status.sta_status.channel = ap_info.primary;
			wifi_status->ap_sta_status.sta_status.rssi = ap_info.rssi;
			wifi_status->ap_sta_status.sta_status.status = STA_CONNECTED;
			if (esp_wifi_get_mac(WIFI_IF_STA, wifi_status->ap_sta_status.sta_status.mac) != ESP_OK) {
				printf("esp get wifi mac failed!\n");
				return -1;
			}
			ifx = TCPIP_ADAPTER_IF_STA;
			tcpip_adapter_get_ip_info(ifx, &ip_info);
			wifi_status->ap_sta_status.sta_status.ipaddr = ip_info.ip.addr;
 
  //         ESP_LOGI(TAG, "sta mode, connected %s", cfg.ap.ssid);
        } else {
			wifi_status->ap_sta_status.sta_status.status = STA_DISCONNECTED;
  //        ESP_LOGI(TAG, "sta mode, disconnected");
        }
    } else {
		wifi_status->mode = NONE_MODE;
  //        ESP_LOGI(TAG, "NULL mode");
        return 0;
    }
    return 0;

}

static int start_sta(void)
{
    if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK) {
		printf("wifi set mode as station failed!\n");
		return -1;
	}
	return 0;
}

static int stop_sta(void)
{
    if (esp_wifi_set_mode(WIFI_MODE_NULL) != ESP_OK) {
		printf("wifi set mode as station failed!\n");
		return -1;
	}
	return 0;
}

static int sta_connect(char *ssid, char *passwd)
{
    wifi_config_t wifi_config = {0};

	strcpy((char *)wifi_config.sta.ssid, ssid);
	strcpy((char *)wifi_config.sta.password, passwd);
    if (esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) != ESP_OK) {
		printf("wifi set station config failed!\n");
		return -1;
	}
	if (esp_wifi_connect() != ESP_OK) {
		printf("Wifi connect to ap:%s failed!\n", ssid);
		return -1;
	}
   // ESP_LOGI(TAG, "wifi_init_sta finished.");
   // ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
   //          EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);

	return 0;
}

static int sta_disconnect(void)
{
	if (esp_wifi_disconnect() != ESP_OK) {
		printf("Wifi disconnect from AP failed!\n");
		return -1;
	}
	return 0;
}

static int start_ap(char *ssid, char* passwd, char* ipaddr)
{
    wifi_config_t wifi_config = {0};

	strcpy((char*)wifi_config.ap.ssid, ssid);
	strcpy((char*)wifi_config.ap.password, passwd);
	wifi_config.ap.ssid_len = strlen(ssid);
	wifi_config.ap.max_connection = 4;
	wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    if (strlen(passwd) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
	if (ipaddr) {
		printf("set ipaddr %s\n", ipaddr);
		tcpip_adapter_ip_info_t ip;
		inet_aton(ipaddr, &ip.ip);
		inet_aton(ipaddr, &ip.gw);
		IP4_ADDR(&ip.netmask, 255, 255 , 255, 0);
		ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
		tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &ip);
		ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));
	}
	if (esp_wifi_set_mode(WIFI_MODE_AP) != ESP_OK) {
		printf("Esp set wifi mode as station failed!\n");
		return -1;
	}
    if (esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config) != ESP_OK) {
		printf("Esp wifi set AP config failed!\n");
		return -1;
	}
	printf("start ap success!\n");
	return 0;
}

static int stop_ap(void)
{
    if (esp_wifi_set_mode(WIFI_MODE_NULL) != ESP_OK) {
		printf("wifi set mode as station failed!\n");
		return -1;
	}
	return 0;
}

static int wifi_scan_start(void)
{
	wifi_scan_config_t scanConf = {0};
	printf("set wifi mode as station!\n");
    if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK) {
		printf("wifi set mode as station failed!\n");
		return -1;
	}
	printf("Start wifi scan!\n");
	if (esp_wifi_scan_start(&scanConf, false) != ESP_OK) {
		printf("Esp wifi start scan failed!\n");
		return -1;
	}
	return 0;
} 
static int wifi_scan_stop(void)
{
	if (esp_wifi_scan_stop() != ESP_OK) {
		printf("Esp wifi stop scan failed!\n");
		return -1;
	}
	return 0;
}

static int wifi_get_scan_list_cnt(uint16_t *ssid_cnt)
{
	esp_wifi_scan_get_ap_num(ssid_cnt);
	return 0;
}

static int wifi_get_scan_list(ssid_t *ssid_array, uint16_t ssid_cnt)
{
	int i = 0;
	wifi_ap_record_t *ap_list = NULL;

	ap_list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * ssid_cnt);
	if (!ap_list) {
		printf("Malloc ap_list failed!\n");
		return -1;
	}

	if (esp_wifi_scan_get_ap_records(&ssid_cnt, ap_list) != ESP_OK) {
		printf("Wifi get scan ap records failed!\n");
		free(ap_list);
		return -1;
	}
	for (i = 0; i < ssid_cnt; i++) {
		strncpy(ssid_array[i].ssid, (char*)ap_list[i].ssid, 33);
		ssid_array[i].signal = ap_list[i].rssi;
	}
	free(ap_list);
	return 0;
}

struct wifi_device_t wifi_dev = {
	.wifi_init = wifi_init,
	.wifi_deinit = wifi_deinit,
	.start_station = start_sta,
	.stop_station = stop_sta,
	.wifi_get_status = wifi_get_status,
	.sta_connect = sta_connect,
	.sta_disconnect = sta_disconnect,
	.start_ap = start_ap,
	.stop_ap = stop_ap,
	.scan_start = wifi_scan_start,
	.scan_stop = wifi_scan_stop,
	.get_scan_list_cnt = wifi_get_scan_list_cnt,
	.get_scan_list = wifi_get_scan_list,
#if CONFIG_WIFI_SMART_SNIFFER_ENABLE
	.smart_sniffer_start = smart_sniffer_start,
	.smart_sniffer_stop = smart_sniffer_stop,
#ifdef SMART_IN_HAL
	.smart_preamble_set = smart_preamble_set,
	.smart_format_set = smart_format_set,
#else
	.smart_set_listener = smart_set_listener,
	.sniffer_channel_set = sniffer_channel_set,
#endif
#endif
};

