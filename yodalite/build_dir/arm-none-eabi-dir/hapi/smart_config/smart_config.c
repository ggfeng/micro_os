/*
 * =====================================================================================
 *
 *       Filename:  smart_config.c
 *
 *    Description:  :
 *
 *        Version:  1.0
 *        Created:  07/16/2019 04:39:30 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  guoliang.hou (Rokid), 
 *   Organization:  Rokid
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sconfig.h"
#include "smart_config.h"

//#define USE_CRC_TABLE
//#define SMART_CONFIG_DEBUG

static uint8_t current_preamble_n = 0; /* Current preamble sequence number*/
static uint8_t bc_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

#ifdef USE_CRC_TABLE
static const uint8_t crc_table[] =
{
    0x00,0x31,0x62,0x53,0xc4,0xf5,0xa6,0x97,0xb9,0x88,0xdb,0xea,0x7d,0x4c,0x1f,0x2e,
    0x43,0x72,0x21,0x10,0x87,0xb6,0xe5,0xd4,0xfa,0xcb,0x98,0xa9,0x3e,0x0f,0x5c,0x6d,
    0x86,0xb7,0xe4,0xd5,0x42,0x73,0x20,0x11,0x3f,0x0e,0x5d,0x6c,0xfb,0xca,0x99,0xa8,
    0xc5,0xf4,0xa7,0x96,0x01,0x30,0x63,0x52,0x7c,0x4d,0x1e,0x2f,0xb8,0x89,0xda,0xeb,
    0x3d,0x0c,0x5f,0x6e,0xf9,0xc8,0x9b,0xaa,0x84,0xb5,0xe6,0xd7,0x40,0x71,0x22,0x13,
    0x7e,0x4f,0x1c,0x2d,0xba,0x8b,0xd8,0xe9,0xc7,0xf6,0xa5,0x94,0x03,0x32,0x61,0x50,
    0xbb,0x8a,0xd9,0xe8,0x7f,0x4e,0x1d,0x2c,0x02,0x33,0x60,0x51,0xc6,0xf7,0xa4,0x95,
    0xf8,0xc9,0x9a,0xab,0x3c,0x0d,0x5e,0x6f,0x41,0x70,0x23,0x12,0x85,0xb4,0xe7,0xd6,
    0x7a,0x4b,0x18,0x29,0xbe,0x8f,0xdc,0xed,0xc3,0xf2,0xa1,0x90,0x07,0x36,0x65,0x54,
    0x39,0x08,0x5b,0x6a,0xfd,0xcc,0x9f,0xae,0x80,0xb1,0xe2,0xd3,0x44,0x75,0x26,0x17,
    0xfc,0xcd,0x9e,0xaf,0x38,0x09,0x5a,0x6b,0x45,0x74,0x27,0x16,0x81,0xb0,0xe3,0xd2,
    0xbf,0x8e,0xdd,0xec,0x7b,0x4a,0x19,0x28,0x06,0x37,0x64,0x55,0xc2,0xf3,0xa0,0x91,
    0x47,0x76,0x25,0x14,0x83,0xb2,0xe1,0xd0,0xfe,0xcf,0x9c,0xad,0x3a,0x0b,0x58,0x69,
    0x04,0x35,0x66,0x57,0xc0,0xf1,0xa2,0x93,0xbd,0x8c,0xdf,0xee,0x79,0x48,0x1b,0x2a,
    0xc1,0xf0,0xa3,0x92,0x05,0x34,0x67,0x56,0x78,0x49,0x1a,0x2b,0xbc,0x8d,0xde,0xef,
    0x82,0xb3,0xe0,0xd1,0x46,0x77,0x24,0x15,0x3b,0x0a,0x59,0x68,0xff,0xce,0x9d,0xac
};

static uint8_t cal_crc_table(uint8_t *ptr, uint8_t len) 
{
    uint8_t  crc = 0x00;

    while (len--)
    {
        crc = crc_table[crc ^ *ptr++];
    }
    return (crc);
}
#else
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
#endif

smart_config_t* smart_config_create(void)
{
	smart_config_t *smart_config = NULL;
	smart_config = (smart_config_t*)malloc(sizeof(smart_config_t));
	if (!smart_config) {
		printf("Malloc smart_config failed!\n");
		return NULL;
	}
	memset(smart_config, 0, sizeof(smart_config_t));
	return smart_config;
}

