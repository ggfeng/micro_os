#ifndef _RK_A2DP_SINK_H_
#define _RK_A2DP_SINK_H_
#include "hardware/bt/bt_common.h"

#define MAX_CONNECTIONS 1

#define TRUE 1
#define FALSE 0

/**
 * brief      Bluetooth service working mode
 */
typedef enum {
    BLUETOOTH_A2DP_SINK,    /*!< A2DP Bluetooth sink audio, ESP32 will receive audio data from other bluetooth devices */
    BLUETOOTH_A2DP_SOURCE,   /*!< A2DP Bluetooth source audio, ESP32 can send audio data to other bluetooth devices */
} bluetooth_service_mode_t;

/**
 * brief      Bluetooth peripheral event id
 */
typedef enum {
    PERIPH_BLUETOOTH_UNKNOWN = 0,       /*!< No event */
    PERIPH_BLUETOOTH_CONNECTED,         /*!< A bluetooth device was connected */
    PERIPH_BLUETOOTH_DISCONNECTED,      /*!< Last connection was disconnected */
    PERIPH_BLUETOOTH_AUDIO_STARTED,     /*!< The audio session has been started */
    PERIPH_BLUETOOTH_AUDIO_SUSPENDED,   /*!< The audio session has been suspended */
    PERIPH_BLUETOOTH_AUDIO_STOPPED,     /*!< The audio session has been stopped */
} periph_bluetooth_event_id_t;

/**
 *brief      Bluetooth service configuration
 */
typedef struct {
    char    device_name[32];   /*!< Bluetooth local device name */
    char    remote_name[32];   /*!< Bluetooth remote device name */
    bluetooth_service_mode_t    mode;           /*!< Bluetooth working mode */
} rk_bluetooth_service_cfg_t;

struct A2dpSink_t
{
    void *caller;
	uint8_t connected;
	BTAddr bda_connected;
	char con_name[32];
    uint8_t enabled;
	uint8_t start;
	uint8_t playing;

    a2dp_sink_callbacks_t listener;
    void *          listener_handle;

    pthread_mutex_t mutex;
};
typedef struct A2dpSink_t A2dpSink;

A2dpSink *a2dpk_create(void *caller);
int a2dpk_destroy(A2dpSink *as);
int rk_a2dp_sink_set_name(const char *name);
int rk_a2dp_sink_enable(A2dpSink *as);
int rk_a2dp_sink_disable(A2dpSink *as);
int rk_a2dp_sink_send_avrc_cmd(A2dpSink *as, uint8_t cmd);
int rk_a2dp_sink_connect(A2dpSink *as, esp_bd_addr_t remote_bda);
int rk_a2dp_sink_disconnect(A2dpSink *as, esp_bd_addr_t remote_bda);
int rk_a2dp_sink_send_get_playstatus(A2dpSink *as);
int rk_a2dp_sink_get_playing(A2dpSink *as);
int rk_a2dp_set_listener(A2dpSink *as, a2dp_sink_callbacks_t listener, void *listener_handler);
int rk_a2dp_sink_get_element_attrs(A2dpSink *as);
int rk_a2dp_sink_get_connected_devices(A2dpSink *as, BTDevice *dev, int dev_cnt);

#endif

