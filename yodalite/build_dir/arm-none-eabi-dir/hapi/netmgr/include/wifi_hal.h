#ifndef _WIFI_HAL_
#define _WIFI_HAL_
#include <hardware/wifi_hal.h>

extern int wifi_hal_init(void);
extern void wifi_hal_exit(void);
extern int wifi_hal_get_capacity(void);
extern int wifi_hal_start_station(void);
extern void wifi_hal_stop_station(void);
extern int wifi_hal_start_ap(char *ssid,char *psk,char *ip);
extern void wifi_hal_stop_ap(void);

extern int wifi_hal_connect_station(char *ssid,char * passwd);
extern int wifi_hal_disconnect_station(void);

//extern int wifi_get_status(void);
extern int wifi_hal_get_scan_result(ssid_t** list,int*num);
extern int wifi_hal_clean_scan_result(ssid_t *list);
extern int wifi_hal_start_scan(void);
extern int wifi_hal_stop_scan(void);
extern int wifi_get_status(wifi_hal_status_t *wifi_status);

#endif
