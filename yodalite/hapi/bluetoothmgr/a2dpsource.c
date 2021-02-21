#include "common.h"


struct a2dpsource_handle *g_bt_a2dpsource = NULL;

void broadcast_a2dpsource_state(void *reply) {
    cJSON *root = cJSON_CreateObject();
    struct a2dpsource_state *a2dpsource_state = &g_bt_a2dpsource->state;
    struct bt_autoconnect_config *a2dpsource_config = &g_bt_a2dpsource->config;
    char *a2dpsource = NULL;
    char *conn_state = NULL;
    char *conn_name = NULL;
    char conn_addr[32] = {0};
    char *broadcast_state = NULL;
    char *re_state = NULL;
    char *action = "stateupdate";

    switch (a2dpsource_state->open_state) {
    case (A2DP_SOURCE_STATE_OPEN): {
        a2dpsource = "open";
        break;
    }
    case (A2DP_SOURCE_STATE_OPENED): {
        a2dpsource = "opened";
        break;
    }
    case (A2DP_SOURCE_STATE_OPENFAILED): {
        a2dpsource = "open failed";
        break;
    }
    case (A2DP_SOURCE_STATE_CLOSE): {
        a2dpsource = "close";
        break;
    }
    case (A2DP_SOURCE_STATE_CLOSING): {
        a2dpsource = "closing";
        break;
    }
    case (A2DP_SOURCE_STATE_CLOSED): {
        a2dpsource = "closed";
        break;
    }
    default:
        a2dpsource = "invalid";
        break;
    }

    switch (a2dpsource_state->connect_state) {
    case (A2DP_SOURCE_STATE_CONNECTED): {
        conn_state = "connected";
        conn_name = a2dpsource_config->dev[0].name;

        sprintf(conn_addr, "%02x:%02x:%02x:%02x:%02x:%02x",
                a2dpsource_config->dev[0].addr[0],
                a2dpsource_config->dev[0].addr[1],
                a2dpsource_config->dev[0].addr[2],
                a2dpsource_config->dev[0].addr[3],
                a2dpsource_config->dev[0].addr[4],
                a2dpsource_config->dev[0].addr[5]);
        break;
    }
    case (A2DP_SOURCE_STATE_CONNECTEFAILED): {
        conn_state = "connect failed";
        break;
    }
    case (A2DP_SOURCE_STATE_DISCONNECTED): {
        conn_state = "disconnected";
        break;
    }
    case (A2DP_SOURCE_STATE_CONNECTEOVER): {
        conn_state = "connect over";
        conn_name = a2dpsource_config->dev[0].name;
        sprintf(conn_addr, "%02x:%02x:%02x:%02x:%02x:%02x",
                a2dpsource_config->dev[0].addr[0],
                a2dpsource_config->dev[0].addr[1],
                a2dpsource_config->dev[0].addr[2],
                a2dpsource_config->dev[0].addr[3],
                a2dpsource_config->dev[0].addr[4],
                a2dpsource_config->dev[0].addr[5]);
        break;
    }
    default:
        conn_state = "invalid";
        break;
    }

    switch (a2dpsource_state->broadcast_state) {
    case (A2DP_SOURCE_BROADCAST_OPENED): {
        broadcast_state = "opened";
        break;
    }
    case (A2DP_SOURCE_BROADCAST_CLOSED): {
        broadcast_state = "closed";
        break;
    }
    default:
        break;
    }

    cJSON_AddNumberToObject(root,"linknum",a2dpsource_config->autoconnect_linknum);

    if (a2dpsource) {
        cJSON_AddStringToObject(root,"a2dpstate",a2dpsource);
    }

    if (conn_state) {
        cJSON_AddStringToObject(root,"connect_state",conn_state);
        if (conn_name) {
            cJSON_AddStringToObject(root,"connect_name",conn_name);
            cJSON_AddStringToObject(root,"connect_addres",conn_addr);
        }
    }

    if (broadcast_state) {
        cJSON_AddStringToObject(root,"broadcast_state",broadcast_state);
    }

    cJSON_AddStringToObject(root,"action",action);

    re_state = (char *)cJSON_Print(root);
    if (reply)
        method_report_reply(BLUETOOTH_FUNCTION_A2DPSOURCE, reply, re_state);
    else
        report_bluetooth_information(BLUETOOTH_FUNCTION_A2DPSOURCE, (uint8_t *)re_state, strlen(re_state));

    yodalite_free(re_state);
    cJSON_Delete(root);
}

