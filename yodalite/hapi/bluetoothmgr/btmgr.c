#include "common.h"

static struct bluetooth_module_t *g_bt_mod = NULL;
static struct bluetooth_device_t *g_bt_dev = NULL;

int bt_log_level = LEVEL_OVER_LOGV;
struct bt_service_handle *g_bt_handle = NULL;

extern  void bt_flora_init(void);
extern  void bt_flora_deinit(void);

static void bt_handle_destory(struct bt_service_handle *handle);

static inline int bluetooth_device_open (const hw_module_t *module, struct bluetooth_device_t **device) {
    return module->methods->open (module, BLUETOOTH_HARDWARE_MODULE_ID, (struct hw_device_t **) device);
}

void discovery_auto_connect(void *caller) {
    struct rk_bluetooth *bt = caller;
    auto_connect_by_scan_results(bt);
}

static void discovery_callback(void *caller, const char *bt_name, BTAddr bt_addr, int is_completed, void *data) {

    g_bt_handle->discovery_is_completed = is_completed;
    a2dpsource_upload_scan_results(bt_name, bt_addr, is_completed, data);
    if (is_completed) {
        discovery_auto_connect(caller);
    }
}

static void bluetooth_device_destory(struct bt_service_handle *handle) {
    if (handle->bt) {
        g_bt_dev->destroy(handle->bt);
        handle->bt = NULL;
    }
}

void manage_listener(void *caller, BT_MGR_EVT event, void *data) {
    BT_MGT_MSG  *msg = data;

    switch (event) {
    case BT_EVENT_MGR_CONNECT:
        if (msg)
            BT_LOGI("BT_EVENT_MGR_CONNECT : enable= %d\n", msg->connect.enable);
        break;
    case BT_EVENT_MGR_DISCONNECT:
        if (msg)
            BT_LOGW("BT_EVENT_MGR_DISCONNECT : reason= %d\n", msg->disconnect.reason);

        //todo upload failed msg
        //bt_handle_destory(g_bt_handle);
        //sleep(2);
        // when bsa && bluetoothd closed, bluetooth_service restart
        exit(-2);
        break;
    case BT_EVENT_MGR_REMOVE_DEVICE:
        if (msg) {
            broadcast_remove_dev(msg->remove.address, msg->remove.status);
        }
        break;
    default:
        break;
    }
    return;
}

static int bluetooth_device_create(struct bt_service_handle *handle) {
    int ret = 0;
    struct rk_bluetooth *bt = NULL;

    if (handle->bt) {
        BT_LOGI("bt is not destory\n");
        return -1;
    }

    ret = g_bt_dev->creat(&bt);
    if (ret || !bt) {
        BT_LOGE("failed to creat bt\n");
        return -2;
    }
    handle->bt = bt;

    bt->set_discovery_listener(bt, discovery_callback, bt);
    bt->set_manage_listener(bt, manage_listener, bt);
#if 0
    while (1) {
        ret = bt_open(handle, NULL);
        if (ret) {
            BT_LOGW("open failed ret :: %d\n", ret);
            sleep(2);
        } else {
            usleep(50 * 1000);
            bt_close(handle);
            return 0;
        }
    }
#endif

    return ret;
}

static int bluetooth_device_init(struct bt_service_handle *handle) {
    int ret;
     struct hw_module_t *module = NULL;


     hw_get_module(BT_HAL_HW_ID,g_bt_mod);
        //open bluetooth
      if (0 != bluetooth_device_open(&g_bt_mod->common, &g_bt_dev)) {
            BT_LOGE("failed to hw module: %s  device open\n", BLUETOOTH_HARDWARE_MODULE_ID);
            return -1;
     }

    ret = bluetooth_device_create(handle);

    if (ret) {
        goto exit;
    }

    return 0;
exit:
    if (g_bt_dev) {
        bluetooth_device_destory(handle);
        g_bt_dev->common.close((struct hw_device_t *)g_bt_dev);
        g_bt_dev = NULL;
    }
    return ret;
}

int bt_handle_init(struct bt_service_handle *handle) {
    int ret = 0;
    struct bt_autoconnect_config *a2dpsink_config;
    struct bt_autoconnect_config *a2dpsource_config;

    a2dpsink_config = &(handle->bt_a2dpsink.config);

    strncpy(a2dpsink_config->autoconnect_filename, ROKIDOS_BT_A2DPSINK_AUTOCONNECT_FILE, strlen(ROKIDOS_BT_A2DPSINK_AUTOCONNECT_FILE));
    a2dpsink_config->autoconnect_num = ROKIDOS_BT_A2DPSINK_AUTOCONNECT_NUM;
    // init bluetooth config
    ret = bt_autoconnect_config_init(a2dpsink_config);
    if (ret < 0) {
        BT_LOGE("create bluetooth a2dpsink config %d\n", ret);
        return -1;
    }

    BT_LOGV("num :: %d\n", a2dpsink_config->autoconnect_num);
    BT_LOGV("linknum :: %d\n", a2dpsink_config->autoconnect_linknum);
    BT_LOGV("mode :: %d\n", a2dpsink_config->autoconnect_mode);

    a2dpsource_config = &(handle->bt_a2dpsource.config);

    a2dpsource_config->autoconnect_num = ROKIDOS_BT_A2DPSOURCE_AUTOCONNECT_NUM;

    strncpy(a2dpsource_config->autoconnect_filename, ROKIDOS_BT_A2DPSOURCE_AUTOCONNECT_FILE, strlen(ROKIDOS_BT_A2DPSOURCE_AUTOCONNECT_FILE));
    // init bluetooth config
    ret = bt_autoconnect_config_init(a2dpsource_config);
    if (ret < 0) {
        BT_LOGE("create bluetooth a2dpsource config %d\n", ret);
        return -1;
    }

    BT_LOGV("num :: %d\n", a2dpsource_config->autoconnect_num);
    BT_LOGV("linknum :: %d\n", a2dpsource_config->autoconnect_linknum);
    BT_LOGV("mode :: %d\n", a2dpsource_config->autoconnect_mode);

    g_bt_ble = &handle->bt_ble;
    g_bt_a2dpsink = &handle->bt_a2dpsink;
    g_bt_a2dpsource = &handle->bt_a2dpsource;
#if defined(BT_SERVICE_HAVE_HFP)
    g_bt_hfp = &handle->bt_hfp;
#endif
  //  eloop_init();

    ret = bt_a2dpsink_timer_init();
    if (ret < 0) {
        BT_LOGE("create bluetooth a2dpsink timer %d\n", ret);
        return -2;
    }

    ret = bt_a2dpsource_timer_init();
    if (ret < 0) {
        BT_LOGE("create bluetooth a2dpsource timer %d\n", ret);
        return -2;
    }

    return 0;
}

