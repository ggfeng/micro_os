/*
 * =====================================================================================
 *
 *       Filename:  wifi_smart_sniffer.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/01/2019 10:27:29 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  guoliang.hou (Rokid), 
 *   Organization:  Rokid
 *
 * =====================================================================================
 */

#include "common/framework/net_ctrl.h"
#include "smartlink/sc_assistant.h"
#include "hardware/wifi_hal.h"
#include "smart_config.h"
#include "wifi_smart_sniffer.h"

#define SC_TIME_OUT 120000
int debug_level = LOG_INFO;
static smartconfig_priv_t *smart_priv;
static sniffer_listener_t sniffer_listener = NULL;

int smart_set_listener(sniffer_listener_t listener)
{
	if (!listener) {
		printf("listener is NULL!\n");
		return -1;
	}
	sniffer_listener = listener;
	return 0;
}

static void smartconfig_recv_rawframe(uint8_t *data, uint32_t len, void *info)
{
    sniffer_packet_into_t packet_info;

	if (len < sizeof(struct ieee80211_frame)) {
		SMART_SNIFFER_DBG(ERROR, "%s():%d, len %u\n", __func__, __LINE__, len);
		return;
	}

	memset(&packet_info, 0, sizeof(sniffer_packet_into_t));
	packet_info.payload = data;
	packet_info.length = len;
	packet_info.info = info;

	if (sniffer_listener) {
		sniffer_listener(&packet_info);
	}

}

static void smartconfig_sw_ch_cb(struct netif *nif, int16_t channel)
{
	/* do nothing */
	printf("Switch to channel %d\n", channel);
}

static wlan_smart_config_status_t smart_sniffer_start_low_level(smartconfig_priv_t *priv)
{
	int ret = -1;
	wlan_smart_config_status_t status = WLAN_SMART_CONFIG_SUCCESS;

	if (!priv) {
		return WLAN_SMART_CONFIG_FAIL;
	}

	priv->status = SC_STATUS_SEARCH_CHAN;

	ret = sc_assistant_monitor_register_rx_cb(priv->nif, smartconfig_recv_rawframe);
	if (ret) {
		SMART_SNIFFER_DBG(LOG_ERROR, "%s monitor set rx cb fail\n", __func__);
		status = WLAN_SMART_CONFIG_FAIL;
		goto out;
	}
	ret = sc_assistant_monitor_register_sw_ch_cb(priv->nif, smartconfig_sw_ch_cb);
	if (ret) {
		SMART_SNIFFER_DBG(LOG_ERROR, "%s monitor sw ch cb fail\n", __func__);
		status = WLAN_SMART_CONFIG_FAIL;
		goto out;
	}

out:
	return status;
}

int smart_sniffer_start(void)
{
	smartconfig_priv_t *priv = smart_priv;
	sc_assistant_fun_t sca_fun;
	sc_assistant_time_config_t config;

	sc_assistant_get_fun(&sca_fun);
	config.time_total = SC_TIME_OUT;
	config.time_sw_ch_long = 400;
	config.time_sw_ch_short = 100;
	sc_assistant_init(g_wlan_netif, &sca_fun, &config);

	if (priv) {
		SMART_SNIFFER_DBG(LOG_ERROR, "%s has already started!\n", __func__);
		return -1;
	}
#if 0
	if (net_switch_mode(WLAN_MODE_MONITOR) < 0) {
		SMART_SNIFFER_DBG(LOG_ERROR, "Set wifi monitor mode failed!\n");
		return -1;
	}
#endif
	priv = malloc(sizeof(smartconfig_priv_t));
	if (!priv) {
		SMART_SNIFFER_DBG(LOG_ERROR, "%s malloc failed!\n", __func__);
		return -ENOMEM;
	}
	printf("func = %s line = %d\n", __func__, __LINE__);
	memset(priv, 0, sizeof(smartconfig_priv_t));
	smart_priv = priv;

	priv->status = SC_STATUS_SEARCH_CHAN;
	priv->nif = g_wlan_netif;

	if (smart_sniffer_start_low_level(priv)) {
		SMART_SNIFFER_DBG(LOG_ERROR, "%s,%d err!\n", __func__, __LINE__);
		goto out;
	}

	return 0;

out:
	smart_priv = NULL;
	free(priv);
	return -1;
}

int smart_sniffer_stop(void)
{
	smartconfig_priv_t *priv = smart_priv;

	if (!priv) {
		printf("priv is NULL!\n");
		return -1;
	}

	SMART_SNIFFER_DBG(LOG_INFO, "stop\n");

	if (sc_assistant_monitor_unregister_rx_cb(priv->nif, smartconfig_recv_rawframe)) {
		SMART_SNIFFER_DBG(LOG_ERROR, "%s,%d cancel rx cb fail\n", __func__, __LINE__);
	}
	if (sc_assistant_monitor_unregister_sw_ch_cb(priv->nif, smartconfig_sw_ch_cb)) {
		SMART_SNIFFER_DBG(LOG_ERROR, "%s,%d cancel sw ch cb fail\n", __func__, __LINE__);
	}

	priv->status = SC_STATUS_END;
	free(priv);
	smart_priv = NULL;
	printf("func %s line = %d", __func__, __LINE__);
//	sc_assistant_deinit(g_wlan_netif);
	printf("func %s line = %d", __func__, __LINE__);
	return 0;
}

/* stop channel change timer and set the channel to ensure we were in the config channel */
int sniffer_channel_set(sniffer_packet_into_t *packet_info)
{
	struct ieee80211_frame *hdr = NULL;
	uint8_t *ap_mac = NULL;

	hdr = packet_info->payload;
	ap_mac = hdr->i_addr2;
	sc_assistant_newstatus(SCA_STATUS_CHANNEL_LOCKED, ap_mac, packet_info->info);
	return 0;
}

