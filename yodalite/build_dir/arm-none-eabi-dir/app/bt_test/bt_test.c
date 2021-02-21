#include <stdlib.h>
#include <string.h>
#include <yodalite_autoconf.h>
#include <hardware/bt/bluetooth.h>
#include <lib/shell/shell.h>

static struct bluetooth_device_t *bt_dev = NULL;
static struct rk_bluetooth *bt = NULL;
static int create_bt_hal(void)
{
	struct bluetooth_module_t *bt_module = NULL;
	struct hw_module_t *module = NULL;
	struct hw_device_t *hw_dev = NULL;
	struct bluetooth_device_t *btdev;
	struct rk_bluetooth *rk_bt = NULL;

	hw_get_module(BT_HAL_HW_ID, bt_module);
	module = &bt_module->common;

	if (module->methods->open(module, BLUETOOTH_HARDWARE_MODULE_ID, &hw_dev)) {
		printf("Open bluetooth hal failed!\n");
		return -1;
	}
	btdev = (struct bluetooth_device_t*)hw_dev;
	if (btdev->creat(&rk_bt)) {
		printf("Create bluetooth failed!\n");
		return -1;
	}
	bt = rk_bt;
	bt_dev = btdev;
	return 0;
}

static int destroy_bt_hal(void)
{

	if (bt_dev) {
		if (bt_dev->destroy(bt)) {
			printf("Destroy bluetooth failed!\n");
			return -1;
		}
		if (bt_dev->common.close(&bt_dev->common)) {
			printf("Close bt device failed!\n");
			return -1;
		}
	}
	return 0;
}

static void a2dp_sink_listener(void *caller, BT_A2DP_SINK_EVT event, void *data)
{
	struct rk_bluetooth *bt = caller;
	BT_AVK_MSG *msg = data;

	switch (event) {
		case BT_AVK_OPEN_EVT:
			if (msg) {
				printf("BT_AVK_OPEN_EVT : status= %d, idx %d, name: %s , addr %02x:%02x:%02x:%02x:%02x:%02x\n",
						 msg->sig_chnl_open.status, msg->sig_chnl_open.idx, msg->sig_chnl_open.name,
						 msg->sig_chnl_open.bd_addr[0], msg->sig_chnl_open.bd_addr[1],
						 msg->sig_chnl_open.bd_addr[2], msg->sig_chnl_open.bd_addr[3],
						 msg->sig_chnl_open.bd_addr[4], msg->sig_chnl_open.bd_addr[5]);
			}
			break;
		case BT_AVK_CLOSE_EVT:
			if (msg) {
				printf("BT_AVK_CLOSE_EVT : status= %d, idx %d\n", msg->sig_chnl_close.status, msg->sig_chnl_close.idx);
			}
			break;

		case BT_AVK_STR_OPEN_EVT:
			if (msg) {
				printf("BT_AVK_STR_OPEN_EVT: status = %d\n", msg->stream_chnl_open.status);
			}
			break;
		
		case BT_AVK_START_EVT:
			if (msg) {
				printf("BT_AVK_START_EVT : status= %d, idx %d\n", msg->start_streaming.status, msg->start_streaming.idx);
			}
			break;

		case BT_AVK_STOP_EVT:
			if (msg) {
				printf("BT_AVK_STOP_EVT : status= %d, idx %d, suspended=%d\n", msg->stop_streaming.status, msg->stop_streaming.idx, msg->stop_streaming.suspended);
			}
			break;

		case BT_AVK_RC_OPEN_EVT:
			if (msg) {
				printf("BT_AVK_RC_OPEN_EVT : status= %d, idx %d\n", msg->rc_open.status, msg->rc_open.idx);
			}
			break;

		case BT_AVK_RC_CLOSE_EVT:
			if (msg) {
				printf("BT_AVK_RC_CLOSE_EVT : status= %d, idx %d\n", msg->rc_close.status, msg->rc_close.idx);
			}
			break;

		case BT_AVK_GET_ELEM_ATTR_EVT:
			if (msg) {
				printf("BT_AVK_GET_ELEM_ATTR_EVT: element attribute id %d is %s\n", 
						msg->elem_attr.attr_entry[0].attr_id, msg->elem_attr.attr_entry[0].data);
			}
			break;
		case BT_AVK_GET_PLAY_STATUS_EVT:
			if (msg) {
				printf("BT_AVK_GET_PLAY_STATUS_EVT : play_status= %d\n", msg->get_play_status.play_status);
			}
			break;
		default:
			break;
	}

}

