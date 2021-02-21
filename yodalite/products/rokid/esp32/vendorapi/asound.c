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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "board.h"
#include "audio_common.h"
#include "audio_pipeline.h"
#include "mp3_decoder.h"
#include "i2s_stream.h"
#include "raw_stream.h"
#include "filter_resample.h"
#include "esp_peripherals.h"
#include "bluetooth_service.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "http_stream.h"
#include "periph_wifi.h"
#include "esp_peripherals.h"
#include "wav_decoder.h"



static const char *TAG = "rokid yodaliteos asound demo";

extern const uint8_t startup_mp3_start[] asm("_binary_startup_mp3_start");
extern const uint8_t startup_mp3_end[]    asm("_binary_startup_mp3_end");

extern const uint8_t play_err_mp3_start[] asm("_binary_play_err_mp3_start");
extern const uint8_t play_err_mp3_end[]    asm("_binary_play_err_mp3_end");

extern const uint8_t awake_mp3_start[] asm("_binary_awake_mp3_start");
extern const uint8_t awake_mp3_end[]    asm("_binary_awake_mp3_end");


int audio_chunksize;
#define PLAYBACK_RATE       48000
#define PLAYBACK_CHANNEL    2
#define PLAYBACK_BITS       16

enum decode_source{
    MP3_source = 0,
    WAV_source,
};
struct pcm_element_handle
{
    audio_element_handle_t i2s_t;
    audio_element_handle_t bt_t;
    audio_element_handle_t fatfs_t;
    audio_element_handle_t mp3_t;
    audio_element_handle_t filter_t;
    audio_element_handle_t http_t;
    audio_element_handle_t rsp_t;
    audio_element_handle_t raw_t;
	
};

typedef struct pcm_element_handle pcm_element_handle_t;

struct pcm_config_init{
    audio_pipeline_handle_t pipeline;
    esp_periph_set_handle_t set; 
    esp_periph_handle_t periph_cfg;
    audio_event_iface_handle_t evt;
    pcm_element_handle_t el;
    int file_source;
    int url_source;
};

typedef struct pcm_config_init *pcm_config_init_t;
static int startup_mp3_pos,awake_mp3_pos,play_err_pos;

audio_pipeline_handle_t pipeline_rec,pipeline_play;
audio_element_handle_t	mp3_decoder,i2s_stream_writer;
audio_element_handle_t i2s_stream_reader, filter;
pcm_config_init_t pcm_bt_config,pcm_mp3_config,pcm_sd_config,pcm_http_config;
pcm_config_init_t pcm_raw_sd_config,pcm_raw_asr_config;

static audio_element_handle_t create_raw()
{
    raw_stream_cfg_t raw_cfg = {
        .out_rb_size = 8 * 1024,
        .type = AUDIO_STREAM_READER,
    };
    return raw_stream_init(&raw_cfg);
}

static audio_element_handle_t create_filter(int source_rate, int source_channel, int dest_rate, int dest_channel, int mode)
{
    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_cfg.src_rate = source_rate;
    rsp_cfg.src_ch = source_channel;
    rsp_cfg.dest_rate = dest_rate;
    rsp_cfg.dest_ch = dest_channel;
    rsp_cfg.type = mode;
    return rsp_filter_init(&rsp_cfg);
}

static audio_element_handle_t create_i2s_stream(int sample_rates, int bits, int channels, audio_stream_type_t type)
{
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = type;
    audio_element_handle_t i2s_stream = i2s_stream_init(&i2s_cfg);
    mem_assert(i2s_stream);
    audio_element_info_t i2s_info = {0};
    audio_element_getinfo(i2s_stream, &i2s_info);
    i2s_info.bits = bits;
    i2s_info.channels = channels;
    i2s_info.sample_rates = sample_rates;
    audio_element_setinfo(i2s_stream, &i2s_info);
    return i2s_stream;
}

int play_err_mp3_cb(audio_element_handle_t el, char *buf, int len, TickType_t wait_time, void *ctx)
{
    int read_size = play_err_mp3_end - play_err_mp3_start - play_err_pos;
    if (read_size == 0) {
        return AEL_IO_DONE;
    } else if (len < read_size) {
        read_size = len;
    }
    memcpy(buf, play_err_mp3_start + play_err_pos, read_size);
    play_err_pos += read_size;
    return read_size;
}

