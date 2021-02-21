#include "speech/speech.h"
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>

#include "hapi/caps.h"
#include "hapi/flora_agent.h"
#include "speech/speech-service.h"
#include "dispatcher.h"
#include "lib/cjson/cJSON.h"

#define DEMO_URI ""
SpeechSrv *_speech_service = NULL;
flora_agent_t _agent;

static void on_speech_error(const char *name, unsigned int msgtype, caps_t caps, void *arg)
{
    assert(caps->mem_type == CAPS_MEMBER_TYPE_INTEGER);
    printf("err %d\n", caps->mem.i32);
}

static void on_speech_message(const char *name, unsigned int msgtype, caps_t caps, void *arg)
{
    assert(caps->mem_type == CAPS_MEMBER_TYPE_STRING);

    cJSON *item;
    cJSON *root = cJSON_Parse(caps->mem.s);

    uint32_t id = 0;
    int type = 0;
    int err_code = 0;
    char *asr = "NULL";
    char *nlp = "NULL";
    char *action = "NULL";
    char *extra = "NULL";

    item = cJSON_GetObjectItem(root, "id");
    item = cJSON_GetObjectItem(root, "type");
    if (item) 
        type = (item->valueint);
    item = cJSON_GetObjectItem(root, "err_code");
    if (item) 
        err_code = (item->valueint);
    item = cJSON_GetObjectItem(root, "asr");
    if (item)
        asr = item->valuestring;
    item = cJSON_GetObjectItem(root, "nlp");
    if (item)
        nlp = item->valuestring;
    item = cJSON_GetObjectItem(root, "action");
    if (item)
        action = item->valuestring;
    item = cJSON_GetObjectItem(root, "extra");
    if (item)
        extra = item->valuestring;

    if (type == 2) {
        printf("id %u type %d err_code %d asr %s nlp %s action %s extra %s",
                id, type, err_code, asr, nlp, action, extra);
    }
}

static void on_speech_callback(const char *name, unsigned int msgtype, caps_t caps, void *arg)
{
    if (strcmp(name, FLORA_SPEECH_ERROR) == 0) {
        on_speech_error(name, msgtype, caps, arg);
    } else if (strcmp(name, FLORA_SPEECH_MESSAGE) ==0) {
        on_speech_message(name, msgtype, caps, arg);
    }
}

static void flora_speech_init(void)
{
    _speech_service = speechsrv_new();
    speechsrv_init(_speech_service);

    _agent = flora_agent_create();
    flora_agent_config_param_t param;
    memset(&param,0,sizeof(flora_agent_config_param_t));
    param.sock_path = "unix:/var/run/flora.sock#active";
    flora_agent_config(_agent,FLORA_AGENT_CONFIG_URI,&param);
    param.value = 1024;
    flora_agent_config(_agent, FLORA_AGENT_CONFIG_BUFSIZE,&param);

    // subscribe speech service message 
   // flora_agent_subscribe(_agent, FLORA_SPEECH_ERROR, on_speech_callback, NULL);
   // flora_agent_subscribe(_agent, FLORA_SPEECH_MESSAGE, on_speech_callback, NULL);
    flora_agent_start(_agent, 0);
}

static void prepare()
{
    char *host = "apigwws.open.rokid.com";
    char *port = "443";
    char *service = "speech";
    char *version = "2";
    char *branch = "/api";
    char *key = "E42B760E9C9B4F5396B1AC1C403AABE1";
    char *device_type_id = "AA12D7458EAD4AC4A8BF8ABB96FD8B74";
    char *device_id = "0414031850000019";
    char *secret = "873FC39307274BB4832EBADB584D8A30";

    // char *key = properties.key;
    // char *device_type_id = properties.deviceTypeId;
    // char *device_id = properties.deviceid;
    // char *secret = properties.secret;

    char buffer[1024];

    snprintf(buffer, sizeof(buffer), "{\"host\":\"%s\", \"port\":\"%s\", \"service\":\"%s\", \"version\":\"%s\", \"branch\":\"%s\", \"key\":\"%s\", \"device_type_id\":\"%s\", \"device_id\":\"%s\", \"secret\":\"%s\"}", host, port, service, version, branch, key, device_type_id, device_id, secret);

    caps_t msg = caps_create();
    caps_write_string(msg, buffer);
    flora_agent_post(_agent, FLORA_SPEECH_PREPARE, msg, CAPS_MEMBER_TYPE_STRING);
    caps_destroy(msg);
}


int dispatcher_init()
{
    printf("dispatch_init========\n");
    flora_speech_init();
    prepare();
    return 0;
}