static int get_a2dpsource_connected_device_num() {
    BTDevice device;
    struct rk_bluetooth *bt = g_bt_handle->bt;

    memset(&device, 0, sizeof(device));

    return bt->a2dp.get_connected_devices(bt, &device, 1);
}

static void a2dp_source_listener (void *caller, BT_A2DP_EVT event, void *data) {
    //struct rk_bluetooth *bt = caller;
    BT_A2DP_MSG *msg = data;
    int index = 0;
    struct rk_bluetooth *bt = caller;
    struct bt_autoconnect_device dev ;
    struct a2dpsource_state *a2dpsource_state = &g_bt_a2dpsource->state;
    struct a2dpsource_timer *a2dpsource_timer = &g_bt_a2dpsource->timer;
    struct bt_autoconnect_config *a2dpsource_config = &g_bt_a2dpsource->config;

    memset(&dev, 0, sizeof(struct bt_autoconnect_device));

    switch (event) {
    case BT_A2DP_OPEN_EVT:
        if (msg) {
            if (msg->open.status == 0) {
                BT_LOGI("BT_A2DP_OPEN_EVT:status= %d, name: %s,addr %02X:%02X:%02X:%02X:%02X:%02X\n",
                       msg->open.status, msg->open.dev.name,
                       msg->open.dev.bd_addr[0], msg->open.dev.bd_addr[1],
                       msg->open.dev.bd_addr[2], msg->open.dev.bd_addr[3],
                       msg->open.dev.bd_addr[4], msg->open.dev.bd_addr[5]);
                memcpy(dev.addr, msg->open.dev.bd_addr, sizeof(dev.addr));

                index = bt_autoconnect_get_index(a2dpsource_config, &dev);
                if (index >= 0) {
                    strncpy(dev.name, a2dpsource_config->dev[index].name, strlen(a2dpsource_config->dev[index].name));
                }
                BT_LOGV("old name :: %s\n", dev.name);

                if (strlen(msg->open.dev.name) != 0) {
                    BT_LOGV("connect name :; %s\n", msg->open.dev.name);
                    memset(dev.name, 0, sizeof(dev.name));
                    strncpy(dev.name, msg->open.dev.name, strlen(msg->open.dev.name));
                }

                eloop_timer_stop(a2dpsource_timer->e_connect_over_id);

                bt_autoconnect_update(a2dpsource_config, &dev);
                bt_autoconnect_sync(a2dpsource_config);

                if (bt->set_visibility(bt, false, false)) {
                    BT_LOGI("set visibility error\n");
                }

                if (a2dpsource_state->open_state != A2DP_SOURCE_STATE_CLOSED) {
                    a2dpsource_state->connect_state = A2DP_SOURCE_STATE_CONNECTED;
                    a2dpsource_state->broadcast_state = A2DP_SOURCE_BROADCAST_CLOSED;
                    broadcast_a2dpsource_state(NULL);
                }

            } else {
                int i;
                if (a2dpsource_config->autoconnect_flag == AUTOCONNECT_BY_HISTORY_WORK) {
                    eloop_timer_start(a2dpsource_timer->e_auto_connect_id);
                    for (i = 0; i < a2dpsource_config->autoconnect_num; i++) {
                        if (memcmp(a2dpsource_config->dev[i].addr, msg->open.dev.bd_addr, sizeof(msg->open.dev.bd_addr)) == 0) {
                            return ;
                        }
                    }
                }
                if (a2dpsource_config->autoconnect_flag == AUTOCONNECT_BY_HISTORY_OVER)  {
                    if (a2dpsource_state->open_state != A2DP_SOURCE_STATE_CLOSED) {
                        a2dpsource_state->connect_state = A2DP_SOURCE_STATE_CONNECTEFAILED;
                        a2dpsource_state->broadcast_state = A2DP_SOURCE_BROADCAST_OPENED;
                        broadcast_a2dpsource_state(NULL);
                    }
                }
            }
        }
        break;
    case BT_A2DP_CLOSE_EVT:
        if (msg)
            BT_LOGI("BT_A2DP_CLOSE_EVT : status= %d\n", msg->close.status);

        a2dpsource_state->connect_state = A2DP_SOURCE_STATE_DISCONNECTED;

        if (bt->set_visibility(bt, true, true)) {
            BT_LOGI("set visibility error\n");
        }

        if (a2dpsource_state->open_state < A2DP_SOURCE_STATE_CLOSED) {
            a2dpsource_state->broadcast_state = A2DP_SOURCE_BROADCAST_OPENED;
            broadcast_a2dpsource_state(NULL);
        }

        if ((a2dpsource_state->open_state < A2DP_SOURCE_STATE_CLOSE) &&
                (a2dpsource_state->open_state >= A2DP_SOURCE_STATE_OPENED)) {
            eloop_timer_start(a2dpsource_timer->e_connect_over_id);
        }

        break;
    case BT_A2DP_RC_OPEN_EVT:
        if (msg)
            BT_LOGI("BT_A2DP_RC_OPEN_EVT : status= %d\n", msg->rc_open.status);
        break;
    case BT_A2DP_RC_CLOSE_EVT:
        if (msg)
            BT_LOGI("BT_A2DP_RC_CLOSE_EVT : status= %d\n", msg->rc_close.status);
        break;
    case BT_A2DP_START_EVT:
        if (msg)
            BT_LOGI("BT_A2DP_START_EVT : status= %d\n", msg->start.status);
        break;
    case BT_A2DP_STOP_EVT:
        if (msg)
            BT_LOGI("BT_A2DP_STOP_EVT : pause= %d\n", msg->stop.pause);
        break;
    case BT_A2DP_REMOTE_CMD_EVT:
        if (msg) {
            BT_LOGI("BT_A2DP_REMOTE_CMD_EVT : rc_cmd= 0x%x\n", msg->remote_cmd.rc_id);
            switch(msg->remote_cmd.rc_id) {
            case BT_AVRC_PLAY:
                BT_LOGI("Play key\n");
                break;
            case BT_AVRC_STOP:
                BT_LOGI("Stop key\n");
                break;
            case BT_AVRC_PAUSE:
                BT_LOGI("Pause key\n");
                break;
            case BT_AVRC_FORWARD:
                BT_LOGI("Forward key\n");
                break;
            case BT_AVRC_BACKWARD:
                BT_LOGI("Backward key\n");
                break;
            default:
                break;
            }
        }
        break;
    case BT_A2DP_REMOTE_RSP_EVT:
        if (msg)
            BT_LOGI("BT_A2DP_REMOTE_RSP_EVT : rc_cmd= 0x%x\n", msg->remote_rsp.rc_id);
        break;
    default:
        break;
    }

    return;
}

