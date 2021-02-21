#ifndef _C_NETWORK_HANDLER_H_
#define _C_NETWORK_HANDLER_H_


#define BLE_ON 1
#define BLE_OFF 2
#define BLE_DATA 3
#define BLE_STATE 4


typedef void (*ble_conn_func)(unsigned char* event);
typedef void (*ble_data_func)(unsigned char* data);

void ble_enable(ble_conn_func ble_conn,ble_data_func ble_get_data);

void ble_disable();
void ble_close();

void send_wifi_status(char* code, char* msg);
void send_sn(char* deviceid);

#endif