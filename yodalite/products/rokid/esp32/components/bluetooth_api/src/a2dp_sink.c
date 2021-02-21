/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "i2s_stream.h"
#include "esp_peripherals.h"
#include "board.h"
#include "pthread.h"

#if (CONFIG_BT_ENABLED && CONFIG_A2DP_ENABLE)
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"

#include "bt_keycontrol.h"
#include "rk_a2dp_sink.h"

static const char *TAG = "A2DP SINK";

static rk_bluetooth_service_cfg_t rk_bt_cfg = {
	.device_name = "ROKID-SPEAKER",
	.mode = BLUETOOTH_A2DP_SINK,
};
static esp_periph_handle_t rk_bt_periph;
static audio_pipeline_handle_t rk_pipeline;
static audio_element_handle_t bt_stream_reader;
static audio_element_handle_t i2s_stream_writer;
static esp_periph_set_handle_t rk_set;

#define VALIDATE_BT(periph, ret) if (!(periph && esp_periph_get_id(periph) == PERIPH_ID_BLUETOOTH)) { \
    ESP_LOGE(TAG, "Invalid Bluetooth periph, at line %d", __LINE__);\
    return ret;\
}

typedef uint8_t esp_peer_bdname_t[ESP_BT_GAP_MAX_BDNAME_LEN + 1];

typedef struct bluetooth_service {
    audio_element_handle_t stream;
    esp_periph_handle_t periph;
    audio_stream_type_t stream_type;
    esp_bd_addr_t remote_bda;
    esp_peer_bdname_t peer_bdname;
    esp_a2d_connection_state_t connection_state;
    esp_a2d_audio_state_t audio_state;
    uint64_t pos;
    uint8_t tl;
    bool avrc_connected;
} bluetooth_service_t;

bluetooth_service_t *g_bt_service = NULL;

static const char *conn_state_str[] = { "Disconnected", "Connecting", "Connected", "Disconnecting" };
static const char *audio_state_str[] = { "Suspended", "Stopped", "Started" };

typedef enum {
    KEY_ACT_STATE_IDLE,
    KEY_ACT_STATE_PRESS,
    KEY_ACT_STATE_RELEASE
} key_act_state_t;

typedef struct {
    key_act_state_t state;
    uint32_t key_code;
    TimerHandle_t key_tmr;
    uint8_t tl;
} key_act_cb_t;

static void key_act_state_hdl_idle(key_act_cb_t *key_cb, bt_key_act_param_t *param);
static void key_act_state_hdl_press(key_act_cb_t *key_cb, bt_key_act_param_t *param);
static void key_act_state_hdl_release(key_act_cb_t *key_cb, bt_key_act_param_t *param);
static void key_act_time_out(void *p);

static key_act_cb_t key_cb;
static A2dpSink *_a2dpk = NULL;

static void rk_bt_key_act_state_machine(bt_key_act_param_t *param)
{
    ESP_LOGD(TAG, "key_ctrl cb: tl %d, state: %d", key_cb.tl, key_cb.state);
    switch (key_cb.state) {
    case KEY_ACT_STATE_IDLE:
        key_act_state_hdl_idle(&key_cb, param);
        break;
    case KEY_ACT_STATE_PRESS:
        key_act_state_hdl_press(&key_cb, param);
        break;
    case KEY_ACT_STATE_RELEASE:
        key_act_state_hdl_release(&key_cb, param);
        break;
    default:
        ESP_LOGD(TAG, "Invalid key_ctrl state: %d", key_cb.state);
        break;
    }
}

static void key_act_time_out(void *p)
{
    bt_key_act_param_t param;
    memset(&param, 0, sizeof(bt_key_act_param_t));
    param.evt = ESP_AVRC_CT_PT_RSP_TO_EVT;
    rk_bt_key_act_state_machine(&param);
}

static esp_err_t rk_bt_key_act_sm_init(void)
{
    if (key_cb.key_tmr) {
        ESP_LOGW(TAG, "%s timer not released", __func__);
        xTimerDelete(key_cb.key_tmr, portMAX_DELAY);
        key_cb.key_tmr = NULL;
    }
    memset(&key_cb, 0, sizeof(key_act_cb_t));
    int tmr_id = 0xfe;
    key_cb.tl = 0;
    key_cb.state = KEY_ACT_STATE_IDLE;
    key_cb.key_code = 0;
    key_cb.key_tmr = xTimerCreate("key_tmr", portMAX_DELAY,
                                  pdFALSE, (void *)tmr_id, key_act_time_out);
    if (key_cb.key_tmr == NULL) {
        ESP_LOGW(TAG, "%s timer creation failure", __func__);
        return false;
    }
    return true;
}