static int bt_a2dpsource_enable(struct rk_bluetooth *bt, char *name) {
    int ret = 0;
    struct a2dpsource_state *a2dpsource_state = &g_bt_a2dpsource->state;

    bt->a2dp.disable(bt);
    ret = bt->a2dp.set_listener(bt, a2dp_source_listener, bt);
    if (ret) {
        BT_LOGE("set ble listener error :: %d\n", ret);
        return -2;
    }

    bt->set_bt_name(bt, name);

    if (bt->a2dp.enable(bt)) {
        BT_LOGE("enable a2dp source failed!\n");
        return -2;
    }

    ret = bt->set_visibility(bt, true, true);
    if (ret) {
        BT_LOGE("set visibility error\n");
        return -3;
    }

    a2dpsource_state->broadcast_state = A2DP_SOURCE_BROADCAST_OPENED;

    return 0;
}

static void a2dpsource_bluetooth_autoconnect() {
    struct bt_autoconnect_config *a2dpsource_config = &g_bt_a2dpsource->config;
    struct a2dpsource_timer *a2dpsource_timer = &g_bt_a2dpsource->timer;

    if (get_a2dpsource_connected_device_num()) {
        a2dpsource_config->autoconnect_index = 0;
        return;
    }
    a2dpsource_config->autoconnect_flag = AUTOCONNECT_BY_HISTORY_WORK;
    a2dpsource_config->autoconnect_index = 0;
    eloop_timer_start(a2dpsource_timer->e_auto_connect_id);
    BT_LOGI("begin autoconnect\n");
}

