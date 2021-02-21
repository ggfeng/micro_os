#include "common.h"

static int msgid = 0;
static char sendbuf[256] = {0};
struct ble_handle *g_bt_ble = NULL;
//cJSON_GetStringValue
void broadcast_ble_state(void *reply) {
    struct ble_state *ble_state = &g_bt_ble->state;
    cJSON*root =cJSON_CreateObject();
    char *state = NULL;
    char *re_state = NULL;

    switch (ble_state->open_state) {
    case BLE_STATE_OPEN: {
        state = "open";
        break;
    }
    case BLE_STATE_OPENING: {
        state = "opening";
        break;
    }
    case BLE_STATE_OPENED: {
        state = "opened";
        break;
    }
    case BLE_STATE_OPENFAILED: {
        state = "openfailed";
        break;
    }
    case BLE_STATE_CONNECTED: {
        state = "connected";
        break;
    }
    case BLE_STATE_HANDSHAKED: {
        state = "handshaked";
        break;
    }
    case BLE_STATE_DISCONNECTED: {
        state = "disconnected";
        break;
    }
    case BLE_STATE_CLOSE: {
        state = "close";
        break;
    }
    case BLE_STATE_CLOSING: {
        state = "closing";
        break;
    }
    case BLE_STATE_CLOSED: {
        state = "closed";
        break;
    }
    default:
        break;
    }

    cJSON_AddStringToObject(root,"state",state);

    re_state = (char *)cJSON_Print(root);

    if (reply)
        method_report_reply(BLUETOOTH_FUNCTION_BLE, reply, re_state);
    else
        report_bluetooth_information(BLUETOOTH_FUNCTION_BLE, (uint8_t *)re_state, strlen(re_state));

    yodalite_free(re_state);
    cJSON_Delete(root);
}

void ble_send_raw_data(struct rk_bluetooth *bt, uint8_t *buf, int len) {
    int index_len = 0;
    uint8_t send_buf[BLE_DATA_LEN] = {0};
    struct ble_state *ble_state = &g_bt_ble->state;

    if ((ble_state->open_state > BLE_STATE_OPENED) && (ble_state->open_state < BLE_STATE_CLOSE)) {
        while (len > 0) {
            memset(send_buf, 0, BLE_DATA_LEN);
            memcpy(send_buf, buf + index_len, BLE_DATA_LEN);
            bt->ble.send_buf(bt, BLE_DATA_CHARACTER, (char *)send_buf, (len > BLE_DATA_LEN) ? BLE_DATA_LEN : len);
            usleep(20 * 1000);

            index_len += BLE_DATA_LEN;
            len -= BLE_DATA_LEN;
        }
    } else {
        broadcast_ble_state(NULL);
    }
}

void rokid_send_ble_data(struct rk_bluetooth *bt, uint8_t *buf, int len) {
    struct rokid_ble_format data;
    struct ble_handle *ble = g_bt_ble;
    int tmp_len = 0;
    int index_len = 0;

    pthread_mutex_lock(&(ble->state_mutex));

    while (len > 0) {
        memset(&data, 0, sizeof(struct rokid_ble_format));
        if (len > ROKID_BLE_DATA_LEN) {
            tmp_len = ROKID_BLE_DATA_LEN;
        } else {
            tmp_len = len;
            data.last_index = 0x80;
        }

        data.msgid = msgid;

        memcpy(data.data, buf + index_len, tmp_len);

        bt->ble.send_buf(bt, BLE_DATA_CHARACTER, (char *)&data, tmp_len + 3);

        usleep(10 * 1000);

        index_len += tmp_len;
        len -= tmp_len;
    }
    pthread_mutex_unlock(&(ble->state_mutex));
}

void rokid_recv_ble_data(uint8_t *buf, int len) {
    struct rokid_ble_format data;
    static int index_len = 0;
    char *re_data = NULL;
    cJSON *root;

    memset(&data, 0, sizeof(struct rokid_ble_format));
    memcpy(&data, buf, len);

    BT_LOGI("msgid :: %02x current :: %02x\n", msgid, data.msgid);
    if (msgid != data.msgid) {
        memset(sendbuf, 0, sizeof(sendbuf));
        index_len = 0;
        msgid = data.msgid;
    }

    if ((data.last_index & 0x0080) == 0) {
        BT_LOGI("not enough\n");
        if (index_len + len -3  > 255) {
            BT_LOGE("err: sendbuf overflow\n");
            index_len = 0;
        }
        memcpy(sendbuf + index_len, data.data, len - 3);
        index_len += (len - 3);
        return;
    } else {
        BT_LOGI("enough\n");
        memcpy(sendbuf + index_len, data.data, len - 3);
        index_len = 0;
    }

    root = cJSON_CreateObject();

    cJSON_AddItemToObject(root,"data",cJSON_Parse(sendbuf));
    re_data = (char *)cJSON_Print(root);

    BT_LOGI("ble real data:: %s\n", re_data);

    report_bluetooth_information(BLUETOOTH_FUNCTION_BLE, (uint8_t *)re_data, strlen(re_data));

    yodalite_free(re_data);
    cJSON_Delete(root);
}