static void rk_bt_key_act_sm_deinit(void)
{
    if (key_cb.key_tmr) {
        xTimerDelete(key_cb.key_tmr, portMAX_DELAY);
        key_cb.key_tmr = NULL;
    }

    memset(&key_cb, 0, sizeof(key_act_cb_t));
}

static void key_act_state_hdl_idle(key_act_cb_t *key_cb, bt_key_act_param_t *param)
{
    AUDIO_MEM_CHECK(TAG, key_cb, return);
    AUDIO_MEM_CHECK(TAG, param, return);
    if(key_cb->state != KEY_ACT_STATE_IDLE) {
        ESP_LOGE(TAG, "ERROR STATE: bluetooth key action state should be KEY_ACT_STATE_IDLE!");
        return;
    }
    if (param->evt == ESP_AVRC_CT_KEY_STATE_CHG_EVT) {
        key_cb->tl = param->tl;
        key_cb->key_code = param->key_code;
        esp_avrc_ct_send_passthrough_cmd(param->tl, param->key_code, ESP_AVRC_PT_CMD_STATE_PRESSED);
        xTimerStart(key_cb->key_tmr, 500 / portTICK_RATE_MS);
        key_cb->state = KEY_ACT_STATE_PRESS;
    }
}

static void key_act_state_hdl_press(key_act_cb_t *key_cb, bt_key_act_param_t *param)
{
    AUDIO_MEM_CHECK(TAG, key_cb, return);
    AUDIO_MEM_CHECK(TAG, param, return);
    if(key_cb->state != KEY_ACT_STATE_PRESS) {
        ESP_LOGE(TAG, "ERROR STATE: bluetooth key action state should be KEY_ACT_STATE_PRESS!");
        return;
    }
    if (param->evt == ESP_AVRC_CT_PASSTHROUGH_RSP_EVT) {
        if (key_cb->tl != param->tl || key_cb->key_code != param->key_code
            || ESP_AVRC_PT_CMD_STATE_PRESSED != param->key_state) {
            ESP_LOGW(TAG, "Key pressed hdlr: invalid state");
            return;
        }
        key_cb->tl = (key_cb->tl + 1) & 0x0F;
        esp_avrc_ct_send_passthrough_cmd(key_cb->tl, param->key_code, ESP_AVRC_PT_CMD_STATE_RELEASED);
        xTimerReset(key_cb->key_tmr, 500 / portTICK_RATE_MS);
        key_cb->state = KEY_ACT_STATE_RELEASE;
    } else if (param->evt == ESP_AVRC_CT_PT_RSP_TO_EVT) {
        key_cb->tl = 0;
        key_cb->key_code = 0;
        key_cb->state = KEY_ACT_STATE_IDLE;
    }
}

static void key_act_state_hdl_release(key_act_cb_t *key_cb, bt_key_act_param_t *param)
{
    AUDIO_MEM_CHECK(TAG, key_cb, return);
    AUDIO_MEM_CHECK(TAG, param, return);
    if(key_cb->state != KEY_ACT_STATE_RELEASE) {
        ESP_LOGE(TAG, "ERROR STATE: bluetooth key action state should be KEY_ACT_STATE_RELEASE!");
        return;
    }
    if (param->evt == ESP_AVRC_CT_PASSTHROUGH_RSP_EVT) {
        if (key_cb->tl != param->tl || key_cb->key_code != param->key_code
            || ESP_AVRC_PT_CMD_STATE_RELEASED != param->key_state) {
            return;
        }
        xTimerStop(key_cb->key_tmr, 500 / portTICK_RATE_MS);
        key_cb->state = KEY_ACT_STATE_IDLE;
        key_cb->key_code = 0;
    } else if (param->evt == ESP_AVRC_CT_PT_RSP_TO_EVT) {
        key_cb->tl = 0;
        key_cb->key_code = 0;
        key_cb->state = KEY_ACT_STATE_IDLE;
    }
}

