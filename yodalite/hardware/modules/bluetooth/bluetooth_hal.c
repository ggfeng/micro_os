/*************************************************************************
        > File Name: bluetooth.c
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>
#include <hardware/hardware.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include <hardware/bt/bluetooth.h>

#define MODULE_NAME "bluetooth"
#define MODULE_AUTHOR "guoliang.hou@rokid.com"

static struct rk_bluetooth rk_bt_dev;

int rk_bluetooth_device_init(struct rk_bluetooth *bt_dev)
{
	if (!bt_dev) {
		printf("Invalid bt_dev!\n");
		return -EINVAL;
	}
	
	memset(&rk_bt_dev, 0, sizeof(struct rk_bluetooth));
    rk_bt_dev.set_manage_listener = bt_dev->set_manage_listener;
    rk_bt_dev.set_discovery_listener = bt_dev->set_discovery_listener;
    rk_bt_dev.open = bt_dev->open;
    rk_bt_dev.close = bt_dev->close;
	rk_bt_dev.create = bt_dev->create;
	rk_bt_dev.destroy = bt_dev->destroy;
    rk_bt_dev.set_bt_name = bt_dev->set_bt_name;
    rk_bt_dev.start_discovery = bt_dev->start_discovery;
    rk_bt_dev.cancel_discovery = bt_dev->cancel_discovery;
    rk_bt_dev.remove_dev = bt_dev->remove_dev;
    rk_bt_dev.set_visibility = bt_dev->set_visibility;
    rk_bt_dev.get_disc_devices = bt_dev->get_disc_devices;
    rk_bt_dev.get_bonded_devices = bt_dev->get_bonded_devices;
    rk_bt_dev.find_scaned_device = bt_dev->find_scaned_device;
    rk_bt_dev.find_bonded_device = bt_dev->find_bonded_device;
    rk_bt_dev.get_module_addr = bt_dev->get_module_addr;
    rk_bt_dev.config_clear = bt_dev->config_clear;

    rk_bt_dev.ble.set_listener = bt_dev->ble.set_listener;
    rk_bt_dev.ble.enable = bt_dev->ble.enable;
    rk_bt_dev.ble.disable = bt_dev->ble.disable;
    rk_bt_dev.ble.send_buf = bt_dev->ble.send_buf;;
    rk_bt_dev.ble.set_ble_visibility = bt_dev->ble.set_ble_visibility;;

    rk_bt_dev.a2dp.set_listener = bt_dev->a2dp.set_listener;
    rk_bt_dev.a2dp.enable = bt_dev->a2dp.enable;
    rk_bt_dev.a2dp.disable = bt_dev->a2dp.disable;
    rk_bt_dev.a2dp.connect = bt_dev->a2dp.connect;
    rk_bt_dev.a2dp.disconnect = bt_dev->a2dp.disconnect;
    rk_bt_dev.a2dp.start = bt_dev->a2dp.start;
    rk_bt_dev.a2dp.send_avrc_cmd = bt_dev->a2dp.send_avrc_cmd;
    rk_bt_dev.a2dp.get_connected_devices = bt_dev->a2dp.get_connected_devices;

    rk_bt_dev.a2dp_sink.set_listener = bt_dev->a2dp_sink.set_listener;
    rk_bt_dev.a2dp_sink.enable = bt_dev->a2dp_sink.enable;
    rk_bt_dev.a2dp_sink.disable = bt_dev->a2dp_sink.disable;
    rk_bt_dev.a2dp_sink.connect = bt_dev->a2dp_sink.connect;
    rk_bt_dev.a2dp_sink.disconnect = bt_dev->a2dp_sink.disconnect;
    rk_bt_dev.a2dp_sink.send_avrc_cmd = bt_dev->a2dp_sink.send_avrc_cmd;
    rk_bt_dev.a2dp_sink.send_get_playstatus = bt_dev->a2dp_sink.send_get_playstatus;
    rk_bt_dev.a2dp_sink.get_playing = bt_dev->a2dp_sink.get_playing;
    rk_bt_dev.a2dp_sink.get_element_attrs = bt_dev->a2dp_sink.get_element_attrs;
    rk_bt_dev.a2dp_sink.get_connected_devices = bt_dev->a2dp_sink.get_connected_devices;
    rk_bt_dev.a2dp_sink.set_mute = bt_dev->a2dp_sink.set_mute;
    rk_bt_dev.a2dp_sink.set_abs_vol = bt_dev->a2dp_sink.set_abs_vol;

    rk_bt_dev.hfp.set_listener = bt_dev->hfp.set_listener;
    rk_bt_dev.hfp.enable = bt_dev->hfp.enable;
    rk_bt_dev.hfp.disable = bt_dev->hfp.disable;
    rk_bt_dev.hfp.connect = bt_dev->hfp.connect;
    rk_bt_dev.hfp.disconnect = bt_dev->hfp.disconnect;
    rk_bt_dev.hfp.answercall = bt_dev->hfp.answercall;
    rk_bt_dev.hfp.hangup = bt_dev->hfp.hangup;
    rk_bt_dev.hfp.dial_num = bt_dev->hfp.dial_num;
    rk_bt_dev.hfp.dial_last_num = bt_dev->hfp.dial_last_num;
    rk_bt_dev.hfp.send_dtmf = bt_dev->hfp.send_dtmf;
    rk_bt_dev.hfp.set_volume = bt_dev->hfp.set_volume;
    rk_bt_dev.hfp.mute_mic = bt_dev->hfp.mute_mic;
    rk_bt_dev.hfp.type = bt_dev->hfp.type;
}

static int rk_bt_create(struct rk_bluetooth *bt)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.create) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.create(bt);
	}
	return ret;
}

static int rk_bt_destroy(struct rk_bluetooth *bt)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.destroy) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.destroy(bt);
	}
	return ret;
}

static int rk_bluetooth_open(struct rk_bluetooth *bt, const char *name)
{
    int ret = BT_STATUS_NOT_READY;

	if (rk_bt_dev.open) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.open(bt, name);
	}
    return ret;
}

static int rk_bluetooth_close(struct rk_bluetooth *bt)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.close){
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.close(bt);
	}
    return ret;
}

static int rk_bluetooth_get_module_addr(struct rk_bluetooth *bt, BTAddr addr)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.get_module_addr) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.get_module_addr(bt, addr);
	}
    return ret;
}

static int rk_bluetooth_set_manage_listener(struct rk_bluetooth *bt, manage_callbacks_t m_cb, void *listener_handle)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.set_manage_listener) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.set_manage_listener(bt, m_cb, listener_handle);
	}
    return ret;
}

static int rk_bluetooth_set_discovery_listener(struct rk_bluetooth *bt, discovery_cb_t dis_cb, void *listener_handle)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.set_discovery_listener) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.set_discovery_listener(bt, dis_cb, listener_handle);
	}
	return ret;
}

static int rk_bluetooth_set_bt_name(struct rk_bluetooth *bt, const char *name)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.set_bt_name) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.set_bt_name(bt, name);
	}
	return ret;
}

static int rk_bluetooth_start_discovery(struct rk_bluetooth *bt, enum bt_profile_type type)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.start_discovery) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.start_discovery(bt, type);
	}
	return ret;
}

static int rk_bluetooth_cancel_discovery(struct rk_bluetooth *bt)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.cancel_discovery) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.cancel_discovery(bt);
	}
	return ret;
}

static int rk_bluetooth_remove_device(struct rk_bluetooth *bt, BTAddr addr)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.remove_dev) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.remove_dev(bt, addr);
	}
	return ret;
}

static int rk_bluetooth_set_visibility(struct rk_bluetooth *bt, bool discoverable, bool connectable)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.set_visibility) {
		BT_CHECK_HANDLE(bt);
		ret =  rk_bt_dev.set_visibility(bt, discoverable, connectable);
	}
	return ret;
}

static int rk_bluetooth_get_disc_devices(struct rk_bluetooth *bt, BTDevice *dev_array, int arr_len)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.get_disc_devices) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.get_disc_devices(bt, dev_array, arr_len);
	}
	return ret;
}

static int rk_bluetooth_find_scaned_device(struct rk_bluetooth *bt, BTDevice *dev)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.find_scaned_device) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.find_scaned_device(bt, dev);
	}
	return ret;
}
static int rk_bluetooth_get_bonded_devices(struct rk_bluetooth *bt, BTDevice *dev_array, int arr_len)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.get_bonded_devices) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.get_bonded_devices(bt, dev_array, arr_len);
	}
	return ret;
}

static int rk_bluetooth_find_bonded_device(struct rk_bluetooth *bt, BTDevice *dev)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.find_bonded_device) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.find_bonded_device(bt, dev);
	}
	return ret;
}

static int rk_bluetooth_config_clear(struct rk_bluetooth *bt)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.config_clear) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.config_clear(bt);
	}
	return ret;
}

/* BLE FUNC BEGIN */
static int rk_bluetooth_ble_enable(struct rk_bluetooth *bt)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.ble.enable) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.ble.enable(bt);
	}
    return ret;
}

