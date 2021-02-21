#ifndef _NETWORK_MODE_H_
#define _NETWORK_MODE_H_

#define NETWORK_WIFI_MODE_KEY     ("persist.netmanager.wifi")
#define NETWORK_WIFI_AP_MODE_KEY  ("persist.netmanager.wifi.ap")

#define NETWORK_WIFI_AP_SSID      ("persist.netmanager.ap.ssid")
#define NETWORK_WIFI_AP_PSK       ("persist.netmanager.ap.psk")
#define NETWORK_WIFI_AP_HOSTIP    ("persist.netmanager.ap.hostip")

#define NETWORK_WIFI_DEFAULT_SSID   ("ROKID.TC")
#define NETWORK_WIFI_DEFAULT_PSK    ("rokidguys")

#define NETWORK_AP_DEFAULT_SSID   ("yodalite")
#define NETWORK_AP_DEFAULT_PSK    ("")
#define NETWORK_AP_DEFAULT_IP     ("192.168.2.1")

#define NETWORK_WIFI_STATION_SSID      ("persist.netmanager.station.ssid")
#define NETWORK_WIFI_STATION_PSK       ("persist.netmanager.station.psk")

extern int network_set_initmode(void);
extern uint32_t get_network_mode(void);

extern int wifi_init_start(void);
extern int network_set_mode(uint32_t mode,uint32_t start);


#endif