void smart_config_destroy(smart_config_t *smart_config)
{
	if (smart_config) {
		free(smart_config);
		smart_config = NULL;
	}
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

void smart_preamble_match(sniffer_packet_into_t *packet_info, smart_config_t *smart_config)
{
	uint32_t length = 0;
	smart_format_t *smart_format = NULL;
	smart_preamble_t *smart_preamble = NULL;
	preamble_match_t *preamble_match = NULL;
	ieee80211_hdr_t *hdr = NULL;

	if (!packet_info) {
		printf("packet_info is NULL!\n");
		return;
	}
	if (!smart_config) {
		printf("smart_config is NULL!\n");
		return;
	}
	smart_format = &smart_config->smart_format;
	smart_preamble = &smart_config->smart_preamble;
	preamble_match = &smart_config->preamble_match;
	hdr = packet_info->payload;

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
			length = packet_info->length - smart_format->data_len_offset_encrypt;
		} else {
			/* Subtract the data len offset for no encrypted data*/
			length = packet_info->length - smart_format->data_len_offset_noencrypt;
		}
		if (length == smart_preamble->preamble[0]) {
			preamble_match->bit_match |= 0x1;
			memcpy(preamble_match->src_mac, hdr->addr3, 6);
			memcpy(preamble_match->bssid, hdr->addr2, 6);
			current_preamble_n ++;
			return;
		}
		if ((preamble_match->bit_match & 0x1) && (cmp_macaddr(hdr->addr3, preamble_match->src_mac) == 0)) {
#ifdef SMART_CONFIG_DEBUG
			printf("smart_preamble->preamble[%d] = %d, length = %d", current_preamble_n, smart_preamble->preamble[current_preamble_n], length);
			printf("preamble_match->bit_match = 0x%x\n", preamble_match->bit_match);
			printf("CHECK_BIT(preamble_match->bit_match, (i-1)) = %d, !CHECK_BIT(preamble_match->bit_match, i) = %d\n", 
					CHECK_BIT(preamble_match->bit_match, (current_preamble_n-1)), CHECK_BIT(preamble_match->bit_match, current_preamble_n));
#endif
			if ((smart_preamble->preamble[current_preamble_n] == length) && CHECK_BIT(preamble_match->bit_match, (current_preamble_n-1))
					&& !CHECK_BIT(preamble_match->bit_match, current_preamble_n)) {
				preamble_match->bit_match |= 1 << current_preamble_n;
				current_preamble_n ++;
			} else {
				printf("Preamble match failed,restart to match!\n");
				memset(preamble_match, 0, sizeof(preamble_match_t));
				current_preamble_n = 0;
				return;
			}
			if (preamble_match->bit_match == smart_preamble->fullbit_match) {
				printf("Preamble matched, AP is %02x:%02x:%02x:%02x:%02x:%02x, Ctrl_dev is %02x:%02x:%02x:%02x:%02x:%02x\n",
						hdr->addr2[0], hdr->addr2[1], hdr->addr2[2],hdr->addr2[3], hdr->addr2[4], hdr->addr2[5], 
						hdr->addr3[0], hdr->addr3[1], hdr->addr3[2], hdr->addr3[3], hdr->addr3[4], hdr->addr3[5]);
				
				preamble_match->matched = TRUE;
				current_preamble_n = 0;
			}
		}

	}
}

