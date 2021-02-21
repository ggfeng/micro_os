#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/cjson/cJSON.h>
#include <hapi/flora_agent.h>
#include <lib/shell/shell.h>

#include <yodalite_autoconf.h>

#ifdef CONFIG_MEM_EXT_ENABLE
#include <lib/mem_ext/mem_ext.h>
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

int hapi_netmgr_exit(void);
int hapi_netmgr_init(void);

typedef struct cmd_info
{ 
  int idx;
  char * device;
  char *command;
  char * info;
  int (*send_func)(char *device,char *command);
}tCMD_INFO;

static flora_agent_t agent;


static void flora_recv_post_func(const char *name, uint32_t msgtype, caps_t msg, void *arg)
{
    const char* msg_string;
    caps_read_string(msg, &msg_string);
    printf("post callback: msg_name(%s), msgtype(%d), msg(%s)\n", name, msgtype, msg_string);
}

int net_flora_init(int subscribe) 
{
   flora_agent_config_param_t param;
   memset(&param,0,sizeof(flora_agent_config_param_t));
   agent = flora_agent_create();
   param.sock_path  = "unix:/var/run/flora.sock";
   flora_agent_config(agent, FLORA_AGENT_CONFIG_URI, &param);
   param.value  = 1024;
   flora_agent_config(agent, FLORA_AGENT_CONFIG_BUFSIZE,&param);

   if(subscribe)
     flora_agent_subscribe(agent,"network.status", flora_recv_post_func, NULL);
// flora_agent_config(agent, FLORA_AGENT_CONFIG_RECONN_INTERVAL, 5000);
   flora_agent_start(agent, 0);
   return 0;
}

// add new
void net_flora_exit(void)
{
  flora_agent_close(agent);
  flora_agent_delete(agent);
}

int floar_xfer(char *path,caps_t send_msg,char *recv,uint32_t len)
{
  int iret = -1;

 do{
  flora_call_result result;

  //if ((iret = flora_agent_call(agent,path,send_msg, "net_manager", &result,5000)) == FLORA_CLI_SUCCESS) 
  if ((iret = flora_agent_call(agent,path,send_msg, "net_manager", &result,0)) == FLORA_CLI_SUCCESS) 
  {

     const char* buf;
     caps_read_string(result.data, &buf);
     printf("read string:%s,ret_code:%d\n",buf,result.ret_code);

  }
  else
  printf("%s->%d,ret:%d\n",__func__,__LINE__,iret);

  flora_call_reply_end(&result);

}while(0);

 return iret;
}

int wifi_send_func(char *device,char *command)
{

    char ssid[36] = {0};
    char psk[64] =  {0};
    const char *net_command = NULL;
    caps_t msg;
    cJSON *root =cJSON_CreateObject();
    cJSON *wifi_info = cJSON_CreateObject();

    strcpy(ssid,"ROKID.TC");
    strcpy(psk,"rokidguys");

    printf("input :: ssid :: %s\n", ssid);
    printf("input :: psk :: %s\n", psk);

    cJSON_AddStringToObject(wifi_info,"SSID",ssid);
    cJSON_AddStringToObject(wifi_info,"PASSWD",psk);

    cJSON_AddStringToObject(root,"device",device);
    cJSON_AddStringToObject(root,"command",command);
    cJSON_AddItemToObject(root,"params",wifi_info);

    net_command = cJSON_Print(root);
    printf("send:%s\n",net_command);

    msg = caps_create();
    caps_write_string(msg, net_command);

    if(floar_xfer("network.command",msg,NULL,0))
    {
      fprintf(stderr,"floar xfer fail\n");
            
    }
   
    yodalite_free((void*)net_command);
    cJSON_Delete(root);
//  cJSON_Delete(wifi_info);
    caps_destroy(msg);

    return 0;
}

int wifi_ap_send_func(char *device,char *command)
{
    char ssid[36] = {0};
    char psk[64] =  {0};
    char ip[64] =  {0};
    char timeout[64] =  {0};
    const char *net_command = NULL;
    caps_t msg;
    cJSON *root =cJSON_CreateObject();
    cJSON *wifi_info = cJSON_CreateObject();


    strcpy(ssid,"shanjiaming");
    strcpy(psk,"");
    strcpy(ip,"192.168.10.88");
    snprintf(timeout,64,"0");

    printf("input :: ssid :: %s\n", ssid);
    printf("input :: psk :: %s\n", psk);
    printf("input :: ip :: %s\n", ip);
    printf("input :: timeout :: %s\n",timeout);

    cJSON_AddStringToObject(wifi_info,"SSID",ssid);
    cJSON_AddStringToObject(wifi_info,"PASSWD",psk);
    cJSON_AddStringToObject(wifi_info,"IP",ip);
    cJSON_AddStringToObject(wifi_info,"TIMEOUT",timeout);

    cJSON_AddStringToObject(root,"device",device);
    cJSON_AddStringToObject(root,"command",command);
    cJSON_AddItemToObject(root,"params",wifi_info);

    net_command = cJSON_Print(root);
    printf("send:%s\n",net_command);

    msg = caps_create();
    caps_write_string(msg, net_command);

    if(floar_xfer("network.command",msg,NULL,0))
    {
      fprintf(stderr,"floar xfer fail\n");
            
    }

    yodalite_free((void *)net_command);
    cJSON_Delete(root);
//  cJSON_Delete(wifi_info);
    caps_destroy(msg);

    return 0;
}


