//#include <unistd.h>
#include <stdio.h>
//#include <stdlib.h>
#include <string.h>

#ifdef YODALITE
#include <osal/pthread.h>
#else
#include <pthread.h>
#endif

#include <lib/lwip/mqtt.h>
#include <lib/libmd5/md5.h>
#include <lib/property/properties.h>
#include <lib/cjson/cJSON.h>
//#include <yodalite_autoconf.h>
//#include <lib/shell/shell.h>
#include <hapi/mqtt_login.h>
#ifdef CONFIG_MEM_EXT_ENABLE
#include <lib/mem_ext/mem_ext.h>
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

#define MQTT_VERSION "1"
#define MQTT_SERVICE "mqtt"

#define MQTT_SERVER_PORT    "8885"
#define MQTT_SERVER_ADDRESS "wormhole.rokid.com"

#define FTM_SEED_KEY       "ro.boot.seed"
#define FTM_SN_KEY         "ro.boot.serialno"
#define FTM_DEVICE_TYPE_ID "ro.boot.devicetypeid"

#define TOKEN_ADDRESS "https://wormhole-registry.rokid.com/api/registryByKey"
#define PAYLOAD_QUERY_FORMAT "key=%s&device_type_id=%s&device_id=%s&service=%s&version=%s&time=%s&secret=%s"
#define PAYLOAD_MSG_FORMAT "appKey=%s&requestSign=%s&deviceTypeId=%s&deviceId=%s&accountId=%s&service=%s&time=%s&version=%s"

const char * header = "POST https://wormhole-registry.rokid.com/api/registryByKey HTTP/1.0\r\n"
                                "Connection: close\r\n" //general header
                                "Host: wormhole-registry.rokid.com\r\n" //request header
                                "Content-Type: application/x-www-form-urlencoded\r\n"; //entity header


static int  sockfd = -1;
static uint8_t*g_mqtt_buf= NULL;
static int g_refresh_thread_flag = 0;
static struct mqtt_client client;
static mqtt_hook_t g_mqtt_publish_hook = NULL;

static unsigned char hex2char(unsigned char c)
{
  if (c > 9)
    return (c + 55);
  else
   return (c + 0x30);
}

static  int hexstochars(unsigned char *src,int num,unsigned char *dst)
{
   int i;
   unsigned char temp;

   for(i=0;i<num;i++){
    temp = src[i]&0xf0;
    dst[2*i] = hex2char(temp >> 4);
    if(dst[2*i]>='a'&&dst[2*i]<='z') 
       dst[2*i] -=32; 
    temp = src[i]&0x0f;
    dst[2*i+1] = hex2char(temp);
    if(dst[2*i+1]>='a'&&dst[2*i+1]<='z') 
       dst[2*i] -=32; 
  }

  return 2*num;
}

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

    if(g_mqtt_publish_hook)
       g_mqtt_publish_hook(topic_name,(char *)published->application_message);

    printf("Received publish('%s'): %s\n", topic_name, (const char*) published->application_message);

    yodalite_free(topic_name);
}

static void* mqtt_loop(void* client)
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

static int yodalite_mqtt_init(char *username,char *passwd,char *topic,mqtt_hook_t func)
{
    const char* addr;
    const char* port;
    uint8_t *sendbuf; /* sendbuf should be large enough to hold multiple whole mqtt messages */
    uint8_t *recvbuf; /* recvbuf should be large enough any whole mqtt message expected to be received */
    pthread_t mqtt_client;
    enum MQTTErrors err;

    if(username == NULL || passwd == NULL || topic == NULL){
      printf("error:parameter is no valid\n");
      return -1;
    }

    printf("username:%s,passwd:%s,topic:%s\n",username,passwd,topic);

    addr = MQTT_SERVER_ADDRESS;
    port = MQTT_SERVER_PORT;

    /* open the non-blocking TCP socket (connecting to the broker) */
    sockfd = open_nb_socket(addr, port);

    if (sockfd == -1) {
        printf("error:failed to open socket\n");
        return -1;
    }

    if((g_mqtt_buf = (uint8_t *)yodalite_malloc(3072)) == NULL){
        printf("error:yodalite_malloc %d bytes fail\n",3072);
        close_nb_socket(sockfd);
        return -1;
    } 

    sendbuf = g_mqtt_buf;
    recvbuf = g_mqtt_buf+2048;
    /* setup a client */
    g_mqtt_publish_hook = func;

    if((err = mqtt_init(&client, sockfd, sendbuf,2048, recvbuf,1024, publish_callback)) != MQTT_OK){
        printf("error:mqtt_init %s \n",mqtt_error_str(err));
        return -1; 
    }
    
    mqtt_connect(&client, "subscribing_client", NULL, NULL, 0,username,passwd, 0, 400);
    /* check that we don't have any errors */
    if (client.error != MQTT_OK) {
        printf("error: mqtt_connect %s\n", mqtt_error_str(client.error));
        close_nb_socket(sockfd);
        yodalite_free(g_mqtt_buf);
        return -1;
    }

    /* start a thread to refresh the client (handle egress and ingree client traffic) */
    if(pthread_create(&mqtt_client, NULL,mqtt_loop, &client)) {
        printf("error:failed to start client daemon.\n");
        close_nb_socket(sockfd);
        yodalite_free(g_mqtt_buf);
        return -1;
    }
    /* subscribe */
    printf("mqtt_subscribe:%s\n",topic);
    mqtt_subscribe(&client, topic, 0);
    return 0;
}