void smart_get_ssid(sniffer_packet_into_t *packet_info, smart_config_t *smart_config) 
{
	smart_format_t *smart_format = NULL;
	preamble_match_t *preamble_match = NULL;
	ieee80211_hdr_t *hdr = NULL;
	uint32_t length = 0;
	ssid_info_t *ssid_info = NULL;
	smart_ssid_t *ssid = NULL;

	if (!packet_info) {
		printf("packet_info is NULL!\n");
		return;
	}
	if (!smart_config) {
		printf("smart_config is NULL!\n");
		return;
	}
	smart_format = &smart_config->smart_format;
	preamble_match = &smart_config->preamble_match;
	hdr = packet_info->payload;
	ssid_info = &smart_config->ssid_info;
	ssid = &ssid_info->ssid;
	if ((hdr->frame_control & WLAN_FC_FROMDS) && (cmp_macaddr(hdr->addr1, bc_addr) == 0) && 
			(cmp_macaddr(hdr->addr3, preamble_match->src_mac) == 0)) {
		if (hdr->frame_control & WLAN_FC_ISWEP) {
			/* Subtract the data len offset for encrypted data*/
			length = packet_info->length - smart_format->data_len_offset_encrypt;
		} else {
			/* Subtract the data len offset for no encrypted data*/
			length = packet_info->length - smart_format->data_len_offset_noencrypt;
		}

		if (length == smart_format->ssid_id) {
			ssid_info->recv_type = RECV_SSID;
			ssid->recv_cnt = 0;
			return;
		}
#ifdef SMART_CONFIG_DEBUG
		printf("len = %d ascii_offset = %d recv_cnt = %d\n", length, smart_format->ascii_offset, ssid->recv_cnt);
#endif
		if ((ssid_info->recv_type == RECV_SSID)) {
			if(ssid->recv_cnt == 0) {
				ssid->ssid_len = length - smart_format->ascii_offset;

#ifdef SMART_CONFIG_DEBUG
				printf("ssid len = %d\n", ssid->ssid_len);
#endif
				if ((ssid->ssid_len > SSID_LEN_MAX) || (ssid->ssid_len < 0)) {
					printf("%s: Wrong ssid len %d !\n", __func__, ssid->ssid_len);
					memset(ssid, 0, sizeof(smart_ssid_t));
					ssid_info->recv_type = RECV_NULL;
					return;
				}
				ssid->recv_cnt++;
			} else {
				if ((ssid->recv_cnt == (ssid->ssid_len + 1))) {
#ifdef USE_CRC_TABLE
					uint8_t crc = cal_crc_table(ssid->ssid, ssid->ssid_len);
#else
					uint8_t crc = cal_crc(ssid->ssid, ssid->ssid_len);
#endif
					printf("ssid = %s crc = %d, length = %d\n", ssid->ssid, crc, length);
					if (length == ((uint32_t)crc + smart_format->ascii_offset)) {
						ssid->recv_success = TRUE;
						ssid_info->recv_type = RECV_NULL;
						printf("%s: Get ssid success, ssid : %s\n", __func__, ssid->ssid);
						return;
					} else {
						printf("%s: ssid crc check failed!\n", __func__);
						memset(ssid, 0, sizeof(smart_ssid_t));
						ssid_info->recv_type = RECV_NULL;
						return;
					}
				}

				if (length < smart_format->ascii_offset) {
					printf("%s: Wrong ssid character!\n", __func__);
					memset(ssid, 0, sizeof(smart_ssid_t));
					ssid_info->recv_type = RECV_NULL;
					return;
				}
				ssid->ssid[ssid->recv_cnt - 1] = length - smart_format->ascii_offset;
				ssid->recv_cnt++;
			}
		}
	}
}

void smart_get_passwd(sniffer_packet_into_t *packet_info, smart_config_t *smart_config) 
{
	smart_format_t *smart_format = NULL;
	preamble_match_t *preamble_match = NULL;
	ieee80211_hdr_t *hdr = NULL;
	uint32_t length = 0;
	ssid_info_t *ssid_info = NULL;
	passwd_t *passwd = NULL;

	if (!packet_info) {
		printf("packet_info is NULL!\n");
		return;
	}
	if (!smart_config) {
		printf("smart_config is NULL!\n");
		return;
	}
	smart_format = &smart_config->smart_format;
	preamble_match = &smart_config->preamble_match;
	hdr = packet_info->payload;
	ssid_info = &smart_config->ssid_info;
	passwd = &ssid_info->passwd;
	if ((hdr->frame_control & WLAN_FC_FROMDS) && (cmp_macaddr(hdr->addr1, bc_addr) == 0) && 
			(cmp_macaddr(hdr->addr3, preamble_match->src_mac) == 0)) {
		if (hdr->frame_control & WLAN_FC_ISWEP) {
			/* Subtract the data len offset for encrypted data*/
			length = packet_info->length - smart_format->data_len_offset_encrypt;
		} else {
			/* Subtract the data len offset for no encrypted data*/
			length = packet_info->length - smart_format->data_len_offset_noencrypt;
		}

		if (length == smart_format->passwd_id) {
			ssid_info->recv_type = RECV_PASSWD;
			memset(passwd, 0, sizeof(passwd_t));
			passwd->recv_cnt = 0;
			return;
		}
		if ((ssid_info->recv_type == RECV_PASSWD)) {
			if(passwd->recv_cnt == 0) {
				passwd->passwd_len = length - smart_format->ascii_offset;
				if ((passwd->passwd_len > PASSWD_LEN_MAX) || (passwd->passwd_len < 0)) {
					printf("%s: Wrong passwd len %d !\n", __func__, passwd->passwd_len);
					memset(passwd, 0, sizeof(passwd_t));
					ssid_info->recv_type = RECV_NULL;
					return;
				}
				passwd->recv_cnt++;
			} else {
				if ((passwd->recv_cnt == (passwd->passwd_len + 1))) {
					/*CRC8 check*/
#ifdef USE_CRC_TABLE
					uint8_t crc = cal_crc_table(passwd->passwd, passwd->passwd_len);
#else
					uint8_t crc = cal_crc(passwd->passwd, passwd->passwd_len);
#endif
					printf("passwd = %s crc = %d length = %d\n", passwd->passwd, crc, length);
					if (length == ((uint32_t)crc + smart_format->ascii_offset)) {
						passwd->recv_success = TRUE;
						ssid_info->recv_type = RECV_NULL;
						printf("%s: Get passwd success, passwd : %s\n", __func__, passwd->passwd);
						return;
					} else {
						printf("%s: passwd crc check failed!\n", __func__);
						memset(passwd, 0, sizeof(passwd_t));
						ssid_info->recv_type = RECV_NULL;
						return;
					}
				}

				if (length < smart_format->ascii_offset) {
					printf("%s: Wrong passwd character!\n", __func__);
					memset(passwd, 0, sizeof(passwd_t));
					ssid_info->recv_type = RECV_NULL;
					return;
				}
				passwd->passwd[passwd->recv_cnt - 1] = length - smart_format->ascii_offset;
				passwd->recv_cnt++;
			}
		}
	}
}