static void ble_listener(void *caller, BT_BLE_EVT event, void *data) {
    struct rk_bluetooth *bt = caller;
    BT_BLE_MSG *msg = data;
    struct rokid_ble_handshake handshake;
    struct ble_handle *ble = g_bt_ble;

    memset(&handshake, 0, sizeof(struct rokid_ble_handshake));
    handshake.magic_num[0] = 'R';
    handshake.magic_num[1] = 'K';
    handshake.version = 2;

    switch (event) {
    case BT_BLE_SE_WRITE_EVT:
        if (ble->state.open_state < BLE_STATE_CLOSE) {
            if (msg) {
                char buf[255] = {0};
                int len = msg->ser_write.len > 255 ? 255 : msg->ser_write.len;
                memcpy(buf, msg->ser_write.value + msg->ser_write.offset, len);
                buf[254] = '\0';
                BT_LOGV("BT_BLE_SE_WRITE_EVT : status %d, len: %d, buf: %s\n", msg->ser_write.status, msg->ser_write.len, buf);

                int i = 0;
                BT_LOGI("ser uuid : %04x\n", msg->ser_write.uuid);
                for (i = 0; i < len; i++) {
                    printf("0x%02x ", buf[i]);
                }
                printf("\n");

                if (msg->ser_write.uuid != BLE_DATA_CHARACTER) {
                    BT_LOGV("not right char id\n");
                    break;
                }

                if (msg->ser_write.len > 0) {
                    if (memcmp(&handshake, buf, sizeof(struct rokid_ble_handshake)) == 0) {
                        ble->state.open_state = BLE_STATE_HANDSHAKED;
                        usleep(50 * 1000);
                        bt->ble.send_buf(bt, BLE_DATA_CHARACTER, (char *)&handshake, sizeof(struct rokid_ble_handshake));
                        broadcast_ble_state(NULL);
                    } else {
                        rokid_recv_ble_data((uint8_t *)buf, msg->ser_write.len);
                    }

                    //report_bluetooth_information(BLUETOOTH_FUNCTION_BLE, (uint8_t *)buf, len);
                }
            }
        }

        break;
    case BT_BLE_SE_OPEN_EVT:
        if (msg)
            BT_LOGI("BT_BLE_SE_OPEN_EVT : reason= %d, conn_id %d\n", msg->ser_open.reason, msg->ser_open.conn_id);

        ble->state.open_state = BLE_STATE_CONNECTED;
        broadcast_ble_state(NULL);
        break;
    case BT_BLE_SE_CLOSE_EVT:
        if (msg)
            BT_LOGI("BT_BLE_SE_CLOSE_EVT : reason= %d, conn_id %d\n", msg->ser_close.reason, msg->ser_close.conn_id);

        ble->state.open_state = BLE_STATE_DISCONNECTED;
        broadcast_ble_state(NULL);
        break;
    default:
        break;
    }

    return;
}

static void bt_ble_on(char *name) {
    int ret = 0;
    struct ble_handle *ble = g_bt_ble;
    struct rk_bluetooth *bt = g_bt_handle->bt;

    if ((ble->state.open_state == BLE_STATE_OPENED) ||
        (ble->state.open_state == BLE_STATE_CONNECTED) ||
        (ble->state.open_state == BLE_STATE_HANDSHAKED)) {
        broadcast_ble_state(NULL);
        return;
    }
    pthread_mutex_lock(&(ble->state_mutex));

     ret = bt_open(g_bt_handle,name);
  //  ret = bt_open(g_bt_handle,NULL);
    if (ret) {
        BT_LOGE("failed to open bt\n");
        ble->state.open_state = BLE_STATE_OPENFAILED;
        broadcast_ble_state(NULL);
        pthread_mutex_unlock(&(ble->state_mutex));
        return ;
    }

    bt->ble.disable(bt);

    bt->ble.set_listener(bt, ble_listener, bt);

   // bt->set_bt_name(bt, name);

    if (0 != bt->ble.enable(bt)) {
        BT_LOGE("enbale ble failed\n");
        ble->state.open_state = BLE_STATE_OPENFAILED;
        broadcast_ble_state(NULL);
        pthread_mutex_unlock(&(ble->state_mutex));
        return ;
    }

    g_bt_handle->status |= BLE_STATUS_MASK;


    ret = bt->ble.set_ble_visibility(bt, true, true);
     if (ret) {
         BT_LOGI("set visibility error\n");
         return ;
     }


    memset(&ble->state, 0, sizeof(struct ble_state));

    ble->state.open_state = BLE_STATE_OPENED;

    broadcast_ble_state(NULL);

    pthread_mutex_unlock(&(ble->state_mutex));

    return ;
}