int general_send_func(char *device,char *command)
{

    const char *net_command = NULL;
    caps_t msg; 
    cJSON *root =cJSON_CreateObject();

    cJSON_AddStringToObject(root,"device",device);
    cJSON_AddStringToObject(root,"command",command);

    net_command = cJSON_Print(root);
    printf("send:%s\n",net_command);

    msg = caps_create();
    caps_write_string(msg, net_command);

    if(floar_xfer("network.command",msg,NULL,0))
    {
      fprintf(stderr,"floar xfer fail\n");
            
    }

    yodalite_free((void*)net_command);
    cJSON_Delete(root);
    caps_destroy(msg);

    return 0 ;
}

int netmgr_init(char *device,char *command)
{
   hapi_netmgr_init();
   return 0;
}

int netmgr_exit(char *device,char *command)
{
   hapi_netmgr_exit();
   return 0;
}


tCMD_INFO g_cmd_list[] = 
{
   {0,"NETWORK","GET_CAPACITY","network get capacity",general_send_func},
   {1,"NETWORK","GET_MODE","network get mode",general_send_func},
   {2,"NETWORK","GET_STATUS","network get status",general_send_func},
   {3,"WIFI","START_SCAN","wifi start scan",general_send_func},
   {4,"WIFI","STOP_SCAN","wifi stop scan",general_send_func},
   {5,"WIFI","CONNECT","wifi connect station",wifi_send_func},
   {6,"WIFI","DISCONNECT","wifi disconnect station",general_send_func},
   {7,"WIFI","GET_WIFILIST","wifi get wifi list",general_send_func},
   {8,"WIFI_AP","CONNECT","wifi connect ap",wifi_ap_send_func},
   {9,"WIFI_AP","DISCONNECT","wifi disconnect ap",general_send_func},
   {10,"WIFI","GET_STATUS","wifi get status",general_send_func},
   {11,"WIFI","GET_CFG_NUM","wifi get config num",general_send_func}, 
   {12,"WIFI","SAVE_CFG"   ,"wifi save config file",general_send_func},
   {13,"WIFI","REMOVE","wifi remove station",wifi_send_func},
   {14,"WIFI","ENABLE","wifi enable all network",general_send_func},
   {15,"WIFI","DISABLE","wifi disabel all network",general_send_func},
   {16,"NETWORK","RESET_DNS","network reset dns",general_send_func},
   {17,"WIFI","REMOVE_ALL","wifi remove all config",general_send_func},
   {18,"WIFI","GET_INFO","wifi get info",general_send_func},
   {19,"netmgr","init","netmgr init",netmgr_init},
   {20,"netmgr","eixt","netmgr init",netmgr_exit},
};

void usage(void)
{
  int i;
  tCMD_INFO *p_info;

   for(i=0;i< sizeof(g_cmd_list)/sizeof(g_cmd_list[0]);i++)
   {
     p_info = &g_cmd_list[i];
     printf("\t%d %s\n",p_info->idx,p_info->info);
   }
}

static int read_cmd(char*prompt)
{
    int n=0;
    char *cmd=NULL;

    cmd = readline(prompt);

    if(cmd != NULL)
       n=atoi(cmd);

    return n;
}


int main_loop(void) 
{
    int choice;
    tCMD_INFO *p_info = &g_cmd_list[0];
    int max_choice = sizeof(g_cmd_list)/sizeof(g_cmd_list[0]);
    
 do{
    choice =  read_cmd("netmgr#");

    printf("choice:%d\n",choice);

    if(choice >=0 && choice < max_choice)
        p_info[choice].send_func(p_info[choice].device,p_info[choice].command);
    else
       usage();

  } while(choice != 99);
      
    return 0;
}


static int netmgr_test_cmd(int argc,int8_t * const argv[])
{
    int num;

    net_flora_init(0);

//    if(argc < 2){
//       printf("usage:netmgr_test num\n");
       usage();
//     return 0;
//   }

 //   printf("%s->%d\n",__func__,__LINE__); 

//    num = atoi(argv[1]);
  
  //  printf("num:%d\n",num);
    main_loop();
    net_flora_exit();

    return 0;
}


static int netmgr_monitor_cmd(int argc,int8_t * const argv[])
{
    net_flora_init(1);
    return 0;
}


#define max_args    (2)
#define test_help  "netmgr_test num"


int cmd_netmgr_test(void)
{
  YODALITE_REG_CMD(netmgr_test,max_args,netmgr_test_cmd,test_help);

  return 0;
}

#define monitor_max_args    (2)
#define monitor_help  "netmgr_monitor"


int cmd_netmgr_monitor(void)
{
  YODALITE_REG_CMD(netmgr_monitor,monitor_max_args,netmgr_monitor_cmd,monitor_help);

  return 0;
}