static int rk_bluetooth_ble_disable(struct rk_bluetooth *bt)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.ble.disable) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.ble.disable(bt);
	}
    return ret;
}

static int rk_bluetooth_ble_send_buf(struct rk_bluetooth *bt, uint16_t uuid,  char *buf, int len)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.ble.send_buf) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.ble.send_buf(bt, uuid, buf, len);
	}
    return ret;
}

static int rk_bluetooth_ble_set_listener(struct rk_bluetooth *bt, ble_callbacks_t cb, void *listener_handle)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.ble.set_listener) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.ble.set_listener(bt, cb, listener_handle);
	}
    return ret;
}

static int rk_bluetooth_ble_set_visibility(struct rk_bluetooth *bt, bool discoverable, bool connectable)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.ble.set_ble_visibility) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.ble.set_ble_visibility(bt, discoverable, connectable);
	}
	return ret;
}
/* BLE FUNC END */

/* A2DP FUNC BEGIN*/
static int rk_bluetooth_a2dp_set_listener(struct rk_bluetooth *bt, a2dp_callbacks_t listener, void *listener_handle)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.a2dp.set_listener) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.a2dp.set_listener(bt, listener, listener_handle);
	}
	return ret;
}

static int rk_bluetooth_a2dp_enable(struct rk_bluetooth *bt)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.a2dp.enable) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.a2dp.enable(bt);
	}
	return ret;
}