static void bt_a2d_sink_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *p_param)
{
    esp_a2d_cb_param_t *a2d = NULL;
	A2dpSink *as = _a2dpk;
    BT_AVK_MSG msg = {0};

    switch (event) {
        case ESP_A2D_CONNECTION_STATE_EVT:
            a2d = (esp_a2d_cb_param_t *)(p_param);
            uint8_t *bda = a2d->conn_stat.remote_bda;
            ESP_LOGD(TAG, "A2DP connection state: %s, [%02x:%02x:%02x:%02x:%02x:%02x]",
                     conn_state_str[a2d->conn_stat.state], bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);

            if (g_bt_service->connection_state == ESP_A2D_CONNECTION_STATE_DISCONNECTED
                    && a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
                memcpy(&g_bt_service->remote_bda, &a2d->conn_stat.remote_bda, sizeof(esp_bd_addr_t));
                g_bt_service->connection_state = a2d->conn_stat.state;
                ESP_LOGD(TAG, "A2DP connection state = CONNECTED");
                if (g_bt_service->periph) {
                    esp_periph_send_event(g_bt_service->periph, PERIPH_BLUETOOTH_CONNECTED, NULL, 0);
                }
				as->connected = TRUE;
				memcpy(as->bda_connected, a2d->conn_stat.remote_bda, sizeof(esp_bd_addr_t));
				if (as->listener) {
					msg.sig_chnl_open.status = a2d->conn_stat.state;
					msg.sig_chnl_open.idx = 0;
					memcpy(msg.sig_chnl_open.bd_addr, a2d->conn_stat.remote_bda, sizeof(esp_bd_addr_t));
					as->listener(as->listener_handle, BT_AVK_OPEN_EVT, &msg);
				}
				
				/* Stream has been opened when init the a2dp sink*/
				if ((as->listener) && (as->connected == TRUE)) {
					msg.stream_chnl_open.status = TRUE;
					msg.stream_chnl_open.idx = 0;
					memcpy(msg.stream_chnl_open.bd_addr, a2d->conn_stat.remote_bda, sizeof(esp_bd_addr_t));
					as->listener(as->listener_handle, BT_AVK_STR_OPEN_EVT, &msg);
				}

            }
            if (memcmp(&g_bt_service->remote_bda, &a2d->conn_stat.remote_bda, sizeof(esp_bd_addr_t)) == 0
                    && g_bt_service->connection_state == ESP_A2D_CONNECTION_STATE_CONNECTED
                    && a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
                memset(&g_bt_service->remote_bda, 0, sizeof(esp_bd_addr_t));
                g_bt_service->connection_state = ESP_A2D_CONNECTION_STATE_DISCONNECTED;
                ESP_LOGD(TAG, "A2DP connection state =  DISCONNECTED");
                if (g_bt_service->stream) {
                    audio_element_report_status(g_bt_service->stream, AEL_STATUS_INPUT_DONE);
                }
                if (g_bt_service->periph) {
                    esp_periph_send_event(g_bt_service->periph, PERIPH_BLUETOOTH_DISCONNECTED, NULL, 0);
                }

				if ((as->listener)) {
					msg.stream_chnl_close.status = FALSE;
					msg.stream_chnl_close.idx = 0;
					memcpy(msg.stream_chnl_close.bd_addr, a2d->conn_stat.remote_bda, 6);
					as->listener(as->listener_handle, BT_AVK_STR_CLOSE_EVT, &msg);
				}

				as->connected = FALSE;
				if (as->listener) {
					msg.sig_chnl_open.status = a2d->conn_stat.state;
					msg.sig_chnl_open.idx = 0;
					memcpy(msg.sig_chnl_open.bd_addr, a2d->conn_stat.remote_bda, 6);
					as->listener(as->listener_handle, BT_AVK_CLOSE_EVT, &msg);
				}

            }
            break;
        case ESP_A2D_AUDIO_STATE_EVT:
            a2d = (esp_a2d_cb_param_t *)(p_param);
            ESP_LOGD(TAG, "A2DP audio state: %s", audio_state_str[a2d->audio_stat.state]);
            g_bt_service->audio_state = a2d->audio_stat.state;
            if (ESP_A2D_AUDIO_STATE_STARTED == a2d->audio_stat.state) {
                g_bt_service->pos = 0;
            }
            if (g_bt_service->periph == NULL) {
                break;
            }

            if (ESP_A2D_AUDIO_STATE_STARTED == a2d->audio_stat.state) {
                esp_periph_send_event(g_bt_service->periph, PERIPH_BLUETOOTH_AUDIO_STARTED, NULL, 0);
				as->playing = TRUE;
				if ((as->listener) && (as->connected == TRUE)) {
					msg.start_streaming.status = a2d->audio_stat.state;
					msg.start_streaming.idx = 0;
					memcpy(msg.start_streaming.bd_addr, a2d->audio_stat.remote_bda, 6);
					as->listener(as->listener_handle, BT_AVK_START_EVT, &msg);
				}
            } else if (ESP_A2D_AUDIO_STATE_REMOTE_SUSPEND == a2d->audio_stat.state) {
                esp_periph_send_event(g_bt_service->periph, PERIPH_BLUETOOTH_AUDIO_SUSPENDED, NULL, 0);
				as->playing = FALSE;
            } else if (ESP_A2D_AUDIO_STATE_STOPPED == a2d->audio_stat.state) {
                esp_periph_send_event(g_bt_service->periph, PERIPH_BLUETOOTH_AUDIO_STOPPED, NULL, 0);
				as->playing = FALSE;
				if ((as->listener) && (as->connected == TRUE)) {
					msg.stop_streaming.status = a2d->audio_stat.state;
					msg.stop_streaming.idx = 0;
					memcpy(msg.stop_streaming.bd_addr, a2d->audio_stat.remote_bda, 6);
					msg.stop_streaming.suspended = TRUE;
					as->listener(as->listener_handle, BT_AVK_STOP_EVT, &msg);
				}
            }

            break;
        case ESP_A2D_AUDIO_CFG_EVT:
            a2d = (esp_a2d_cb_param_t *)(p_param);
            ESP_LOGD(TAG, "A2DP audio stream configuration, codec type %d", a2d->audio_cfg.mcc.type);
            if (a2d->audio_cfg.mcc.type == ESP_A2D_MCT_SBC) {
                int sample_rate = 16000;
                char oct0 = a2d->audio_cfg.mcc.cie.sbc[0];
                if (oct0 & (0x01 << 6)) {
                    sample_rate = 32000;
                } else if (oct0 & (0x01 << 5)) {
                    sample_rate = 44100;
                } else if (oct0 & (0x01 << 4)) {
                    sample_rate = 48000;
                }
                ESP_LOGD(TAG, "Bluetooth configured, sample rate=%d", sample_rate);
                if (g_bt_service->stream == NULL) {
                    break;
                }
                audio_element_info_t bt_info = {0};
                audio_element_getinfo(g_bt_service->stream, &bt_info);
                bt_info.sample_rates = sample_rate;
                bt_info.channels = 2;
                bt_info.bits = 16;
                audio_element_setinfo(g_bt_service->stream, &bt_info);
                audio_element_report_info(g_bt_service->stream);
            }
            break;
        default:
            ESP_LOGD(TAG, "Unhandled A2DP event: %d", event);
            break;
    }
}

