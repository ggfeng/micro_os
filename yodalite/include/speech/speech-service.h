#ifndef _SPEECH_SERVICE_H_
#define _SPEECH_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

// speech service to subscribe, client should to post
#define FLORA_SPEECH_PREPARE "speech.prepare"
#define FLORA_SPEECH_START_VOICE "speech.start.voice"
#define FLORA_SPEECH_PUT_VOICE "speech.put.voice"
#define FLORA_SPEECH_STOP_VOICE "speech.stop.voice"
#define FLORA_SPEECH_SET_VOICE_TRIGGER "speech.set.voice.trigger"
#define FLORA_SPEECH_CONFIG "speech.config"

// speech service to post, client should to subscribe 
#define FLORA_SPEECH_ERROR "speech.error"
#define FLORA_SPEECH_MESSAGE "speech.message"

typedef struct SpeechSrv SpeechSrv;
SpeechSrv *speechsrv_new();
int speechsrv_init(SpeechSrv *srv);
void speechsrv_deinit(SpeechSrv *srv);
void speechsrv_free(SpeechSrv *srv);

#ifdef __cplusplus
}
#endif

#endif