static void bt_handle_uninit(struct bt_service_handle *handle) {
    bt_a2dpsource_timer_uninit();
    bt_a2dpsink_timer_uninit();
    bt_autconnect_config_uninit(&handle->bt_a2dpsource.config);
    bt_autconnect_config_uninit(&handle->bt_a2dpsink.config);
}

static int bt_handle_create(struct bt_service_handle **handle) {

    if (*handle) {
        BT_LOGW("bt handle has init \n");
        return -1;
    }

    *handle = yodalite_calloc(1, sizeof(struct bt_service_handle));
    if (*handle == NULL) {
        BT_LOGE("bt handle yodalite_calloc error \n");
        return -2;
    }

    pthread_mutex_init(&((*handle)->bt_ble.state_mutex), NULL);
    pthread_mutex_init(&((*handle)->bt_a2dpsink.state_mutex), NULL);

    pthread_mutex_init(&((*handle)->bt_a2dpsource.state_mutex), NULL);
#if defined(BT_SERVICE_HAVE_HFP)
    pthread_mutex_init(&((*handle)->bt_hfp.state_mutex), NULL);
#endif

    return 0;
}

static void bt_handle_destory(struct bt_service_handle *handle) {
    if (handle) {
        bt_ble_off();
        bt_a2dpsink_off();
        bt_a2dpsource_off();
#if defined(BT_SERVICE_HAVE_HFP)
        bt_hfp_off();
#endif
     //   eloop_stop();
      //  eloop_exit();

        bt_handle_uninit(handle);

        bluetooth_device_destory(handle);

        pthread_mutex_destroy(&handle->bt_ble.state_mutex);
        pthread_mutex_destroy(&handle->bt_a2dpsink.state_mutex);
        pthread_mutex_destroy(&handle->bt_a2dpsource.state_mutex);
#if defined(BT_SERVICE_HAVE_HFP)
        pthread_mutex_destroy(&handle->bt_hfp.state_mutex);
#endif
        yodalite_free(handle);
        g_bt_handle = NULL;
    }
}

int handle_module_sleep(cJSON*obj, struct bt_service_handle *handle) {
    char *command = NULL;
    cJSON *pm_cmd = NULL;

    if (cJSON_IsInvalid(obj)) {
        return -1;
    }

    if((pm_cmd = cJSON_GetObjectItemCaseSensitive(obj, "sleep")) != NULL){
      if (cJSON_IsString(pm_cmd) && (pm_cmd->valuestring != NULL)){
        command = (char *)pm_cmd->valuestring;
        BT_LOGI("pms :: sleep %s \n", command);

        if (strcmp(command, "ON") == 0) {
            bt_ble_off();
            bt_a2dpsink_off();
            bt_a2dpsource_off();
        #if defined(BT_SERVICE_HAVE_HFP)
            bt_hfp_off();
        #endif
        }
      }
   }

    return 0;
}

int handle_power_sleep(cJSON *obj,struct bt_service_handle *handle) {
    char *command = NULL;
    cJSON *pm_cmd = NULL;

    if (cJSON_IsInvalid(obj)) {
        return -1;
    }

    if((pm_cmd = cJSON_GetObjectItemCaseSensitive(obj, "sleep")) != NULL){
      if (cJSON_IsString(pm_cmd) && (pm_cmd->valuestring != NULL)){
        command = (char *)pm_cmd->valuestring;
        BT_LOGI("pms :: sleep %s \n", command);

        if (strcmp(command, "OFF") == 0) {
          //  system("/etc/init.d/bsa restart");
        }
      }
   }

    return 0;
}

int btmgr_init(void) 
{
    int ret = 0;

    ret = bt_handle_create(&g_bt_handle);
    if (ret < 0) {
        BT_LOGE("create bluetooth device error %d\n", ret);
        return -1;
    }

    ret = bluetooth_device_init(g_bt_handle);

    if (ret < 0) {
        BT_LOGE("create bluetooth device error %d\n", ret);
        return -2;
    }

    ret = bt_handle_init(g_bt_handle);

    if (ret < 0) {
        BT_LOGE("init bluetooth device error %d\n", ret);
        return -3;
    }
    
    bt_flora_init();

    return 0;
}

int btmgr_deinit(void)
{
    bt_handle_destory(g_bt_handle);
    bt_flora_deinit();
}
