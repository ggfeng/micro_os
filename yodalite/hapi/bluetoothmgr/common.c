#include "common.h"

int bt_open(struct bt_service_handle *handle, const char *name)
{
    int ret;
    struct rk_bluetooth *bt = handle->bt;

    if (handle->open) return 0;
    ret = bt->open(bt, name);
    if (ret) {
        BT_LOGE("failed");
        return ret;
    }
    handle->open = 1;
    return 0;
}

int bt_close(struct bt_service_handle *handle)
{
    //int ret;
    struct rk_bluetooth *bt = handle->bt;

    BT_LOGI("status:%x", handle->status);
    //if (handle->open && !handle->status) {
    if (handle->open) {
        /*
        ret = bt->set_visibility(bt, false, false);
        if (ret) {
            BT_LOGE("bt set visibility off error\n");
        }*/
        bt->close(bt);
        handle->open = 0;
    }
    return 0;
}


static int bt_get_mac_addr(struct bt_service_handle *handle, void *reply)
{
    int ret;
    BTAddr bd_addr;
    cJSON *root = cJSON_CreateObject();
    char *re_state;
    char address[18] = {0};
    struct rk_bluetooth *bt = handle->bt;

    ret = bt->get_module_addr(bt, bd_addr);
    snprintf(address, sizeof(address), "%02X:%02X:%02X:%02X:%02X:%02X",
                    bd_addr[0], bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
    cJSON_AddStringToObject(root,"module_address",address);

    re_state = (char *)cJSON_Print(root);

    if (reply)
        method_report_reply(BLUETOOTH_FUNCTION_COMMON, reply, re_state);
    else
        report_bluetooth_information(BLUETOOTH_FUNCTION_COMMON, (uint8_t *)re_state, strlen(re_state));

    yodalite_free(re_state);
    cJSON_Delete(root);

    return ret;
}

static int bt_set_discoveryable(struct rk_bluetooth *bt, bool discoverable, void *reply) {
    int ret = 0;
    cJSON*root = cJSON_CreateObject();
    char *re_state = NULL;
    char *broadcast_state = NULL;

    ret = bt->set_visibility(bt, discoverable, true);
    if (ret) {
        BT_LOGE("set visibility error\n");
    }
    if (discoverable)
        broadcast_state = "opened";
    else
        broadcast_state = "closed";

    cJSON_AddStringToObject(root, "broadcast_state",broadcast_state);
    re_state = (char *)cJSON_Print(root);

    if (reply)
        method_report_reply(BLUETOOTH_FUNCTION_COMMON, reply, re_state);
    else
        report_bluetooth_information(BLUETOOTH_FUNCTION_COMMON, (uint8_t *)re_state, strlen(re_state));

    yodalite_free(re_state);
    cJSON_Delete(root);
    return 0;
}

static void upload_get_bt_paired_list(cJSON*obj, void *reply) {
    char *buf = NULL;
    char *action = "pairedList";
    cJSON*root =cJSON_CreateObject();

    cJSON_AddStringToObject(root, "action",action);
    cJSON_AddItemToObject(root,"results", obj);

    buf = (char *)cJSON_Print(root);
    BT_LOGI("results :: %s\n", buf);

    if (reply)
        method_report_reply(BLUETOOTH_FUNCTION_COMMON, reply, buf);
    else
        report_bluetooth_information(BLUETOOTH_FUNCTION_COMMON, (uint8_t *)buf, strlen(buf));

    yodalite_free(buf);
    cJSON_Delete(root);
}

static int bt_get_paired_list(struct rk_bluetooth *bt, void *reply) {
    int dev_count, i;
    BTDevice devices[BT_DISC_NB_DEVICES];
    cJSON*root=cJSON_CreateObject();
    cJSON*array = cJSON_CreateArray();
    cJSON*item[BT_DISC_NB_DEVICES];
    char address[18] = {0};

    dev_count = bt->get_bonded_devices(bt, devices, BT_DISC_NB_DEVICES);

    for (i = 0; i < dev_count; i++) {
        memset(address, 0, sizeof(address));

        sprintf(address, "%02x:%02x:%02x:%02x:%02x:%02x",
                    devices[i].bd_addr[0],
                    devices[i].bd_addr[1],
                    devices[i].bd_addr[2],
                    devices[i].bd_addr[3],
                    devices[i].bd_addr[4],
                    devices[i].bd_addr[5]);
        item[i] = cJSON_CreateObject();
        cJSON_AddStringToObject(item[i],"address",address);
        cJSON_AddStringToObject(item[i],"name",devices[i].name);
        cJSON_AddItemToArray(array,item[i]);
    }

    cJSON_AddItemToObject(root,"deviceList",array);
    upload_get_bt_paired_list(root, reply);

/*
    for (i = 0; i < dev_count; i++) {
        cJSON_Delete(item[i]);
    }
    cJSON_Delete(array);
*/
    cJSON_Delete(root);

    return 0;
}


void broadcast_remove_dev(const char* addr, int status)
{
    char *data = NULL;
    char *action = "remove_dev";
    cJSON*root =cJSON_CreateObject ();

    cJSON_AddStringToObject(root, "action",action);
    cJSON_AddStringToObject(root, "address",addr);

    if (status == 99) {//devices list empty yet
        status = 0;
        cJSON_AddItemToObject(root, "empty",cJSON_CreateBool(1));
    }

    cJSON_AddItemToObject(root, "status",cJSON_CreateNumber(status));
    data = (char *)cJSON_Print(root);
    report_bluetooth_information(BLUETOOTH_FUNCTION_COMMON, (uint8_t *)data, strlen(data));
    yodalite_free(data);
    cJSON_Delete(root);
}

static int bt_remove_dev(struct rk_bluetooth *bt, const char *address)
{
    int ret;
    struct bt_autoconnect_device dev ;
    struct bt_autoconnect_config *a2dpsink_config = &g_bt_a2dpsink->config;
    struct bt_autoconnect_config *a2dpsource_config = &g_bt_a2dpsource->config;
    int i;

    ret = bt_open(g_bt_handle, NULL);
    if (strcmp(address, "*") == 0) {
         ret = bt->config_clear(bt);
         if (ret)
            broadcast_remove_dev(address, ret);
        for (i = 0; i < a2dpsink_config->autoconnect_num; i++) {
           memset(&a2dpsink_config->dev[i], 0, sizeof(a2dpsink_config->dev[i]));
        }
        a2dpsink_config->autoconnect_linknum = 0;
        bt_autoconnect_sync(a2dpsink_config);

        for (i = 0; i < a2dpsource_config->autoconnect_num; i++) {
           memset(&a2dpsource_config->dev[i], 0, sizeof(a2dpsource_config->dev[i]));
        }
        a2dpsource_config->autoconnect_linknum = 0;
        bt_autoconnect_sync(a2dpsource_config);
    } else {
        ret = bd_strtoba(dev.addr, address);
        if (ret) {
            broadcast_remove_dev(address, ret);
            goto exit;
        }
        ret = bt->remove_dev(bt, dev.addr);
        if (ret) {
            broadcast_remove_dev(address, ret);
            //goto exit;
        }

        ret = bt_autoconnect_remove(a2dpsink_config, &dev);
        if (ret == 0)
            bt_autoconnect_sync(a2dpsink_config);

        ret = bt_autoconnect_remove(a2dpsource_config, &dev);
        if (ret == 0)
            bt_autoconnect_sync(a2dpsource_config);
    }
exit:
    bt_close(g_bt_handle);
    return ret;
}

int handle_common_handle(cJSON*obj, struct bt_service_handle *handle, void *reply) {
    char *command = NULL;
    cJSON*bt_cmd = NULL;
    struct rk_bluetooth *bt = handle->bt;

    if (cJSON_IsInvalid(obj)) {
        BT_LOGE("error:obj is not valid\n");
        return -1;
    }

    if((bt_cmd = cJSON_GetObjectItemCaseSensitive(obj,"command")) == NULL){
        BT_LOGE("error:bt_cmd==NULL\n");
        return -1;;
    }

    if (cJSON_IsString(bt_cmd) && (bt_cmd->valuestring != NULL)){
        command = (char *)cJSON_GetStringValue(bt_cmd);
        BT_LOGI("common :: command %s \n", command);
        if (strcmp(command, "MODULE_ADDRESS") == 0) {
            bt_get_mac_addr(handle, reply);
        } else if (strcmp(command, "VISIBILITY") == 0) {
         cJSON*bt_discoverable = NULL;
         bool discoverable;

         if((bt_discoverable = cJSON_GetObjectItemCaseSensitive(obj,"discoverable")) == NULL){
            BT_LOGE("error:bt_discoverable==NULL\n");
            return -1;;
         }
          
         if(cJSON_IsBool(bt_discoverable)){
            discoverable =cJSON_IsTrue(bt_discoverable)?1:0;
            bt_set_discoveryable(bt, discoverable, reply);
          }else{
            BT_LOGE("error:bt_discoverable is not Bool\n");
            return -1;
          }
        } else if (strcmp(command, "PAIREDLIST") == 0) {
            bt_get_paired_list(bt, reply);
        } else if (strcmp(command, "REMOVE_DEV") == 0) {
            cJSON*bt_addr = NULL;
            if((bt_addr = cJSON_GetObjectItemCaseSensitive(obj, "address")) == NULL){
              BT_LOGE("error:bt_addr == NULL\n");
              return -1;
            }
            if (cJSON_IsString(bt_addr) && (bt_addr->valuestring != NULL)){
                     bt_remove_dev(bt, (char *)cJSON_GetStringValue(bt_addr));
           }
       }
    }

    return 0;
}
