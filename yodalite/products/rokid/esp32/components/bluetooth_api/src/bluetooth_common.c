#include <string.h>
#include <pthread.h>
#include "ble_server.h"
#include "rk_a2dp_sink.h"
#include "bluetooth_common.h"
#include "esp_gap_bt_api.h"

#if CONFIG_BT_ENABLED
#define BT_TAG "ROKID_BLUETOOTH"

static int esp_bt_open(void)
{
    esp_err_t ret;

    /* Initialize NVS. */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

//    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(BT_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        goto error;
    }

#ifdef CONFIG_BTDM_CONTROLLER_MODE_BTDM
    ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
#elif defined(CONFIG_BTDM_CONTROLLER_MODE_BLE_ONLY)
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
#elif defined(CONFIG_BTDM_CONTROLLER_MODE_BR_EDR_ONLY)
    ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
#endif

    if (ret) {
        ESP_LOGE(BT_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        goto error;
    }

    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(BT_TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
        goto error;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(BT_TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
        goto error;
    }
error:
	return ret;

}

static esp_err_t esp_bt_close(void)
{
    esp_err_t err;
    ESP_LOGD(BT_TAG, "Free mem at start of ble_server_stop_service %d", esp_get_free_heap_size());
    err = esp_bluedroid_disable();
    if (err != ESP_OK) {
        return ESP_FAIL;
    }
    ESP_LOGD(BT_TAG, "esp_bluedroid_disable called successfully");
    err = esp_bluedroid_deinit();
    if (err != ESP_OK) {
        return err;
    }
    ESP_LOGD(BT_TAG, "esp_bluedroid_deinit called successfully");
    err = esp_bt_controller_disable();
    if (err != ESP_OK) {
        return ESP_FAIL;
    }

    /* The API `esp_bt_controller_deinit` will have to be removed when we add support for
     * `reset to provisioning`
     */
    ESP_LOGD(BT_TAG, "esp_bt_controller_disable called successfully");
    err = esp_bt_controller_deinit();
    if (err != ESP_OK) {
        return ESP_FAIL;
    }
    ESP_LOGD(BT_TAG, "esp_bt_controller_deinit called successfully");

    ESP_LOGD(BT_TAG, "Free mem at end of ble_server_stop_service %d", esp_get_free_heap_size());
    return ESP_OK;
}

/* COMMON MANAGE FUNC BEGIN */
static int rokidbt_open(struct rk_bluetooth *rokid_bt, const char *name)
{
	int ret = ESP_OK;

	if (name) {
		ret = rk_ble_set_name(name);
		if (ret) {
			ESP_LOGE(BT_TAG, "Set BLE name failed!\n");
			return ret;
		}
#if CONFIG_A2DP_ENABLE
		ret = rk_a2dp_sink_set_name(name);
		if (ret) {
			ESP_LOGE(BT_TAG, "Set A2DP SINK name failed!\n");
			return ret;
		}
#endif
	}
	ret = esp_bt_open();
	if (ret) {
		ESP_LOGE(BT_TAG, "esp bt open failed!\n");
		return ret;
	}
	return ret;
}

static int rokidbt_close(struct rk_bluetooth *rokid_bt)
{
	int ret = ESP_OK;
	ret = esp_bt_close();
	if (ret) {
		ESP_LOGE(BT_TAG, "esp bt close failed!\n");
		return ret;
	}
	return ret;
}

static int rokidbt_create(struct rk_bluetooth *rokid_bt)
{
    RKBluetooth_t *bt = calloc(1, sizeof(*bt));

    if (!bt) {
        ESP_LOGE(BT_TAG, "Can not allocate memory for the rokidbt_create");
        return -BT_STATUS_NOMEM;
    }
    bt->ble_ctx = ble_create((void*)bt);
#if CONFIG_A2DP_ENABLE
	bt->a2dp_sink = a2dpk_create(bt);
#endif
#if 0
    bt->a2dp_ctx = a2dp_create(bt);
    bt->hs_ctx = hs_create(bt);
#endif
	pthread_mutex_init(&bt->bt_mutex, NULL);
    rokid_bt->handle = (void *)bt;
    return 0;
}

static int rokidbt_destroy(struct rk_bluetooth *rokid_bt)
{
    RKBluetooth_t *bt = (RKBluetooth_t *)rokid_bt->handle;

    BT_CHECK_HANDLE(bt);

//    rokidbt_close(rokid_bt);
#if 0
	if (bt->hs_ctx) {
        hs_destroy(bt->hs_ctx);
        bt->hs_ctx = NULL;
    }
    if (bt->a2dp_ctx) {
        a2dp_destroy(bt->a2dp_ctx);
        bt->a2dp_ctx = NULL;
    }
#endif
#if CONFIG_A2DP_ENABLE
    if (bt->a2dp_sink) {
        a2dpk_destroy(bt->a2dp_sink);
        bt->a2dp_sink = NULL;
    }
#endif
    if (bt->ble_ctx) {
        ble_destroy(bt->ble_ctx);
        bt->ble_ctx = NULL;
    }

    pthread_mutex_destroy(&bt->bt_mutex);
    free(bt);

    return 0;
}

static int rokidbt_set_name(struct rk_bluetooth *rokid_bt, const char *name)
{
	int ret = 0;
	//set ble name
	ret = rk_ble_set_name(name);
	if (ret) {
		ESP_LOGE(BT_TAG, "Set ble name failed!\n");
		return ret;
	}
	//set a2dp_sink name
#if CONFIG_A2DP_ENABLE
	ret = rk_a2dp_sink_set_name(name);
	if (ret) {
		ESP_LOGE(BT_TAG, "Set A2DP SINK name failed!\n");
		return ret;
	}
#endif
	return ret;
}

static int rokidbt_set_visibility(struct rk_bluetooth *bt, bool discoverable, bool connectable)
{
	if (discoverable && connectable) {
		esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
	} else if (connectable) {
		esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE);
	} else {
		esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_NONE);
	}
	return 0;
}