int smart_format_set(smart_format_t *format, smart_config_t *smart_config)
{
	smart_format_t *smart_format = NULL;

	if (!format) {
		printf("Format is NULL!\n");
		return -1;
	}
	if (!smart_config) {
		printf("smart_config is not created!\n");
		return -1;
	}
	smart_format = &smart_config->smart_format;

	smart_format->ascii_offset = format->ascii_offset;
	smart_format->ssid_id = format->ssid_id;
	smart_format->passwd_id = format->passwd_id;
	smart_format->data_len_offset_encrypt = format->data_len_offset_encrypt;
	smart_format->data_len_offset_noencrypt = format->data_len_offset_noencrypt;

	return 0;
}

int smart_preamble_set(smart_config_t *smart_config, uint32_t *buf, uint8_t len)
{
	int i;
	smart_preamble_t *smart_preamble = NULL;
	if (!buf) {
		printf("Preamble buf is NULL!\n");
		return -1;
	}
	if (!smart_config) {
		printf("smart_config is not created!\n");
		return -1;
	}

	if (len > SMART_SNIFFER_PREAMBLE_MAX) {
		printf("Smart sniffer preamble is too long!\n");
		return -1;
	}
	smart_preamble = &smart_config->smart_preamble;
	memset(smart_preamble, 0, sizeof(smart_preamble_t));
	memcpy(smart_preamble->preamble, buf, len * sizeof(uint32_t));
	smart_preamble->len = len;
	for (i = 0; i < len; i++) {
		smart_preamble->fullbit_match |= 1 << i;
	}
	return 0;
}

int smart_analize_packets(sniffer_packet_into_t *packet_info, smart_config_t *smart_config, sniffer_chan_set_t cb, sconfig_ssid_t *s_ssid)
{
	preamble_match_t *preamble_match;
	ssid_info_t *ssid_info;

	if (!packet_info) {
		printf("packet_info is NULL!\n");
		return -1;
	}
	if (!smart_config) {
		printf("smart_config is NULL!\n");
		return -1;
	}

	if (!s_ssid) {
		printf("sconfig ssid is NULL, please check it!\n");
		return -1;
	}
	ssid_info = &smart_config->ssid_info;
	preamble_match = &smart_config->preamble_match;
//	printf("func = %s line = %d preamble_match = %d\n", __func__, __LINE__, preamble_match->matched);
	if (preamble_match->matched != TRUE) {
		smart_preamble_match(packet_info, smart_config);
		if (preamble_match->matched == TRUE) {
			/* stop channel change timer and set the channel to ensure we were in the config channel */
			if (cb) {
				cb(packet_info);
			}
			return SMART_PREAMBLE_MATCH;
		}
	} else {
		if (ssid_info->recv_type != RECV_PASSWD && !ssid_info->ssid.recv_success) {
			smart_get_ssid(packet_info, smart_config);
		}
		if (ssid_info->recv_type != RECV_SSID && !ssid_info->passwd.recv_success) {
			smart_get_passwd(packet_info, smart_config);
		}
		if (ssid_info->ssid.recv_success && ssid_info->passwd.recv_success) {
			printf("Recv ssid info success!\n");
			strcpy(s_ssid->ssid, ssid_info->ssid.ssid);
			strcpy(s_ssid->passwd, ssid_info->passwd.passwd);

			memset(preamble_match, 0, sizeof(preamble_match_t));
			memset(ssid_info, 0, sizeof(ssid_info_t));
			return SMART_ANALIZE_SUCCESS;
		}
	}
	return -1;
}