int startup_mp3_cb(audio_element_handle_t el, char *buf, int len, TickType_t wait_time, void *ctx)
{
    int read_size = startup_mp3_end - startup_mp3_start - startup_mp3_pos;
    if (read_size == 0) {
        return AEL_IO_DONE;
    } else if (len < read_size) {
        read_size = len;
    }
    memcpy(buf, startup_mp3_start + startup_mp3_pos, read_size);
    startup_mp3_pos += read_size;
    return read_size;
}

int awake_mp3_cb(audio_element_handle_t el, char *buf, int len, TickType_t wait_time, void *ctx)
{
    int read_size = awake_mp3_end - awake_mp3_start - awake_mp3_pos;
    if (read_size == 0) {
        return AEL_IO_DONE;
    } else if (len < read_size) {
        read_size = len;
    }
    memcpy(buf, awake_mp3_start + awake_mp3_pos, read_size);
    awake_mp3_pos += read_size;
    return read_size;
}

static pcm_config_init_t esp32_pcm_read_init(struct asound_resource *pasound_res)
{
    audio_pipeline_handle_t pipeline = NULL;
    pcm_config_init_t pcm_config = calloc(1,sizeof(struct pcm_config_init));
   // sleep(1);
   // audio_board_handle_t board_handle = audio_board_init();
   // audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    audio_element_handle_t i2s_stream_reader = create_i2s_stream(48000, 16, 2, AUDIO_STREAM_READER);
    audio_element_handle_t  filter = create_filter(48000,2,16000,1,AUDIO_CODEC_TYPE_ENCODER);
    audio_element_handle_t raw_read = create_raw();
    audio_pipeline_register(pipeline, i2s_stream_reader, "i2s");
    audio_pipeline_register(pipeline, filter, "filter");
    audio_pipeline_register(pipeline, raw_read, "raw");
    audio_pipeline_link(pipeline, (const char *[]) {"i2s", "filter", "raw"}, 3);
    pcm_config->pipeline = pipeline;
    pcm_config->el.i2s_t = i2s_stream_reader;
    pcm_config->el.filter_t = filter;
    pcm_config->el.raw_t= raw_read;
    return pcm_config;
}

static pcm_config_init_t esp32_http_iis_write_init(struct asound_resource *pasound_res)
{
    char *url;
    char *test_url;
    test_url = ".mp3";
    audio_pipeline_handle_t pipeline = NULL;

    pcm_config_init_t pcm_config = calloc(1, sizeof(struct pcm_config_init));
    AUDIO_MEM_CHECK(TAG, pcm_config, return NULL);
    url = pasound_res->url_str;
    if(strstr(url,test_url))
    {
      printf("start mp3++++++\n");
      pcm_config->url_source = MP3_source;
    }
    else
    {
      printf("wav++++++\n");
      pcm_config->url_source = WAV_source;
    }
    printf("url:%s/n",url);

    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);

    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);
    http_stream_cfg_t http_cfg = HTTP_STREAM_CFG_DEFAULT();
    audio_element_handle_t http_stream_reader = http_stream_init(&http_cfg);
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    audio_element_handle_t i2s_stream_writer = i2s_stream_init(&i2s_cfg);
    audio_pipeline_register(pipeline, http_stream_reader, "http");
    if(pcm_config->url_source == MP3_source)
    {
      mp3_decoder_cfg_t mp3_cfg = DEFAULT_MP3_DECODER_CONFIG();
      audio_element_handle_t mp3_decoder = mp3_decoder_init(&mp3_cfg);
      audio_pipeline_register(pipeline, mp3_decoder,"mp3");
      pcm_config->el.mp3_t = mp3_decoder;
    }
    else if(pcm_config->url_source == WAV_source)
    {
      wav_decoder_cfg_t wav_cfg = DEFAULT_WAV_DECODER_CONFIG();
      audio_element_handle_t  wav_decoder = wav_decoder_init(&wav_cfg);	  
      audio_pipeline_register(pipeline,wav_decoder,"wav");  
      pcm_config->el.mp3_t = wav_decoder;
    }
    audio_element_handle_t resample = create_filter(48000, 2, 48000, 2, RESAMPLE_DECODE_MODE);
   // audio_pipeline_register(pipeline, resample, "filter");
    audio_pipeline_register(pipeline, i2s_stream_writer,  "i2s");

    if(pcm_config->url_source == MP3_source)
      audio_pipeline_link(pipeline, (const char *[]) {"http", "mp3","i2s"}, 3);
    else if(pcm_config->url_source == WAV_source) 	
      audio_pipeline_link(pipeline, (const char *[]) {"http", "wav","i2s"}, 3);
    audio_element_set_uri(http_stream_reader, url);
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt_http = audio_event_iface_init(&evt_cfg);
    audio_pipeline_set_listener(pipeline, evt_http);
    pcm_config->pipeline = pipeline;
    pcm_config->evt = evt_http;
    pcm_config->el.http_t = http_stream_reader;
    pcm_config->el.i2s_t = i2s_stream_writer;
    pcm_config->el.rsp_t = resample;
    return pcm_config;
}

