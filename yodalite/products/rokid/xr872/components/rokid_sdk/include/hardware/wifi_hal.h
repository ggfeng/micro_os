#ifndef _HARDWARE_WIFI_H
#define _HARDWARE_WIFI_H

//#define SMART_IN_HAL

#include "yodalite_autoconf.h"
#include <hardware/hardware.h>
#ifndef SMART_IN_HAL
#include <hapi/smart_config/smart_config.h>
#endif
#include <stdint.h>


__BEGIN_DECLS;
#define WIFI_OK 0
#define MAX_SSID_SIZE 32
#define AP_MODE 0
#define STA_MODE 1
#define NONE_MODE 2
#define STA_CONNECTED 1
#define STA_DISCONNECTED 0
typedef struct wifi_ap_status {
	uint8_t mac[6];
	uint32_t ipaddr;
	uint8_t ssid[MAX_SSID_SIZE + 1];
	uint8_t passwd[64];
	uint8_t channel;
} wifi_ap_status_t;

typedef struct wifi_sta_status {
	uint8_t mac[6];
	uint32_t ipaddr;
	uint8_t ssid[MAX_SSID_SIZE + 1];
	uint8_t passwd[64];
	uint8_t channel;
	uint8_t bssid[6];
	uint8_t status;
	int8_t rssi;
} wifi_sta_status_t;

//typedef union wifi_ap_sta_status {
//	wifi_ap_status_t ap_status,
//	wifi_sta_status_t sta_status
//} wifi_ap_sta_status_t;

typedef struct wifi_hal_status {
	int mode;
	union {
		wifi_ap_status_t ap_status;
		wifi_sta_status_t sta_status;
	} ap_sta_status;
} wifi_hal_status_t;

typedef struct scan_ssid
{
    char ssid[MAX_SSID_SIZE + 1];
    int signal;
} ssid_t;

struct wifi_module_t {
	struct hw_module_t common;
};

#ifdef SMART_IN_HAL
typedef struct {
    void *payload;
    uint32_t length;
    uint32_t seconds;
    uint32_t microseconds;
	uint32_t channel;
	uint32_t secondary_channel;
} sniffer_packet_into_t;

typedef struct {
	int32_t ascii_offset;
	int32_t ssid_id;
	int32_t passwd_id;
} smart_format_t;
#endif

typedef void (*sniffer_listener_t)(sniffer_packet_into_t *packet_info);

struct wifi_device_t {
	struct hw_device_t common;
	int (*wifi_init)(void);
	int (*wifi_deinit)(void);
	int (*start_station)(void);
	int (*stop_station)(void);
	int (*wifi_get_status)(wifi_hal_status_t *wifi_status);
	int (*sta_connect)(char *ssid, char *passwd);
	int (*sta_disconnect)(void);
	int (*start_ap)(char *ssid, char *passwd, char *ipaddr);
	int (*stop_ap)(void);
	int (*start_scan)(void);
	int (*stop_scan)(void);
	int (*scan_start)(void);
	int (*scan_stop)(void);
	int (*get_scan_list_cnt)(uint16_t *ssid_cnt);
	int (*get_scan_list)(ssid_t *ssid, uint16_t ssid_cnt);
#if CONFIG_WIFI_SMART_SNIFFER_ENABLE
	int (*smart_sniffer_start)(void);
	int (*smart_sniffer_stop)(void);
#ifdef SMART_IN_HAL
	int (*smart_format_set)(smart_format_t *format);
	int (*smart_preamble_set)(uint32_t *buf, uint8_t len);
#else
	int (*smart_set_listener)(sniffer_listener_t listener);
	int (*sniffer_channel_set)(sniffer_packet_into_t *packet_info);
#endif
#endif
};

extern int wifi_device_init(struct wifi_device_t *dev);

#define WIFI_HAL_HW_ID wifi_hal

#define WIFI_HARDWARE_MODULE_ID "wifi_hal"
extern struct hw_module_t HAL_MODULE_INFO_SYM(WIFI_HAL_HW_ID);
__END_DECLS;
#endif // _HARDWARE_WIFI_H
