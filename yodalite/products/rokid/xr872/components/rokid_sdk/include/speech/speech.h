#ifndef _SPEECH_H_
#define _SPEECH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "utils.h"

enum SpeechError {
    SPEECH_SUCCESS = 0,
    SPEECH_UNAUTHENTICATED = 2,
    SPEECH_CONNECTION_EXCEED,
    SPEECH_SERVER_RESOURCE_EXHASTED,
    SPEECH_SERVER_BUSY,
    SPEECH_SERVER_INTERNAL,
    SPEECH_VAD_TIMEOUT,
    SPEECH_NLP_EMPTY,
    SPEECH_UNINITIALIZED,
    SPEECH_DUP_INITIALIZED,
    SPEECH_BADREQUEST,

    SPEECH_SERVICE_UNAVAILABLE = 101,
    SPEECH_SDK_CLOSED,
    SPEECH_TIMEOUT,
    SPEECH_UNKNOWN,
};

typedef struct PrepareOptions PrepareOptions;
struct PrepareOptions {
  char *host;
  char *port;
  char *branch;
  char *key;
  char *device_type_id;
  char *secret;
  char *device_id;
  char *version;
  char *service;
};

enum SpeechResultType {
    SPEECH_RES_INTER = 0,
    SPEECH_RES_START,
    SPEECH_RES_ASR_FINISH,
    SPEECH_RES_END,
    SPEECH_RES_CANCELLED,
    SPEECH_RES_ERROR
};

enum SpeechLang {
    SPEECH_ZH,
    SPEECH_EN
};

typedef struct SpeechConfig SpeechConfig;
typedef struct Speech Speech;

// 参数调用完会马上释放,如果需要请自行保存。
typedef struct {
    // 错误回调，一般为网络错误。
void (*on_error)(Speech *speech, int err_code, void *user_data);
    // asr-nlp 消息回调
    // id : session id 
    // type : 类型　@SpeechResultType
    // err_code  : 0 表示成功, 否则表示云端错误码
    // asr : asr 文本
    // nlp : nlp 文本
    // action : action 文本
    // extra : extra 文本
    // user_data : 用户设定的指针
void (*on_message)(Speech *speech, uint32_t id, uint32_t type, int err_code,
        const char *asr,    const char *nlp,
        const char *action, const char  *extra,
        char *reserved, int reserve_len, void *user_data);
    // tts 消息回调　finish表示是否最后一个消息。
    // id : session id
    // err_code : 0 表示成功　否则是云端错误码
    // finish : 是否最后一个消息
    // voice  : 音频数据, 数据格式看定义.
    // len    : 音频数据长度, 有可能为0
    // text   : tts 编码
    // user_data : 用户设定的指针
void (*on_tts)(Speech *speech, uint32_t id, int err_code, int finish, void *voice, int len, const char *text, void *user_data);
} SpeechCallbacks;

// start_voice 参数
typedef struct VoiceOptions VoiceOptions;
struct VoiceOptions {
    Chunk stack;
    Chunk voice_trigger;
    uint32_t trigger_start;
    uint32_t trigger_length;
    // 云端二次确认，默认为1
    int32_t trigger_confirm_by_cloud;
    float   voice_power;
    // json string
    // extra data that will send to skill service
    Chunk skill_options;
    Chunk voice_extra;
};

Speech *speech_create();

void speech_destroy(Speech *s);

int  speech_prepare(Speech *speech, const PrepareOptions *options);
int  speech_config(Speech *speech, SpeechConfig *config);
void speech_set_callbacks(Speech *s, SpeechCallbacks *callbacks, void *user_data);
void speech_cancel_voice(Speech *speech, unsigned int id);
void speech_end_voice(Speech *speech, unsigned int id);

//// asr-nlp 接口
int  speech_start_voice(Speech *speech, const VoiceOptions *options);
int  speech_put_voice(Speech *speech, void *voice, int size);

//// tts 接口
int  speech_put_speak(Speech *speech, const char *speak);

////// config

typedef struct SpeechConfig SpeechConfig;
struct SpeechConfig {
	int lang;           // defualt SPEECH_ZH @SpeechLang
    int codec;          // default pcm
	int vad_mode;       // cloud
	uint32_t vend_timeout;
	uint32_t vad_begin;
	char log_host[32];
	uint32_t voice_fragment;// = 0xffffffff;
	int32_t log_port;
	uint32_t no_nlp;
	uint32_t no_intermediate_asr;
	uint32_t tts_sample_rate;

	uint32_t mask;
};

void speech_config_default(SpeechConfig *config);

// default pcm
void speech_config_set_codec(SpeechConfig *config, int codec);
// default 0
void speech_config_set_vad_begin(SpeechConfig *config, int vad_begin);
void speech_config_set_vend_timeout(SpeechConfig *config, uint32_t vend_timeout);
// defualt 0xffffffff
void speech_config_set_voice_fragment(SpeechConfig *config, uint32_t fragment);
// defualt 0
void speech_config_set_no_nlp(SpeechConfig *config, uint32_t no_nlp);
// default 0
void speech_config_set_no_intermediate_asr(SpeechConfig *config, uint32_t no_intermediate_asr);
void speech_config_set_log_port(SpeechConfig *config, uint32_t log_port);
void speech_config_set_log_host(SpeechConfig *config, const char *log_host);

// defualt 24000
void speech_config_set_tts_sample_rate(SpeechConfig *config, uint32_t sample_rate);
void voice_config_default(VoiceOptions *vo);

#ifdef __cplusplus
}
#endif

#endif