static int rk_bluetooth_a2dp_disable(struct rk_bluetooth *bt)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.a2dp.disable) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.a2dp.disable(bt);
	}
	return ret;
}

static int rk_bluetooth_a2dp_connect(struct rk_bluetooth *bt, BTAddr bd_addr)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.a2dp.connect) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.a2dp.connect(bt, bd_addr);
	}
	return ret;
}

static int rk_bluetooth_a2dp_disconnect(struct rk_bluetooth *bt, BTAddr bd_addr)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.a2dp.disconnect) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.a2dp.disconnect(bt, bd_addr);
	}
	return ret;
}

static int rk_bluetooth_a2dp_start(struct rk_bluetooth *bt, BT_A2DP_CODEC_TYPE type)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.a2dp.start) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.a2dp.start(bt, type);
	}
	return ret;
}

static int rk_bluetooth_a2dp_send_avrc_cmd(struct rk_bluetooth *bt, uint8_t command)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.a2dp.send_avrc_cmd) {
		BT_CHECK_HANDLE(bt);
		printf("send avrc cmd %d\n", command);
		ret = rk_bt_dev.a2dp.send_avrc_cmd(bt, command);
	}
	return ret;
}

static int rk_bluetooth_a2dp_get_connected_devices(struct rk_bluetooth *bt, BTDevice *dev_array, int arr_len)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.a2dp.get_connected_devices) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.a2dp.get_connected_devices(bt, dev_array, arr_len);
	}
	return ret;
}
/* A2DP FUNC END */

/* A2DP SINK FUNC BEGIN*/
static int rk_bluetooth_a2dp_sink_set_listener(struct rk_bluetooth *bt, a2dp_sink_callbacks_t listener, void *listener_handle)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.a2dp_sink.set_listener) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.a2dp_sink.set_listener(bt, listener, listener_handle);
	}
	return ret;
}

static int rk_bluetooth_a2dp_sink_enable(struct rk_bluetooth *bt)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.a2dp_sink.enable) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.a2dp_sink.enable(bt);
	}
	return ret;
}

