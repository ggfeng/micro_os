#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "yodalite_autoconf.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

// #include "freertos/event_groups.h"
#include "lib/cjson/cJSON.h"

#include "mbedtls/platform.h"

#include "mbedtls/net_sockets.h"

#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "login.h"
#include "common.h"
#include "lib/cjson/cJSON.h"
#include <lib/mem_ext/mem_ext.h>

#include <lib/libaes/aes.h>
#include <hapi/auth/auth.h>
#include <lib/libmd5/md5.h>
#include <lib/property/properties.h>

static const char *TAG = "login";

#define WEB_SERVER "device-account.rokid.com"
#define WEB_PORT "443"
#define WEB_URL "https://www.howsmyssl.com/a/check"

static const char *header = "POST https://cloudapigw.open.rokid.com/v1/device/deviceManager/addOrUpdateDeviceInfo HTTP/1.0\r\n"
                                "Connection: close\r\n" //general header
                                "Host: cloudapigw.open.rokid.com\r\n" //request header
                                "Content-Type: application/json\r\n"; //entity header

static void get_sign(char *src, char *dst)
{
    uint8_t out[16] = {0};
    int i;

    //get md5
    md5(src, strlen(src), out);

    //printf("get md5 out:%s\n",out);
    //upper md5 result
    memset(dst,0,strlen(dst));
    for (i = 0; i < 16; i++) {
        sprintf(dst + i*2 ,"%02X",out[i]);
    }
}

static int json_parse(char *msg)
{
    cJSON *monitor_json = cJSON_Parse(msg);
    cJSON *code = NULL;

    if (monitor_json)
    {
        code = cJSON_GetObjectItemCaseSensitive(monitor_json, "resultCode");
        if(code) {
            printf("code->valuestring:%s\n",code->valueint);
            if(code->valueint)
               goto update_succeed;
        }
    }
    cJSON_Delete(monitor_json);
    return -1;
update_succeed:
    cJSON_Delete(monitor_json);
    return 0;
}


int update_basic_info()
{

    char *time_stamp = "1557014847";
    char sign[64] = {0};

    char *data = (char *)yodalite_malloc(512);
    memset(data, 0, 512);
    sprintf(data,"key=%s&device_type_id=%s&device_id=%s&service=undefined&version=1&time=1563280872&secret=%s"
                                ,properties.key, properties.deviceTypeId,properties.deviceid, properties.secret);
    printf("now data:%s\n",data);
    get_sign(data, sign);
    yodalite_free(data);

    printf("now get sign:%s\n", sign);

    char *auth = (char *)yodalite_malloc(512);
    memset(auth, 0, 512);
    sprintf(auth,"version=1;time=1563280872;sign=%s;key=%s;deviceTypeId=%s;device_id=%s;service=undefined"
                                ,sign, properties.key, properties.deviceTypeId,properties.deviceid);
    printf("now get auth:%s\n", auth);


    char *https_header = (char *)yodalite_malloc(1024);
    memset(https_header, 0, 1024);

    sprintf(https_header, "%sAuthorization: %s\r\n\r\n", header, auth);

    //sprintf(https_header, "%sAuthorization: %s\r\n\r\n", header, "version=1;time=1563278909;sign=D26C0F6200EE88F0460D3AA647D486E6;key=C2B4B0FA918F44178DA5FEE5EB99334A;device_type_id=F2E4DB269C5547BDA07684FB09FF98E2;device_id=110002460003501;service=undefined");
    
    yodalite_free(auth); 

    char *https_contents = (char *)yodalite_malloc(512);
    memset(https_contents, 0, 512);

    sprintf(https_contents,"{\"namespace\":\"basic_info\",\"values\":{\"sn\":\"%s\",\"device_id\":\"%s\",\"device_type_id\":\"%s\",\"ota\":\"7.36.1-20190716-082153\",\"mac\":\"10:d0:7a:6f:20:3b\",\"lan_ip\":\"10.88.4.126\"}}"
                                ,properties.deviceid, properties.deviceid, properties.deviceTypeId);
    //sprintf(https_contents,"{\"namespace\":\"basic_info\",\"values\":{\"sn\":\"110002460003501\",\"device_id\":\"110002460003501\",\"device_type_id\":\"F2E4DB269C5547BDA07684FB09FF98E2\",\"ota\":\"7.36.1-20190716-082153\",\"mac\":\"10:d0:7a:6f:20:3b\",\"lan_ip\":\"10.88.4.126\"}}");
    printf("https_contents:%s\n",https_contents);
    char *resp = https_post(https_header,https_contents);
    yodalite_free(https_contents);
    yodalite_free(https_header);

    if(resp) {
        printf("update_basic_info resp:%s\n",resp);
        if(json_parse(resp))
        {
            yodalite_free(resp);
            return -1;
        }
        yodalite_free(resp);
        return 0;     
    }
    return -1;
}
