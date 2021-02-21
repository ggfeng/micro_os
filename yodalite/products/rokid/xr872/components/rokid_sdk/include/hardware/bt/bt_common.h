#ifndef __BT_COMMON_H__
#define __BT_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
//#include <pthread.h>

#include "bt_type.h"
#include "bt_log.h"
#include "bt_avrc.h"
#include "bt_ble.h"
#include "bt_a2dp.h"
#include "bt_a2dp_sink.h"
#include "bt_hfp.h"


#define BT_DISC_NB_DEVICES 30
#define BT_DEFAULT_UIPC_PATH "/var/run/bluetooth/"
/* Default local Name */
#define DEFAULT_BT_NAME             "rokid-bt-test"

/** Bluetooth Error Status */
/** We need to build on this */

typedef enum {
    BT_STATUS_SUCCESS = 0,
    BT_STATUS_FAIL,
    BT_STATUS_NOT_READY,
    BT_STATUS_NOMEM,
    BT_STATUS_BUSY,
    BT_STATUS_DONE,        /* request already completed */
    BT_STATUS_UNSUPPORTED,
    BT_STATUS_PARM_INVALID,
    BT_STATUS_UNHANDLED,
    BT_STATUS_AUTH_FAILURE,
    BT_STATUS_RMT_DEV_DOWN,
    BT_STATUS_AUTH_REJECTED,
    BT_STATUS_JNI_ENVIRONMENT_ERROR,
    BT_STATUS_JNI_THREAD_ATTACH_ERROR,
    BT_STATUS_WAKELOCK_ERROR
} bt_status_t;

#define BT_CHECK_HANDLE(handle) \
    do {                                                                \
        if (BT_UNLIKELY(!(handle))) {                                     \
            return -BT_STATUS_FAIL;\
        }                                                               \
    } while (false)

/* Bluetooth Adapter and Remote Device property types */
typedef enum {
  /* Properties common to both adapter and remote device */
  /**
   * Description - Bluetooth Device Name
   * Access mode - Adapter name can be GET/SET. Remote device can be GET
   * Data type   - bt_bdname_t
   */
  BT_PROPERTY_BDNAME = 0x1,
  /**
   * Description - Bluetooth Device Address
   * Access mode - Only GET.
   * Data type   - RawAddress
   */
  BT_PROPERTY_BDADDR,
  /**
   * Description - Bluetooth Service 128-bit UUIDs
   * Access mode - Only GET.
   * Data type   - Array of bluetooth::Uuid (Array size inferred from property
   *               length).
   */
  BT_PROPERTY_UUIDS,
  /**
   * Description - Bluetooth Class of Device as found in Assigned Numbers
   * Access mode - Only GET.
   * Data type   - uint32_t.
   */
  BT_PROPERTY_CLASS_OF_DEVICE,
  /**
   * Description - Device Type - BREDR, BLE or DUAL Mode
   * Access mode - Only GET.
   * Data type   - bt_device_type_t
   */
  BT_PROPERTY_TYPE_OF_DEVICE,
  /**
   * Description - Bluetooth Service Record
   * Access mode - Only GET.
   * Data type   - bt_service_record_t
   */
  BT_PROPERTY_SERVICE_RECORD,

  /* Properties unique to adapter */
  /**
   * Description - Bluetooth Adapter scan mode
   * Access mode - GET and SET
   * Data type   - bt_scan_mode_t.
   */
  BT_PROPERTY_ADAPTER_SCAN_MODE,
  /**
   * Description - List of bonded devices
   * Access mode - Only GET.
   * Data type   - Array of RawAddress of the bonded remote devices
   *               (Array size inferred from property length).
   */
  BT_PROPERTY_ADAPTER_BONDED_DEVICES,
  /**
   * Description - Bluetooth Adapter Discovery timeout (in seconds)
   * Access mode - GET and SET
   * Data type   - uint32_t
   */
  BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT,

  /* Properties unique to remote device */
  /**
   * Description - User defined friendly name of the remote device
   * Access mode - GET and SET
   * Data type   - bt_bdname_t.
   */
  BT_PROPERTY_REMOTE_FRIENDLY_NAME,
  /**
   * Description - RSSI value of the inquired remote device
   * Access mode - Only GET.
   * Data type   - int32_t.
   */
  BT_PROPERTY_REMOTE_RSSI,
  /**
   * Description - Remote version info
   * Access mode - SET/GET.
   * Data type   - bt_remote_version_t.
   */

  BT_PROPERTY_REMOTE_VERSION_INFO,

  /**
   * Description - Local LE features
   * Access mode - GET.
   * Data type   - bt_local_le_features_t.
   */
  BT_PROPERTY_LOCAL_LE_FEATURES,

  BT_PROPERTY_REMOTE_DEVICE_TIMESTAMP = 0xFF,
} bt_property_type_t;

/* Structure associated with BSA_MGT_STATUS_EVT */
typedef struct
{
    uint8_t enable; /* Bluetooth Enable parameter */
} BT_MSG_CONNECT_MSG;

typedef struct
{
    uint16_t reason; /* Disconnection reason */
} BT_MSG_DISCONNECT_MSG;

/* Structure associated with BSA_MGT_STATUS_EVT */
typedef struct
{
    const char *address;
    int status;
} BT_MSG_REMOVE_MSG;

/*MGR callback events */
typedef enum
{
    BT_EVENT_MGR_CONNECT,
    BT_EVENT_MGR_DISCONNECT,
    BT_EVENT_MGR_REMOVE_DEVICE,
} BT_MGR_EVT;

/* Union of all Management messages structures (parameters) */
typedef union
{
    BT_MSG_CONNECT_MSG connect; /* connect status parameter */
    BT_MSG_DISCONNECT_MSG disconnect; /* disconnection parameter */
    BT_MSG_REMOVE_MSG remove;
} BT_MGT_MSG;



typedef void (*discovery_cb_t) (void *listener_handle, const char *bt_name, BTAddr bt_addr, int is_completed, void *data);
typedef void (*manage_callbacks_t) (void *listener_handle, BT_MGR_EVT event, void *data);
typedef void (*ble_callbacks_t) (void *listener_handle, BT_BLE_EVT event, void *data);
typedef void (*a2dp_callbacks_t) (void *listener_handle, BT_A2DP_EVT event, void *data);
typedef void (*a2dp_sink_callbacks_t) (void *listener_handle, BT_A2DP_SINK_EVT event, void *data);
typedef void (*hfp_callbacks_t) (void *listener_handle, BT_HS_EVT event, void *data);

#ifdef __cplusplus
}
#endif

#endif /* __BT_COMMON_H__ */
