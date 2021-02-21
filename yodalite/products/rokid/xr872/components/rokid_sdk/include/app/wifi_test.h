#ifndef _WIFI_TEST_H_
#define _WIFI_TEST_H_

extern int cmd_wifi_init(void);
extern int cmd_wifi_deinit(void);

extern int cmd_wifi_ap_start(void);
extern int cmd_wifi_ap_stop(void);
extern int cmd_wifi_sta_connect_ap(void);
extern int cmd_wifi_sta_disconnect(void);
extern int cmd_wifi_get_status(void);
extern int cmd_wifi_scan_start(void);
extern int cmd_wifi_scan_stop(void);
extern int cmd_wifi_get_scan_list(void);

extern int cmd_wifi_station_start(void);
extern int cmd_wifi_station_stop(void);
extern int cmd_smart_preamble_set(void);
extern int cmd_smart_format_set(void);
extern int cmd_smart_start(void);
extern int cmd_smart_stop(void);

#endif
