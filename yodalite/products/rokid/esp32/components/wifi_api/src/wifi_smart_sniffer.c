/* cmd_sniffer example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <sys/unistd.h>
#include <sys/fcntl.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "lib/eloop/eloop.h"
#include "sdkconfig.h"
#include "hardware/wifi_hal.h"
#include <osal/pthread.h>
#include "wifi_smart_sniffer.h"

//#define SMART_CONFIG_DEBUG

static const char *TAG = "wifi_smart_sniffer";

static bool sniffer_running = false;
static QueueHandle_t sniffer_work_queue = NULL;
#if 0
static SemaphoreHandle_t sem_task_over = NULL;
#endif
static pthread_mutex_t sniffer_mutex = NULL;

static eloop_id_t ch_change_id;

#define SMART_SNIFFER_MAX_CHANNEL_NUM         17
#define SMART_SNIFFER_CHANNEL_CHANGE_PERIOD   200 //ms
#define SMART_SNIFFER_MIN_RSSI                -90

#define WIFI_SNIFFER_WORK_QUEUE_LENGTH 256
#define WIFI_SNIFFER_TASK_STACK_SIZE 2560
#define WIFI_SNIFFER_TASK_PRIORITY 2

#define SMART_SNIFFER_DEBUG_ON

static int cur_chan_idx = SMART_SNIFFER_MAX_CHANNEL_NUM - 1;
typedef struct {
    bool ap_exist;
    uint8_t primary_chan;
    wifi_second_chan_t second_chan;
} smart_sniffer_chan_t;

static smart_sniffer_chan_t s_smart_sniffer_chan_tab[SMART_SNIFFER_MAX_CHANNEL_NUM] = {
    {false, 1, WIFI_SECOND_CHAN_ABOVE},
    {false, 2, WIFI_SECOND_CHAN_ABOVE},
    {false, 3, WIFI_SECOND_CHAN_ABOVE},
    {false, 4, WIFI_SECOND_CHAN_ABOVE},
    {false, 5, WIFI_SECOND_CHAN_ABOVE},
    {false, 5, WIFI_SECOND_CHAN_BELOW},
    {false, 6, WIFI_SECOND_CHAN_ABOVE},
    {false, 6, WIFI_SECOND_CHAN_BELOW},
    {false, 7, WIFI_SECOND_CHAN_ABOVE},
    {false, 7, WIFI_SECOND_CHAN_BELOW},
    {false, 8, WIFI_SECOND_CHAN_BELOW},
    {false, 9, WIFI_SECOND_CHAN_BELOW},
    {false, 10, WIFI_SECOND_CHAN_BELOW},
    {false, 11, WIFI_SECOND_CHAN_BELOW},
    {false, 12, WIFI_SECOND_CHAN_BELOW},
    {false, 13, WIFI_SECOND_CHAN_BELOW},
    {false, 14, WIFI_SECOND_CHAN_NONE},
};

#ifdef SMART_IN_HAL
static smart_preamble_t smart_preamble = {0};
static preamble_match_t preamble_match = {0};
static smart_format_t smart_format = {0};
static ssid_info_t ssid_info = {0};

static uint8_t current_preamble_n = 0;
static uint8_t bc_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#else
static sniffer_listener_t sniffer_listener = NULL;
#endif

#define CHECK_BIT(a, bit) ((a) & (1 << (bit)))

#define WLAN_FC_TODS		0x0100
#define WLAN_FC_FROMDS		0x0200
#define WLAN_FC_ISWEP		0x4000

#define DATA_LEN_OFFSET_NOENCRYPT 64
#define DATA_LEN_OFFSET_ENCRYPT 80

int smart_sniffer_stop(void);

#ifdef SMART_IN_HAL
static uint8_t cal_crc(uint8_t *ptr, uint8_t len)
{
    uint8_t i; 
    uint8_t crc=0x00;

    while(len--)
    {
        crc ^= *ptr++;   
        for (i=8; i>0; --i)    
        { 
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x31;
            else
                crc = (crc << 1);
        }
    }
    return (crc); 
}

static int cmp_macaddr(uint8_t *mac1, uint8_t *mac2)
{
	int i = 0;
	for (i = 0; i < 6; i++) {
		if (mac1[i] != mac2[i]) {
			return -1;
		}
	}
	return 0;
}

static void smart_preamble_match(sniffer_packet_into_t *packet_info)
{
	uint32_t length = 0;
	ieee80211_hdr_t *hdr = packet_info->payload;
	/* Only receive the bcast data frame from AP*/
	if ((hdr->frame_control & WLAN_FC_FROMDS) && (cmp_macaddr(hdr->addr1, bc_addr) == 0)) {
		/* For length test*/
#ifdef SMART_CONFIG_DEBUG
		{
			uint8_t src_mac[6] = {0xd0, 0xc5, 0xd3, 0xad, 0x48, 0x9f};
			if (cmp_macaddr(hdr->addr3, src_mac) == 0) {
				printf("packet length = %d\n", packet_info->length);
			}
		}
#endif
		if (hdr->frame_control & WLAN_FC_ISWEP) {
			/* Subtract the data len offset for encrypted data*/
			length = packet_info->length - DATA_LEN_OFFSET_ENCRYPT;
		} else {
			/* Subtract the data len offset for no encrypted data*/
			length = packet_info->length - DATA_LEN_OFFSET_NOENCRYPT;
		}
		if (length == smart_preamble.preamble[0]) {
			preamble_match.bit_match |= 0x1;
			memcpy(preamble_match.src_mac, hdr->addr3, 6);
			memcpy(preamble_match.bssid, hdr->addr2, 6);
			current_preamble_n ++;
			return;
		}
		if ((preamble_match.bit_match & 0x1) && (cmp_macaddr(hdr->addr3, preamble_match.src_mac) == 0)) {
#ifdef SMART_CONFIG_DEBUG
			printf("smart_preamble.preamble[%d] = %d, length = %d", current_preamble_n, smart_preamble.preamble[current_preamble_n], length);
			printf("preamble_match.bit_match = 0x%x\n", preamble_match.bit_match);
			printf("CHECK_BIT(preamble_match.bit_match, (i-1)) = %d, !CHECK_BIT(preamble_match.bit_match, i) = %d\n", 
					CHECK_BIT(preamble_match.bit_match, (current_preamble_n-1)), CHECK_BIT(preamble_match.bit_match, current_preamble_n));
#endif
			if ((smart_preamble.preamble[current_preamble_n] == length) && CHECK_BIT(preamble_match.bit_match, (current_preamble_n-1))
					&& !CHECK_BIT(preamble_match.bit_match, current_preamble_n)) {
				preamble_match.bit_match |= 1 << current_preamble_n;
				current_preamble_n ++;
			} else {
				printf("Preamble match failed,restart to match!\n");
				memset(&preamble_match, 0, sizeof(preamble_match));
				current_preamble_n = 0;
				return;
			}
			if (preamble_match.bit_match == smart_preamble.fullbit_match) {
				printf("Preamble matched, AP is %02x:%02x:%02x:%02x:%02x:%02x, Ctrl_dev is %02x:%02x:%02x:%02x:%02x:%02x\n",
						hdr->addr2[0], hdr->addr2[1], hdr->addr2[2],hdr->addr2[3], hdr->addr2[4], hdr->addr2[5], 
						hdr->addr3[0], hdr->addr3[1], hdr->addr3[2], hdr->addr3[3], hdr->addr3[4], hdr->addr3[5]);
				
				preamble_match.matched = TRUE;
				current_preamble_n = 0;
				/* stop channel change timer and set the channel to ensure we were in the config channel */
				eloop_timer_stop(ch_change_id);
				esp_wifi_set_channel(packet_info->channel, packet_info->secondary_channel);
			}
		}

	}
}

