/*
 * =====================================================================================
 *
 *       Filename:  ble_server.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/11/2019 02:41:34 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  glhou, 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <string.h>
#include "ble_server.h"

#define BLE_TAG "Rokid_ble_server"

#define PROFILE_NUM                 1
#define PROFILE_APP_IDX             0
#define ESP_APP_ID                  0x55
#define SAMPLE_DEVICE_NAME          "ROKID_BLE"
#define SVC_INST_ID                 0

/* The max length of characteristic value. When the gatt client write or prepare write, 
*  the data length must be less than GATTS_CHAR_VAL_LEN_MAX. 
*/
#define GATTS_CHAR_VAL_LEN_MAX 500
#define PREPARE_BUF_MAX_SIZE        1024
#define CHAR_DECLARATION_SIZE       (sizeof(uint8_t))

#define ADV_CONFIG_FLAG             (1 << 0)
#define SCAN_RSP_CONFIG_FLAG        (1 << 1)

static uint8_t adv_config_done       = 0;

uint16_t heart_rate_handle_table[HRS_IDX_NB];

typedef struct {
    uint8_t                 *prepare_buf;
    int                     prepare_len;
} prepare_type_env_t;

static char rokid_ble_name[32] = SAMPLE_DEVICE_NAME;
static prepare_type_env_t prepare_write_env;

//#define CONFIG_SET_RAW_ADV_DATA
#ifdef CONFIG_SET_RAW_ADV_DATA
static uint8_t raw_adv_data[] = {
        /* flags */
        0x02, 0x01, 0x06,
        /* tx power*/
        0x02, 0x0a, 0xeb,
        /* service uuid */
        0x03, 0x03, 0xFF, 0x00,
        /* device name */
        0x0f, 0x09, 'E', 'S', 'P', '_', 'G', 'A', 'T', 'T', 'S', '_', 'D','E', 'M', 'O'
};
static uint8_t raw_scan_rsp_data[] = {
        /* flags */
        0x02, 0x01, 0x06,
        /* tx power */
        0x02, 0x0a, 0xeb,
        /* service uuid */
        0x03, 0x03, 0xFF,0x00
};

#else
static uint8_t service_uuid[16] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xE1, 0xFF, 0x00, 0x00,
};

/* The length of adv data must be less than 31 bytes */
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp        = false,
    .include_name        = true,
    .include_txpower     = true,
    .min_interval        = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval        = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
    .appearance          = 0x00,
    .manufacturer_len    = 0,    //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //test_manufacturer,
    .service_data_len    = 0,
    .p_service_data      = NULL,
    .service_uuid_len    = sizeof(service_uuid),
    .p_service_uuid      = service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// scan response data
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp        = true,
    .include_name        = true,
    .include_txpower     = true,
    .min_interval        = 0x0006,
    .max_interval        = 0x0010,
    .appearance          = 0x00,
    .manufacturer_len    = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //&test_manufacturer[0],
    .service_data_len    = 0,
    .p_service_data      = NULL,
    .service_uuid_len    = 16,
    .p_service_uuid      = service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};
#endif /* CONFIG_SET_RAW_ADV_DATA */