static pcm_config_init_t esp32_bt_sink_write_init(struct asound_resource *pasound_res)
{
    pcm_config_init_t pcm_config = calloc(1,sizeof(struct pcm_config_init));
#if 0 
	audio_pipeline_handle_t pipeline;

	esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    printf("Create Bluetooth service \n");
    bluetooth_service_cfg_t bt_cfg = {
        .device_name = "ESP-ADF-SPEAKER",
        .mode = BLUETOOTH_A2DP_SINK,
    };
    bluetooth_service_start(&bt_cfg);

    printf("Start codec chip \n");
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);
	
	esp_periph_handle_t bt_periph  = bluetooth_service_create_periph(); 

    esp_periph_config_t periph_cfg = DEFAULT_ESP_PHERIPH_SET_CONFIG();
    esp_periph_set_handle_t  set_bt = esp_periph_set_init(&periph_cfg);
    esp_periph_start(set_bt, bt_periph);
	
    printf("Create audio pipeline for playback\n");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);

    audio_element_handle_t i2s_stream_writer = create_i2s_stream(pasound_res->sample_rate, pasound_res->format, pasound_res->channels, AUDIO_STREAM_WRITER);
    

    printf("Get Bluetooth stream \n");
    audio_element_handle_t bt_stream_reader = bluetooth_service_create_stream();

   
    printf("Register all elements to audio pipeline \n");
    audio_pipeline_register(pipeline, bt_stream_reader, "bt");
    audio_pipeline_register(pipeline, i2s_stream_writer, "i2s");
    ESP_LOGI(TAG, "[ 5 ] Set up  event listener");
	
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt_bt = audio_event_iface_init(&evt_cfg);

    printf("Link it together [Bluetooth]-->bt_stream_reader-->i2s_stream_writer-->[codec_chip] \n");
   // audio_pipeline_link(pipeline, (const char *[]) {"bt", "i2s"}, 2);
	
    pcm_config->pipeline = pipeline;
	pcm_config->set = set_bt;
	pcm_config->evt = evt_bt;
	pcm_config->periph_cfg = bt_periph;
    pcm_config->el.bt_t = bt_stream_reader;
	pcm_config->el.i2s_t = i2s_stream_writer;
#endif
    return pcm_config;
}


