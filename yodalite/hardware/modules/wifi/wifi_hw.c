#define LOG_TAG "WifiHAL"

//#include <yodalite_autoconf.h>
#include <hardware/hardware.h>
#include <hardware/wifi_hal.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define MODULE_NAME "WifiHAL"
#define MODULE_AUTHOR "guoliang.hou@rokid.com"

static struct wifi_device_t wifi_dev;

int wifi_device_init(struct wifi_device_t *dev)
{
	if (!dev) {
		return -EINVAL;
	}
	wifi_dev.wifi_init = dev->wifi_init;
	wifi_dev.wifi_deinit = dev->wifi_deinit;
	wifi_dev.wifi_get_status = dev->wifi_get_status;
	wifi_dev.start_station = dev->start_station;
	wifi_dev.stop_station = dev->stop_station;
	wifi_dev.sta_connect = dev->sta_connect;
	wifi_dev.sta_disconnect = dev->sta_disconnect;
	wifi_dev.start_ap = dev->start_ap;
	wifi_dev.stop_ap = dev->stop_ap;
	wifi_dev.scan_start = dev->scan_start;
	wifi_dev.scan_stop = dev->scan_stop;
	wifi_dev.get_scan_list_cnt = dev->get_scan_list_cnt;
	wifi_dev.get_scan_list = dev->get_scan_list;
#if CONFIG_WIFI_SMART_SNIFFER_ENABLE
	wifi_dev.smart_sniffer_start = dev->smart_sniffer_start;
	wifi_dev.smart_sniffer_stop = dev->smart_sniffer_stop;
#ifdef SMART_IN_HAL
	wifi_dev.smart_preamble_set = dev->smart_preamble_set;
	wifi_dev.smart_format_set = dev->smart_format_set;
#else
	wifi_dev.smart_set_listener = dev->smart_set_listener;
	wifi_dev.sniffer_channel_set = dev->sniffer_channel_set;
#endif
#endif
	return WIFI_OK;
}

static int wifi_init(void)
{
	int ret = -EACCES;
	if (wifi_dev.wifi_init) {
		ret = wifi_dev.wifi_init();
	}
	return ret;
}

static int wifi_deinit(void)
{
	int ret = -EACCES;
	if (wifi_dev.wifi_deinit) {
		ret = wifi_dev.wifi_deinit();
	}
	return ret;
}

static int wifi_get_status(wifi_hal_status_t *wifi_status)
{
	int ret = -EACCES;
	if (wifi_dev.wifi_get_status) {
		ret = wifi_dev.wifi_get_status(wifi_status);
	}
	return ret;
}

static int start_station(void)
{
	int ret = -EACCES;
	if (wifi_dev.start_station) {
		ret = wifi_dev.start_station();
	}
	return ret;
}

static int stop_station(void)
{
	int ret = -EACCES;
	if (wifi_dev.stop_station) {
		ret = wifi_dev.stop_station();
	}
	return ret;
}

static int start_ap(char *ssid, char *passwd, char *ipaddr)
{
	int ret = -EACCES;
	if (wifi_dev.start_ap) {
		ret = wifi_dev.start_ap(ssid, passwd, ipaddr);
	}
	return ret;
}

static int stop_ap(void)
{
	int ret = -EACCES;
	if (wifi_dev.stop_ap) {
		ret = wifi_dev.stop_ap();
	}
	return ret;
}

static int sta_connect(char *ssid, char *passwd)
{
	int ret = -EACCES;
	if (wifi_dev.sta_connect) {
		ret = wifi_dev.sta_connect(ssid, passwd);
	}
	return ret;
}

static int sta_disconnect(void)
{
	int ret = -EACCES;
	if (wifi_dev.sta_disconnect) {
		ret = wifi_dev.sta_disconnect();
	}
	return ret;
}

static int wifi_scan_start(void)
{
	int ret = -EACCES;
	if (wifi_dev.scan_start) {
		ret = wifi_dev.scan_start();
	}
	return ret;
}

static int wifi_scan_stop(void)
{
	int ret = -EACCES;
	if (wifi_dev.scan_stop) {
		ret = wifi_dev.scan_stop();
	}
	return ret;
}

static int wifi_get_scan_list_cnt(uint16_t *ssid_cnt)
{
	int ret = -EACCES;
	if (wifi_dev.get_scan_list) {
		ret = wifi_dev.get_scan_list_cnt(ssid_cnt);
	}
	return ret;
}