static void smart_get_ssid(sniffer_packet_into_t *packet_info) 
{
	ieee80211_hdr_t *hdr = packet_info->payload;
	uint32_t length = 0;
	smart_ssid_t *ssid = &ssid_info.ssid;

	if ((hdr->frame_control & WLAN_FC_FROMDS) && (cmp_macaddr(hdr->addr1, bc_addr) == 0) && 
			(cmp_macaddr(hdr->addr3, preamble_match.src_mac) == 0)) {
		if (hdr->frame_control & WLAN_FC_ISWEP) {
			/* Subtract the data len offset for encrypted data*/
			length = packet_info->length - DATA_LEN_OFFSET_ENCRYPT;
		} else {
			/* Subtract the data len offset for no encrypted data*/
			length = packet_info->length - DATA_LEN_OFFSET_NOENCRYPT;
		}

		if (length == smart_format.ssid_id) {
			ssid_info.recv_type = RECV_SSID;
			ssid->recv_cnt = 0;
			return;
		}
#ifdef SMART_CONFIG_DEBUG
		printf("len = %d ascii_offset = %d recv_cnt = %d\n", length, smart_format.ascii_offset, ssid->recv_cnt);
#endif
		if ((ssid_info.recv_type == RECV_SSID)) {
			if(ssid->recv_cnt == 0) {
				ssid->ssid_len = length - smart_format.ascii_offset;

#ifdef SMART_CONFIG_DEBUG
				printf("ssid len = %d\n", ssid->ssid_len);
#endif
				if ((ssid->ssid_len > SSID_LEN_MAX) || (ssid->ssid_len < 0)) {
					printf("%s: Wrong ssid len %d !\n", __func__, ssid->ssid_len);
					memset(&ssid_info.ssid, 0, sizeof(ssid_info.ssid));
					ssid_info.recv_type = RECV_NULL;
					return;
				}
				ssid->recv_cnt++;
			} else {
				if ((ssid->recv_cnt == (ssid->ssid_len + 1))) {
					uint8_t crc = cal_crc((uint8_t*)ssid->ssid, ssid->ssid_len);
					printf("ssid = %s crc = %d\n", ssid->ssid, crc);
					if (length == ((uint32_t)crc + smart_format.ascii_offset)) {
						ssid->recv_success = TRUE;
						ssid_info.recv_type = RECV_NULL;
						printf("%s: Get ssid success, ssid : %s\n", __func__, ssid->ssid);
						return;
					} else {
						printf("%s: ssid crc check failed!\n", __func__);
						memset(&ssid_info.ssid, 0, sizeof(ssid_info.ssid));
						ssid_info.recv_type = RECV_NULL;
						return;
					}
				}

				if (length < smart_format.ascii_offset) {
					printf("%s: Wrong ssid character!\n", __func__);
					memset(&ssid_info.ssid, 0, sizeof(ssid_info.ssid));
					ssid_info.recv_type = RECV_NULL;
					return;
				}
				ssid->ssid[ssid->recv_cnt - 1] = length - smart_format.ascii_offset;
				ssid->recv_cnt++;
			}
		}
	}
}