/* COMMON MANAGE FUNC END*/

/* BLE FUNC BEGIN */
static int rokidbt_ble_enable(struct rk_bluetooth *rokid_bt)
{
    RKBluetooth_t *bt = (RKBluetooth_t *)rokid_bt->handle;
    BT_CHECK_HANDLE(bt);

    return rokid_ble_enable(bt->ble_ctx);
}

static int rokidbt_ble_disable(struct rk_bluetooth *rokid_bt)
{
    int ret;
    RKBluetooth_t *bt = (RKBluetooth_t *)rokid_bt->handle;
    BT_CHECK_HANDLE(bt);

    ret = rokid_ble_disable(bt->ble_ctx);
    return ret;
}

static int rokidbt_ble_set_visibility(struct rk_bluetooth *rokid_bt, bool discoverable, bool connectable)
{
    BT_CHECK_HANDLE(rokid_bt->handle);
	return rk_ble_set_visibility(discoverable);
}


static int rokidbt_ble_send_buf(struct rk_bluetooth *rokid_bt, uint16_t uuid,  char *buf, int len)
{
    int ret;
    RKBluetooth_t *bt = (RKBluetooth_t *)rokid_bt->handle;
    BT_CHECK_HANDLE(bt);
    ret = rk_ble_send(bt->ble_ctx, uuid, (uint8_t*)buf, len);
    return ret;
}

static int rokidbt_ble_set_listener(struct rk_bluetooth *rokid_bt, ble_callbacks_t listener, void *listener_handle)
{
    RKBluetooth_t *bt = (RKBluetooth_t*)rokid_bt->handle;
    BT_CHECK_HANDLE(bt);
    return ble_server_set_listener(bt->ble_ctx, listener, listener_handle);
}
/* BLE FUNC END */

#if CONFIG_A2DP_ENABLE
/* A2DP SINK FUNC BEGIN*/
static int rokidbt_a2dp_sink_enable(struct rk_bluetooth *rokid_bt)
{
    RKBluetooth_t *bt = (RKBluetooth_t*)rokid_bt->handle;
    BT_CHECK_HANDLE(bt);
	return rk_a2dp_sink_enable(bt->a2dp_sink);
}

static int rokidbt_a2dp_sink_disable(struct rk_bluetooth *rokid_bt)
{
    RKBluetooth_t *bt = (RKBluetooth_t*)rokid_bt->handle;
    BT_CHECK_HANDLE(bt);

	return rk_a2dp_sink_disable(bt->a2dp_sink);
}