static void bt_a2d_sink_data_cb(const uint8_t *data, uint32_t len)
{
    if (g_bt_service->stream) {
        if (audio_element_get_state(g_bt_service->stream) == AEL_STATE_RUNNING) {
            audio_element_output(g_bt_service->stream, (char *)data, len);
        }
    }
}



static esp_err_t rk_bluetooth_service_start(rk_bluetooth_service_cfg_t *config)
{
    if (g_bt_service) {
        ESP_LOGE(TAG, "Bluetooth service have been initialized");
        return ESP_FAIL;
    }

    g_bt_service = calloc(1, sizeof(bluetooth_service_t));
    AUDIO_MEM_CHECK(TAG, g_bt_service, return ESP_ERR_NO_MEM);

    if (config->device_name) {
        esp_bt_dev_set_device_name(config->device_name);
    } else {
		esp_bt_dev_set_device_name("ROKID_A2DP_SPEAKER");
    }

	esp_a2d_sink_init();
	esp_a2d_sink_register_data_callback(bt_a2d_sink_data_cb);
	esp_a2d_register_callback(bt_a2d_sink_cb);
	// TODO: Use this function for IDF version higher than v3.0
	// esp_a2d_sink_register_data_callback(bt_a2d_data_cb);
	g_bt_service->stream_type = AUDIO_STREAM_READER;

    /* set discoverable and connectable mode */
    esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);

    return ESP_OK;
}

static esp_err_t rk_bluetooth_service_destroy()
{
    if (g_bt_service &&
            (g_bt_service->stream || g_bt_service->periph)) {

        AUDIO_ERROR(TAG, "Stream and periph need to stop first");
        return ESP_FAIL;
    }
    if (g_bt_service) {
        if(g_bt_service->stream_type == AUDIO_STREAM_READER) {
            esp_a2d_sink_deinit();
        } else {
            esp_a2d_source_deinit();
        }
		free(g_bt_service);
        g_bt_service = NULL;
    }
    return ESP_OK;
}