static int rk_bluetooth_a2dp_sink_connect(struct rk_bluetooth *bt, BTAddr bd_addr)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.a2dp_sink.connect) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.a2dp_sink.connect(bt, bd_addr);
	}
	return ret;
}

static int rk_bluetooth_a2dp_sink_disconnect(struct rk_bluetooth *bt, BTAddr bd_addr)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.a2dp_sink.disconnect) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.a2dp_sink.disconnect(bt, bd_addr);
	}
	return ret;
}

static int rk_bluetooth_a2dp_sink_disable(struct rk_bluetooth *bt)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.a2dp_sink.disable) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.a2dp_sink.disable(bt);
	}
	return ret;
}

static int rk_bluetooth_a2dp_sink_send_get_playstatus(struct rk_bluetooth *bt)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.a2dp_sink.send_get_playstatus) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.a2dp_sink.send_get_playstatus(bt);
	}
	return ret;
}

static int rk_bluetooth_a2dp_sink_send_avrc_cmd(struct rk_bluetooth *bt, uint8_t command)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.a2dp_sink.send_avrc_cmd) {
		BT_CHECK_HANDLE(bt);
		printf("send avrc cmd %d\n", command);
		ret = rk_bt_dev.a2dp_sink.send_avrc_cmd(bt, command);
	}
	return ret;
}

static int rk_bluetooth_a2dp_sink_get_playing(struct rk_bluetooth *bt)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.a2dp_sink.get_playing) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.a2dp_sink.get_playing(bt);
	}
	return ret;
}

static int rk_bluetooth_a2dp_sink_get_element_attrs(struct rk_bluetooth *bt)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.a2dp_sink.get_element_attrs) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.a2dp_sink.get_element_attrs(bt);
	}
	return ret;
}

static int rk_bluetooth_a2dp_sink_get_connected_devices(struct rk_bluetooth *bt, BTDevice *dev_array, int arr_len)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.a2dp_sink.get_connected_devices) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.a2dp_sink.get_connected_devices(bt, dev_array, arr_len);
	}
	return ret;
}

static int rk_bluetooth_a2dp_sink_set_mute(struct rk_bluetooth *bt, bool mute)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.a2dp_sink.set_mute) {
		BT_CHECK_HANDLE(bt);
		printf("set mute %d\n", mute);
		ret = rk_bt_dev.a2dp_sink.set_mute(bt, mute);
	}
	return ret;
}

static int rk_bluetooth_a2dp_sink_set_abs_vol(struct rk_bluetooth *bt, uint8_t vol)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.a2dp_sink.set_abs_vol) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.a2dp_sink.set_abs_vol(bt, vol);
	}
	return ret;
}
/* A2DP SINK FUNC END */

/* HFP FUNC BEGIN*/
static int rk_bluetooth_hfp_set_listener(struct rk_bluetooth *bt, hfp_callbacks_t listener, void *listener_handle)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.hfp.set_listener) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.hfp.set_listener(bt, listener, listener_handle);
	}
	return ret;
}

static int rk_bluetooth_hfp_enable(struct rk_bluetooth *bt)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.hfp.enable) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.hfp.enable(bt);
	}
	return ret;
}

static int rk_bluetooth_hfp_disable(struct rk_bluetooth *bt)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.hfp.disable) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.hfp.disable(bt);
	}
	return ret;
}

static int rk_bluetooth_hfp_connect(struct rk_bluetooth *bt, BTAddr bd_addr)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.hfp.connect) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.hfp.connect(bt, bd_addr);
	}
	return ret;
}

static int rk_bluetooth_hfp_disconnect(struct rk_bluetooth *bt)
{
	int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.hfp.disconnect) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.hfp.disconnect(bt);
	}
	return ret;
}

static int rk_bluetooth_hfp_answercall(struct rk_bluetooth *bt)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.hfp.answercall) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.hfp.answercall(bt);
	}
	return ret;
}

static int rk_bluetooth_hfp_hangup(struct rk_bluetooth *bt)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.hfp.hangup) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.hfp.hangup(bt);
	}
	return ret;
}

static int rk_bluetooth_hfp_dial_num(struct rk_bluetooth *bt, const char *num)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.hfp.dial_num) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.hfp.dial_num(bt, num);
	}
	return ret;
}

