#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "yodalite_autoconf.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

// #include "freertos/event_groups.h"

#include "mbedtls/platform.h"

#include "mbedtls/net_sockets.h"

#ifdef YODALITE
#include <osal/pthread.h>
#else
#include <pthread.h>
#endif

#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "login.h"
#include "common.h"
#include <hapi/mqtt_login.h>
#include "lib/cjson/cJSON.h"
#include <lib/mem_ext/mem_ext.h>

#include <lib/libaes/aes.h>
#include <hapi/auth/auth.h>
#include <lib/libmd5/md5.h>
#include <lib/property/properties.h>
#include <dispatcher.h>

#include "cloudgw.h"

static const char *TAG = "login";

#define WEB_SERVER "device-account.rokid.com"
#define WEB_PORT "443"
#define WEB_URL "https://www.howsmyssl.com/a/check"

// extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
// extern const uint8_t server_root_cert_pem_end[]   asm("_binary_server_root_cert_pem_end");

static const char *header = "POST https://device-account.rokid.com/device/loginV2.do HTTP/1.0\r\n"
                                "Connection: close\r\n" //general header
                                "Host: device-account.rokid.com\r\n" //request header
                                "Content-Type: application/x-www-form-urlencoded\r\n"; //entity header

struct properties_t properties;
static int  on_mqtt_message(char*topic,char *msg){

   if(topic != NULL &&  msg != NULL){

      printf("topic= %s\n", topic);
      printf("msg =  %s\n", msg);
   }

   return 0;
}

static void get_secret(char *src, char *key, char *dst)
{
    uint8_t out[16] = {0};
    int i;

	uint8_t tmp_src[64] = {0};
	uint8_t tmp_key[64] = {0};

    if(src == NULL || key == NULL || dst == NULL) {
        printf("error src:%s,key:%s,dst:%s\n", src, key, dst);
        return;
    }

    //get aec result
    strcpy(tmp_src, src);
	strcpy(tmp_key, key);
    auth_rokid_aes(tmp_src, tmp_key, dst);

    //get md5
    md5(dst, strlen(dst), out);

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
        code = cJSON_GetObjectItemCaseSensitive(monitor_json, "code");
        if(code) {
            printf("code->valuestring:%s\n",code->valuestring);
            if(memcmp(code->valuestring,"200",strlen(code->valuestring)) == 0) {
                cJSON *data = cJSON_GetObjectItemCaseSensitive(monitor_json, "data");
                if(data)
                {                
                    cJSON * basic = NULL;
                    printf("origin data:%s\n",data->valuestring);
                    cJSON *data_json = cJSON_Parse(data->valuestring);
                    if(data_json) {
                        cJSON *master_id = NULL;
                        cJSON *basic_info;
                        cJSON *key = cJSON_GetObjectItemCaseSensitive(data_json, "key");
                        cJSON *secret = cJSON_GetObjectItemCaseSensitive(data_json, "secret");
                        cJSON *account_id = cJSON_GetObjectItemCaseSensitive(data_json, "accountId");
                        cJSON *extraInfo  = cJSON_GetObjectItemCaseSensitive(data_json, "extraInfo");

                        if(extraInfo  != NULL){
                            basic_info =  cJSON_GetObjectItemCaseSensitive(extraInfo,"basic_info");
                            printf("basic_info:%s\n",basic_info->valuestring);
                            if(basic_info){
                                basic = cJSON_Parse(basic_info->valuestring);
                                if(basic)
                                   master_id = cJSON_GetObjectItemCaseSensitive(basic,"master");
                              }
                         }
                          
                        if(key != NULL && secret != NULL && account_id != NULL && master_id != NULL){
							memcpy(properties.secret, secret->valuestring, strlen(secret->valuestring));
							memcpy(properties.key, key->valuestring, strlen(key->valuestring));
                           printf("key:%s,sceret:%s,accountId:%s,masterId:%s\n",key->valuestring,secret->valuestring,account_id->valuestring,master_id->valuestring);
                           property_set("ro.key",key->valuestring);
                           property_set("ro.secret",secret->valuestring);
                           property_set("ro.account_id",account_id->valuestring);
                           property_set("ro.master_id",master_id->valuestring);

                           //yodalite_mqtt_login(account_id->valuestring,master_id->valuestring,key->valuestring,secret->valuestring,on_mqtt_message);
                      }else
                         printf("%s->%d:parm is no valid\n",__func__,__LINE__);
                     
                    }

                    if(basic)
                      cJSON_Delete(basic);

                    cJSON_Delete(data_json);
                    goto login_succeed;
                }
            } 
        }
    }
    cJSON_Delete(monitor_json);
    return -1;