static void ble_listener(void *caller, BT_BLE_EVT event, void *data) {
    struct rk_bluetooth *bt = caller;
    BT_BLE_MSG *msg = data;
	uint32_t i;
	uint32_t len;
	
    switch (event) {
    case BT_BLE_SE_WRITE_EVT:
		printf("Write event\n");
		printf("Write uuid %d, len %d, value :", msg->ser_write.uuid, msg->ser_write.len);
		len = msg->ser_write.len;
		for (i = 0; i < len; i++) {
			printf("0x%x ", msg->ser_write.value[i]);
		}
		printf("\n");
		printf("Write offset = %d, status = %d\n", msg->ser_write.offset, msg->ser_write.status);
		break;
	case BT_BLE_SE_OPEN_EVT:
		printf("open connect event\n");
		printf("Connect id = %d\n", msg->ser_open.conn_id);
		break;
	case BT_BLE_SE_CLOSE_EVT:
		printf("close disconnect event\n");
		printf("disconnect id = %d, reason is %d\n", msg->ser_close.conn_id, msg->ser_close.reason);
		break;
	default:
		break;
	}
}

static int bt_open(int argc, int8_t *const argv[])
{
	int8_t name[16];

	if (argc < 2) {
		printf("Need more args such as: ble_open name_test\n");
		return -1;
	}

	if (create_bt_hal()) {
		printf("Create bt hal failed!\n");
		return -1;
	}
	strcpy(name, argv[1]);
	bt->open(bt, name);
	return 0;
}

static int bt_close(int argc, int8_t *const argv[])
{
	if (!bt) {
		printf("bt hal has not been opened!\n");
		return -1;
	}

	bt->close(bt);
	if (destroy_bt_hal()) {
		printf("Destroy bt hal failed!\n");
		return -1;
	}
	return 0;
}
/* ble test */
static int ble_open(int argc, int8_t *const argv[])
{
	if (!bt) {
		printf("bt hal has not been opened!\n");
		return -1;
	}
	bt->ble.set_listener(bt, ble_listener, bt);
	bt->ble.enable(bt);

	return 0;
}

static int ble_close(int argc, int8_t *const argv[])
{
	if (!bt) {
		printf("bt hal has not been opened!\n");
		return -1;
	}
	bt->ble.disable(bt);
	return 0;
}

static int ble_set_visibility(int argc, int8_t *const argv[])
{
	char *enable = NULL;
	if (argc < 2) {
		printf("Need more args such as: ble_set_visibility enable|disable\n");
		return -1;
	}
	enable = argv[1];
	if (bt) {
		if (!strcmp("enable", enable)) {
			bt->ble.set_ble_visibility(bt, 1, 1);
		} else if (!strcmp("disable", enable)) {
			bt->ble.set_ble_visibility(bt, 0, 0);
		}
	} else {
		printf("bt is not opened!\n");
	}
	return 0;
}

static int ble_send_buf(int argc, int8_t *const argv[])
{
	uint16_t uuid;
	char *data = NULL;
	if (argc < 3) {
		printf("Need more args such as: ble_set_visibility enable\n");
		return -1;
	}
	uuid = (uint16_t)strtoul(argv[1], NULL, 16);
	data = argv[2];
	if (bt) {
		bt->ble.send_buf(bt, uuid, data, strlen(data));
	} else {
		printf("bt is not opened!\n");
	}
	return 0;
}
/* ble test end */