static esp_err_t _bt_stream_destroy(audio_element_handle_t self)
{
    g_bt_service->stream = NULL;
    return ESP_OK;
}

audio_element_handle_t bluetooth_service_create_stream()
{
    if (g_bt_service && g_bt_service->stream) {
        ESP_LOGE(TAG, "Bluetooth stream have been created");
        return NULL;
    }

    audio_element_cfg_t cfg = DEFAULT_AUDIO_ELEMENT_CONFIG();
    cfg.task_stack = -1; // No need task
    cfg.destroy = _bt_stream_destroy;
    cfg.tag = "bt";
    g_bt_service->stream = audio_element_init(&cfg);

    AUDIO_MEM_CHECK(TAG, g_bt_service->stream, return NULL);

    audio_element_setdata(g_bt_service->stream, g_bt_service);

    return g_bt_service->stream;
}

static void bt_avrc_ct_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *p_param)
{
    esp_avrc_ct_cb_param_t *rc = p_param;
	A2dpSink *as = _a2dpk;
    BT_AVK_MSG msg = {0};

    switch (event) {
        case ESP_AVRC_CT_CONNECTION_STATE_EVT: {
                uint8_t *bda = rc->conn_stat.remote_bda;
                g_bt_service->avrc_connected = rc->conn_stat.connected;
                if (rc->conn_stat.connected) {
                    ESP_LOGD(TAG, "ESP_AVRC_CT_CONNECTION_STATE_EVT");
                    rk_bt_key_act_sm_init();
					if ((as->listener) && (as->connected == TRUE)) {
						msg.rc_open.status = rc->conn_stat.connected;
						memcpy(msg.rc_open.bd_addr, rc->conn_stat.remote_bda, 6);
						msg.rc_open.idx = 0;
						as->listener(as->listener_handle, BT_AVK_RC_OPEN_EVT, &msg);
					}
                } else if (0 == rc->conn_stat.connected){
                    rk_bt_key_act_sm_deinit();
					if ((as->listener) && (as->connected == TRUE)) {
						msg.rc_open.status = rc->conn_stat.connected;
						memcpy(msg.rc_open.bd_addr, rc->conn_stat.remote_bda, 6);
						msg.rc_open.idx = 0;
						as->listener(as->listener_handle, BT_AVK_RC_CLOSE_EVT, &msg);
					}
                }

                ESP_LOGD(TAG, "AVRC conn_state evt: state %d, [%02x:%02x:%02x:%02x:%02x:%02x]",
                                         rc->conn_stat.connected, bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
                break;
            }
        case ESP_AVRC_CT_PASSTHROUGH_RSP_EVT: {
                if(g_bt_service->avrc_connected) {
                    ESP_LOGD(TAG, "AVRC passthrough rsp: key_code 0x%x, key_state %d", rc->psth_rsp.key_code, rc->psth_rsp.key_state);
                    bt_key_act_param_t param;
                    memset(&param, 0, sizeof(bt_key_act_param_t));
                    param.evt = event;
                    param.tl = rc->psth_rsp.tl;
                    param.key_code = rc->psth_rsp.key_code;
                    param.key_state = rc->psth_rsp.key_state;
                    rk_bt_key_act_state_machine(&param);
                }
                break;
            }
        case ESP_AVRC_CT_METADATA_RSP_EVT: {
                ESP_LOGD(TAG, "AVRC metadata rsp: attribute id 0x%x, %s", rc->meta_rsp.attr_id, rc->meta_rsp.attr_text);
                // free(rc->meta_rsp.attr_text);
				if ((as->listener) && (as->connected == 1)) {
					msg.elem_attr.num_attr = 1;
					msg.elem_attr.attr_entry[0].attr_id = rc->meta_rsp.attr_id;
					snprintf(msg.elem_attr.attr_entry[0].data, sizeof(msg.elem_attr.attr_entry[0].data), "%s", rc->meta_rsp.attr_text);
					as->listener(as->listener_handle, BT_AVK_GET_ELEM_ATTR_EVT, &msg);
				}
                break;
            }
        case ESP_AVRC_CT_CHANGE_NOTIFY_EVT: {
                ESP_LOGD(TAG, "AVRC event notification: %d, param: %d", rc->change_ntf.event_id, rc->change_ntf.event_parameter);
                break;
            }
        case ESP_AVRC_CT_REMOTE_FEATURES_EVT: {
                ESP_LOGD(TAG, "AVRC remote features %x", rc->rmt_feats.feat_mask);
                break;
            }
        default:
            ESP_LOGE(TAG, "%s unhandled evt %d", __func__, event);
            break;
    }
}

static esp_err_t _bt_periph_init(esp_periph_handle_t periph)
{
    esp_avrc_ct_init();
    esp_avrc_ct_register_callback(bt_avrc_ct_cb);
    return ESP_OK;
}

static esp_err_t _bt_periph_run(esp_periph_handle_t self, audio_event_iface_msg_t *msg)
{
    return ESP_OK;
}

static esp_err_t _bt_periph_destroy(esp_periph_handle_t periph)
{
    esp_avrc_ct_deinit();
    g_bt_service->periph = NULL;
    return ESP_OK;
}

static esp_periph_handle_t rk_bluetooth_service_create_periph()
{
    if (g_bt_service && g_bt_service->periph) {
        ESP_LOGE(TAG, "Bluetooth periph have been created");
        return NULL;
    }

    g_bt_service->periph = esp_periph_create(PERIPH_ID_BLUETOOTH, "periph_bt");
    esp_periph_set_function(g_bt_service->periph, _bt_periph_init, _bt_periph_run, _bt_periph_destroy);
    return g_bt_service->periph;
}

static esp_err_t rk_periph_bluetooth_passthrough_cmd(esp_periph_handle_t periph, uint8_t cmd)
{
    VALIDATE_BT(periph, ESP_FAIL);
    if (g_bt_service->audio_state != ESP_A2D_AUDIO_STATE_STARTED) {
        //return ESP_FAIL;
    }
    esp_err_t err = ESP_OK;

    if (g_bt_service->avrc_connected) {
        bt_key_act_param_t param;
        memset(&param, 0, sizeof(bt_key_act_param_t));
        param.evt = ESP_AVRC_CT_KEY_STATE_CHG_EVT;
        param.key_code = cmd;
        param.key_state = 0;
        param.tl = (g_bt_service->tl) & 0x0F;
        g_bt_service->tl = (g_bt_service->tl + 2) & 0x0f;
        rk_bt_key_act_state_machine(&param);
    }

    return err;
}

static esp_err_t rk_periph_bluetooth_play(esp_periph_handle_t periph)
{
    esp_err_t err = ESP_OK;
    if(g_bt_service->stream_type == AUDIO_STREAM_READER) {
        err = rk_periph_bluetooth_passthrough_cmd(periph, ESP_AVRC_PT_CMD_PLAY);
    } else {
        err = esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_CHECK_SRC_RDY);
    }
    return err;
}

