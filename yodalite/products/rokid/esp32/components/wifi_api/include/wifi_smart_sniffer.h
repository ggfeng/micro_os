#ifndef _WIFI_SMART_SNIFFER_H_
#define _WIFI_SMART_SNIFFER_H_

#ifdef SMART_IN_HAL
#define TRUE 1
#define FALSE 0
#define SMART_SNIFFER_PREAMBLE_MAX 8
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
#endif

#ifdef __CHECKER__
#define __force __attribute__((force))
#define __bitwise __attribute__((bitwise))
#else
#define __force
#define __bitwise
#endif

typedef uint16_t __bitwise be16;
typedef uint16_t __bitwise le16;
typedef uint32_t __bitwise be32;
typedef uint32_t __bitwise le32;
typedef uint64_t __bitwise be64;
typedef uint64_t __bitwise le64;

typedef struct ieee80211_hdr {
	le16 frame_control;
	le16 duration_id;
	uint8_t addr1[6];
	uint8_t addr2[6];
	uint8_t addr3[6];
	le16 seq_ctrl;
	/* followed by 'u8 addr4[6];' if ToDS and FromDS is set in data frame
	 */
} ieee80211_hdr_t;

int smart_sniffer_start(void);
int smart_sniffer_stop(void);
#ifdef SMART_IN_HAL
int smart_format_set(smart_format_t *format);
int smart_preamble_set(uint32_t *buf, uint8_t len);
#else
int smart_set_listener(sniffer_listener_t listener);
int sniffer_channel_set(sniffer_packet_into_t *packet_info);
#endif

#endif