static pcm_config_init_t esp32_mp3_iis_write_init(struct asound_resource *pasound_res)
{
   audio_pipeline_handle_t pipeline = NULL;
   pcm_config_init_t pcm_config = calloc(1,sizeof(struct pcm_config_init));
  // audio_board_handle_t board_handle = audio_board_init();
  // audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);
   audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
   pipeline = audio_pipeline_init(&pipeline_cfg);
   mp3_decoder_cfg_t mp3_cfg = DEFAULT_MP3_DECODER_CONFIG();
   audio_element_handle_t mp3_decoder = mp3_decoder_init(&mp3_cfg);
   if(pasound_res->file_source == START_UP)
    {
     audio_element_set_read_cb(mp3_decoder, startup_mp3_cb, NULL);
    }
    else if(pasound_res->file_source == AWEAK)
    {
     audio_element_set_read_cb(mp3_decoder, awake_mp3_cb, NULL);
    }
    else if(pasound_res->file_source == PLAY_ERR)
    {
     audio_element_set_read_cb(mp3_decoder, play_err_mp3_cb, NULL);
    }
   audio_element_handle_t i2s_stream_writer = create_i2s_stream(pasound_res->sample_rate, pasound_res->format, pasound_res->channels, AUDIO_STREAM_WRITER);
    audio_element_handle_t resample = create_filter(48000, 2, 48000, 2, RESAMPLE_DECODE_MODE);
    audio_pipeline_register(pipeline, mp3_decoder, "mp3");
    audio_pipeline_register(pipeline, resample, "filter");
    audio_pipeline_register(pipeline, i2s_stream_writer, "i2s");
    audio_pipeline_link(pipeline, (const char *[]) {"mp3", "filter", "i2s"}, 3);	  
    audio_event_iface_cfg_t evt_cfg_mp3 = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt_mp3 = audio_event_iface_init(&evt_cfg_mp3);
    audio_pipeline_set_listener(pipeline, evt_mp3);
    pcm_config->pipeline = pipeline;
    pcm_config->set = NULL;
    pcm_config->evt = evt_mp3;
    pcm_config->periph_cfg = NULL;
    pcm_config->el.i2s_t = i2s_stream_writer;
    pcm_config->el.mp3_t = mp3_decoder;
    pcm_config->el.rsp_t = resample;
    pcm_config->file_source = pasound_res->file_source;
    return pcm_config;
}
static int esp32_pcm_init(struct asound_resource *pasound_res)
{
    static char cordc_init_flag;
    if(cordc_init_flag == 0)
    {
      cordc_init_flag = 1;
      audio_board_handle_t board_handle = audio_board_init();
      audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);
    }
    if(pasound_res->pcm_direction == YODALITE_PCM_OUT)
    {
     if(pasound_res->source == MP3_IIS)
     {
 	pcm_mp3_config = esp32_mp3_iis_write_init(pasound_res);
	return MP3_IIS;
     }
     else if(pasound_res->source == BT_SINK_IIS)
     {
        pcm_bt_config = esp32_bt_sink_write_init(pasound_res);
	return BT_SINK_IIS;
     }
     else if (pasound_res->source == HTTP_IIS) 
     {
       pcm_http_config = esp32_http_iis_write_init(pasound_res);
       return HTTP_IIS;
     }	  
    }
    if(pasound_res->pcm_direction == YODALITE_PCM_IN)
    {
     if(pasound_res->source == RAW_ASR)
     {
       pcm_raw_asr_config = esp32_pcm_read_init(pasound_res);
       audio_pipeline_run(pcm_raw_asr_config->pipeline);
       return RAW_ASR;
     }
    }
    return 0;
}
static int esp32_pcm_read(int fd, void *data, unsigned int count)
{   
   if (fd == RAW_ASR)
    raw_stream_read(pcm_raw_asr_config->el.raw_t, (char *)data, count);
   return 0;
}

static int play_mp3_iis(pcm_config_init_t pcm_config)
{
  if(pcm_config->file_source == AWEAK)
  {
    awake_mp3_pos = 0;
  }
  else if(pcm_config->file_source == PLAY_ERR)
  {
    startup_mp3_pos = 0;
  }
  else if(pcm_config->file_source == START_UP)
  {
   play_err_pos = 0;
  }   
    audio_pipeline_reset_ringbuffer(pcm_config->pipeline);
    audio_pipeline_reset_items_state(pcm_config->pipeline);
    audio_pipeline_run(pcm_config->pipeline);

    while (1) {
        audio_event_iface_msg_t msg;
        memset(&msg, 0, sizeof(audio_event_iface_msg_t));
        esp_err_t ret = audio_event_iface_listen(pcm_config->evt, &msg, portMAX_DELAY);
		if(ret != ESP_OK)
			continue;
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *)(pcm_config->el.mp3_t)
            && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO) {
            audio_element_info_t music_info = {0};
            audio_element_getinfo(pcm_config->el.mp3_t, &music_info);
            rsp_filter_set_src_info(pcm_config->el.rsp_t, music_info.sample_rates, music_info.channels);
            continue;
        }
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) (pcm_config->el.i2s_t)
            && msg.cmd == AEL_MSG_CMD_REPORT_STATUS
            && (((int)msg.data == AEL_STATUS_STATE_STOPPED) || ((int)msg.data == AEL_STATUS_STATE_FINISHED))) {
            if(pcm_config->file_source == AWEAK)
            {
              awake_mp3_pos = 0;
            }
            else if(pcm_config->file_source ==START_UP)
            {
  	      startup_mp3_pos = 0;
            }
	    else if(pcm_config->file_source ==PLAY_ERR)
	    {
	      play_err_pos = 0;
	    }
           // audio_pipeline_stop(pcm_config->pipeline);
           // audio_pipeline_wait_for_stop(pcm_config->pipeline);
           // audio_pipeline_terminate(pcm_config->pipeline);
            break;
        }
    }
    return 0;
}

