#ifndef _BLUETOOTH_COMMON_H_
#define _BLUETOOTH_COMMON_H_
#include "ble_server.h"
#include "rk_a2dp_sink.h"
#include "hardware/bt/bluetooth.h"
extern struct rk_bluetooth rk_bt_dev;

typedef struct RKBluetooth
{
    BleServer_t       *ble_ctx;
    A2dpSink        *a2dp_sink;
#if 0
    A2dpCtx         *a2dp_ctx;
    HFP_HS           *hs_ctx;
#endif
    pthread_mutex_t bt_mutex;
} RKBluetooth_t;
#endif