/* a2dp test */
static int a2dpk_enable(int argc, int8_t *const argv[])
{
	if (!bt) {
		printf("bt hal has not been opened!\n");
		return -1;
	}
//	bt->a2dp_sink.set_listener(bt, ble_listener, bt);
	bt->a2dp_sink.set_listener(bt, a2dp_sink_listener, bt);
	bt->a2dp_sink.enable(bt);

	return 0;
}

static int a2dpk_disable(int argc, int8_t *const argv[])
{
	if (!bt) {
		printf("bt hal has not been opened!\n");
		return -1;
	}
	bt->a2dp_sink.disable(bt);
	return 0;
}

static void a2dpk_avrc_cmd_usage(void)
{
	printf("a2dp avrc cmd list:\n");
	printf("1. play\n");
	printf("2. pause\n");
	printf("3. stop\n");
	printf("4. forward\n");
	printf("5. backward\n");
	printf("6. fast forward\n");
}

static int a2dpk_send_avrc_cmd(int argc, int8_t *const argv[])
{
	int cmd = 0;
	if (!bt) {
		printf("bt hal has not been opened!\n");
		return -1;
	}
	if (argc < 2) {
		a2dpk_avrc_cmd_usage();
		return -1;
	}
	cmd = atoi(argv[1]);
	switch (cmd) {
		case 1:
			bt->a2dp_sink.send_avrc_cmd(bt, BT_AVRC_PLAY);
			break;
		case 2:
			bt->a2dp_sink.send_avrc_cmd(bt, BT_AVRC_PAUSE);
			break;
		case 3:
			bt->a2dp_sink.send_avrc_cmd(bt, BT_AVRC_STOP);
			break;
		case 4:
			bt->a2dp_sink.send_avrc_cmd(bt, BT_AVRC_FORWARD);
			break;
		case 5:
			bt->a2dp_sink.send_avrc_cmd(bt, BT_AVRC_BACKWARD);
			break;
		case 6:
			bt->a2dp_sink.send_avrc_cmd(bt, BT_AVRC_FAST_FOR);
			break;
		default:
			printf("Invalid cmd!\n");
			break;
	}
	return 0;
}

static int bd_strtoba(uint8_t *addr, const char *address) {
    int i;
    int len = strlen(address);
    char *dest = (char *)addr;
    char *begin = (char *)address;
    char *end_ptr;

    if (!address || !addr || len != 17) {
        printf("faile to addr:%s, len:%d\n", address, len);
        return -1;
    }
    for (i = 0; i < 6; i++) {
        dest[i] = (char)strtoul(begin, &end_ptr, 16);
        if (!end_ptr) break;
        if (*end_ptr == '\0') break;
        if (*end_ptr != ':') {
            printf("faile to addr:%s, len:%d, end_ptr: %c, %s\n", address, len, *end_ptr, end_ptr);
            return -1;
        }
        begin = end_ptr +1;
        end_ptr = NULL;
    }
    if (i != 5) {
        printf("faile to addr:%s, len:%d\n", address, len);
        return -1;
    }
    printf("%02X:%02X:%02X:%02X:%02X:%02X\n",
           dest[0], dest[1], dest[2], dest[3], dest[4], dest[5]);
    return 0;
}

static int a2dpk_connect(int argc, int8_t *const argv[])
{
	uint8_t bda[6];

	if (!bt) {
		printf("bt hal has not been opened!\n");
		return -1;
	}

	if (argc < 2) {
		printf("Too less args!\n");
		return -1;
	}
	if (bd_strtoba(bda, argv[1]) != 0) {
		printf("Invalid bt mac!\n");
		return -1;
	}
	if (bt->a2dp_sink.connect(bt, bda) != 0) {
		printf("Connect to %s failed!\n", argv[1]);
		return -1;
	}
	return 0;
}

