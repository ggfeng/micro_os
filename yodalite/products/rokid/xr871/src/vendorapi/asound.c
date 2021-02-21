/*
 * (C) Copyright 2019 Rokid Corp.
 * Zhu Bin <bin.zhu@rokid.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#include "hardware/platform.h"
#include "hardware/pal_asound.h"
// add by gufeng
#include "common/framework/fs_ctrl.h"
#include "common/apps/player_app.h"
#include "common/framework/platform_init.h"
#include "audio/pcm/audio_pcm.h"
#include "audio/manager/audio_manager.h"


#define PLAYER_THREAD_STACK_SIZE    (1024 * 2)
#define SOUND_PLAYCARD			AUDIO_CARD0
#define SOUND_CAPCARD			AUDIO_CARD0
static uint32_t sampleRate[] = {8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000};





#define PLAYBACK_RATE       48000
#define PLAYBACK_CHANNEL    2
#define PLAYBACK_BITS       16
struct pcm_config pcm_raw_asr_config;
static int pcm_asound_count;

static int AudioConfigIsValid(int samplerate, int channels)
{
	int i;
	int sr_num;

	if ((channels != 1) && (channels != 2))
		return 0;

	sr_num = sizeof(sampleRate) / sizeof(sampleRate[0]);
	for (i = 0;i < sr_num; i++) {
		if (sampleRate[i] == samplerate) {
			return 1;
		}
	}
	return 0;
}

static int AudioSetConfig(int samplerate, int channels, struct pcm_config *config)
{
	if (!AudioConfigIsValid(samplerate, channels)) {
		return -1;
	}

	config->channels = channels;
	config->rate = samplerate;
	config->period_size = 320;//2048;
	config->period_count = 2;
	config->format = PCM_FORMAT_S16_LE;
	config->mix_mode = 0;
	return 0;
}

static struct pcm_config xr871_pcm_read_init(struct asound_resource *pasound_res)
{
    int samplerate,channels;
    struct pcm_config config;
    samplerate = 16000;
    channels = 1;
    if (AudioSetConfig(samplerate, channels, &config)) {
	printf("invalid audio cap param.\n");
//	return 0;
    }
    if (snd_pcm_open(&config, SOUND_CAPCARD, PCM_IN) != 0)
    {
	printf("sound card open err\n");
//	return 0;
    }
    printf("snd_pcm_open ok\n");
    return config;
}


static int xr872_pcm_init(struct asound_resource *pasound_res)
{
    
    if(pasound_res->pcm_direction == YODALITE_PCM_OUT)
    {
     if(pasound_res->source == MP3_IIS)
     {
 	
	return MP3_IIS;
     }
     else if(pasound_res->source == BT_SINK_IIS)
     {
        
	return BT_SINK_IIS;
     }
     else if (pasound_res->source == HTTP_IIS) 
     {
       
       return HTTP_IIS;
     }	  
    }
    if(pasound_res->pcm_direction == YODALITE_PCM_IN)
    {
     if(pasound_res->source == RAW_ASR)
     {
      printf("start to raw asr init ++++\n");
      pcm_raw_asr_config = xr871_pcm_read_init(pasound_res);
      printf("channels:%d,rate:%d\n",pcm_raw_asr_config.channels,pcm_raw_asr_config.rate);
      return RAW_ASR;
     }
    }
    return 0;
}
static int xr872_pcm_read(int fd, void *data, unsigned int count)
{   
   if (fd == RAW_ASR)
   pcm_asound_count = snd_pcm_read(&pcm_raw_asr_config, SOUND_CAPCARD, data, count);
   return pcm_asound_count;
}





static int xr872_pcm_write(int fd, void *data, unsigned int frame_cnt)
{
   if(fd == MP3_IIS)
   {
    return MP3_IIS;
   } else if (fd == HTTP_IIS) {
    return HTTP_IIS;
   }
   return 0;
}
static int xr872_pcm_ioctl(int fd, unsigned cmd, void *pvalue)
{
   return 0;
}



static int xr872_pcm_pause(int fd)
{
   if(fd ==HTTP_IIS)
   {
     
   }
   else if(fd == MP3_IIS)
   {
     
   }
   return 0;
}

static int xr872_pcm_resume(int fd)
{
   if(fd == HTTP_IIS)
   {
     
   }
   else if(fd == MP3_IIS)
   { 
     
   }
   return 0;
}

static int xr872_pcm_stop(int fd)
{
   if(fd == HTTP_IIS)
   {
    return fd;
   }
   else if(fd == MP3_IIS)
   {
    
    return fd;
   }
   return 0;
}

static int xr872_pcm_rerun(int fd)
{
   if(fd == HTTP_IIS)
   {
    
    return fd;
   }
   else if(fd == MP3_IIS)
   {
    
    return fd; 
   }
   return 0;
}

static int xr872_pcm_destroy(int fd)
{
   if(fd == MP3_IIS)
   {

     printf("destroy mp3 iis \n");
   }else if (fd == RAW_ASR) {
     
     printf("destory raw data\n");
   }else if (fd == HTTP_IIS) {
    
    printf("destroy http iis play +++++++\n");
   }
   return 0;
}

struct asound_lapi xr872_asound_lapi = {
    .yodalite_pcm_init = xr872_pcm_init,
    .yodalite_pcm_read = xr872_pcm_read,
    .yodalite_pcm_write = xr872_pcm_write,
    .yodalite_pcm_ioctl = xr872_pcm_ioctl,
    .yodalite_pcm_destroy = xr872_pcm_destroy,
    .yodalite_pcm_pause = xr872_pcm_pause,
    .yodalite_pcm_resume = xr872_pcm_resume,
    .yodalite_pcm_stop = xr872_pcm_stop,
    .yodalite_pcm_rerun = xr872_pcm_rerun,
};