static void smart_get_passwd(sniffer_packet_into_t *packet_info) 
{
	ieee80211_hdr_t *hdr = packet_info->payload;
	uint32_t length = 0;
	passwd_t *passwd = &ssid_info.passwd;

	if ((hdr->frame_control & WLAN_FC_FROMDS) && (cmp_macaddr(hdr->addr1, bc_addr) == 0) && 
			(cmp_macaddr(hdr->addr3, preamble_match.src_mac) == 0)) {
		if (hdr->frame_control & WLAN_FC_ISWEP) {
			/* Subtract the data len offset for encrypted data*/
			length = packet_info->length - DATA_LEN_OFFSET_ENCRYPT;
		} else {
			/* Subtract the data len offset for no encrypted data*/
			length = packet_info->length - DATA_LEN_OFFSET_NOENCRYPT;
		}

		if (length == smart_format.passwd_id) {
			ssid_info.recv_type = RECV_PASSWD;
			passwd->recv_cnt = 0;
			return;
		}
		if ((ssid_info.recv_type == RECV_PASSWD)) {
			if(passwd->recv_cnt == 0) {
				passwd->passwd_len = length - smart_format.ascii_offset;
				if ((passwd->passwd_len > PASSWD_LEN_MAX) || (passwd->passwd_len < 0)) {
					printf("%s: Wrong passwd len %d !\n", __func__, passwd->passwd_len);
					memset(&ssid_info.passwd, 0, sizeof(ssid_info.passwd));
					ssid_info.recv_type = RECV_NULL;
					return;
				}
				passwd->recv_cnt++;
			} else {
				if ((passwd->recv_cnt == (passwd->passwd_len + 1))) {
					uint8_t crc = cal_crc((uint8_t*)passwd->passwd, passwd->passwd_len);
					printf("ssid = %s crc = %d\n", passwd->passwd, crc);
					if (length == ((uint32_t)crc + smart_format.ascii_offset)) {
						passwd->recv_success = TRUE;
						ssid_info.recv_type = RECV_NULL;
						printf("%s: Get passwd success, passwd : %s\n", __func__, passwd->passwd);
						return;
					} else {
						printf("%s: passwd crc check failed!\n", __func__);
						memset(&ssid_info.passwd, 0, sizeof(ssid_info.passwd));
						ssid_info.recv_type = RECV_NULL;
						return;
					}
				}

				if (length < smart_format.ascii_offset) {
					printf("%s: Wrong passwd character!\n", __func__);
					memset(&ssid_info.passwd, 0, sizeof(ssid_info.passwd));
					ssid_info.recv_type = RECV_NULL;
					return;
				}
				passwd->passwd[passwd->recv_cnt - 1] = length - smart_format.ascii_offset;
				passwd->recv_cnt++;
			}
		}
	}
}