static int https_parse(char *msg,char *username,char *passwd)
{
    cJSON *root = cJSON_Parse(msg);

    if (root){
        cJSON *name = cJSON_GetObjectItemCaseSensitive(root, "username");
        cJSON *token = cJSON_GetObjectItemCaseSensitive(root, "token");

        if(name == NULL || token  ==NULL){
           printf("error: result is not valid\n");
           cJSON_Delete(root);
           return -1;
        }
        property_set("ro.username",name->valuestring);
        property_set("ro.passwd",token->valuestring);
        strcpy(username,name->valuestring);
        strcpy(passwd,token->valuestring);
        printf("username:%s,passwd:%s\n",username,passwd);
    }

   cJSON_Delete(root);

   return 0;
}


int yodalite_mqtt_login(char *account_id,char *master_id,char *key,char *secret,mqtt_hook_t func)
{
  int iret;
  char *resp;
  char*in;
  char*out;
  char*sign;
  char*topic;
  char*passwd;
  char*username;
  char*device_id;
  char*device_type_id;
  char *version= MQTT_VERSION;
  char *service=MQTT_SERVICE;

  long cur_time;
  char *str_time;
  int buf_size = 4096;

  if(account_id == NULL || master_id == NULL || key == NULL || secret == NULL){
    printf("error:parm is not valid\n");
    return -1;
  }

  printf("%s->%d:key:%s,sceret:%s,account_id:%s master_id:%s\n",__func__,__LINE__,key,secret,account_id,master_id);

  if((in = yodalite_malloc(buf_size)) == NULL){
    printf("error:yodalite_malloc %d fail\n",buf_size);
    return -1;
  }

   memset(in,0,buf_size);

   out = in + 1024;
   sign = out + 256;
   device_id = sign + 256;
   device_type_id = device_id +256;
   topic = device_type_id + 256;
   str_time = topic + 128;
   username = str_time +128;
   passwd =  username +128;
    

  if(property_get(FTM_SN_KEY,device_id,"0") <= 0  || 
     property_get(FTM_DEVICE_TYPE_ID,device_type_id,"0") <= 0){
        printf("error:device id or sn is not valid\n");
        yodalite_free(in);
        return  -1; 
   }

   cur_time = time(NULL);
   memset(str_time,0,32);
   snprintf(str_time,32,"%ld",cur_time);
 
   snprintf(in,1024,PAYLOAD_QUERY_FORMAT,key,device_type_id,device_id,service,version,str_time,secret);
   printf("%s\n",in);
   md5(in, strlen(in), out);
   hexstochars(out,16,sign);
   printf("%s\n",sign);
   snprintf(in,1024,PAYLOAD_MSG_FORMAT,key,sign,device_type_id,device_id,account_id,service,str_time,version);
   printf("%s\n",in);

   resp = https_post(header,in);

  // yodalite_free(in);

   printf("get token resp%s\n",resp);

  /* get master_id ,username,passwd **/
  if(resp) {
        if(https_parse(resp,username,passwd)){
            yodalite_free(resp);
            printf("error:https_parse fail\n");
            yodalite_free(in);
            return -1; 
        }
        yodalite_free(resp);
    }

  snprintf(topic,256,"u/%s/deviceType/%s/deviceId/%s/rc",master_id,device_type_id,device_id);
  printf("%s\n",topic);

#if CONFIG_ESP32
 iret =  esp32_mqtt_init(username,passwd,topic,func);
#else
 iret  =  yodalite_mqtt_init(username,passwd,topic,func);
#endif
 yodalite_free(in);
 return iret;
}

int mqtt_login(void){
   char*buf; 
   char *key;
   char*secret;
   char*master_id;
   char*account_id;
   int iret;

  if((buf = yodalite_malloc(1024)) == NULL){
    printf("error:yodalite_malloc fail\n"); 
    return -1;
  }

  memset(buf,0,1024);

  key = buf;
  secret = buf + 256;
  master_id = secret + 256;
  account_id = master_id + 256;

  if(property_get("ro.key",key,"0") <=0 || 
     property_get("ro.secret",secret,"0") <= 0 || 
     property_get("ro.account_id",account_id,"0") <=0  ||
     property_get("ro.master_id",master_id,"0") <=0 ){

     printf("error:%s->%d parm is not valid\n",__func__,__LINE__);
     return -1;
  }
//  printf("%s->%d:key:%s,sceret:%s,master_id:%s\n",__func__,__LINE__,key,secret,master_id);

  iret =  yodalite_mqtt_login(account_id,master_id,key,secret,NULL);

  printf("mqtt_login return %d\n",iret);

  yodalite_free(buf);

  return 0;
}