static esp_ble_adv_params_t adv_params = {
    .adv_int_min         = 0x20,
    .adv_int_max         = 0x40,
    .adv_type            = ADV_TYPE_IND,
    .own_addr_type       = BLE_ADDR_TYPE_PUBLIC,
    .channel_map         = ADV_CHNL_ALL,
    .adv_filter_policy   = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

static void gatts_profile_event_handler(esp_gatts_cb_event_t event,
					esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

static int ble_get_server_uuid(BleServer_t *bles, int attr_id, uint16_t *uuid);
static int ble_server_set_attr_id(BleServer_t *bles, int attr_id, uint16_t uuid);
/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_profile_inst heart_rate_profile_tab[PROFILE_NUM] = {
    [PROFILE_APP_IDX] = {
        .gatts_cb = gatts_profile_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};

BleServer_t *_ble_server;
/* Service */
static const uint16_t ROKID_SERVICE_UUID      = 0xFFE1;
static const uint16_t ROKID_CHAR_UUID_A       = 0x2A06;
static const uint16_t ROKID_CHAR_UUID_B       = 0x2A07;

static const uint16_t primary_service_uuid         = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid   = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t char_prop_read                =  ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_write               = ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t char_prop_read_write_notify   = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t heart_measurement_ccc[2]      = {0x00, 0x00};
static const uint8_t char_value[4]                 = {0x11, 0x22, 0x33, 0x44};


/* Full Database Description - Used to add attributes into the database */
static esp_gatts_attr_db_t gatt_db[HRS_IDX_NB] =
{
    // Service Declaration
    [IDX_SVC]        =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
      sizeof(uint16_t), sizeof(ROKID_SERVICE_UUID), (uint8_t *)&ROKID_SERVICE_UUID}},

    /* Characteristic Declaration */
    [IDX_CHAR_A]     =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_A] =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&ROKID_CHAR_UUID_A, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_CHAR_VAL_LEN_MAX, sizeof(char_value), (uint8_t *)char_value}},

    /* Client Characteristic Configuration Descriptor */
    [IDX_CHAR_CFG_A]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      sizeof(uint16_t), sizeof(heart_measurement_ccc), (uint8_t *)heart_measurement_ccc}},
	
    /* Characteristic Declaration */
    [IDX_CHAR_B]     =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_B] =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&ROKID_CHAR_UUID_B, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_CHAR_VAL_LEN_MAX, sizeof(char_value), (uint8_t *)char_value}},

    /* Client Characteristic Configuration Descriptor */
    [IDX_CHAR_CFG_B]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      sizeof(uint16_t), sizeof(heart_measurement_ccc), (uint8_t *)heart_measurement_ccc}},

};

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    #ifdef CONFIG_SET_RAW_ADV_DATA
        case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
            adv_config_done &= (~ADV_CONFIG_FLAG);
            if (adv_config_done == 0){
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
        case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
            adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
            if (adv_config_done == 0){
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
    #else
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~ADV_CONFIG_FLAG);
            if (adv_config_done == 0){
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
        case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
            if (adv_config_done == 0){
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
    #endif
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            /* advertising start complete event to indicate advertising start successfully or failed */
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(BLE_TAG, "advertising start failed");
            }else{
                ESP_LOGI(BLE_TAG, "advertising start successfully");
            }
            break;
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(BLE_TAG, "Advertising stop failed");
            }
            else {
                ESP_LOGI(BLE_TAG, "Stop adv successfully\n");
            }
            break;
        case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
            ESP_LOGI(BLE_TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                  param->update_conn_params.status,
                  param->update_conn_params.min_int,
                  param->update_conn_params.max_int,
                  param->update_conn_params.conn_int,
                  param->update_conn_params.latency,
                  param->update_conn_params.timeout);
            break;
        default:
            break;
    }
}

void example_prepare_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param)
{
    ESP_LOGI(BLE_TAG, "prepare write, handle = %d, value len = %d", param->write.handle, param->write.len);
    esp_gatt_status_t status = ESP_GATT_OK;
    if (prepare_write_env->prepare_buf == NULL) {
        prepare_write_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE * sizeof(uint8_t));
        prepare_write_env->prepare_len = 0;
        if (prepare_write_env->prepare_buf == NULL) {
            ESP_LOGE(BLE_TAG, "%s, Gatt_server prep no mem", __func__);
            status = ESP_GATT_NO_RESOURCES;
        }
    } else {
        if(param->write.offset > PREPARE_BUF_MAX_SIZE) {
            status = ESP_GATT_INVALID_OFFSET;
        } else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE) {
            status = ESP_GATT_INVALID_ATTR_LEN;
        }
    }
    /*send response when param->write.need_rsp is true */
    if (param->write.need_rsp){
        esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
        if (gatt_rsp != NULL){
            gatt_rsp->attr_value.len = param->write.len;
            gatt_rsp->attr_value.handle = param->write.handle;
            gatt_rsp->attr_value.offset = param->write.offset;
            gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
            memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
            esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);
            if (response_err != ESP_OK){
               ESP_LOGE(BLE_TAG, "Send response error");
            }
            free(gatt_rsp);
        }else{
            ESP_LOGE(BLE_TAG, "%s, malloc failed", __func__);
        }
    }
    if (status != ESP_GATT_OK){
        return;
    }
    memcpy(prepare_write_env->prepare_buf + param->write.offset,
           param->write.value,
           param->write.len);
    prepare_write_env->prepare_len += param->write.len;

}

