#ifndef _BLE_SERVER_H_
#define _BLE_SERVER_H_
#include "hardware/bt/bt_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#define TRUE 1
#define FALSE 0
/* Attributes State Machine */
enum
{
    IDX_SVC,
    IDX_CHAR_A,
    IDX_CHAR_VAL_A,
    IDX_CHAR_CFG_A,

    IDX_CHAR_B,
    IDX_CHAR_VAL_B,
    IDX_CHAR_CFG_B,

    HRS_IDX_NB,
};

enum
{
    GATTC_ATTR_TYPE_INCL_SRVC,
    GATTC_ATTR_TYPE_CHAR,
    GATTC_ATTR_TYPE_CHAR_DESCR,
    GATTC_ATTR_TYPE_SRVC
};
typedef unsigned char BOOLEAN;

#define BLE_ATTRIBUTE_MAX 5
#define BLE_SERVER_MAX 2

/* Maximum UUID size - 16 bytes, and structure to hold any type of UUID. */
#define MAX_UUID_SIZE              16
typedef struct
{
#define LEN_UUID_16     2
#define LEN_UUID_32     4
#define LEN_UUID_128    16

	uint16_t          len;

    union
    {
        uint16_t      uuid16;
        uint32_t      uuid32;
        uint8_t       uuid128[MAX_UUID_SIZE];
    } uu;

} bt_uuid_t;

typedef struct
{
    bt_uuid_t      uuid;
    uint16_t         service_id;
    uint16_t         attr_id;
    uint8_t          attr_type;
    uint8_t          prop;
    BOOLEAN        is_pri;
    BOOLEAN        wait_flag;
} ble_attr_t;

typedef struct
{
    BOOLEAN             enabled;
    esp_gatt_if_t       server_if;
    uint16_t            conn_id;
    ble_attr_t			attr[BLE_ATTRIBUTE_MAX];
} ble_server_t;

typedef struct BleServer
{
    BOOLEAN             enabled;

    ble_server_t ble_server[BLE_SERVER_MAX];

    void *listener_handle;
    ble_callbacks_t listener;
    void *caller;
} BleServer_t;

BleServer_t *ble_create(void *caller);
int ble_destroy(BleServer_t *bles);
int rokid_ble_enable(BleServer_t *bles);
int rokid_ble_disable(BleServer_t *bles);
int ble_server_set_listener(BleServer_t *bles, ble_callbacks_t listener, void *listener_handle);
int rk_ble_send(BleServer_t *bles, uint16_t uuid, uint8_t *buf, int len);
int rk_ble_set_visibility(int discoverable);
int rk_ble_set_name(const char *name);
#endif