static int rk_bluetooth_hfp_last_num_dial(struct rk_bluetooth *bt)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.hfp.dial_last_num) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.hfp.dial_last_num(bt);
	}
	return ret;
}

static int rk_bluetooth_hfp_send_dtmf(struct rk_bluetooth *bt, char dtmf)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.hfp.send_dtmf) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.hfp.send_dtmf(bt, dtmf);
	}
	return ret;
}

static int rk_bluetooth_hfp_set_volume(struct rk_bluetooth *bt, BT_HS_VOLUME_TYPE type, int volume)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.hfp.set_volume) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.hfp.set_volume(bt, type, volume);
	}
	return ret;
}

static int rk_bluetooth_hfp_mute_mic(struct rk_bluetooth *bt, bool mute)
{
    int ret = BT_STATUS_NOT_READY;
	if (rk_bt_dev.hfp.mute_mic) {
		BT_CHECK_HANDLE(bt);
		ret = rk_bt_dev.hfp.mute_mic(bt, mute);
	}
	return ret;
}
/* HFP FUNC END*/

int rk_bluetooth_creat(struct rk_bluetooth **bt)
{
    int ret = BT_STATUS_NOT_READY;
    struct rk_bluetooth *rk_dev =
            calloc(1, sizeof(struct rk_bluetooth));

    *bt = NULL;
    if (!rk_dev) {
		printf("Can not allocate memory for the rk_dev\n");
        return -BT_STATUS_NOMEM;
    }
    ret = rk_bt_create(rk_dev);
    if (ret != BT_STATUS_SUCCESS) {
        ret = -BT_STATUS_FAIL;
       goto creat_err;
    }
    rk_dev->set_manage_listener = rk_bluetooth_set_manage_listener;
    rk_dev->set_discovery_listener = rk_bluetooth_set_discovery_listener;
    rk_dev->open = rk_bluetooth_open;
    rk_dev->close = rk_bluetooth_close;
    rk_dev->set_bt_name = rk_bluetooth_set_bt_name;
    rk_dev->start_discovery = rk_bluetooth_start_discovery;
    rk_dev->cancel_discovery = rk_bluetooth_cancel_discovery;
    rk_dev->remove_dev = rk_bluetooth_remove_device;
    rk_dev->set_visibility = rk_bluetooth_set_visibility;
    rk_dev->get_disc_devices = rk_bluetooth_get_disc_devices;
    rk_dev->get_bonded_devices = rk_bluetooth_get_bonded_devices;
    rk_dev->find_scaned_device = rk_bluetooth_find_scaned_device;
    rk_dev->find_bonded_device = rk_bluetooth_find_bonded_device;
    rk_dev->get_module_addr = rk_bluetooth_get_module_addr;
    rk_dev->config_clear = rk_bluetooth_config_clear;

    rk_dev->ble.set_listener = rk_bluetooth_ble_set_listener;
    rk_dev->ble.enable = rk_bluetooth_ble_enable;
    rk_dev->ble.disable = rk_bluetooth_ble_disable;
    rk_dev->ble.send_buf = rk_bluetooth_ble_send_buf;
    rk_dev->ble.set_ble_visibility = rk_bluetooth_ble_set_visibility;

    rk_dev->a2dp.set_listener = rk_bluetooth_a2dp_set_listener;
    rk_dev->a2dp.enable = rk_bluetooth_a2dp_enable;
    rk_dev->a2dp.disable = rk_bluetooth_a2dp_disable;
    rk_dev->a2dp.connect = rk_bluetooth_a2dp_connect;
    rk_dev->a2dp.disconnect = rk_bluetooth_a2dp_disconnect;
    rk_dev->a2dp.start = rk_bluetooth_a2dp_start;
    rk_dev->a2dp.send_avrc_cmd = rk_bluetooth_a2dp_send_avrc_cmd;
    rk_dev->a2dp.get_connected_devices = rk_bluetooth_a2dp_get_connected_devices;

    rk_dev->a2dp_sink.set_listener = rk_bluetooth_a2dp_sink_set_listener;
    rk_dev->a2dp_sink.enable = rk_bluetooth_a2dp_sink_enable;
    rk_dev->a2dp_sink.disable = rk_bluetooth_a2dp_sink_disable;
    rk_dev->a2dp_sink.connect = rk_bluetooth_a2dp_sink_connect;
    rk_dev->a2dp_sink.disconnect = rk_bluetooth_a2dp_sink_disconnect;
    rk_dev->a2dp_sink.send_avrc_cmd = rk_bluetooth_a2dp_sink_send_avrc_cmd;
    rk_dev->a2dp_sink.send_get_playstatus = rk_bluetooth_a2dp_sink_send_get_playstatus;
    rk_dev->a2dp_sink.get_playing = rk_bluetooth_a2dp_sink_get_playing;
    rk_dev->a2dp_sink.get_element_attrs = rk_bluetooth_a2dp_sink_get_element_attrs;
    rk_dev->a2dp_sink.get_connected_devices = rk_bluetooth_a2dp_sink_get_connected_devices;
    rk_dev->a2dp_sink.set_mute = rk_bluetooth_a2dp_sink_set_mute;
    rk_dev->a2dp_sink.set_abs_vol = rk_bluetooth_a2dp_sink_set_abs_vol;

    rk_dev->hfp.set_listener = rk_bluetooth_hfp_set_listener;
    rk_dev->hfp.enable = rk_bluetooth_hfp_enable;
    rk_dev->hfp.disable = rk_bluetooth_hfp_disable;
    rk_dev->hfp.connect = rk_bluetooth_hfp_connect;
    rk_dev->hfp.disconnect = rk_bluetooth_hfp_disconnect;
    rk_dev->hfp.answercall = rk_bluetooth_hfp_answercall;
    rk_dev->hfp.hangup = rk_bluetooth_hfp_hangup;
    rk_dev->hfp.dial_num = rk_bluetooth_hfp_dial_num;
    rk_dev->hfp.dial_last_num = rk_bluetooth_hfp_last_num_dial;
    rk_dev->hfp.send_dtmf = rk_bluetooth_hfp_send_dtmf;
    rk_dev->hfp.set_volume = rk_bluetooth_hfp_set_volume;
    rk_dev->hfp.mute_mic = rk_bluetooth_hfp_mute_mic;
    rk_dev->hfp.type = rk_bt_dev.hfp.type;

    *bt = rk_dev;
    return 0;

creat_err:
    if (rk_dev)
        free(rk_dev);
    return ret;
}