int smart_format_set(smart_format_t *format)
{
	if (!format) {
		printf("Format is NULL!\n");
		return -1;
	}
	smart_format.ascii_offset = format->ascii_offset;
	smart_format.ssid_id = format->ssid_id;
	smart_format.passwd_id = format->passwd_id;
	return 0;
}

int smart_preamble_set(uint32_t *buf, uint8_t len)
{
	int i;
	if (!buf) {
		printf("Preamble buf is NULL!\n");
		return -1;
	}
	if (len > SMART_SNIFFER_PREAMBLE_MAX) {
		printf("Smart sniffer preamble is too long!\n");
		return -1;
	}
	memset(&smart_preamble, 0, sizeof(smart_preamble));
	memcpy(smart_preamble.preamble, buf, len * sizeof(uint32_t));
	smart_preamble.len = len;
	for (i = 0; i < len; i++) {
		smart_preamble.fullbit_match |= 1 << i;
	}
	printf("line = %d, len = %d\n", __LINE__, len);
	return 0;
}
#endif

static void wifi_sniffer_cb(void *recv_buf, wifi_promiscuous_pkt_type_t type)
{
    if (sniffer_running) {
        sniffer_packet_into_t packet_info;
        wifi_promiscuous_pkt_t *sniffer = (wifi_promiscuous_pkt_t *)recv_buf;
        /* prepare packet_info */
        packet_info.seconds = sniffer->rx_ctrl.timestamp / 1000000U;
        packet_info.microseconds = sniffer->rx_ctrl.timestamp % 1000000U;
        packet_info.length = sniffer->rx_ctrl.sig_len;
		packet_info.channel = sniffer->rx_ctrl.channel;
		packet_info.secondary_channel = sniffer->rx_ctrl.secondary_channel;
       //wifi_promiscuous_pkt_t *backup = malloc(sniffer->rx_ctrl.sig_len);
		uint8_t *backup = malloc(sizeof(ieee80211_hdr_t));
        if (backup) {
            //memcpy(backup, sniffer->payload, sniffer->rx_ctrl.sig_len);
            memcpy(backup, sniffer->payload, sizeof(ieee80211_hdr_t));
            packet_info.payload = backup;
            if (sniffer_work_queue) {
                /* send packet_info */
                if (xQueueSend(sniffer_work_queue, &packet_info, 100 / portTICK_PERIOD_MS) != pdTRUE) {
                    ESP_LOGE(TAG, "sniffer work queue full");
                }
            }
        } else {
            ESP_LOGE(TAG, "No enough memory for promiscuous packet");
        }
    }
}

static void *sniffer_task(void *parameters)
{
    sniffer_packet_into_t packet_info;
    BaseType_t ret = 0;

	pthread_detach(pthread_self());
    while (sniffer_running) {
        /* receive paclet info from queue */
		pthread_mutex_lock(sniffer_mutex);
        ret = xQueueReceive(sniffer_work_queue, &packet_info, 100 / portTICK_PERIOD_MS);
		pthread_mutex_unlock(sniffer_mutex);
        if (ret != pdTRUE) {
            continue;
        }

#ifdef SMART_IN_HAL
		if (preamble_match.matched != TRUE) {
			smart_preamble_match(&packet_info);
		} else {
			if (ssid_info.recv_type != RECV_PASSWD && !ssid_info.ssid.recv_success) {
				smart_get_ssid(&packet_info);
			}
			if (ssid_info.recv_type != RECV_SSID && !ssid_info.passwd.recv_success) {
				smart_get_passwd(&packet_info);
			}
			if (ssid_info.ssid.recv_success && ssid_info.passwd.recv_success) {
				printf("Recv ssid info success!\n");
				/* stop smart sniffer */
				if (smart_sniffer_stop()) {
					printf("Smart sniffer stop failed!\n");
				}
			}
		}
#else
		//printf("func: %s line = %d sniffer_listener = %p\n", __func__, __LINE__, sniffer_listener);
		if (sniffer_listener) {
			sniffer_listener(&packet_info);
		}
#endif

        free(packet_info.payload);
    }
	return NULL;
}