void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
    if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC && prepare_write_env->prepare_buf){
        esp_log_buffer_hex(BLE_TAG, prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
    }else{
        ESP_LOGI(BLE_TAG,"ESP_GATT_PREP_WRITE_CANCEL");
    }
    if (prepare_write_env->prepare_buf) {
        free(prepare_write_env->prepare_buf);
        prepare_write_env->prepare_buf = NULL;
    }
    prepare_write_env->prepare_len = 0;
}

static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
	BT_BLE_MSG data = {0};
	BleServer_t *bles = _ble_server;

    switch (event) {
        case ESP_GATTS_REG_EVT:{
			ESP_LOGI(BLE_TAG, "set device name %s\n", rokid_ble_name);
            esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(rokid_ble_name);
            if (set_dev_name_ret){
                ESP_LOGE(BLE_TAG, "set device name failed, error code = %x", set_dev_name_ret);
            }
    #ifdef CONFIG_SET_RAW_ADV_DATA
            esp_err_t raw_adv_ret = esp_ble_gap_config_adv_data_raw(raw_adv_data, sizeof(raw_adv_data));
            if (raw_adv_ret){
                ESP_LOGE(BLE_TAG, "config raw adv data failed, error code = %x ", raw_adv_ret);
            }
            adv_config_done |= ADV_CONFIG_FLAG;
            esp_err_t raw_scan_ret = esp_ble_gap_config_scan_rsp_data_raw(raw_scan_rsp_data, sizeof(raw_scan_rsp_data));
            if (raw_scan_ret){
                ESP_LOGE(BLE_TAG, "config raw scan rsp data failed, error code = %x", raw_scan_ret);
            }
            adv_config_done |= SCAN_RSP_CONFIG_FLAG;
    #else
            //config adv data
            esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
            if (ret){
                ESP_LOGE(BLE_TAG, "config adv data failed, error code = %x", ret);
            }
            adv_config_done |= ADV_CONFIG_FLAG;
            //config scan response data
            ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
            if (ret){
                ESP_LOGE(BLE_TAG, "config scan response data failed, error code = %x", ret);
            }
            adv_config_done |= SCAN_RSP_CONFIG_FLAG;
    #endif
            esp_err_t create_attr_ret = esp_ble_gatts_create_attr_tab(gatt_db, gatts_if, HRS_IDX_NB, SVC_INST_ID);
            if (create_attr_ret){
                ESP_LOGE(BLE_TAG, "create attr table failed, error code = %x", create_attr_ret);
            }
        }
       	    break;
        case ESP_GATTS_READ_EVT:
            ESP_LOGI(BLE_TAG, "ESP_GATTS_READ_EVT");
       	    break;
        case ESP_GATTS_WRITE_EVT:
			if (bles->listener) {
				uint16_t uuid = 0;
				if (ble_get_server_uuid(bles, param->write.handle, &uuid)){
					ESP_LOGE(BLE_TAG, "Get uuid failed, handle is %d\n", param->write.handle);
					return;
				}
				ESP_LOGI(BLE_TAG, "ESP_GATTS_WRITE_EVT uuid 0x%x, handle = %d\n", uuid, param->write.handle);
				data.ser_write.uuid = uuid;
				data.ser_write.len = param->write.len;
				memcpy(data.ser_write.value, param->write.value, param->write.len);
				data.ser_write.offset = param->write.offset;
				data.ser_write.status = 0;
				bles->listener(bles->listener_handle, BT_BLE_SE_WRITE_EVT, &data);
			}

            if (!param->write.is_prep){
                // the data length of gattc write  must be less than GATTS_CHAR_VAL_LEN_MAX.
                ESP_LOGI(BLE_TAG, "GATT_WRITE_EVT, handle = %d, value len = %d, value :", param->write.handle, param->write.len);
                esp_log_buffer_hex(BLE_TAG, param->write.value, param->write.len);
                if (heart_rate_handle_table[IDX_CHAR_CFG_A] == param->write.handle && param->write.len == 2){
                    uint16_t descr_value = param->write.value[1]<<8 | param->write.value[0];
                    if (descr_value == 0x0001){
                        ESP_LOGI(BLE_TAG, "notify enable");
                        uint8_t notify_data[15];
                        for (int i = 0; i < sizeof(notify_data); ++i)
                        {
                            notify_data[i] = i % 0xff;
                        }
                        //the size of notify_data[] need less than MTU size
                        esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, heart_rate_handle_table[IDX_CHAR_VAL_A],
                                                sizeof(notify_data), notify_data, false);
                    }else if (descr_value == 0x0002){
                        ESP_LOGI(BLE_TAG, "indicate enable");
                        uint8_t indicate_data[15];
                        for (int i = 0; i < sizeof(indicate_data); ++i)
                        {
                            indicate_data[i] = i % 0xff;
                        }
                        //the size of indicate_data[] need less than MTU size
                        esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, heart_rate_handle_table[IDX_CHAR_VAL_A],
                                            sizeof(indicate_data), indicate_data, true);
                    }
                    else if (descr_value == 0x0000){
                        ESP_LOGI(BLE_TAG, "notify/indicate disable ");
                    }else{
                        ESP_LOGE(BLE_TAG, "unknown descr value");
                        esp_log_buffer_hex(BLE_TAG, param->write.value, param->write.len);
                    }

                }
                /* send response when param->write.need_rsp is true*/
                if (param->write.need_rsp){
                    esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
                }
            }else{
                /* handle prepare write */
                example_prepare_write_event_env(gatts_if, &prepare_write_env, param);
            }
      	    break;
        case ESP_GATTS_EXEC_WRITE_EVT: 
            // the length of gattc prapare write data must be less than GATTS_CHAR_VAL_LEN_MAX. 
            ESP_LOGI(BLE_TAG, "ESP_GATTS_EXEC_WRITE_EVT");
            example_exec_write_event_env(&prepare_write_env, param);
            break;
        case ESP_GATTS_MTU_EVT:
            ESP_LOGI(BLE_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
            break;
        case ESP_GATTS_CONF_EVT:
            ESP_LOGI(BLE_TAG, "ESP_GATTS_CONF_EVT, status = %d, attr_handle %d", param->conf.status, param->conf.handle);
            break;
        case ESP_GATTS_START_EVT:
            ESP_LOGI(BLE_TAG, "SERVICE_START_EVT, status %d, service_handle %d", param->start.status, param->start.service_handle);
            break;
        case ESP_GATTS_CONNECT_EVT:
            ESP_LOGI(BLE_TAG, "ESP_GATTS_CONNECT_EVT, conn_id = %d", param->connect.conn_id);
			heart_rate_profile_tab[PROFILE_APP_IDX].conn_id = param->connect.conn_id;
            esp_log_buffer_hex(BLE_TAG, param->connect.remote_bda, 6);
            esp_ble_conn_update_params_t conn_params = {0};
            memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
            /* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
            conn_params.latency = 0;
            conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
            conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
            conn_params.timeout = 400;    // timeout = 400*10ms = 4000ms
            //start sent the update connection parameters to the peer device.
            esp_ble_gap_update_conn_params(&conn_params);

			bles->ble_server[0].conn_id = param->connect.conn_id;
			if (bles->listener) {
				data.ser_open.reason = 0;
				data.ser_open.conn_id = param->connect.conn_id;
				bles->listener(bles->listener_handle, BT_BLE_SE_OPEN_EVT, &data);
			}

            break;
        case ESP_GATTS_DISCONNECT_EVT:
            ESP_LOGI(BLE_TAG, "ESP_GATTS_DISCONNECT_EVT, reason = 0x%x", param->disconnect.reason);
			if (bles->listener) {
				data.ser_close.reason = param->disconnect.reason;
				data.ser_close.conn_id = param->disconnect.conn_id;
				bles->listener(bles->listener_handle, BT_BLE_SE_CLOSE_EVT, &data);
			}

            esp_ble_gap_start_advertising(&adv_params);
            break;
        case ESP_GATTS_CREAT_ATTR_TAB_EVT:{
            if (param->add_attr_tab.status != ESP_GATT_OK){
                ESP_LOGE(BLE_TAG, "create attribute table failed, error code=0x%x", param->add_attr_tab.status);
            }
            else if (param->add_attr_tab.num_handle != HRS_IDX_NB){
                ESP_LOGE(BLE_TAG, "create attribute table abnormally, num_handle (%d) \
                        doesn't equal to HRS_IDX_NB(%d)", param->add_attr_tab.num_handle, HRS_IDX_NB);
            }
            else {
                ESP_LOGI(BLE_TAG, "create attribute table successfully, the number handle = %d\n",param->add_attr_tab.num_handle);
                memcpy(heart_rate_handle_table, param->add_attr_tab.handles, sizeof(heart_rate_handle_table));
                esp_ble_gatts_start_service(heart_rate_handle_table[IDX_SVC]);
				ble_server_set_attr_id(bles, heart_rate_handle_table[IDX_CHAR_VAL_A], ROKID_CHAR_UUID_A);
				ble_server_set_attr_id(bles, heart_rate_handle_table[IDX_CHAR_VAL_B], ROKID_CHAR_UUID_B);
				printf("ROKID_CHAR_UUID_A handle = %d ROKID_CHAR_UUID_B handle = %d\n", heart_rate_handle_table[IDX_CHAR_VAL_A], 
						heart_rate_handle_table[IDX_CHAR_VAL_B]);
            }
            break;
        }
        case ESP_GATTS_STOP_EVT:
        case ESP_GATTS_OPEN_EVT:
        case ESP_GATTS_CANCEL_OPEN_EVT:
        case ESP_GATTS_CLOSE_EVT:
        case ESP_GATTS_LISTEN_EVT:
        case ESP_GATTS_CONGEST_EVT:
        case ESP_GATTS_UNREG_EVT:
        case ESP_GATTS_DELETE_EVT:
        default:
            break;
    }
}


static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
	BleServer_t *bles = _ble_server;

    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            heart_rate_profile_tab[PROFILE_APP_IDX].gatts_if = gatts_if;
			bles->ble_server[0].server_if = gatts_if;
        } else {
            ESP_LOGE(BLE_TAG, "reg app failed, app_id %04x, status %d",
                    param->reg.app_id,
                    param->reg.status);
            return;
        }
    }
    do {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++) {
            /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
            if (gatts_if == ESP_GATT_IF_NONE || gatts_if == heart_rate_profile_tab[idx].gatts_if) {
                if (heart_rate_profile_tab[idx].gatts_cb) {
                    heart_rate_profile_tab[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}



static int esp_ble_server_start_service(void)
{
    esp_err_t ret;

    ESP_LOGD(BLE_TAG, "Free mem at start of ble_server_start_service %d", esp_get_free_heap_size());
	ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret){
        ESP_LOGE(BLE_TAG, "gatts register error, error code = %x", ret);
        goto error;
    }

    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret){
        ESP_LOGE(BLE_TAG, "gap register error, error code = %x", ret);
        goto error;
    }

    ret = esp_ble_gatts_app_register(ESP_APP_ID);
    if (ret){
        ESP_LOGE(BLE_TAG, "gatts app register error, error code = %x", ret);
        goto error;
    }

    ret = esp_ble_gatt_set_local_mtu(500);
    if (ret){
        ESP_LOGE(BLE_TAG, "set local  MTU failed, error code = %x", ret);
    }
    ESP_LOGD(BLE_TAG, "Free mem at end of ble_server_start_service %d", esp_get_free_heap_size());
error:
	return ret;
}

static int esp_ble_server_stop_service(void)
{
#if 1
	esp_gatt_if_t gatts_if;

	gatts_if = heart_rate_profile_tab[PROFILE_APP_IDX].gatts_if;
#endif
	if (esp_ble_gap_stop_advertising()) {
		printf("Ble stop advertising failed!\n");
		return -1;
	}
#if 1
	if (esp_ble_gatts_app_unregister(gatts_if)) {
		printf("Ble gatts unregister failed!\n");
		return -1;
	}
#endif
	return 0;
}
/*****************************************************************/
static int ble_server_find_free_attr(BleServer_t *bles, uint16_t server)
{
    int index;

	BT_CHECK_HANDLE(bles);
    for (index = 0; index < BLE_ATTRIBUTE_MAX; index++)
    {
        if (!bles->ble_server[server].attr[index].uuid.uu.uuid16)
        {
            return index;
        }
    }
    return -1;
}

static int ble_server_create_service(BleServer_t *bles, int server_num, uint16_t uuid)
{
    uint16_t service;
    int attr_num;

	BT_CHECK_HANDLE(bles);
    if ((server_num < 0) || (server_num >= BLE_SERVER_MAX))
    {
        ESP_LOGE(BLE_TAG, "Wrong server number! = %d", server_num);
        return -1;
    }

    service = uuid;//rokid define
    if (!service)
    {
        ESP_LOGE(BLE_TAG, "ERROR::wrong value = %d\n", service);
        return -1;
    }

    attr_num = ble_server_find_free_attr(bles, server_num);
    if (attr_num < 0) {
        ESP_LOGE(BLE_TAG, "Wrong attr number! = %d", attr_num);
        return -1;
    }

    bles->ble_server[server_num].attr[attr_num].wait_flag = TRUE;
    /* store information on control block */
	bles->ble_server[server_num].enabled = TRUE;
    bles->ble_server[server_num].attr[attr_num].uuid.len = 2;
    bles->ble_server[server_num].attr[attr_num].uuid.uu.uuid16 = service;
    bles->ble_server[server_num].attr[attr_num].is_pri = TRUE;
    bles->ble_server[server_num].attr[attr_num].attr_type = GATTC_ATTR_TYPE_SRVC;

    return 0;
}


static int ble_server_add_char(BleServer_t *bles, int server_num, uint16_t uuid)
{
    int characteristic_property = 0;
    int char_attr_num = 0;

    BT_CHECK_HANDLE(bles);
    if ((server_num < 0) || (server_num >= BLE_SERVER_MAX))
    {
        ESP_LOGE(BLE_TAG, "Wrong server number! = %d", server_num);
        return -1;
    }

    if (bles->ble_server[server_num].enabled != TRUE)
    {
        ESP_LOGE(BLE_TAG, "ERROR::Server was not enabled!\n");
        return -1;
    }

    char_attr_num = ble_server_find_free_attr(bles, server_num);
    bles->ble_server[server_num].attr[char_attr_num].wait_flag = TRUE;

    /* save all information */
    bles->ble_server[server_num].attr[char_attr_num].uuid.len = 2;
    bles->ble_server[server_num].attr[char_attr_num].uuid.uu.uuid16 = uuid;
    bles->ble_server[server_num].attr[char_attr_num].prop = characteristic_property;
    bles->ble_server[server_num].attr[char_attr_num].attr_type = GATTC_ATTR_TYPE_CHAR;
    return 0;
}

#if 0
int ble_server_find_index_by_interface(BleServer *bles, tBSA_BLE_IF if_num)
{
    int index;

    for (index = 0; index < BLE_SERVER_MAX; index++)
    {
        if (bles->ble_server[index].server_if == if_num)
        {
            return index;
        }
    }
    return -1;
}
#endif

static int ble_get_server_uuid(BleServer_t *bles, int attr_id, uint16_t *uuid)
{
	int ret = -1;	
    int index = 0;
    int attr_num = 0;

    for (index = 0; index < BLE_SERVER_MAX; index++)
    {
        if (bles->ble_server[index].enabled)
        {
            printf("%d:BLE Server server_if:%d\n", index,
                       bles->ble_server[index].server_if);
            for (attr_num = 0; attr_num < BLE_ATTRIBUTE_MAX ; attr_num++)
            {
                if (bles->ble_server[index].attr[attr_num].uuid.uu.uuid16)
                {

                    if (bles->ble_server[index].attr[attr_num].attr_id == attr_id)
                    {
                        *uuid = bles->ble_server[index].attr[attr_num].uuid.uu.uuid16;
						ret = 0;
                    }
                }
            }
        }
    }
    return ret;
}

static int ble_server_set_attr_id(BleServer_t *bles, int attr_id, uint16_t uuid)
{
	int index;
	int attr_num;
    for (index = 0; index < BLE_SERVER_MAX; index++)
    {
        if (bles->ble_server[index].enabled)
        {
            printf("%d:BLE Server server_if:%d\n", index,
                       bles->ble_server[index].server_if);
            for (attr_num = 0; attr_num < BLE_ATTRIBUTE_MAX ; attr_num++)
            {
                if (bles->ble_server[index].attr[attr_num].uuid.uu.uuid16 == uuid) 
                {
					bles->ble_server[index].attr[attr_num].attr_id = attr_id;
					return 0;
                }
            }
        }
    }
	return -1;
}

BleServer_t *ble_create(void *caller)
{
    BleServer_t *bles = calloc(1, sizeof(*bles));

    bles->caller = caller;
    _ble_server = bles;
    return bles;
}

int ble_destroy(BleServer_t *bles)
{
    if (bles) {
        //rokid_ble_disable(bles);
        free(bles);
    }
    _ble_server = NULL;
    return 0;
}

int rokid_ble_enable(BleServer_t *bles)
{
	BT_CHECK_HANDLE(bles);
	if (bles->enabled) {
		printf("ble server is already enabled!\n");
		return 0;
	}

	if (ble_server_create_service(bles, 0, ROKID_SERVICE_UUID)) {
		printf("Failed to create ble service!\n");
		return -1;
	}
	gatt_db[IDX_SVC].att_desc.value = (uint8_t*)&ROKID_SERVICE_UUID;
	gatt_db[IDX_SVC].att_desc.length = sizeof(ROKID_SERVICE_UUID);

	if (ble_server_add_char(bles, 0, ROKID_CHAR_UUID_A)) {
		printf("Failed to add character 0x%x\n", ROKID_CHAR_UUID_A);
		return -1;
	}
	gatt_db[IDX_CHAR_VAL_A].att_desc.uuid_p = (uint8_t*)&ROKID_CHAR_UUID_A;

	if (ble_server_add_char(bles, 0, ROKID_CHAR_UUID_B)) {
		printf("Failed to add character 0x%x\n", ROKID_CHAR_UUID_B);
		return -1;
	}
	gatt_db[IDX_CHAR_VAL_B].att_desc.uuid_p = (uint8_t*)&ROKID_CHAR_UUID_B;

	if (esp_ble_server_start_service()) {
		ESP_LOGE(BLE_TAG, "Start esp ble server failed!\n");
		return -1;
	}

	bles->enabled = TRUE;
	return 0;
}

int rokid_ble_disable(BleServer_t *bles)
{
	bles->enabled = FALSE;
	memset(bles, 0, sizeof(BleServer_t));
	esp_ble_server_stop_service();
	return 0;
}

int ble_server_set_listener(BleServer_t *bles, ble_callbacks_t listener, void *listener_handle)
{

    BT_CHECK_HANDLE(bles);
    bles->listener = listener;
    bles->listener_handle = listener_handle;
   return 0;
}

int rk_ble_send(BleServer_t *bles, uint16_t uuid, uint8_t *buf, int len)
{
	esp_gatt_if_t gatts_if;
	uint16_t conn_id;

	gatts_if = heart_rate_profile_tab[PROFILE_APP_IDX].gatts_if;
	conn_id = heart_rate_profile_tab[PROFILE_APP_IDX].conn_id;
	if (ROKID_CHAR_UUID_A == uuid) {
		esp_ble_gatts_send_indicate(gatts_if, conn_id, heart_rate_handle_table[IDX_CHAR_VAL_A],
					len, buf, false);
	} else if (ROKID_CHAR_UUID_B == uuid) {
		esp_ble_gatts_send_indicate(gatts_if, conn_id, heart_rate_handle_table[IDX_CHAR_VAL_B],
					len, buf, false);
	}
    return 0;
}

int rk_ble_set_visibility(int discoverable)
{
	if (discoverable) {
		esp_ble_gap_start_advertising(&adv_params);
	} else {
		esp_ble_gap_stop_advertising();
	}
	return 0;
}

int rk_ble_set_name(const char *name)
{
	if (name) {
		strcpy(rokid_ble_name, name);
	}
	return 0;
}

