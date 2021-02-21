/*
 * =====================================================================================
 *
 *       Filename:  wifi_api.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/30/2019 08:05:36 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  guoliang.hou (Rokid), 
 *   Organization:  Rokid
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <string.h>
#include "yodalite_autoconf.h"
#include "common/cmd/cmd_util.h"
#include "common/cmd/cmd.h"
#include "net/wlan/wlan.h"
#include "common/framework/net_ctrl.h"
#include "hardware/wifi_hal.h"

#if CONFIG_WIFI_SMART_SNIFFER_ENABLE
#include "wifi_smart_sniffer.h"
#endif

static int wifi_mode;
#define WIFI_STA 0
#define WIFI_AP 1
#define WIFI_MONITOR 2
#define WIFI_NULL 3

static uint16_t wlan_event = NET_CTRL_MSG_WLAN_DISCONNECTED;
static uint16_t wlan_connect = FALSE;
static void wlan_msg_recv(uint32_t event, uint32_t data, void *arg)
{
	uint16_t type = EVENT_SUBTYPE(event);
	printf("%s msg type:%d\n", __func__, type);

	switch (type) {
	case NET_CTRL_MSG_WLAN_CONNECTED:
		wlan_event = NET_CTRL_MSG_WLAN_CONNECTED;
		wlan_connect = TRUE;
		break;
	case NET_CTRL_MSG_WLAN_DISCONNECTED:
		wlan_event = NET_CTRL_MSG_WLAN_DISCONNECTED;
		wlan_connect = FALSE;
		break;
	case NET_CTRL_MSG_WLAN_SCAN_SUCCESS:
	case NET_CTRL_MSG_WLAN_SCAN_FAILED:
	case NET_CTRL_MSG_WLAN_4WAY_HANDSHAKE_FAILED:
	case NET_CTRL_MSG_WLAN_CONNECT_FAILED:
		break;
	case NET_CTRL_MSG_CONNECTION_LOSS:
		wlan_event = WLAN_EVENT_CONNECTION_LOSS;
		break;
	case NET_CTRL_MSG_NETWORK_UP:
		wlan_event = NET_CTRL_MSG_NETWORK_UP;
		break;
	case NET_CTRL_MSG_NETWORK_DOWN:
		wlan_event = NET_CTRL_MSG_NETWORK_DOWN;
		break;
#if (!defined(__CONFIG_LWIP_V1) && LWIP_IPV6)
	case NET_CTRL_MSG_NETWORK_IPV6_STATE:
		break;
#endif
	default:
		printf("unknown msg (%u, %u)\n", type, data);
		break;
	}
}

static int wlan_msg_init(void)
{
	observer_base *ob = sys_callback_observer_create(CTRL_MSG_TYPE_NETWORK,
	                                                 NET_CTRL_MSG_ALL,
	                                                 wlan_msg_recv,
	                                                 NULL);
	if (ob == NULL)
		return -1;
	if (sys_ctrl_attach(ob) != 0)
		return -1;

	return 0;
}

static int wifi_init(void)
{
	return 0;
}

static int wifi_deinit(void)
{
   	return 0;
}

static int wifi_get_status(wifi_hal_status_t *wifi_status)
{
	if (wifi_mode == WIFI_STA) {
		wlan_sta_ap_t ap;
		wlan_sta_config_t config;

		wifi_status->mode = STA_MODE;
		if (wlan_connect == TRUE) {
			wifi_status->ap_sta_status.sta_status.status = STA_CONNECTED;
			wifi_status->ap_sta_status.sta_status.ipaddr = g_wlan_netif->ip_addr.addr;
			if (wlan_sta_ap_info(&ap) < 0) {
				printf("Get connected AP info failed!\n");
				return -1;
			}
			memcpy(wifi_status->ap_sta_status.sta_status.ssid, ap.ssid.ssid, 32);
			memcpy(wifi_status->ap_sta_status.sta_status.bssid, ap.bssid, 6);
			wifi_status->ap_sta_status.sta_status.channel = ap.channel;
			wifi_status->ap_sta_status.sta_status.rssi = ap.rssi;
			if (wlan_sta_get_config(&config) < 0) {
				printf("Get station config failed!\n");
				return -1;
			}
			memcpy(wifi_status->ap_sta_status.sta_status.passwd, config.u.psk, 64);
	
		} else {
			wifi_status->ap_sta_status.sta_status.status = STA_DISCONNECTED;
		}

	} else if (wifi_mode == WIFI_AP){
		wifi_status->mode = AP_MODE;
		wlan_ap_config_t ap_config;

		if(wlan_ap_get_config(&ap_config) < 0) {
			printf("Get ap config failed!\n");
			return -1;
		}
		memcpy(wifi_status->ap_sta_status.ap_status.ssid, ap_config.u.ssid.ssid, 32);
		memcpy(wifi_status->ap_sta_status.ap_status.passwd, ap_config.u.psk, 64);
		wifi_status->ap_sta_status.ap_status.channel = ap_config.u.channel;	

		wifi_status->ap_sta_status.ap_status.ipaddr = g_wlan_netif->ip_addr.addr;
	} else {
		wifi_status->mode = NONE_MODE;
	}
    return 0;

}

static int start_sta(void)
{
	int ret = -1;
	enum wlan_mode mode;
	
	mode = WLAN_MODE_STA;
	ret = net_switch_mode(mode);
	if (ret < 0) {
		printf("Set station mode failed!\n");
		return -1;
	}
	ret = wlan_sta_enable();
	if (ret < 0) {
		printf("Enable station failed!\n");
		return -1;
	}
	if (wlan_msg_init() < 0) {
		printf("wlan msg init failed!\n");
		return -1;
	}
	wifi_mode = WIFI_STA;
	return 0;
}

static int stop_sta(void)
{
	int ret = -1;
	ret = wlan_sta_disable();
	if (ret < 0) {
		printf("Enable station failed!\n");
		return -1;
	}
	wifi_mode = WIFI_NULL;
	return 0;
}

static int sta_connect(char *ssid, char *passwd)
{
	int ret = -1;

	printf("start to connect to ap\n");
	ret = wlan_sta_set((uint8_t *)ssid, strlen(ssid), (uint8_t *)passwd);
	if (ret < 0) {
		printf("Set station ssid and passwd failed!\n");
		return -1;
	}

	ret = wlan_sta_connect();
	if (ret < 0) {
		printf("Station connect failed!\n");
		return -1;
	}

	return 0;
}

static int sta_disconnect(void)
{
	int ret = -1;
	ret = wlan_sta_disconnect();
	if (ret < 0) {
		printf("Station disconnect failed!\n");
		return -1;
	}
	return 0;
}

static int start_ap(char *ssid, char* passwd, char* ipaddr)
{
	int ret = -1;
	enum wlan_mode mode;
//	enum cmd_status status;
	
	mode = WLAN_MODE_HOSTAP;
	ret = net_switch_mode(mode);
	if (ret < 0) {
		printf("Set AP mode failed!\n");
		return -1;
	}
	printf("ssid = %s passwd = %s\n", ssid, passwd);
	ret = wlan_ap_set((uint8_t *)ssid, strlen(ssid), (uint8_t *)passwd);
	if (ret < 0) {
		printf("Set AP ssid and passwd failed!\n");
		return -1;
	}
	ret = wlan_ap_disable();
	if (ret < 0) {
		printf("Disable AP failed or AP has been disabled!\n");
	}

	ret = wlan_ap_enable();
	if (ret < 0) {
		printf("Enable AP failed!\n");
		return -1;
	}

	wifi_mode = WIFI_AP;
	return 0;
}

static int stop_ap(void)
{
	int ret = -1;

	ret = wlan_ap_disable();
	if (ret < 0) {
		printf("Disable AP failed!\n");
		return -1;
	}
	wifi_mode = WIFI_NULL;
	return 0;
}

static int wifi_scan_start(void)
{
	int ret = -1;

	if (wifi_mode != WIFI_STA) {
		printf("Wifi is not in station mode,scan failed!\n");
	}

	ret = wlan_sta_scan_once();
	if (ret < 0) {
		printf("Start scan failed!\n");
		return -1;
	}
	return 0;
} 
static int wifi_scan_stop(void)
{
	return 0;
}

static int wifi_get_scan_list_cnt(uint16_t *ssid_cnt)
{
/* The ssid items is defined by externel*/
	*ssid_cnt = 10;
	return 0;
}

static int wifi_get_scan_list(ssid_t *ssid_array, uint16_t ssid_cnt)
{
	wlan_sta_scan_results_t results;
	int ret = -1;
	int i = 0;

	results.ap = malloc(ssid_cnt * sizeof(wlan_sta_ap_t));
	if (!results.ap) {
		printf("Malloc scan results failed!\n");
		return -1;
	}
	results.size = ssid_cnt;
	ret = wlan_sta_scan_result(&results);
	if (ret < 0) {
		printf("Get scan result failed!\n");
		return -1;
	}
	for (i = 0; i < ssid_cnt; i++) {
		strcpy(ssid_array[i].ssid, (char*)results.ap[i].ssid.ssid);
		ssid_array[i].signal = results.ap[i].rssi;
	}
	free(results.ap);
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
	.smart_set_listener = smart_set_listener,
	.sniffer_channel_set = sniffer_channel_set,
#endif
};