void bt_ble_off(void) {
    struct ble_handle *ble = g_bt_ble;
    struct rk_bluetooth *bt = g_bt_handle->bt;

    pthread_mutex_lock(&(ble->state_mutex));

    if (ble->state.open_state >= BLE_STATE_CLOSE) {
        broadcast_ble_state(NULL);
        pthread_mutex_unlock(&(ble->state_mutex));
        return ;
    }

    usleep(100 * 1000);

    ble->state.open_state = BLE_STATE_CLOSE;
    bt->ble.disable(bt);
    g_bt_handle->status &= ~BLE_STATUS_MASK;
    bt_close(g_bt_handle);
    ble->state.open_state = BLE_STATE_CLOSED;
    broadcast_ble_state(NULL);
    pthread_mutex_unlock(&(ble->state_mutex));
}

void bt_ble_on_prepare(struct bt_service_handle *handle,cJSON*obj) {
    int unique = 0;
    cJSON *bt_unique = NULL;
    struct a2dpsink_handle *a2dpsink = &handle->bt_a2dpsink;
    struct a2dpsource_handle *a2dpsource = &handle->bt_a2dpsource;
#if defined(BT_SERVICE_HAVE_HFP)
    struct hfp_handle *hfp = &handle->bt_hfp;
#endif
    if((bt_unique = cJSON_GetObjectItemCaseSensitive(obj, "unique")) != NULL){
       if(cJSON_IsBool(bt_unique)){
        unique = cJSON_IsTrue(bt_unique)?1:0;
        BT_LOGI("bt_ble :: unique :: %d\n", unique);
        if (unique) {
            if ((a2dpsink->state.open_state >= A2DP_SINK_STATE_OPEN) &&
                    (a2dpsink->state.open_state < A2DP_SINK_STATE_CLOSED)) {
                bt_a2dpsink_off();
            }
        #if defined(BT_SERVICE_HAVE_HFP)
            if (hfp->state.open_state == HFP_STATE_OPENED) {
                bt_hfp_off();
            }
        #endif

            if ((a2dpsource->state.open_state >= A2DP_SOURCE_STATE_OPEN) &&
                    (a2dpsource->state.open_state < A2DP_SOURCE_STATE_CLOSED)) {
                bt_a2dpsource_off();
            }
        }
     }
  }
}

int handle_ble_handle(cJSON *obj, struct bt_service_handle *handle, void *reply) {
    char *command = NULL;
    char *data = NULL;
    cJSON*bt_cmd = NULL;
    cJSON*bt_data = NULL;

    if (cJSON_IsInvalid(obj)) {
        return -1;
    }

    if((bt_cmd = cJSON_GetObjectItemCaseSensitive(obj, "command")) != NULL){
        if (cJSON_IsString(bt_cmd) && (bt_cmd->valuestring != NULL)){
          command = (char *)cJSON_GetStringValue(bt_cmd);
          BT_LOGI("bt_ble :: command %s \n", command);

        // g_bt_ble = &handle->bt_ble;
        //ble_state = &g_bt_ble->state;
        if (strcmp(command, "ON") == 0) {
            cJSON *bt_name = NULL;
            char *name = NULL;

            if((bt_name = cJSON_GetObjectItemCaseSensitive(obj,"name")) != NULL){
              if (cJSON_IsString(bt_name) && (bt_name->valuestring != NULL)){
                name = (char *)cJSON_GetStringValue(bt_name);
                BT_LOGI("ble:name %s \n", name);
              }else{
                BT_LOGI("ble:name NULL \n");
              }
            } else {
                name = "ROKID-BT-9999zz";
            }

            bt_ble_on_prepare(handle, obj);
            bt_ble_on(name);
        } else if (strcmp(command, "OFF") == 0) {
            bt_ble_off();
        } else if (strcmp(command, "GETSTATE") == 0) {
            broadcast_ble_state(reply);
        }
      }

      }else if((bt_data = cJSON_GetObjectItemCaseSensitive(obj,"data")) != NULL){
        data = (char *)cJSON_GetStringValue(bt_data);
        rokid_send_ble_data(g_bt_handle->bt, (uint8_t *)data, strlen(data));
      }else if((bt_data = cJSON_GetObjectItemCaseSensitive(obj,"rawdata")) != NULL){
        data = (char *)cJSON_GetStringValue(bt_data);
        ble_send_raw_data(g_bt_handle->bt, (uint8_t *)data, strlen(data));
  }
  return 0;
}