static int wifi_get_scan_list(ssid_t *ssid, uint16_t ssid_cnt)
{
	int ret = -EACCES;
	if (wifi_dev.get_scan_list) {
		ret = wifi_dev.get_scan_list(ssid, ssid_cnt);
	}
	return ret;
}

#if CONFIG_WIFI_SMART_SNIFFER_ENABLE
static int wifi_smart_sniffer_start(void)
{
	int ret = -EACCES;
	if (wifi_dev.smart_sniffer_start) {
		ret = wifi_dev.smart_sniffer_start();
	}
	return ret;
}

static int wifi_smart_sniffer_stop(void)
{
	int ret = -EACCES;
	if (wifi_dev.smart_sniffer_stop) {
		ret = wifi_dev.smart_sniffer_stop();
	}
	return ret;
}

#ifdef SMART_IN_HAL
static int wifi_smart_preamble_set(uint32_t *buf, uint8_t len)
{
	int ret = -EACCES;
	if (wifi_dev.smart_preamble_set) {
		ret = wifi_dev.smart_preamble_set(buf, len);
	}
	return ret;
}

static int wifi_smart_format_set(smart_format_t *format)
{
	int ret = -EACCES;
	if (wifi_dev.smart_format_set) {
		ret = wifi_dev.smart_format_set(format);
	}
	return ret;
}
#else
static int wifi_smart_set_listener(sniffer_listener_t listener)
{
	int ret = -EACCES;
	if (wifi_dev.smart_set_listener) {
		ret = wifi_dev.smart_set_listener(listener);
	}
	return ret;
}

static int wifi_sniffer_channel_set(sniffer_packet_into_t *packet_info)
{
	int ret = -EACCES;
	if (wifi_dev.sniffer_channel_set) {
		ret = wifi_dev.sniffer_channel_set(packet_info);
	}
	return ret;
}

#endif
#endif

static int wifi_device_close(struct hw_device_t *device)
{
	struct wifi_device_t *wifi_device = (struct wifi_device_t *)device;

	if (wifi_device) {
		free(wifi_device);
	}
	return 0;
}

static int wifi_device_open(const struct hw_module_t *module, const char *name,
			     struct hw_device_t **device)
{
	struct wifi_device_t *dev;
	dev = (struct wifi_device_t *)malloc(sizeof(struct wifi_device_t));

	if (!dev) {
		printf("Wifi HAL: failed to alloc space\n");
		return -EFAULT;
	}

	memset(dev, 0, sizeof(struct wifi_device_t));
	dev->common.tag = HARDWARE_DEVICE_TAG;
	dev->common.version = 0;
	dev->common.module = (hw_module_t *) module;
	dev->common.close = wifi_device_close;

	dev->wifi_init = wifi_init;
	dev->wifi_deinit = wifi_deinit;
	dev->wifi_get_status = wifi_get_status;
	dev->start_station = start_station;
	dev->stop_station = stop_station;
	dev->sta_connect = sta_connect;
	dev->sta_disconnect = sta_disconnect;
	dev->start_ap = start_ap;
	dev->stop_ap = stop_ap;
	dev->scan_start = wifi_scan_start;
	dev->scan_stop = wifi_scan_stop;
	dev->get_scan_list_cnt = wifi_get_scan_list_cnt;
	dev->get_scan_list = wifi_get_scan_list;
#if CONFIG_WIFI_SMART_SNIFFER_ENABLE
	dev->smart_sniffer_start = wifi_smart_sniffer_start;
	dev->smart_sniffer_stop = wifi_smart_sniffer_stop;
#ifdef SMART_IN_HAL
	dev->smart_preamble_set = wifi_smart_preamble_set;
	dev->smart_format_set = wifi_smart_format_set;
#else
	dev->smart_set_listener = wifi_smart_set_listener;
	dev->sniffer_channel_set = wifi_sniffer_channel_set;
#endif
#endif
	*device = &(dev->common);
	printf("Wifi HAL: open successfully.\n");

	return 0;
}

static struct hw_module_methods_t wifi_module_methods = {
	.open = wifi_device_open,
};

struct hw_module_t HAL_MODULE_INFO_SYM(WIFI_HAL_HW_ID) = {
	.tag = HARDWARE_MODULE_TAG,
	.version_major = 1,
	.version_minor = 0,
	.id = WIFI_HARDWARE_MODULE_ID,
	.name = MODULE_NAME,
	.author = MODULE_AUTHOR,
	.methods = &wifi_module_methods,
};
