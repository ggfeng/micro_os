#ifndef _SMART_CONFIG_H_
#define _SMART_CONFIG_H_
#include <stdint.h>
#include "yodalite_autoconf.h"

#define WLAN_FC_TODS		0x0100
#define WLAN_FC_FROMDS		0x0200
#define WLAN_FC_ISWEP		0x4000
#ifndef bool
#define bool uint8_t
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define SMART_SNIFFER_PREAMBLE_MAX 8

#define CHECK_BIT(a, bit) ((a) & (1 << (bit)))


#define SMART_PREAMBLE_MATCH 1
#define SMART_ANALIZE_SUCCESS 2

typedef struct {
    void *payload;
    uint32_t length;
    uint32_t seconds;
    uint32_t microseconds;
	uint32_t channel;
	uint32_t secondary_channel;
	void *info;
} sniffer_packet_into_t;

typedef int (*sniffer_channel_set_t)(sniffer_packet_into_t *packet_info);

typedef struct {
	uint32_t preamble[SMART_SNIFFER_PREAMBLE_MAX];
	uint8_t len;
	uint8_t fullbit_match;
} smart_preamble_t;

typedef enum {
	RECV_NULL = 0,
	RECV_SSID,
	RECV_PASSWD,
} recv_type_t;

#define SSID_LEN_MAX 32
#define PASSWD_LEN_MAX 63

typedef struct {
	char ssid[SSID_LEN_MAX + 1];
	int32_t ssid_len;
	uint8_t recv_cnt;
	bool recv_success;
} smart_ssid_t;

typedef struct {
	char passwd[PASSWD_LEN_MAX + 1];
	int32_t passwd_len;
	uint8_t recv_cnt;
	bool recv_success;
} passwd_t;

typedef struct {
	uint8_t bssid[6];
	uint8_t src_mac[6];
	smart_ssid_t ssid;
	passwd_t passwd;
	recv_type_t  recv_type;
} ssid_info_t;

typedef struct {
	uint8_t bssid[6];
	uint8_t src_mac[6];  //The control device macaddr
	uint8_t bit_match;
	bool matched;
} preamble_match_t;

typedef struct {
	int32_t ascii_offset;
	int32_t data_len_offset_noencrypt;
	int32_t data_len_offset_encrypt;
	int32_t ssid_id;
	int32_t passwd_id;
} smart_format_t;

typedef struct {
	smart_preamble_t smart_preamble;
	preamble_match_t preamble_match;
	smart_format_t smart_format;
	ssid_info_t ssid_info;
} smart_config_t;

typedef struct {
	char ssid[SSID_LEN_MAX + 1];
	char passwd[PASSWD_LEN_MAX + 1];
} sconfig_ssid_t;

typedef int (*sniffer_chan_set_t)(sniffer_packet_into_t *packet_info);

smart_config_t* smart_config_create(void);
void smart_config_destroy(smart_config_t *smart_config);
int smart_format_set(smart_format_t *format, smart_config_t *smart_config);
int smart_preamble_set(smart_config_t *smart_config, uint32_t *buf, uint8_t len);
void smart_preamble_match(sniffer_packet_into_t *packet_info, smart_config_t *smart_config);
void smart_get_ssid(sniffer_packet_into_t *packet_info, smart_config_t *smart_config);
void smart_get_passwd(sniffer_packet_into_t *packet_info, smart_config_t *smart_config);
int smart_analize_packets(sniffer_packet_into_t *packet_info, smart_config_t *smart_config, sniffer_chan_set_t cb, sconfig_ssid_t *s_ssid);

#endif