static void auto_connect_a2dpsource_by_history_index(void *eloop_ctx) {
    struct rk_bluetooth *bt = g_bt_handle->bt;
    struct a2dpsource_timer *a2dpsource_timer = &g_bt_a2dpsource->timer;
    struct bt_autoconnect_config *a2dpsource_config = &g_bt_a2dpsource->config;
    struct a2dpsource_state *a2dpsource_state = &g_bt_a2dpsource->state;
    char zero_addr[6] = {0};
    int ret;


    if (get_a2dpsource_connected_device_num()) {
        a2dpsource_config->autoconnect_index = 0;
        a2dpsource_config->autoconnect_flag = AUTOCONNECT_BY_HISTORY_OVER;
        eloop_timer_stop(a2dpsource_timer->e_auto_connect_id);
        return;
    }
    if (a2dpsource_config->autoconnect_flag != AUTOCONNECT_BY_HISTORY_WORK) {
        a2dpsource_config->autoconnect_index = 0;
        return ;
    }

    BT_LOGI("autoconnect index ;:; %d\n", a2dpsource_config->autoconnect_index);
    if (a2dpsource_config->autoconnect_index >= 0) {
        if (a2dpsource_config->autoconnect_index < a2dpsource_config->autoconnect_num) {
            BT_LOGI("autoconnect  %02X:%02X:%02X:%02X:%02X:%02X\n",
                     a2dpsource_config->dev[a2dpsource_config->autoconnect_index].addr[0],
                     a2dpsource_config->dev[a2dpsource_config->autoconnect_index].addr[1],
                     a2dpsource_config->dev[a2dpsource_config->autoconnect_index].addr[2],
                     a2dpsource_config->dev[a2dpsource_config->autoconnect_index].addr[3],
                     a2dpsource_config->dev[a2dpsource_config->autoconnect_index].addr[4],
                     a2dpsource_config->dev[a2dpsource_config->autoconnect_index].addr[5]);
            BT_LOGI("autoconnect name :: %s\n", a2dpsource_config->dev[a2dpsource_config->autoconnect_index].name);
            if (memcmp(a2dpsource_config->dev[a2dpsource_config->autoconnect_index].addr, zero_addr, sizeof(zero_addr)) != 0) {
                ret = bt->a2dp.connect(bt, a2dpsource_config->dev[a2dpsource_config->autoconnect_index].addr);
                if (ret) {
                    BT_LOGI("autoconnect failed :: %d\n", ret);//busy  auto connect next
                    a2dpsource_config->autoconnect_index ++;
                    eloop_timer_start(a2dpsource_timer->e_auto_connect_id);
                    return;
                } else
                    a2dpsource_state->connect_state = A2DP_SOURCE_STATE_CONNECTING;
            }
        } else {
            a2dpsource_config->autoconnect_index = 0;
            a2dpsource_config->autoconnect_flag = AUTOCONNECT_BY_HISTORY_OVER;
            if ((a2dpsource_state->connect_state != A2DP_SOURCE_STATE_CONNECTED) &&
                (a2dpsource_config->autoconnect_linknum > 0)) {
                a2dpsource_state->connect_state = A2DP_SOURCE_STATE_CONNECTEOVER;
                broadcast_a2dpsource_state(NULL);
            }
            eloop_timer_stop(a2dpsource_timer->e_auto_connect_id);
            return ;
        }
    }

    a2dpsource_config->autoconnect_index++;
}

static void a2dpsource_autoconnect_over(void *eloop_ctx) {
    if (get_a2dpsource_connected_device_num()) {
        return;
    } else {
        bt_a2dpsource_off();
    }
}