static esp_err_t rk_periph_bluetooth_pause(esp_periph_handle_t periph)
{
    esp_err_t err = ESP_OK;
    if(g_bt_service->stream_type == AUDIO_STREAM_READER) {
        err = rk_periph_bluetooth_passthrough_cmd(periph, ESP_AVRC_PT_CMD_PAUSE);
    } else {
        err = esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_SUSPEND);
    }
    return err;
}

static esp_err_t rk_periph_bluetooth_stop(esp_periph_handle_t periph)
{
    esp_err_t err = ESP_OK;
    if(g_bt_service->stream_type == AUDIO_STREAM_READER) {
        err = rk_periph_bluetooth_passthrough_cmd(periph, ESP_AVRC_PT_CMD_STOP);
    } else {
        esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_STOP);
        g_bt_service->audio_state = ESP_A2D_AUDIO_STATE_STOPPED;
    }
    return err;
}

static esp_err_t rk_periph_bluetooth_next(esp_periph_handle_t periph)
{
    return rk_periph_bluetooth_passthrough_cmd(periph, ESP_AVRC_PT_CMD_FORWARD);
}

static esp_err_t rk_periph_bluetooth_prev(esp_periph_handle_t periph)
{
    return rk_periph_bluetooth_passthrough_cmd(periph, ESP_AVRC_PT_CMD_BACKWARD);
}

static esp_err_t rk_periph_bluetooth_rewind(esp_periph_handle_t periph)
{
    return rk_periph_bluetooth_passthrough_cmd(periph, ESP_AVRC_PT_CMD_REWIND);
}

static esp_err_t rk_periph_bluetooth_fast_forward(esp_periph_handle_t periph)
{
    return rk_periph_bluetooth_passthrough_cmd(periph, ESP_AVRC_PT_CMD_FAST_FORWARD);
}