static int rokidbt_a2dp_sink_connect(struct rk_bluetooth *rokid_bt, BTAddr bd_addr)
{
    RKBluetooth_t *bt = (RKBluetooth_t*)rokid_bt->handle;
    BT_CHECK_HANDLE(bt);

	return rk_a2dp_sink_connect(bt->a2dp_sink, bd_addr);
}

static int rokidbt_a2dp_sink_disconnect(struct rk_bluetooth *rokid_bt, BTAddr bd_addr)
{
    RKBluetooth_t *bt = (RKBluetooth_t*)rokid_bt->handle;
    BT_CHECK_HANDLE(bt);

	return rk_a2dp_sink_disconnect(bt->a2dp_sink, bd_addr);
}

static int rokidbt_a2dp_sink_send_avrc_cmd(struct rk_bluetooth *rokid_bt, uint8_t cmd)
{
	RKBluetooth_t *bt = (RKBluetooth_t*)rokid_bt->handle;
    BT_CHECK_HANDLE(bt);

	return rk_a2dp_sink_send_avrc_cmd(bt->a2dp_sink, cmd);
}

static int rokidbt_a2dp_sink_send_get_playstatus(struct rk_bluetooth *rokid_bt)
{
	RKBluetooth_t *bt = (RKBluetooth_t*)rokid_bt->handle;
    BT_CHECK_HANDLE(bt);

	return rk_a2dp_sink_send_get_playstatus(bt->a2dp_sink);
}

static int rokidbt_a2dp_sink_get_playing(struct rk_bluetooth *rokid_bt)
{
	RKBluetooth_t *bt = (RKBluetooth_t*)rokid_bt->handle;
    BT_CHECK_HANDLE(bt);

	return rk_a2dp_sink_get_playing(bt->a2dp_sink);
}

static int rokidbt_a2dp_sink_set_listener(struct rk_bluetooth *rokid_bt, a2dp_sink_callbacks_t listener, void *listener_handle)
{
	RKBluetooth_t *bt = (RKBluetooth_t*)rokid_bt->handle;
    BT_CHECK_HANDLE(bt);

	return rk_a2dp_set_listener(bt->a2dp_sink, listener, listener_handle);
}

static int rokidbt_a2dp_sink_get_element_attrs(struct rk_bluetooth *rokid_bt)
{
	RKBluetooth_t *bt = (RKBluetooth_t*)rokid_bt->handle;
    BT_CHECK_HANDLE(bt);

	return rk_a2dp_sink_get_element_attrs(bt->a2dp_sink);
}

static int rokid_a2dp_sink_get_connected_devices(struct rk_bluetooth *rokid_bt, BTDevice *dev, int dev_cnt)
{
	RKBluetooth_t *bt = (RKBluetooth_t*)rokid_bt->handle;
    BT_CHECK_HANDLE(bt);

	return rk_a2dp_sink_get_connected_devices(bt->a2dp_sink, dev, dev_cnt);
}
/* A2DP SINK FUNC END*/
#endif
struct rk_bluetooth rk_bt_dev = {
	.open = rokidbt_open,
	.close = rokidbt_close,
	.create = rokidbt_create,
	.destroy = rokidbt_destroy,
	.set_bt_name = rokidbt_set_name,
#if CONFIG_CLASSIC_BT_ENABLED
	.set_visibility = rokidbt_set_visibility,
#endif

	.ble = {
		.enable = rokidbt_ble_enable,
		.disable = rokidbt_ble_disable,
		.set_listener = rokidbt_ble_set_listener,
		.send_buf = rokidbt_ble_send_buf,
		.set_ble_visibility = rokidbt_ble_set_visibility,
	},

#if CONFIG_A2DP_ENABLE
	.a2dp_sink = {
		.enable = rokidbt_a2dp_sink_enable,
		.disable = rokidbt_a2dp_sink_disable,
		.connect = rokidbt_a2dp_sink_connect,
		.disconnect = rokidbt_a2dp_sink_disconnect,
		.send_avrc_cmd = rokidbt_a2dp_sink_send_avrc_cmd,
		.set_listener = rokidbt_a2dp_sink_set_listener,
		.send_get_playstatus = rokidbt_a2dp_sink_send_get_playstatus,
		.get_playing = rokidbt_a2dp_sink_get_playing,
		.get_element_attrs = rokidbt_a2dp_sink_get_element_attrs,
		.get_connected_devices = rokid_a2dp_sink_get_connected_devices,

	},
#endif
};
#endif

