#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <hardware/platform.h>
#include <hardware/pal_asound.h>

#include <hapi/flora_agent.h>
#include "player.h"
#ifdef CONFIG_MEM_EXT_ENABLE
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif
#define FLORA_AGET_KWS_URI "unix:/var/run/flora.sock#audio_out"
#define FLORA_KWS_COMING  "rokid.kws_voice_coming"
#define FLORA_KWS_CAPTURE "rokid.kws_voice_capture"
#define FLORA_KWS_PLAY_END "rokid.kws_voice_play_end"
static flora_agent_t audio_out_agent;
int mp3_audio_out=0;
static void handle_kws_cmd(char *buf)
{
   caps_t audio_out_msg;
   audio_out_msg = caps_create();
    if(!strcmp(buf,"AWAKE"))
    {
      play_pcm_data(mp3_audio_out);
      player_stop(mp3_audio_out);
      caps_write_string(audio_out_msg,"PLAYER_OVER");
      flora_agent_post(audio_out_agent, FLORA_KWS_PLAY_END,audio_out_msg,FLORA_MSGTYPE_INSTANT);
    }
    caps_destroy(audio_out_msg);
}

static void play_command_callback(const char *name, uint32_t msgtype, caps_t msg, void *arg){

    const char *buf = NULL;

    if (strcmp(name, FLORA_KWS_CAPTURE) == 0) {
        caps_read_string(msg, &buf);
        handle_kws_cmd(buf);
    } 
}

void flora_player_init(void)
{
    flora_agent_config_param_t param;
    audio_out_agent = flora_agent_create();
    memset(&param,0,sizeof(flora_agent_config_param_t));
    param.sock_path  = FLORA_AGET_KWS_URI;
    flora_agent_config(audio_out_agent, FLORA_AGENT_CONFIG_URI,&param );
    param.value  = 1024;
    flora_agent_config(audio_out_agent, FLORA_AGENT_CONFIG_BUFSIZE,&param);
    flora_agent_subscribe(audio_out_agent, FLORA_KWS_CAPTURE, play_command_callback, NULL);
    flora_agent_start(audio_out_agent, 0);
}

void audio_player_init(void)
{
    player_init();
    mp3_audio_out = play_local_init(AWEAK);
}

void flora_player_deinit(void)
{
   flora_agent_close(audio_out_agent);
   flora_agent_delete(audio_out_agent);
   player_destory(mp3_audio_out);
}