int bt_a2dpsource_timer_init() {
    struct a2dpsource_timer *timer = &g_bt_a2dpsource->timer;

    timer->e_auto_connect_id = eloop_timer_add(auto_connect_a2dpsource_by_history_index, NULL, 1 * 1000, 0);
    timer->e_connect_over_id = eloop_timer_add(a2dpsource_autoconnect_over, NULL, BLUETOOTH_A2DPSOURCE_AUTOCONNECT_LIMIT_TIME, 0);
    //timer->e_discovery_id = eloop_timer_add(a2dpsource_upload_scan_results, NULL, BLUETOOTH_A2DPSOURCE_DISCOVERY_LIMIT_TIME, BLUETOOTH_A2DPSOURCE_DISCOVERY_LIMIT_TIME);

    if ((timer->e_auto_connect_id <= 0) ||
        (timer->e_connect_over_id <= 0)) {
        return -1;
    }

    return 0;
}

void bt_a2dpsource_timer_uninit() {
    struct a2dpsource_timer *timer = &g_bt_a2dpsource->timer;

    eloop_timer_stop(timer->e_auto_connect_id);
    eloop_timer_stop(timer->e_connect_over_id);
    //eloop_timer_stop(timer->e_discovery_id);

    eloop_timer_delete(timer->e_auto_connect_id);
    eloop_timer_delete(timer->e_connect_over_id);
    //eloop_timer_delete(timer->e_discovery_id);
}
static void bt_a2dpsource_on(char *name) {
    int ret = 0;
    struct a2dpsource_handle *a2dpsource = g_bt_a2dpsource;
    struct rk_bluetooth *bt = g_bt_handle->bt;

    if (a2dpsource->state.open_state == A2DP_SOURCE_STATE_OPENED) {
        broadcast_a2dpsource_state(NULL);
        a2dpsource_bluetooth_autoconnect();
        return;
    }
    pthread_mutex_lock(&(a2dpsource->state_mutex));

    ret = bt_open(g_bt_handle, NULL);
    if (ret) {
        BT_LOGE("failed to open bt\n");
        a2dpsource->state.open_state = A2DP_SOURCE_STATE_OPENFAILED;
        broadcast_a2dpsource_state(NULL);
        pthread_mutex_unlock(&(a2dpsource->state_mutex));
        return ;
    }

    memset(&a2dpsource->state, 0, sizeof(struct a2dpsource_state));
    ret = bt_a2dpsource_enable(bt, name);
    if (ret == 0) {
        a2dpsource->state.open_state = A2DP_SOURCE_STATE_OPENED;
        g_bt_handle->status |= A2DP_SOURCE_STATUS_MASK;
    } else {
        a2dpsource->state.open_state = A2DP_SOURCE_STATE_OPENFAILED;
        broadcast_a2dpsource_state(NULL);
        pthread_mutex_unlock(&(a2dpsource->state_mutex));
        return ;
    }

    broadcast_a2dpsource_state(NULL);

    a2dpsource->config.autoconnect_flag = AUTOCONNECT_BY_HISTORY_INIT;

    a2dpsource_bluetooth_autoconnect();

    eloop_timer_start(a2dpsource->timer.e_connect_over_id);

    pthread_mutex_unlock(&(a2dpsource->state_mutex));
    return ;
}

void bt_a2dpsource_off() {
    int ret = 0;
    struct a2dpsource_handle *a2dpsource = g_bt_a2dpsource;
    struct rk_bluetooth *bt = g_bt_handle->bt;

    if (a2dpsource->state.open_state == A2DP_SOURCE_STATE_CLOSED) {
        broadcast_a2dpsource_state(NULL);
        return;
    }
    pthread_mutex_lock(&(a2dpsource->state_mutex));

    ret = bt->cancel_discovery(bt);
    if (ret) {
        BT_LOGV("a2dpsource cancel discovery error\n");
    }

    ret = bt->a2dp.disable(bt);
    if (ret) {
        BT_LOGI("a2dpsource OFF error\n");
    }

    g_bt_handle->status &= ~A2DP_SOURCE_STATUS_MASK;
    bt_close(g_bt_handle);

    eloop_timer_stop(a2dpsource->timer.e_connect_over_id);
    eloop_timer_stop(a2dpsource->timer.e_auto_connect_id);
    //eloop_timer_stop(a2dpsource->timer.e_discovery_id);
    a2dpsource->scanning = 0;

    memset(&a2dpsource->state, 0, sizeof(struct a2dpsource_state));
    a2dpsource->state.broadcast_state = A2DP_SOURCE_BROADCAST_CLOSED;
    a2dpsource->state.open_state = A2DP_SOURCE_STATE_CLOSED;
    broadcast_a2dpsource_state(NULL);

    pthread_mutex_unlock(&(a2dpsource->state_mutex));
}