static int get_next_channel_idx(void)
{
    do {
        if (cur_chan_idx >= SMART_SNIFFER_MAX_CHANNEL_NUM) {
            cur_chan_idx = 0;
        } else {
            cur_chan_idx++;
        }
    } while (s_smart_sniffer_chan_tab[cur_chan_idx].ap_exist == false);
    return cur_chan_idx;
}

//switch channel
static void channel_change_callback(void *timer_arg)
{
    int chan_idx = 0;
	smart_sniffer_chan_t *channel;
#if 0
    if (s_sniffer_stop_flag == 1) {
        return;
    }
#endif
    chan_idx = get_next_channel_idx();
	channel = &s_smart_sniffer_chan_tab[chan_idx];
    ESP_LOGD(TAG, "ch%d-%d", channel->primary_chan,channel->second_chan);
	printf("channel %d-%d\n", channel->primary_chan,channel->second_chan);
    esp_wifi_set_channel(channel->primary_chan, channel->second_chan);
}

static void smart_sniffer_wifi_scan_ap(void)
{
    wifi_scan_config_t *scan_config = NULL;
    uint16_t ap_num = 0;
    wifi_ap_record_t *ap_record = NULL;
    scan_config = calloc(1, sizeof(wifi_scan_config_t));
    if (scan_config == NULL) {
        ESP_LOGE(TAG, "scan config allocate fail");
        return;
    }
    for (int scan_cnt = 0; scan_cnt < 2; scan_cnt++) {
        bzero(scan_config, sizeof(wifi_scan_config_t));
        scan_config->show_hidden = true;
        esp_wifi_scan_start(scan_config, true);
        esp_wifi_scan_get_ap_num(&ap_num);
        if (ap_num) {
            ap_record = calloc(1, ap_num * sizeof(wifi_ap_record_t));
            if (ap_record == NULL) {
                ESP_LOGE(TAG, "ap record allocate fail");
                continue;
            }
            esp_wifi_scan_get_ap_records(&ap_num, ap_record);
#ifdef SMART_SNIFFER_DEBUG_ON
            ESP_LOGI(TAG, "scan ap number: %d", ap_num);
            for (int i = 0; i < ap_num; i++) {
                ESP_LOGI(TAG, "scan ap: %s, "MACSTR", %u, %u, %d", ap_record[i].ssid,
                         MAC2STR(ap_record[i].bssid), ap_record[i].primary,
                         ap_record[i].second, ap_record[i].rssi);
            }
#endif
            for (int i = 0; i < SMART_SNIFFER_MAX_CHANNEL_NUM; i++) {
                if (s_smart_sniffer_chan_tab[i].ap_exist == true) {
                    continue;
                }
                for (int j = 0; j < ap_num; j++) {
                    if (ap_record[j].rssi < SMART_SNIFFER_MIN_RSSI) {
                        continue;
                    }
                    if (ap_record[j].primary == s_smart_sniffer_chan_tab[i].primary_chan) {
                        s_smart_sniffer_chan_tab[i].ap_exist = true;
                    }
                }
            }
            free(ap_record);
        }
    }

    free(scan_config);
}

