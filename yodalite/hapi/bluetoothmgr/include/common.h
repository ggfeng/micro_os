#ifndef __BT_SERVICE_COMMON_H_
#define __BT_SERVICE_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <lib/property/properties.h>
#include <hardware/bt/bluetooth.h>
#include <lib/eloop/eloop.h>
#include <lib/cjson/cJSON.h>
#include <hapi/flora_agent.h>
#include <yodalite_autoconf.h>

#include <osal/pthread.h>
#include <osal/signal.h>
#include <osal/errno.h>
#include <osal/time.h>

#include "config.h"
#include "ble.h"
#include "a2dpsink.h"
#include "hfp.h"
#include "a2dpsource.h"
#include "ipc.h"
#include "turen.h"


#ifdef CONFIG_MEM_EXT_ENABLE
#include <lib/mem_ext/mem_ext.h>
#define yodalite_malloc   malloc_ext
#define yodalite_free     free_ext
#define yodalite_calloc   calloc_ext
#define yodalite_realloc  realloc_ext
#else
#define yodalite_malloc   malloc
#define yodalite_free     free
#define yodalite_calloc   calloc
#define yodalite_realloc  realloc
#endif


#define BLE_STATUS_MASK     (1 << 0)
#define BLE_CLIENT_STATUS_MASK     (1 << 1)
#define A2DP_SOURCE_STATUS_MASK     (1 << 2)
#define A2DP_SINK_STATUS_MASK   (1 << 3)
#define HFP_STATUS_MASK     (1 << 4)
#define HID_HOST_STATUS_MASK     (1 << 5)
#define HID_DEVICE_STATUS_MASK     (1 << 6)

enum {
    BLUETOOTH_FUNCTION_COMMON,
    BLUETOOTH_FUNCTION_BLE,
    BLUETOOTH_FUNCTION_A2DPSINK,
    BLUETOOTH_FUNCTION_A2DPSOURCE,
    BLUETOOTH_FUNCTION_HFP,
} BLUETOOTH_FUNCTION;

extern struct bt_service_handle *g_bt_handle;

struct bt_service_handle {
    int status;
    uint8_t open;
    int discovery_is_completed;
    struct rk_bluetooth *bt;
    struct ble_handle bt_ble;
    struct a2dpsink_handle bt_a2dpsink;
    struct a2dpsource_handle bt_a2dpsource;
#if defined(BT_SERVICE_HAVE_HFP)
    struct hfp_handle bt_hfp;
#endif
};

extern int bt_open(struct bt_service_handle *handle, const char *name);
extern int bt_close(struct bt_service_handle *handle);

extern void broadcast_remove_dev(const char* addr, int status);

extern int handle_common_handle(cJSON*obj, struct bt_service_handle *bt, void *reply);
extern int handle_ble_handle(cJSON*obj, struct bt_service_handle *bt, void *reply);
extern int handle_a2dpsink_handle(cJSON*obj, struct bt_service_handle *bt, void *reply);
extern int handle_a2dpsource_handle(cJSON*obj, struct bt_service_handle *bt, void *reply);
#if defined(BT_SERVICE_HAVE_HFP)
extern int handle_hfp_handle(cJSON*obj, struct bt_service_handle *handle, void *reply);
#endif

extern int handle_module_sleep(cJSON*obj, struct bt_service_handle *handle);

extern int handle_power_sleep(cJSON*obj, struct bt_service_handle *handle);

#ifdef __cplusplus
}
#endif
#endif