int esp_a2dp_sink_enable(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    ESP_LOGI(TAG, "[ 1 ] Create Bluetooth service");
    if (rk_bluetooth_service_start(&rk_bt_cfg)) {
		ESP_LOGI(TAG, "Create bluetooth service as a2dp sink failed!\n");
		return -1;
	}
    ESP_LOGI(TAG, "[ 2 ] Start codec chip");
    audio_board_handle_t board_handle = audio_board_init();
    if (audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START)) {
		ESP_LOGI(TAG, "audio_hal_ctrl_codec failed!\n");
		return -1;
	}

    ESP_LOGI(TAG, "[ 3 ] Create audio pipeline for playback");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    rk_pipeline = audio_pipeline_init(&pipeline_cfg);

    ESP_LOGI(TAG, "[3.1] Create i2s stream to write data to codec chip");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    i2s_stream_writer = i2s_stream_init(&i2s_cfg);

    ESP_LOGI(TAG, "[3.2] Get Bluetooth stream");
    bt_stream_reader = bluetooth_service_create_stream();

    ESP_LOGI(TAG, "[3.2] Register all elements to audio pipeline");
    audio_pipeline_register(rk_pipeline, bt_stream_reader, "bt");
    audio_pipeline_register(rk_pipeline, i2s_stream_writer, "i2s");

    ESP_LOGI(TAG, "[3.3] Link it together [Bluetooth]-->bt_stream_reader-->i2s_stream_writer-->[codec_chip]");
    if (audio_pipeline_link(rk_pipeline, (const char *[]) {"bt", "i2s"}, 2)) {
		ESP_LOGI(TAG, "audio_pipeline_link failed!\n");
		return -1;
	}
    ESP_LOGI(TAG, "[ 4 ] Initialize peripherals");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    rk_set = esp_periph_set_init(&periph_cfg);

    ESP_LOGI(TAG, "[4.2] Create Bluetooth peripheral");
    rk_bt_periph = rk_bluetooth_service_create_periph();

    ESP_LOGI(TAG, "[4.2] Start all peripherals");
    esp_periph_start(rk_set, rk_bt_periph);

    ESP_LOGI(TAG, "[ 6 ] Start audio_pipeline");
    audio_pipeline_run(rk_pipeline);

	return 0;
}

int esp_a2dp_sink_disable(void)
{
    ESP_LOGI(TAG, "[ 8 ] Stop audio_pipeline");
    audio_pipeline_terminate(rk_pipeline);

    audio_pipeline_unregister(rk_pipeline, bt_stream_reader);
    audio_pipeline_unregister(rk_pipeline, i2s_stream_writer);

    /* Terminate the pipeline before removing the listener */
    audio_pipeline_remove_listener(rk_pipeline);

    /* Stop all peripherals before removing the listener */
    esp_periph_set_stop_all(rk_set);
    /* Release all resources */
    audio_pipeline_deinit(rk_pipeline);
    audio_element_deinit(bt_stream_reader);
    audio_element_deinit(i2s_stream_writer);
    esp_periph_set_destroy(rk_set);

	if (rk_bluetooth_service_destroy() != ESP_OK) {
		printf("Destroy bluetooth service failed!\n ");
		return -1;
	}
	return 0;
}

/***************************************************************************************/
A2dpSink *a2dpk_create(void *caller)
{
    A2dpSink *as = calloc(1, sizeof(*as));

    as->caller = caller;
    pthread_mutex_init(&as->mutex, NULL);

    _a2dpk = as;
    return as;
}

int a2dpk_destroy(A2dpSink *as)
{
    if (as) {
        //a2dpk_disable(as);
        pthread_mutex_destroy(&as->mutex);
        free(as);
    }
    _a2dpk = NULL;
    return 0;
}

int rk_a2dp_sink_set_name(const char *name)
{
	if (name) {
		strcpy(rk_bt_cfg.device_name, name);
	}
	return 0;
}

int rk_a2dp_sink_enable(A2dpSink *as)
{
	if (!as) {
		printf("%s: as is NULL!\n", __func__);
		return -1;
	}
	if (as->enabled == FALSE) {
		if (esp_a2dp_sink_enable()) {
			printf("esp_a2dp_sink_enable failed!\n");
			return -1;
		}
		as->enabled = TRUE;
	}
	return 0;
}

