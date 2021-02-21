/* Examples of speech recognition with multiple keywords.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "player.h"
#include <yodalite_autoconf.h>
#ifdef CONFIG_MEM_EXT_ENABLE
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif
int player_init()
{
    return 0;
}
int player_destory(int fd)
{
    pcm_close(fd);
    return 0;
}

int player_stop(int fd)
{
    pcm_stop(fd);
    return 0;
}
int player_rerun(int fd)
{
    pcm_rerun(fd);
    return 0;
}

int player_pause(int fd)
{
    pcm_pause(fd);
    return 0;
}

int player_resume(int fd)
{
   pcm_resume(fd);
   return 0;
}

int play_local_init(int src_type)
{
    struct yodalite_pcm_config * config;
    unsigned int card, device, flags;
    int open_id;
    config = (struct yodalite_pcm_config *)yodalite_malloc(sizeof(struct yodalite_pcm_config));
    memset(config, 0x0, sizeof(struct yodalite_pcm_config));
    card = 2;
    device = 2;
    flags = YODALITE_PCM_OUT;
    config->format = 16;
    config->channels = 2;
    config->rate = 48000;
    config->period_count = sizeof(short);
    config->source = MP3_IIS;
    config->file_source = src_type;
    open_id =  pcm_open(card, device, flags, config);
    yodalite_free(config);
    return open_id;
}

int play_url_init(char * url)
{
    struct yodalite_pcm_config * config;
    unsigned int card, device, flags;
    int open_id;
    config = (struct yodalite_pcm_config *)malloc(sizeof(struct yodalite_pcm_config));
    memset(config, 0x0, sizeof(struct yodalite_pcm_config));
    printf("begin play url %s\n",url);
    card = 2;
    device = 2;
    flags = YODALITE_PCM_OUT;
    config->format = 16;
    config->channels = 2;
    config->rate = 48000;
    config->period_count = sizeof(short);
    config->source = HTTP_IIS;
    config->url_str = url;
    open_id = pcm_open(card, device, flags, config);
    yodalite_free(config);
    return open_id;
    
}

int  play_pcm_data(int fd)
{
    void *buff = NULL;
    pcm_write(fd,  (void *)buff,   1);
    return 0;
}