static int play_http_iis(pcm_config_init_t pcm_config)
{
    audio_pipeline_reset_ringbuffer(pcm_config->pipeline);
    audio_pipeline_reset_items_state(pcm_config->pipeline);
    audio_pipeline_run(pcm_config->pipeline);
    while (1) {
        audio_event_iface_msg_t msg;		
	memset(&msg,0,sizeof(audio_event_iface_msg_t));
        esp_err_t ret = audio_event_iface_listen(pcm_config->evt, &msg, portMAX_DELAY);
        if (ret != ESP_OK) {
            continue;
        }

        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT
            && msg.source == (void *)(pcm_config->el.mp3_t)
            && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO) {
            audio_element_info_t music_info = {0};
            audio_element_getinfo(pcm_config->el.mp3_t, &music_info);
            audio_element_setinfo(pcm_config->el.i2s_t, &music_info);
            i2s_stream_set_clk(pcm_config->el.i2s_t, music_info.sample_rates, music_info.bits, music_info.channels);
            // rsp_filter_set_src_info(pcm_config->el.rsp_t, music_info.sample_rates, music_info.channels);
            continue;
        }
	ESP_LOGE(TAG, "source_type=%d, data=%d, cmd=%d",msg.source_type, (int)msg.data, (int)msg.cmd);
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) (pcm_config->el.i2s_t)
            && msg.cmd == AEL_MSG_CMD_REPORT_STATUS && (((int)msg.data == AEL_STATUS_STATE_STOPPED) || ((int)msg.data == AEL_STATUS_STATE_FINISHED)))  {
	   // audio_pipeline_stop(pcm_config->pipeline);
           // audio_pipeline_terminate(pcm_config->pipeline);
            break;
        }
    }
    return 0;
}

static int esp32_pcm_write(int fd, void *data, unsigned int frame_cnt)
{
   if(fd == MP3_IIS)
   {
    play_mp3_iis(pcm_mp3_config);
    return MP3_IIS;
   } else if (fd == HTTP_IIS) {
    play_http_iis(pcm_http_config);
    return HTTP_IIS;
   }
   return 0;
}
static int esp32_pcm_ioctl(int fd, unsigned cmd, void *pvalue)
{
   return 0;
}

void  resume_iis(pcm_config_init_t pcm_config)
{
   audio_pipeline_resume(pcm_config->pipeline);
}


void  pause_iis(pcm_config_init_t pcm_config)
{
   audio_pipeline_pause(pcm_config->pipeline);
}

static int esp32_pcm_pause(int fd)
{
   if(fd ==HTTP_IIS)
   {
     pause_iis(pcm_http_config);
   }
   else if(fd == MP3_IIS)
   {
     pause_iis(pcm_mp3_config);
   }
   return 0;
}

static int esp32_pcm_resume(int fd)
{
   if(fd == HTTP_IIS)
   {
     resume_iis(pcm_http_config);
   }
   else if(fd == MP3_IIS)
   { 
     resume_iis(pcm_mp3_config);
   }
   return 0;
}

static int esp32_pcm_stop(int fd)
{
   if(fd == HTTP_IIS)
   {
    audio_pipeline_stop(pcm_http_config->pipeline);
    audio_pipeline_wait_for_stop(pcm_http_config->pipeline);
    audio_pipeline_terminate(pcm_http_config->pipeline);
    return fd;
   }
   else if(fd == MP3_IIS)
   {
    audio_pipeline_stop(pcm_mp3_config->pipeline);
    audio_pipeline_wait_for_stop(pcm_mp3_config->pipeline);
    audio_pipeline_terminate(pcm_mp3_config->pipeline);
    return fd;
   }
   return 0;
}

