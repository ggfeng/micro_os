
/**
 * @file
 * A simple program that subscribes to a topic.
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <osal/pthread.h>
#include <lib/lwip/mqtt.h>
#include <yodalite_autoconf.h>
#include <lib/shell/shell.h>

#ifdef CONFIG_MEM_EXT_ENABLE
#include <lib/mem_ext/mem_ext.h>
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

extern int mqtt_login(void);

static int  sockfd = -1;
static uint8_t*g_mqtt_buf= NULL;
static int g_refresh_thread_flag = 0;

static void publish_callback(void** unused, struct mqtt_response_publish *published) 
{
    /* note that published->topic_name is NOT null-terminated (here we'll change it to a c-string) */
    char* topic_name = (char*) yodalite_malloc(published->topic_name_size + 1);

    if(topic_name == NULL){
       printf("error:malloc %d bytes\n",published->topic_name_size + 1);
       return;
     }

    memcpy(topic_name, published->topic_name, published->topic_name_size);
    topic_name[published->topic_name_size] = '\0';

    printf("Received publish('%s'): %s\n", topic_name, (const char*) published->application_message);

    yodalite_free(topic_name);
}

static void* client_refresher(void* client)
{
    pthread_detach(pthread_self());

    g_refresh_thread_flag  = 1;

    while(g_refresh_thread_flag) 
    {
        mqtt_sync((struct mqtt_client*) client);
      //usleep(100000U);
        sleep(1);
    }

    return NULL;
}

static struct mqtt_client client;

static int mqtt_init_cmd(int argc,int8_t * const argv[])
{
    const char* addr;
    const char* port;
    const char* topic;
    uint8_t *sendbuf; /* sendbuf should be large enough to hold multiple whole mqtt messages */
    uint8_t *recvbuf; /* recvbuf should be large enough any whole mqtt message expected to be received */
    pthread_t client_daemon;
    enum MQTTErrors err;
//    struct mqtt_client client;

    /* get address (argv[1] if present) */
    if (argc > 1) {
        addr = argv[1];
    } else {
        addr = "test.mosquitto.org";
    }

    /* get port number (argv[2] if present) */
    if (argc > 2) {
        port = argv[2];
    } else {
        port = "1883";
    }

    /* get the topic name to publish */
    if (argc > 3) {
        topic = argv[3];
    } else {
        topic = "datetime";
    }

    /* open the non-blocking TCP socket (connecting to the broker) */
    sockfd = open_nb_socket(addr, port);

    if (sockfd == -1) {
        printf("error:failed to open socket\n");
        close(sockfd);
        return -1;
    }

    if((g_mqtt_buf = (uint8_t *)yodalite_malloc(3072)) == NULL){
        printf("error:yodalite_malloc %d bytes fail\n",3072);
        close(sockfd);
        return -1;
    } 

    sendbuf = g_mqtt_buf;
    recvbuf = g_mqtt_buf+2048;
    /* setup a client */
    printf("%s->%d\n",__func__,__LINE__);
    //if((err = mqtt_init(&client, sockfd, sendbuf, sizeof(sendbuf), recvbuf, sizeof(recvbuf), publish_callback)) != MQTT_OK){
    if((err = mqtt_init(&client, sockfd, sendbuf,2048, recvbuf,1024, publish_callback)) != MQTT_OK){
        printf("error:mqtt_init %s \n",mqtt_error_str(err));
        return -1; 
    }
    
    printf("%s->%d\n",__func__,__LINE__);
    mqtt_connect(&client, "subscribing_client", NULL, NULL, 0, NULL, NULL, 0, 400);

    printf("%s->%d\n",__func__,__LINE__);
    /* check that we don't have any errors */
    if (client.error != MQTT_OK) {
        printf("error: mqtt_connect %s\n", mqtt_error_str(client.error));
        close(sockfd);
        yodalite_free(g_mqtt_buf);
        return -1;
    }

    /* start a thread to refresh the client (handle egress and ingree client traffic) */

    if(pthread_create(&client_daemon, NULL, client_refresher, &client)) {
        printf("error:failed to start client daemon.\n");
        close(sockfd);
        yodalite_free(g_mqtt_buf);
        return -1;
    }

    /* subscribe */
    mqtt_subscribe(&client, topic, 0);

    /* start publishing the time */
    printf("%s listening for '%s' messages.\n",argv[0], topic);
    /* exit */ 
    return 0;
}


static int mqtt_deinit_cmd(int argc,int8_t * const argv[])
{
   if(sockfd){
     sockfd = 0;
     close(sockfd);
   }

   g_refresh_thread_flag = 0;
    yodalite_free(g_mqtt_buf);
   return 0; 
}


#define init_max_args      (1)
#define init_help          "mqtt_init"

int cmd_mqtt_init(void)
{
  YODALITE_REG_CMD(mqtt_init,init_max_args,mqtt_init_cmd,init_help);
  return 0;
}


#define deinit_max_args      (1)
#define deinit_help          "mqtt_deinit"

int cmd_mqtt_deinit(void)
{
  YODALITE_REG_CMD(mqtt_deinit,deinit_max_args,mqtt_deinit_cmd,deinit_help);
  return 0;
}

static int login_cmd(int argc,int8_t * const argv[]){
   mqtt_login();
  return 0;
}

#define login_max_args      (1)
#define login_help          "login"

int cmd_login(void)
{
  YODALITE_REG_CMD(login,login_max_args,login_cmd,login_help);
  return 0;
}
