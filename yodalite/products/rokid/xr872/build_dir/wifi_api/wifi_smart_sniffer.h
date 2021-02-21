#ifndef _WIFI_SMART_SNIFFER_H_
#define _WIFI_SMART_SNIFFER_H_

#define LOG_ERROR 0
#define LOG_DEBUG 1
#define LOG_INFO 2

#define SMART_SNIFFER_DBG(level, fmt, args...)                  \
	do {                                            \
		if (level <= debug_level)              \
			printf("[SMART_SNIFFER]"fmt,##args);       \
	} while (0)

int smart_sniffer_start(void);
int smart_sniffer_stop(void);
int smart_set_listener(sniffer_listener_t listener);
int sniffer_channel_set(sniffer_packet_into_t *packet_info);

#endif