void bt_a2dpsource_on_prepare(struct bt_service_handle *handle,cJSON*obj) {
    int unique = 0;
    cJSON*bt_unique = NULL;
    struct ble_handle *ble = &handle->bt_ble;
    struct a2dpsink_handle *a2dpsink = &handle->bt_a2dpsink;
#if defined(BT_SERVICE_HAVE_HFP)
    struct hfp_handle *hfp = &handle->bt_hfp;
#endif

    if((bt_unique = cJSON_GetObjectItemCaseSensitive(obj, "action")) != NULL){
      if(!cJSON_IsBool(bt_unique)){
         return ; 
     }

     unique = cJSON_IsTrue(bt_unique)?1:0;
     BT_LOGI("unique :: %d\n", unique);
    if (unique) {
      if ((ble->state.open_state >= BLE_STATE_OPEN) &&
          (ble->state.open_state < BLE_STATE_CLOSED)) {
               bt_ble_off();
       }
      #if defined(BT_SERVICE_HAVE_HFP)
      if (hfp->state.open_state == HFP_STATE_OPENED) {
              bt_hfp_off();
       }
      #endif
      if ((a2dpsink->state.open_state >= A2DP_SINK_STATE_OPEN) &&
          (a2dpsink->state.open_state < A2DP_SINK_STATE_CLOSED)) {
              bt_a2dpsink_off();
        }
      }
    }
}

static void bt_a2dpsource_disconnect_device(struct a2dpsource_handle *a2dpsource) {
    BTDevice device;
    int count = 0;
    int ret = 0;
    struct rk_bluetooth *bt = g_bt_handle->bt;
    struct a2dpsource_state *a2dpsource_state = &a2dpsource->state;

    memset(&device, 0, sizeof(device));

    count = bt->a2dp.get_connected_devices(bt, &device, 1);
    if (count) {
        BT_LOGI("disconnect bdaddr:%02x:%02x:%02x:%02x:%02x:%02x\n",
                 device.bd_addr[0],
                 device.bd_addr[1],
                 device.bd_addr[2],
                 device.bd_addr[3],
                 device.bd_addr[4],
                 device.bd_addr[5]);
        ret = bt->a2dp.disconnect(bt, device.bd_addr);
        if (ret) {
            BT_LOGE("a2dp DISCONNECT error\n");
        }
    } else {
        a2dpsource_state->connect_state = A2DP_SOURCE_STATE_DISCONNECTED;
        broadcast_a2dpsource_state(NULL);
    }
}

void upload_bt_scan_results(cJSON*obj) {
    cJSON *root = cJSON_CreateObject();
    char *action = "discovery";
    char *buf = NULL;

    cJSON_AddStringToObject(root,"action",action);
    cJSON_AddItemToObject(root,"results",obj);

    buf = (char *)cJSON_Print(root);
    BT_LOGI("results :: %s\n", buf);

    report_bluetooth_information(BLUETOOTH_FUNCTION_A2DPSOURCE, (uint8_t *)buf, strlen(buf));

    yodalite_free(buf); 
    cJSON_Delete(root);
}