int smart_sniffer_stop(void)
{
	/* Disable wifi promiscuous mode */
#ifdef SMART_CONFIG_DEBUG
	printf("func:%s, line = %d\n", __func__, __LINE__);
#endif
	esp_wifi_set_promiscuous(false);
    /* stop sniffer local task */
    sniffer_running = false;
#if 0
    /* wait for task over */
    xSemaphoreTake(sem_task_over, portMAX_DELAY);
    vSemaphoreDelete(sem_task_over);
    sem_task_over = NULL;
#endif

	pthread_mutex_lock(sniffer_mutex);
	/* make sure to free all resources in the left items */
#ifdef SMART_CONFIG_DEBUG
	printf("func:%s, line = %d\n", __func__, __LINE__);
#endif
    UBaseType_t left_items = uxQueueMessagesWaiting(sniffer_work_queue);
	printf("left_items = %d\n", left_items);
#ifdef SMART_CONFIG_DEBUG
	printf("func:%s, line = %d\n", __func__, __LINE__);
#endif
    sniffer_packet_into_t packet_info;
    while (left_items--) {
        xQueueReceive(sniffer_work_queue, &packet_info, 100 / portTICK_PERIOD_MS);
        free(packet_info.payload);
    }
    /* delete queue */
    vQueueDelete(sniffer_work_queue);
    sniffer_work_queue = NULL;
	pthread_mutex_unlock(sniffer_mutex);
	if (eloop_timer_stop(ch_change_id)) {
		printf("Stop channel change timer failed!\n");
		return -1;
	}
	eloop_timer_delete(ch_change_id);
    ESP_LOGI(TAG, "Sniffer Stopped");
    return ESP_OK;
}

int smart_sniffer_start(void)
{
	int chan_idx;
	smart_sniffer_chan_t *channel;
    int expires_ms = SMART_SNIFFER_CHANNEL_CHANGE_PERIOD;
    int period_ms = SMART_SNIFFER_CHANNEL_CHANGE_PERIOD;
    wifi_promiscuous_filter_t wifi_filter;
	pthread_t pthread_sniffer;

#ifdef SMART_IN_HAL
	memset(&preamble_match, 0, sizeof(preamble_match));
	memset(&ssid_info, 0, sizeof(ssid_info));
#endif
    /* set sniffer running status before it starts to run */
    sniffer_running = true;
    sniffer_work_queue = xQueueCreate(WIFI_SNIFFER_WORK_QUEUE_LENGTH, sizeof(sniffer_packet_into_t));
	pthread_mutex_init(sniffer_mutex, NULL);
#if 0
    sem_task_over = xSemaphoreCreateBinary();
    /* sniffer task going to run*/
    xTaskCreate(sniffer_task, "sniffer", WIFI_SNIFFER_TASK_STACK_SIZE, NULL, WIFI_SNIFFER_TASK_PRIORITY, NULL);
#endif
	pthread_create(&pthread_sniffer, NULL, sniffer_task, NULL);
	/* Scan all the channel to get ap existed channel */
	smart_sniffer_wifi_scan_ap();
	/* Set channel */
    chan_idx = get_next_channel_idx();
	channel = &s_smart_sniffer_chan_tab[chan_idx];
    ESP_LOGD(TAG, "ch%d-%d", channel->primary_chan,channel->second_chan);
    esp_wifi_set_channel(channel->primary_chan, channel->second_chan);

	/* Set Promicuous Mode */
	wifi_filter.filter_mask = WIFI_PROMIS_FILTER_MASK_DATA;
	esp_wifi_set_promiscuous_filter(&wifi_filter);
	esp_wifi_set_promiscuous_rx_cb(wifi_sniffer_cb);
	ESP_LOGI(TAG, "Start WiFi Promicuous Mode");
	ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));

	/* Create timer to change the channel */
	ch_change_id = eloop_timer_add(channel_change_callback, NULL, expires_ms, period_ms);
	if (ch_change_id == NULL) {
		printf("Add channel change timer falied!\n");
		return -1;
	}
	if (eloop_timer_start(ch_change_id)) {
		printf("Start channel change timer failed!\n");
		return -1;
	}
    return ESP_OK;
}

#ifndef SMART_IN_HAL
int smart_set_listener(sniffer_listener_t listener)
{
	if (!listener) {
		printf("listener is NULL!\n");
		return -1;
	}
	sniffer_listener = listener;
	return 0;
}
#endif

/* stop channel change timer and set the channel to ensure we were in the config channel */
int sniffer_channel_set(sniffer_packet_into_t *packet_info)
{
	if (!packet_info) {
		printf("packet info is NULL!\n");
		return -1;
	}
	eloop_timer_stop(ch_change_id);
	esp_wifi_set_channel(packet_info->channel, packet_info->secondary_channel);
	return 0;
}
