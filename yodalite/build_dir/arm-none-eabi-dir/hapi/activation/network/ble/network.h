#ifndef _C_NETWORK_H_
#define _C_NETWORK_H_

#include <common.h>

int network_int(void);
void network_ext(void);
int change_to_sta_mode(char *qlink_ssid,char *qlink_psk);
int change_to_ble_mode();
int get_network_status();
#endif