static int a2dpk_disconnect(int argc, int8_t *const argv[])
{
	uint8_t bda[6];

	if (!bt) {
		printf("bt hal has not been opened!\n");
		return -1;
	}

	if (argc < 2) {
		printf("Too less args!\n");
		return -1;
	}
	if (bd_strtoba(bda, argv[1]) != 0) {
		printf("Invalid bt mac!\n");
		return -1;
	}
	if (bt->a2dp_sink.disconnect(bt, bda) != 0) {
		printf("disconnect to %s failed!\n", argv[1]);
		return -1;
	}
	return 0;
}

static int a2dpk_send_get_playstatus(int argc, int8_t *const argv[])
{
	if (!bt) {
		printf("bt hal has not been opened!\n");
		return -1;
	}
	bt->a2dp_sink.send_get_playstatus(bt);
	return 0;
}

static int a2dpk_get_playing(int argc, int8_t *const argv[])
{
	int playing;
	if (!bt) {
		printf("bt hal has not been opened!\n");
		return -1;
	}
	playing = bt->a2dp_sink.get_playing(bt);
	printf("Playing status is %d!\n", playing);
	return 0;
}

static int a2dpk_get_elem_attrs(int argc, int8_t *const argv[])
{
	if (!bt) {
		printf("bt hal has not been opened!\n");
		return -1;
	}
	bt->a2dp_sink.get_element_attrs(bt);
	return 0;
}

static int a2dpk_get_connected_devices(int argc, int8_t *const argv[])
{
	BTDevice dev = {0};
	if (!bt) {
		printf("bt hal has not been opened!\n");
		return -1;
	}
	bt->a2dp_sink.get_connected_devices(bt, &dev, 1);
	printf("Connect device %x:%x:%x:%x:%x:%x\n", dev.bd_addr[0], dev.bd_addr[1],
			dev.bd_addr[2], dev.bd_addr[3], dev.bd_addr[4], dev.bd_addr[5]);
	return 0;
}

/* a2dp test end */

#define max_btopen_args        (2)
#define bt_open_help     "bt_open name"

static int cmd_bt_open(void)
{

  YODALITE_REG_CMD(bt_open,max_btopen_args,bt_open,bt_open_help);

  return 0;
}

#define max_btclose_args        (2)
#define bt_close_help     "bt_close"

static int cmd_bt_close(void)
{

  YODALITE_REG_CMD(bt_close,max_btclose_args,bt_close,bt_close_help);

  return 0;
}

#define max_bleopen_args        (1)
#define ble_open_help     "ble_open"

static int cmd_ble_open(void)
{

  YODALITE_REG_CMD(ble_open,max_bleopen_args,ble_open,ble_open_help);

  return 0;
}

#define max_bleclose_args        (1)
#define ble_close_help     "ble_close"

static int cmd_ble_close(void)
{

  YODALITE_REG_CMD(ble_close, max_bleclose_args,ble_close,ble_close_help);

  return 0;
}

#define max_blesetvisi_args        (2)
#define ble_set_visibility_help     "ble_set_visibility enable|disable"

static int cmd_ble_set_visibility(void)
{

  YODALITE_REG_CMD(ble_set_visibility, max_blesetvisi_args, ble_set_visibility, ble_set_visibility_help);

  return 0;
}

#define max_blesend_args        (3)
#define ble_send_buf_help     "ble_send_buf uuid data"

static int cmd_ble_send_buf(void)
{

  YODALITE_REG_CMD(ble_send_buf, max_blesend_args, ble_send_buf, ble_send_buf_help);

  return 0;
}

#define max_a2dpk_enable_args        (1)
#define a2dpk_enable_help     "a2dpk_enable"

static int cmd_a2dpk_enable(void)
{

  YODALITE_REG_CMD(a2dpk_enable, max_a2dpk_enable_args, a2dpk_enable, a2dpk_enable_help);

  return 0;
}