int rk_bluetooth_destroy(struct rk_bluetooth *bt)
{
	int ret = BT_STATUS_SUCCESS;
    if(bt) {
		ret = rk_bt_destroy(bt);
		if (ret != BT_STATUS_SUCCESS) {
			printf("Destroy bluetooth failed!\n");
			return ret;
		}	
		free(bt);
    }

    return ret;
}

static int bluetooth_device_close(struct hw_device_t* dev)
{
    struct bluetooth_device_t* btdev = (struct bluetooth_device_t*)dev;
    if (btdev) {
        free(btdev);
    }
    return 0;
}

static int bluetooth_device_open(const struct hw_module_t* module,
    const char* name, struct hw_device_t** device)
{
    struct bluetooth_device_t *btdev =
            calloc(1, sizeof(struct bluetooth_device_t));

    if (!btdev) {
        printf("Can not allocate memory for the bluetooth device\n");
        return -BT_STATUS_NOMEM;
    }

    btdev->common.tag = HARDWARE_DEVICE_TAG;
    btdev->common.module = (hw_module_t *) module;
    btdev->common.version = HARDWARE_DEVICE_API_VERSION(1, 0);
    btdev->common.close = bluetooth_device_close;

    btdev->creat = rk_bluetooth_creat;
    btdev->destroy= rk_bluetooth_destroy;

    *device = &btdev->common;
    return 0;
}

static struct hw_module_methods_t bluetooth_module_methods = {
    .open = bluetooth_device_open,
};

struct bluetooth_module_t HAL_MODULE_INFO_SYM(BT_HAL_HW_ID) = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 1,
        .version_minor = 0,
        .id = BLUETOOTH_HARDWARE_MODULE_ID,
        .name = MODULE_NAME,
        .author = MODULE_AUTHOR,
        .methods = &bluetooth_module_methods,
    },
};
