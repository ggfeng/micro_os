#include <stdio.h>
// #include <string.h>
// #include <stdlib.h>

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/event_groups.h"
// #include "freertos/semphr.h"
// #include "freertos/timers.h"

//#include <unistd.h>

#ifdef YODALITE
#include <osal/pthread.h>
#else
#include <pthread.h>
#endif

 #include "lib/cjson/cJSON.h"
 #include <lib/mem_ext/mem_ext.h>
// #include "player.h"
 #include "cloudapp.h"

struct player_attr {
    int local_fd;
    int local_state;
    int url_fd;
    int url_state;
};

struct player_attr g_player_attr = {0};

static void play_url_thread(void)
{
    if(g_player_attr.url_fd) {
        printf("begin play_url_thread\n");
        play_pcm_data(g_player_attr.url_fd);
    } else {
        printf("play_url_thread error\n");
    }
    printf("play_url_thread end\n");
}

static int cloud_media(char *url)
{
    if(g_player_attr.url_fd) {
        printf("begin destroy url_fd:%d\n",g_player_attr.url_fd);
        player_destory(g_player_attr.url_fd);
        g_player_attr.url_fd = 0;
    }

    if(g_player_attr.url_fd == 0) {
        g_player_attr.url_fd = play_url_init(url);
        printf("end init url_fd:%d\n",g_player_attr.url_fd);
    }

    pthread_t pthread_play;
    (void) pthread_create(&pthread_play, NULL,play_url_thread,NULL);
    pthread_detach(pthread_play);
}

static int cloud_voice()
{
    printf("暂时不支持tts播放\n");
}

static int cloud_action_parse(const char *cloud_action)
{
    cJSON *monitor_json = cJSON_Parse((char *)cloud_action);
    cJSON *appId = NULL;
    cJSON *response = NULL;
    cJSON *action = NULL;
    cJSON *directives = NULL;
    cJSON *directive1 = NULL;
    cJSON *url = NULL;

    static int net_status = 0;
    int i = 0;

    if (monitor_json)
    {
        appId = cJSON_GetObjectItemCaseSensitive(monitor_json, "appId");
        printf("appId:%s\n",appId->valuestring);
        response = cJSON_GetObjectItem(monitor_json, "response");
        if(response){
            action = cJSON_GetObjectItem(response, "action");
            if(action)
            {
                 directives = cJSON_GetObjectItem(action, "directives");
                 if(directives){
                     int dir_num = cJSON_GetArraySize(directives);
                     printf("now dir_num is %d\n",dir_num);
                     cJSON *type = NULL;
                     for(i = 0; i < dir_num ; i++) {
                          printf("now step i is %d\n",i);
                         directive1 = cJSON_GetArrayItem(directives,i);
                         type = cJSON_GetObjectItem(directive1,"type");
                         if(memcmp(type->valuestring,"media",strlen(type->valuestring)) == 0){
                            cJSON *item = cJSON_GetObjectItem(directive1,"item");
                            if(item){
                                url = cJSON_GetObjectItem(item,"url");
                                if(url) {
                                    printf("url:%s\n",url->valuestring);
                                    cloud_media(url->valuestring);
                                }        
                            }
                         } else if(memcmp(type->valuestring,"voice",strlen(type->valuestring)) == 0) {
                             cloud_voice();
                         } else {
                             printf("暂不支持其它操作\n");
                         }
                     } 
                 }
            }
        }
    }
    if(monitor_json)
        cJSON_Delete(monitor_json);
    return 0;
}

int cloud_docommand(const char *cloud_action)
{
    cloud_action_parse(cloud_action);
}