void a2dpsource_upload_scan_results(const char *bt_name, BTAddr bt_addr, int is_completed, void *data) {
    struct a2dpsource_handle *a2dpsource = g_bt_a2dpsource;
    struct rk_bluetooth *bt = g_bt_handle->bt;
    BTDevice connect_dev;
    char address[18] = {0};
    BTDevice scan_devices[20];
    int scan_devices_num = 0;
    int i;
    cJSON *root;
    cJSON *array;
    cJSON * connect_device;
    cJSON *item[20]; 

    if (!a2dpsource->scanning) return;

    root = cJSON_CreateObject();
    array = cJSON_CreateArray();
    connect_device = cJSON_CreateObject();

    if (is_completed) {
        a2dpsource->scanning = 0;
    }
    memset(scan_devices, 0, sizeof(scan_devices));
    scan_devices_num = bt->get_disc_devices(bt, scan_devices, 20);

    for (i = 0; i < scan_devices_num; i++) {
        memset(address, 0, sizeof(address));

        sprintf(address, "%02x:%02x:%02x:%02x:%02x:%02x",
                    scan_devices[i].bd_addr[0],
                    scan_devices[i].bd_addr[1],
                    scan_devices[i].bd_addr[2],
                    scan_devices[i].bd_addr[3],
                    scan_devices[i].bd_addr[4],
                    scan_devices[i].bd_addr[5]);

        if (strlen(scan_devices[i].name) > 0) {
            item[i] = cJSON_CreateObject();
            cJSON_AddStringToObject(item[i], "address",address);
            cJSON_AddStringToObject(item[i], "name",scan_devices[i].name);
            cJSON_AddItemToArray(array,item[i]);
        }
    }

    if (scan_devices_num > 0) {
        cJSON_AddItemToObject(root,"deviceList",array);
    }

    memset(&connect_dev, 0, sizeof(BTDevice));

    if (bt->a2dp.get_connected_devices(bt, &connect_dev, 1)) {
        memset(address, 0, sizeof(address));
        sprintf(address, "%02x:%02x:%02x:%02x:%02x:%02x",
                    connect_dev.bd_addr[0],
                    connect_dev.bd_addr[1],
                    connect_dev.bd_addr[2],
                    connect_dev.bd_addr[3],
                    connect_dev.bd_addr[4],
                    connect_dev.bd_addr[5]);

        cJSON_AddStringToObject(connect_device,"name",connect_dev.name);
        cJSON_AddStringToObject(connect_device,"address",address);
        cJSON_AddItemToObject(root,"currentDevice", connect_device);
    }

    cJSON_AddItemToObject(root, "is_completed",cJSON_CreateBool(is_completed));
    upload_bt_scan_results(root);

//   for (i = 0; i < scan_devices_num; i++) {
//      cJSON_Delete(item[i]);
//   }
//   cJSON_Delete(array);
//   cJSON_Delete(connect_device);
    cJSON_Delete(root);
}

static void bt_a2dpsource_discovery() {
    struct a2dpsource_handle *a2dpsource = g_bt_a2dpsource;
    struct rk_bluetooth *bt = g_bt_handle->bt;
    //struct a2dpsource_timer *a2dpsource_timer = &g_bt_a2dpsource->timer;
    int ret = 0;

    if (a2dpsource->state.open_state != A2DP_SOURCE_STATE_OPENED) {
        broadcast_a2dpsource_state(NULL);
        return;
    }
    if (bt->start_discovery) {
        BT_LOGV("scanning  :: \n");
        if (a2dpsource->scanning) {
            BT_LOGW("scanning already :: \n");
            return ;
        }

        ret = bt->start_discovery(bt, BT_A2DP_SINK);
        BT_LOGI("scanning begin :: \n");
        if (ret) {
            BT_LOGW("discovery error\n");
        } else {
            a2dpsource->scanning = 1;
        }
    }
}

void bt_a2dpsource_connect(const char *addr) {
    struct a2dpsource_handle *a2dpsource = g_bt_a2dpsource;
    uint8_t bd_addr[6] = {0};
    int ret = 0;
    struct rk_bluetooth *bt = g_bt_handle->bt;

    if (a2dpsource->state.open_state != A2DP_SOURCE_STATE_OPENED ||
        get_a2dpsource_connected_device_num()) {
        broadcast_a2dpsource_state(NULL);
        return;
    }

    ret = bd_strtoba(bd_addr, addr);
    if (ret != 0) {
        BT_LOGE("mac not valid\n");
        return ;
    }

    ret = bt->a2dp.connect(bt, bd_addr);
    if (ret != 0) {
        BT_LOGE("connect error\n");
        a2dpsource->state.connect_state = A2DP_SOURCE_STATE_CONNECTEFAILED;
        broadcast_a2dpsource_state(NULL);
        return ;
    }
}