login_succeed:
    cJSON_Delete(monitor_json);
    return 0;
}

static int get_rl_sign(char *secret, char *deviceid, char *deviceTypeId, char *time_stamp, char *dst)
{
    if(secret == NULL || deviceid == NULL || deviceTypeId == NULL || time_stamp == NULL || dst == NULL) {
        printf("error secret:%s,deviceid:%s,deviceTypeId:%s\n", secret, deviceid, deviceTypeId);
        return -1;
    }
    char tmp_sign[256] = {0};
    if((strlen(secret) * 2 + strlen(deviceid) + strlen(deviceTypeId) + strlen(time_stamp)) >= 256) {
        printf("tmp_sign too short\n");
        return -1;
    }
    memset(tmp_sign, 0, 256);
    strcat(tmp_sign, secret);
    strcat(tmp_sign, deviceTypeId);
    strcat(tmp_sign, deviceid);
    strcat(tmp_sign, time_stamp);
    strcat(tmp_sign, secret);

    //get md5 and upper result
    uint8_t out[16] = {0};
    int i;
    md5(tmp_sign, strlen(tmp_sign), out);
    for (i = 0; i < 16; i++) {
        sprintf(dst + i*2 ,"%02X",out[i]);
    }
    return 0;
}

int change_to_login_mode(char *masterid)
{

    pthread_t pthread_disp;
        //play need init first,then init record
    (void) pthread_create(&pthread_disp, NULL,dispatcher_init,NULL);
    pthread_detach(pthread_disp);
    return 0;

    // property_get("ro.boot.serialno",properties.deviceid,"0");
    // property_get("ro.boot.seed",properties.rokidseed,"0");
    // property_get("ro.boot.devicetypeid",properties.deviceTypeId,"0");

    // /*
    // char *deviceid = "110002460003501";
    // char *deviceTypeId = "F2E4DB269C5547BDA07684FB09FF98E2";
    // char *rokidseed = "2Cce93a6e88h5AGfrr9OvL4f4LhzW4";*/
    // if(strlen(properties.deviceid) <= 1 && strlen(properties.rokidseed) <= 1 && strlen(properties.deviceTypeId) <= 1) {
    //     printf("property get failed\n");
    //     return -1;
    // }

    // char *time_stamp = "1557014847";
    // char secret[64] = {0};
    // get_secret(properties.rokidseed, properties.deviceid, secret);
    // printf("now get secet:%s\n", secret);

    // char rl_sign[32];
    // if(get_rl_sign(secret, properties.deviceid, properties.deviceTypeId, time_stamp, rl_sign))
    // {
    //     printf("get_rl_sign failed\n");
    //     return -1;
    // }
        
    // printf("now get rl_sign:%s\n", rl_sign);
    // char *https_contents = (char *)yodalite_malloc(512);
    // memset(https_contents, 0, 512);
    // if(masterid && strlen(masterid) > 0) {
    //     sprintf(https_contents,"deviceId=%s&deviceTypeId=%s&namespaces=basic_info,custom_config&time=%s&sign=%s&userId=%s"
    //                             ,properties.deviceid, properties.deviceTypeId, time_stamp, rl_sign, masterid);
    // } else {
    //     sprintf(https_contents,"deviceId=%s&deviceTypeId=%s&namespaces=basic_info,custom_config&time=%s&sign=%s"
    //                             ,properties.deviceid, properties.deviceTypeId, time_stamp, rl_sign);
    // }

    // char *resp = https_post(header,https_contents);
    // yodalite_free(https_contents);
    // if(resp) {
    //     printf("changeto_login_mode end resp:%s\n",resp);
    //     if(json_parse(resp))
    //     {
    //         yodalite_free(resp);
    //         return -1;
    //     }
    //     yodalite_free(resp);

    //     printf("mac:%s,ip:%s\n",properties.mac,properties.ip);

    //     pthread_t pthread_disp;
    //         //play need init first,then init record
    //     (void) pthread_create(&pthread_disp, NULL,dispatcher_init,NULL);
    //     pthread_detach(pthread_disp);
    //     //dispatcher_init(); 
    //     //update_basic_info();
    //     return 0;    
    // }
    // return -1;
}