static int esp32_pcm_rerun(int fd)
{
   if(fd == HTTP_IIS)
   {
    audio_pipeline_reset_ringbuffer(pcm_http_config->pipeline);
    audio_pipeline_reset_items_state(pcm_http_config->pipeline);
    audio_pipeline_run(pcm_http_config->pipeline);
    return fd;
   }
   else if(fd == MP3_IIS)
   {
    audio_pipeline_reset_ringbuffer(pcm_mp3_config->pipeline);
    audio_pipeline_reset_items_state(pcm_mp3_config->pipeline);
    audio_pipeline_run(pcm_mp3_config->pipeline);
    return fd; 
   }
   return 0;
}

static int esp32_pcm_destroy(int fd)
{
   if(fd == MP3_IIS)
   {
     audio_pipeline_terminate(pcm_mp3_config->pipeline);
     audio_pipeline_unregister(pcm_mp3_config->pipeline, pcm_mp3_config->el.mp3_t);
     audio_pipeline_unregister(pcm_mp3_config->pipeline, pcm_mp3_config->el.i2s_t);
     audio_pipeline_unregister(pcm_mp3_config->pipeline, pcm_mp3_config->el.rsp_t);
     audio_pipeline_remove_listener(pcm_mp3_config->pipeline);
     audio_event_iface_destroy(pcm_mp3_config->evt);
     audio_pipeline_deinit(pcm_mp3_config->pipeline);
     audio_element_deinit(pcm_mp3_config->el.mp3_t);
     audio_element_deinit(pcm_mp3_config->el.i2s_t);
     audio_element_deinit(pcm_mp3_config->el.rsp_t);
     free(pcm_mp3_config);
     printf("destroy mp3 iis \n");
   }else if (fd == RAW_ASR) {
     audio_pipeline_terminate(pcm_raw_asr_config->pipeline);
     audio_pipeline_remove_listener(pcm_raw_asr_config->pipeline);
     audio_pipeline_unregister(pcm_raw_asr_config->pipeline, pcm_raw_asr_config->el.raw_t);
     audio_pipeline_unregister(pcm_raw_asr_config->pipeline, pcm_raw_asr_config->el.i2s_t);
     audio_pipeline_unregister(pcm_raw_asr_config->pipeline, pcm_raw_asr_config->el.filter_t);
     audio_pipeline_deinit(pcm_raw_asr_config->pipeline);
     audio_element_deinit(pcm_raw_asr_config->el.raw_t);
     audio_element_deinit(pcm_raw_asr_config->el.i2s_t);
     audio_element_deinit(pcm_raw_asr_config->el.filter_t);
     printf("destory raw data\n");
   }else if (fd == HTTP_IIS) {
    audio_pipeline_terminate(pcm_http_config->pipeline);
    audio_pipeline_unregister(pcm_http_config->pipeline, pcm_http_config->el.http_t);
    audio_pipeline_unregister(pcm_http_config->pipeline, pcm_http_config->el.i2s_t);
    audio_pipeline_unregister(pcm_http_config->pipeline, pcm_http_config->el.mp3_t);
    audio_pipeline_remove_listener(pcm_http_config->pipeline);
    audio_event_iface_destroy(pcm_http_config->evt);
    audio_pipeline_deinit(pcm_http_config->pipeline);
    audio_element_deinit(pcm_http_config->el.http_t);
    audio_element_deinit(pcm_http_config->el.i2s_t);
    audio_element_deinit(pcm_http_config->el.mp3_t);
    printf("destroy http iis play +++++++\n");
   }
   return 0;
}

struct asound_lapi esp32_asound_lapi = {
    .yodalite_pcm_init = esp32_pcm_init,
    .yodalite_pcm_read = esp32_pcm_read,
    .yodalite_pcm_write = esp32_pcm_write,
    .yodalite_pcm_ioctl = esp32_pcm_ioctl,
    .yodalite_pcm_destroy = esp32_pcm_destroy,
    .yodalite_pcm_pause = esp32_pcm_pause,
    .yodalite_pcm_resume = esp32_pcm_resume,
    .yodalite_pcm_stop = esp32_pcm_stop,
    .yodalite_pcm_rerun = esp32_pcm_rerun,
};