#define max_a2dpk_disable_args        (1)
#define a2dpk_disable_help     "a2dpk_disable"

static int cmd_a2dpk_disable(void)
{

  YODALITE_REG_CMD(a2dpk_disable, max_a2dpk_disable_args, a2dpk_disable, a2dpk_disable_help);

  return 0;
}

#define max_a2dpk_sendcmd_args        (2)
#define a2dpk_send_cmd_help     "a2dpk_send_cmd cmd"

static int cmd_a2dpk_send_cmd(void)
{

  YODALITE_REG_CMD(a2dpk_send_avrc_cmd, max_a2dpk_sendcmd_args, a2dpk_send_avrc_cmd, a2dpk_send_cmd_help);

  return 0;
}

#define max_a2dpk_connect_args        (2)
#define a2dpk_connect_help     "a2dpk_connect bda"

static int cmd_a2dpk_connect(void)
{

  YODALITE_REG_CMD(a2dpk_connect, max_a2dpk_connect_args, a2dpk_connect, a2dpk_connect_help);

  return 0;
}

#define max_a2dpk_disconnect_args        (2)
#define a2dpk_disconnect_help     "a2dpk_disconnect bda"

static int cmd_a2dpk_disconnect(void)
{

  YODALITE_REG_CMD(a2dpk_disconnect, max_a2dpk_disconnect_args, a2dpk_disconnect, a2dpk_disconnect_help);

  return 0;
}

#define max_a2dpk_send_getstatus_args        (1)
#define a2dpk_send_getstatus_help     "a2dpk_send_get_playstatus"

static int cmd_a2dpk_send_getstatus(void)
{

  YODALITE_REG_CMD(a2dpk_send_get_playstatus, max_a2dpk_send_getstatus_args, a2dpk_send_get_playstatus, a2dpk_send_getstatus_help);

  return 0;
}

#define max_a2dpk_get_playing_args        (1)
#define a2dpk_get_playing_help     "a2dpk_get_playing"

static int cmd_a2dpk_get_playing(void)
{

  YODALITE_REG_CMD(a2dpk_get_playing, max_a2dpk_get_playing_args, a2dpk_get_playing, a2dpk_get_playing_help);

  return 0;
}

#define max_a2dpk_get_eattrs_args        (1)
#define a2dpk_get_eattrs_help     "a2dpk_get_elem_attrs"

static int cmd_a2dpk_get_eattrs(void)
{

  YODALITE_REG_CMD(a2dpk_get_elem_attrs, max_a2dpk_get_eattrs_args, a2dpk_get_elem_attrs, a2dpk_get_eattrs_help);

  return 0;
}

#define max_a2dpk_get_devices_args        (1)
#define a2dpk_get_devices_help     "a2dpk_get_connected_devices"

static int cmd_a2dpk_get_devices(void)
{

  YODALITE_REG_CMD(a2dpk_get_connected_devices, max_a2dpk_get_devices_args, a2dpk_get_connected_devices, a2dpk_get_devices_help);

  return 0;
}


typedef int (*cmd_init_t)(void);

static cmd_init_t cmd_bt_tbl[]=
{
	cmd_bt_open,
	cmd_bt_close,
/* ble*/
	cmd_ble_open,
	cmd_ble_close,
	cmd_ble_set_visibility,
	cmd_ble_send_buf,
/* a2dp*/
	cmd_a2dpk_enable,
	cmd_a2dpk_disable,
	cmd_a2dpk_send_cmd,
	cmd_a2dpk_connect,
	cmd_a2dpk_disconnect,
	cmd_a2dpk_send_getstatus,
	cmd_a2dpk_get_playing,
	cmd_a2dpk_get_eattrs,
	cmd_a2dpk_get_devices,
	NULL
};

int add_bt_cmd(void) 
{
	cmd_init_t *cmd = &cmd_bt_tbl[0];
   while(*cmd != NULL)
   {
      (*cmd)();
      cmd ++;
   }
   return 0;
}