int handle_a2dpsource_handle(cJSON*obj, struct bt_service_handle *handle, void *reply) {
    char *command = NULL;
    cJSON*bt_cmd = NULL;
    cJSON*bt_name = NULL;
    cJSON*bt_addr = NULL;
    struct a2dpsource_state *a2dpsource_state = NULL;
    struct a2dpsource_timer *a2dpsource_timer = &g_bt_a2dpsource->timer;
    struct rk_bluetooth *bt = NULL;
    BTAddr bd_addr = {0};
    char name[128] = {0};

    if (cJSON_IsInvalid(obj)) {
         BT_LOGE("error:obj is not valid\n");
        return -1;
    }

    if((bt_cmd = cJSON_GetObjectItemCaseSensitive(obj, "command")) != NULL){
      if (!cJSON_IsString(bt_cmd) || (bt_cmd->valuestring == NULL)){
           BT_LOGE("error:bt_cmd is not valid\n");
           return -1;
        }

        command = (char *)cJSON_GetStringValue(bt_cmd);
        BT_LOGI("bt_a2dpsource :: command %s \n", command);

        a2dpsource_state = &g_bt_a2dpsource->state;
        bt = g_bt_handle->bt;

        if (strcmp(command, "ON") == 0) {
            if (a2dpsource_state->open_state == A2DP_SOURCE_STATE_OPENED &&
                a2dpsource_state->connect_state != A2DP_SOURCE_STATE_CONNECTED) {
                a2dpsource_state->connect_state = A2DP_SOURCE_STATE_CONNECT_INVALID;
                eloop_timer_stop(a2dpsource_timer->e_connect_over_id);
                eloop_timer_start(a2dpsource_timer->e_connect_over_id);
            }

            if((bt_name = cJSON_GetObjectItemCaseSensitive(obj,"name")) != NULL){
               if (cJSON_IsString(bt_name) && (bt_name->valuestring != NULL)){
                  snprintf(name, sizeof(name), "%s", (char *)cJSON_GetStringValue(bt_name));
               }else{
                  BT_LOGE("error:bt_name is not valid\n");
                  return -1;
               }
            }else{
               bt->get_module_addr(bt, bd_addr);
               snprintf(name, sizeof(name), "rokidsource_%02x:%02x:%02x:%02x:%02x:%02x",
                         bd_addr[0], bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
           }
            BT_LOGI("bt_a2dpsource :: name %s \n", name);

            bt_a2dpsource_on_prepare(handle, obj);
            bt_a2dpsource_on(name);
        } else if (strcmp(command, "OFF") == 0) {
            bt_a2dpsource_off();
        } else if (strcmp(command, "GETSTATE") == 0) {
            broadcast_a2dpsource_state(reply);
        } else if (strcmp(command, "DISCOVERY") == 0) {
            bt_a2dpsource_discovery();
        } else if (strcmp(command, "DISCONNECT") == 0) {
            bt_a2dpsource_disconnect_device(g_bt_a2dpsource);
        } else if (strcmp(command, "DISCONNECT_PEER") == 0) {
            bt_a2dpsource_disconnect_device(g_bt_a2dpsource);
        } else if (strcmp(command, "CONNECT") == 0) {
            if((bt_addr = cJSON_GetObjectItemCaseSensitive(obj,"address")) != NULL){
               if (cJSON_IsString(bt_addr) && (bt_addr->valuestring != NULL)){
                  bt_a2dpsource_connect((char *)cJSON_GetStringValue(bt_addr));
               }else{
                  BT_LOGE("error:bt_addr is not valid\n");
                  return -1;
               }
          }
        }
    }
   return 0;
}