int rk_a2dp_sink_disable(A2dpSink *as)
{
	if (!as) {
		printf("%s: as is NULL!\n", __func__);
		return -1;
	}
	if (as->enabled) {
		if (esp_a2dp_sink_disable()) {
			printf("esp_a2dp_sink_disable failed!\n");
			return -1;
		}
		as->enabled = FALSE;
	}
	return 0;
}

int rk_a2dp_sink_send_avrc_cmd(A2dpSink *as, uint8_t cmd)
{
	if (!as) {
		printf("%s : as is NULL!\n", __func__);
		return -1;
	}
	switch (cmd) {
		case ESP_AVRC_PT_CMD_PLAY:
			rk_periph_bluetooth_play(rk_bt_periph);
			as->playing = TRUE;
			break;
		case ESP_AVRC_PT_CMD_PAUSE:
			rk_periph_bluetooth_pause(rk_bt_periph);
			as->playing = FALSE;
			break;
		case ESP_AVRC_PT_CMD_STOP:
			rk_periph_bluetooth_stop(rk_bt_periph);
			as->playing = FALSE;
			break;
		case ESP_AVRC_PT_CMD_FORWARD:
			rk_periph_bluetooth_next(rk_bt_periph);
			break;
		case ESP_AVRC_PT_CMD_BACKWARD:
			rk_periph_bluetooth_prev(rk_bt_periph);
			break;
		case ESP_AVRC_PT_CMD_REWIND:
			rk_periph_bluetooth_rewind(rk_bt_periph);
			break;
		case ESP_AVRC_PT_CMD_FAST_FORWARD:
			rk_periph_bluetooth_fast_forward(rk_bt_periph);
			break;
		default:
			printf("Invalid AVRC cmd %d\n", cmd);
			break;
	}
	return 0;
}

int rk_a2dp_sink_connect(A2dpSink *as, esp_bd_addr_t remote_bda)
{
	if (!as) {
		printf("%s : as is NULL!\n", __func__);
		return -1;
	}

	if (esp_a2d_sink_connect(remote_bda)) {
		printf("Esp_a2d_sink_connect failed!\n");
		return -1;
	}
	as->connected = TRUE;
	return 0;
}

int rk_a2dp_sink_disconnect(A2dpSink *as, esp_bd_addr_t remote_bda)
{
	if (!as) {
		printf("%s : as is NULL!\n", __func__);
		return -1;
	}

	if (esp_a2d_sink_disconnect(remote_bda)) {
		printf("esp_a2d_sink_disconnect failed!\n");
		return -1;
	}
	as->connected = FALSE;
	return 0;
}

int rk_a2dp_sink_send_get_playstatus(A2dpSink *as)
{
    BT_AVK_MSG msg = {0};

    if (as->playing)
        msg.get_play_status.play_status = BT_AVRC_PLAYSTATE_PLAYING;
    else if (!as->start)
        msg.get_play_status.play_status = BT_AVRC_PLAYSTATE_STOPPED;
    else
        msg.get_play_status.play_status = BT_AVRC_PLAYSTATE_PAUSED;

    if ((as->listener) && as->connected) {
        as->listener(as->listener_handle, BT_AVK_GET_PLAY_STATUS_EVT, &msg);
    }
    return 0;
}

int rk_a2dp_sink_get_playing(A2dpSink *as)
{
	return as ? as->playing : 0;
}

int rk_a2dp_set_listener(A2dpSink *as, a2dp_sink_callbacks_t listener, void *listener_handle)
{
	if (!as) {
		printf("%s : as is NULL!\n", __func__);
		return -1;
	}
	as->listener = listener;
	as->listener_handle = listener_handle;
	return 0;
}

int rk_a2dp_sink_get_element_attrs(A2dpSink *as)
{
    //Register notifications and request metadata
    esp_avrc_ct_send_metadata_cmd(0, ESP_AVRC_MD_ATTR_TITLE | ESP_AVRC_MD_ATTR_ARTIST | ESP_AVRC_MD_ATTR_ALBUM | ESP_AVRC_MD_ATTR_GENRE);
    esp_avrc_ct_send_register_notification_cmd(1, ESP_AVRC_RN_TRACK_CHANGE, 0);
	return 0;
}

int rk_a2dp_sink_get_connected_devices(A2dpSink *as, BTDevice *dev, int dev_cnt)
{
	if (as->connected) {
		snprintf(dev->name, sizeof(dev->name), "%s", as->con_name);
		memcpy(dev->bd_addr, as->bda_connected, 6);	
		return 1;
	}
	return 0;
}

#endif
