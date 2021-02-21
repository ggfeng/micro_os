#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <yodalite_autoconf.h>
#include <unistd.h>
#include <assert.h>
#include <hardware/platform.h>
#include <hardware/pal_asound.h>

#include <hapi/flora_agent.h>
#include "player.h"
#include "kws.h"
#include <lib/mem_ext/mem_ext.h>
#include <lib/cjson/cJSON.h>

#ifdef CONFIG_MEM_EXT_ENABLE
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif
#define FLORA_AGET_KWS_URI "unix:/var/run/flora.sock#record"
#define FLORA_SPEECH_START_VOICE "speech.start.voice"
#define FLORA_SPEECH_MESSAGE "speech.message"
#define FLORA_SPEECH_PUT_VOICE "speech.put.voice"
#define FLORA_KWS_CAPTURE  "rokid.kws_voice_capture" 
#define FLORA_KWS_PLAY_END "rokid.kws_voice_play_end"
#define BUFF_MS_SIZE   480  //30ms
static flora_agent_t record_agent;
caps_t kws_data_msg;
int kws_flag_flora = 0;
static void start_voice();
static void on_speech_message(const char *name, unsigned int msgtype, caps_t caps, void *arg);
static void start_voice()
{
    char buffer[10] = { 0 };

    caps_t msg = caps_create();
    caps_write_string(msg, buffer);
    flora_agent_post(record_agent, FLORA_SPEECH_START_VOICE, msg, CAPS_MEMBER_TYPE_STRING);
    caps_destroy(msg);
}


static void kws_command_callback(const char *name, uint32_t msgtype, caps_t msg, void *arg){
    printf("kws player %s\n",name);
    if (strcmp(name, FLORA_KWS_PLAY_END) == 0) {
     printf("player over++++\n");
     kws_flag_flora = 1;
    } 
     else if (strcmp(name, FLORA_SPEECH_MESSAGE) ==0) {
        on_speech_message(name, msgtype,msg, arg);
    }
}
static void on_speech_message(const char *name, unsigned int msgtype, caps_t caps, void *arg)
{
    assert(caps->mem_type == CAPS_MEMBER_TYPE_STRING);

    cJSON *item;
    cJSON *root /* = cJSON_Parse(caps->mem.s)*/;

    uint32_t id = 0;
    int type = 0;
    int err_code = 0;
    char *asr = "NULL";
    char *nlp = "NULL";
    char *action = "NULL";
    char *extra = "NULL";
    char *buf;

    caps_read_string(caps, &buf);

    printf("####%s->%d:%s\n",__func__,__LINE__,buf);
    root = cJSON_Parse(buf);
	//root = cJSON_Parse(caps->mem.s);

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
	printf("pcm send data over\n");
        kws_flag_flora = 0;
        printf("id %u type %d err_code %d asr %s nlp %s action %s extra %s\n",
                id, type, err_code, asr, nlp, action, extra);
    }

    cJSON_Delete(root);
}



void record_init(void)
{
    struct yodalite_pcm_config *config;
    unsigned int card, device, flags;
    caps_t kws_msg;
    caps_t kws_data_msg;
    void *pcm_data;
    config = (struct yodalite_pcm_config *)yodalite_malloc(sizeof(struct yodalite_pcm_config));
    memset(config, 0x0, sizeof(struct yodalite_pcm_config));
    card = 1;
    device = 1;
    flags = YODALITE_PCM_IN;
    config->channels = 1;
    config->rate = 16000;
    config->format = YODA_PCM_FORMAT_S16_LE;
    config->period_count = sizeof(short);
    config->source = RAW_ASR;
    config->period_size = 480;
    int us_pcm_read = pcm_open(card, device, flags, config);
    yodalite_free(config);
    printf("us_pcm_read+++%d\n",us_pcm_read);
    float thresholdSetting[3] = {0.63,1.0,1.0};
    unsigned char channel_num = 1;
    unsigned char min_gap = 5;
    kws_task task = kws_create("123", channel_num, thresholdSetting, min_gap);
    pcm_data = yodalite_malloc(BUFF_MS_SIZE*2);
    if (NULL == pcm_data) {
        printf("Memory allocation failed!\n");
        kws_destroy(task);
        task = NULL;
	yodalite_free(pcm_data);
        return;
    }
    static float *in_ptr[8];
    static float in_buffer[BUFF_MS_SIZE];
    int  pcm_buff_count;
    int i;
    in_ptr[0] = in_buffer;
    while (1) 
    {
       pcm_read(us_pcm_read,  (void *)pcm_data, BUFF_MS_SIZE*2 );
       for (i = 0; i < BUFF_MS_SIZE; i++) {
           in_buffer[i]= (float)((short *)pcm_data)[i]*20;
      }
        int active_status = kws_status(task, (const float ** )in_ptr, BUFF_MS_SIZE,NULL);
#if 1
      if (active_status == 1) {
         kws_msg = caps_create();
         caps_write_string(kws_msg, "AWAKE");
         flora_agent_post(record_agent, FLORA_KWS_CAPTURE,kws_msg,FLORA_MSGTYPE_INSTANT);
         caps_destroy(kws_msg);
	 start_voice();
         printf("Wake up111111\n");
        }

       if(kws_flag_flora == 1)
       {
        kws_data_msg = caps_create();
        caps_write_binary(kws_data_msg,(void *)pcm_data,2*BUFF_MS_SIZE);
        flora_agent_post(record_agent,FLORA_SPEECH_PUT_VOICE,kws_data_msg,CAPS_MEMBER_TYPE_BINARY);
        caps_destroy(kws_data_msg);
       
     }
#endif
#if 0
      if (active_status == 1) {
        
	 printf("wake up 00000\n");
	 kws_msg = caps_create();
         caps_write_string(kws_msg, "AWAKE");
         flora_agent_post(record_agent, FLORA_KWS_CAPTURE,kws_msg,FLORA_MSGTYPE_INSTANT);
         caps_destroy(kws_msg);

	}
#endif
   }
    yodalite_free(pcm_data);
}

void flora_record_init(void)
{
    flora_agent_config_param_t param;
    record_agent = flora_agent_create();
    memset(&param,0,sizeof(flora_agent_config_param_t));
    param.sock_path  = FLORA_AGET_KWS_URI;
    flora_agent_config(record_agent, FLORA_AGENT_CONFIG_URI,&param);
    param.value  = 1024;
    flora_agent_config(record_agent, FLORA_AGENT_CONFIG_BUFSIZE,&param);
    flora_agent_subscribe(record_agent, FLORA_KWS_PLAY_END, kws_command_callback, NULL);
    flora_agent_subscribe(record_agent, FLORA_SPEECH_MESSAGE, kws_command_callback, NULL);
    flora_agent_start(record_agent, 0);
}


void flora_record_deinit(void)
{
   flora_agent_close(record_agent);
   flora_agent_delete(record_agent);
}

