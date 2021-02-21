#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

static int connecting = 0;
static flora_agent_t agent;

static void bluetooth_usage(void)
{
   printf("Bluetooth test CMD menu:\n");
   printf("    1 ble\n");
   printf("    2 a2dp_sink\n");
   printf("    3 a2dp_source\n");
   printf("    4 hfp\n");
}

static void a2dp_source_usage(void)
{
   printf("Bluetooth a2dp test menu:\n");
   printf("    1 a2dp source on\n");
   printf("    2 a2dp source off\n");
   printf("    3 a2dp source discovery then connect one\n");
   printf("    4 disconnect_peer\n");
   printf("    5 call getstate\n");
   printf("    99 exit a2dp\n");
}

static void a2dp_sink_usage(void)
{
  printf("Bluetooth test CMD menu:\n");
  printf("    1 a2dp sink on\n");
  printf("    2 a2dp sink off\n");
  printf("    3 play\n");
  printf("    4 pause\n");
  printf("    5 stop\n");
  printf("    6 next\n");
  printf("    7 prev\n");
  printf("    8 disconnect\n");
  printf("    9 on play\n");
  printf("    10 mute\n");
  printf("    11 unmute\n");
  printf("    12 disconnect_peer\n");
  printf("    13 set volume\n");
  printf("    14 getstate\n");
  printf("    15 call getstate\n");
  printf("    16 getsong attrs\n");

  printf("    20 play & unmute\n");
  printf("    21 pause & mute\n");
  printf("    50 a2dp sink on &&  hfp on\n");
  printf("    51 a2dp sink off &&  hfp off\n");

  printf("    99 exit a2dp sink\n");
  printf("    100 only break, not off\n");

}

static void ble_usage(void)
{
  printf("Bluetooth ble test CMD menu:\n");
  printf("    1 ble on\n");
  printf("    2 ble off\n");
  printf("    3 send data\n");
  printf("    4 send raw data\n");
  printf("    5 call getstate\n");
  printf("    99 exit ble\n");
}


static void hfp_usage(void)
{
   printf("Bluetooth test CMD menu:\n");
   printf("    1 hfp on\n");
   printf("    2 hfp off\n");
   printf("    3 answer\n");
   printf("    4 hangup\n");
   printf("    5 dailnum\n");
   printf("    6 call getstate\n");
   printf("    7 disconnect\n");
   printf("    99 exit hfp\n");
}


static char* read_string(char*prompt)
{
    return readline(prompt);
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

static int a2dp_source_operation(void)
{
    int  ret = 0;
    int  choice;
    char *name = "Rokid-Me-9999zz";
    char *proto = "A2DPSOURCE";
    char *command = NULL;
    char *bt_command = NULL;
    //int unique = 1;
    int flora_call = 0;
    cJSON *root = NULL;
    caps_t msg;

    do {
        while (connecting != 0) {
            sleep(1);
        }

        root =cJSON_CreateObject();

        choice = read_cmd("source#");

        switch (choice) {
        case 1:
            command = "ON";
            cJSON_AddStringToObject(root,"name",name);
            break;
        case 2:
            command = "OFF";
            break;
        case 3:
            command = "DISCOVERY";
            connecting = 1;
            break;
        case 4:
            command = "DISCONNECT_PEER";
            break;
        case 5:
            flora_call = 1;
            command = "GETSTATE";
            break;
        default:
            cJSON_Delete(root);
            continue;
        }

        cJSON_AddStringToObject(root,"proto",proto);
        cJSON_AddStringToObject(root,"command",command);

        bt_command = cJSON_Print(root);

         msg = caps_create();
         caps_write_string(msg, (const char *)bt_command);
        if (flora_call) {
            flora_call = 0;
            flora_call_result result = {0};
            if (flora_agent_call(agent, "bluetooth.a2dpsource.command", msg, "bluetoothservice-agent", &result, 0) == FLORA_CLI_SUCCESS) {
                if (result.data && result.ret_code == FLORA_CLI_SUCCESS) {
                    const char *buf = NULL;
                    caps_read_string(result.data, &buf);
                    printf("data :: %s\n", buf);
                }
            } else {
                printf("failed to flora_agent_call\n");
            }

         flora_call_reply_end(&result);
        } else {
            flora_agent_post(agent, "bluetooth.a2dpsource.command", msg, FLORA_MSGTYPE_INSTANT);
        }
       yodalite_free(bt_command);
       caps_destroy(msg);
       cJSON_Delete(root);
    } while (choice != 99);
    return ret;
}

static int a2dp_sink_operation(void) 
{
    int ret = 0;
    int choice;
    char *name = "Rokid-Me-9999zz";
    char *proto = "A2DPSINK";
    char *subquent = "PLAY";
    char *command = NULL;
    char *bt_command = NULL;
    int unique = 1;
    int flora_call = 0;
    cJSON *root = NULL;
    caps_t msg;

    do {
        while (connecting != 0) {
            sleep(1);
        }

        root =cJSON_CreateObject();
        choice = read_cmd("sink#");
        switch(choice) {
        case 1:
            command = "ON";
            cJSON_AddStringToObject(root,"name",name);
            break;
        case 2:
            command = "OFF";
            break;
        case 3:
            command = "PLAY";
            break;
        case 4:
            command = "PAUSE";
            break;
        case 5:
            command = "STOP";
            break;
        case 6:
            command = "NEXT";
            break;
        case 7:
            command = "PREV";
            break;
        case 8:
            command = "DISCONNECT";
            break;
        case 9:
            command = "ON";

            cJSON_AddStringToObject(root,"name",name);
            cJSON_AddStringToObject(root,"subsequent",subquent);
            cJSON_AddNumberToObject(root,"unique",unique);

            break;
        case 10:
            command = "MUTE";
            break;
        case 11:
            command = "UNMUTE";
            break;
        case 12:
            command = "DISCONNECT_PEER";
            break;
        case 13:
        {
            int vol;
            command = "VOLUME";
            printf("input volume(0-100)\n");
            vol = read_cmd(NULL);

            if (vol < 0) vol = 0;
            if (vol > 100) vol = 100;
            cJSON_AddNumberToObject(root,"value",vol);
            break;
         }
        case 14:
            command = "GETSTATE";
            break;
        case 15:
            flora_call = 1;
            command = "GETSTATE";
            break;
        case 16:
            command = "GETSONG_ATTRS";
            break;

        case 20:
            command = "PLAY_UNMUTE";
            break;
        case 21:
            command = "PAUSE_MUTE";
            break;
        case 50:
        {
            command = "ON";
            char *sec_pro = "HFP";
            cJSON_AddStringToObject(root,"name",name);
            cJSON_AddStringToObject(root,"sec_pro",sec_pro);
        }
            break;
        case 51:
        {
            command = "OFF";
            char *sec_pro = "HFP";
            cJSON_AddStringToObject(root,"sec_pro",sec_pro);
        }
            break;
   //     case 99:
   //     command = "OFF";
   //     break;
        case 100:
            command = NULL;
            break;
        default:
            cJSON_Delete(root);
            a2dp_sink_usage();
            continue;
        }
        if (!command) {
            cJSON_Delete(root);
            break;
        }

        cJSON_AddStringToObject(root,"proto",proto);
        cJSON_AddStringToObject(root,"command",command);

        bt_command = cJSON_Print(root);

        msg = caps_create();
        caps_write_string(msg, (const char *)bt_command);
        if (flora_call) {
            flora_call = 0;
            flora_call_result result = {0};
            if (flora_agent_call(agent, "bluetooth.a2dpsink.command", msg, "bluetoothservice-agent", &result, 0) == FLORA_CLI_SUCCESS) {
                if (result.data && result.ret_code == FLORA_CLI_SUCCESS) {
                    const char *buf = NULL;
                    caps_read_string(result.data, &buf);
                    printf("data :: %s\n", buf);
                }
            } else {
                printf("failed to flora_agent_call\n");
            }

            flora_call_reply_end(&result);
        } else {
            printf("%s->%d:%s\n",__func__,__LINE__,bt_command);
            flora_agent_post(agent, "bluetooth.a2dpsink.command", msg, FLORA_MSGTYPE_INSTANT);
        }
        yodalite_free(bt_command);
        caps_destroy(msg);
        cJSON_Delete(root);
    } while (choice != 99);

    return ret;
}

static int ble_operation(void)
{
    int ret = 0;
    int choice = 0;
    char *name = "Rokid-Pebble-9999zz";
    char *proto = "ROKID_BLE";
    char *command = NULL;
    char *data = NULL;
    char *bt_command = NULL;
    int unique = 1;
    int flora_call = 0;
    cJSON *root = NULL;
    caps_t msg;

    do {
        root =cJSON_CreateObject();
        choice = read_cmd("ble#");

        switch(choice) {
        case 1:
            command = "ON";

            cJSON_AddStringToObject(root,"name",name);
            cJSON_AddStringToObject(root,"command",command);
            cJSON_AddNumberToObject(root,"unique",unique);
            break;
        case 2:
            command = "OFF";
            cJSON_AddStringToObject(root,"command",command);
            break;
        case 3:
            data = "dashuaige";
            cJSON_AddStringToObject(root,"data",data);
            break;
        case 4:
            data = "yaojingnalizou";
            cJSON_AddStringToObject(root,"rawdata",data);
            break;
        case 5:
            flora_call = 1;
            command = "GETSTATE";
            cJSON_AddStringToObject(root,"command",command);
            break;
  //      case 99:
  //         command = "OFF";
  //          cJSON_AddStringToObject(root,"command",command);
  //          break;
        default:
            ble_usage();
            cJSON_Delete(root);
            continue;
        }

        cJSON_AddStringToObject(root,"proto",proto);
        bt_command = cJSON_Print(root);

        msg = caps_create();
        caps_write_string(msg, (const char *)bt_command);
        if (flora_call) {
            int iret = -1;
            flora_call = 0;
            flora_call_result result = {0};
            if ((iret=flora_agent_call(agent, "bluetooth.ble.command", msg, "bluetoothservice-agent", &result,0)) == FLORA_CLI_SUCCESS) {
                    const char *buf = NULL;
                    caps_read_string(result.data, &buf);
                    printf("data :: %s\n", buf);
            } else {
                printf("failed to flora_agent_call,iret %d\n",iret);
            }
           flora_call_reply_end(&result);
        } else {
            ret = flora_agent_post(agent, "bluetooth.ble.command", msg, FLORA_MSGTYPE_INSTANT);
        }
       yodalite_free(bt_command);
       caps_destroy(msg);
       cJSON_Delete(root);

    } while (choice != 99);

    return 0;
}

static int hfp_operation(void) 
{
    int ret = 0;
    int choice;
    char *proto = "HFP";
    char *name = "Rokid-Me-9999zz";
    char *command = NULL;
    char *bt_command = NULL;
    int flora_call = 0;
    cJSON *root = NULL;
    caps_t msg;

    do {
        root =cJSON_CreateObject();

        choice = read_cmd("hfp#");

        switch(choice) {
        case 1:
            command = "ON";
            cJSON_AddStringToObject(root,"name",name);
            break;
        case 2:
            command = "OFF";
            break;
        case 3:
            command = "ANSWERCALL";
            break;
        case 4:
            command = "HANGUP";
            break;
        case 5: {
            int  i = 0;
            char c = 0;
            char *num = {0};
            command = "DIALING";
            printf("input dial numbers\n");
            num = read_string(NULL);
            printf("your input: %s\n", num);
            cJSON_AddStringToObject(root,"NUMBER",num);
        }
            break;
        case 6:
            flora_call = 1;
            command = "GETSTATE";
            break;
        case 7:
            command = "DISCONNECT";
            break;
        case 99:
            command = "OFF";
            break;
        default:
            cJSON_Delete(root);
            continue;
        }

        cJSON_AddStringToObject(root,"proto",proto);
        cJSON_AddStringToObject(root,"command",command);

        bt_command = cJSON_Print(root);

        msg = caps_create();
        caps_write_string(msg, (const char *)bt_command);
        if (flora_call) {
            flora_call = 0;
            flora_call_result result = {0};
            if (flora_agent_call(agent, "bluetooth.hfp.command", msg, "bluetoothservice-agent", &result, 0) == FLORA_CLI_SUCCESS)            {
                if (result.data && result.ret_code == FLORA_CLI_SUCCESS) {
                    const char *buf = NULL;
                    caps_read_string(result.data, &buf);
                    printf("data :: %s\n", buf);
                }
            } else {
                printf("failed to flora_agent_call\n");
            }
           flora_call_reply_end(&result);
        } else {
            flora_agent_post(agent, "bluetooth.hfp.command", msg, FLORA_MSGTYPE_INSTANT);
        }
       yodalite_free(bt_command);
       caps_destroy(msg);
       cJSON_Delete(root);
    } while (choice != 99);

    return ret;
}


static int test_operation(void) 
{
    int i;
    int choice;
    char command[10] = {0};
    char *bt_command = NULL;

    cJSON *bt_cmd = NULL;
    caps_t msg;

    do {

        choice = read_cmd("test#");

        if (choice <= 0) break;
        printf("test times: %d\n", choice);
        for (i=0; i<choice; i++)
        {

            cJSON *root =cJSON_CreateObject();
            snprintf(command, sizeof(command), "haha%d", i);
            printf("command:%s\n",command);
            cJSON_AddStringToObject(root,"command",command);

            bt_command = cJSON_Print(root);

            msg = caps_create();
            caps_write_string(msg, (const char *)bt_command);
            flora_agent_post(agent, "bluetooth.hfp.command", msg, FLORA_MSGTYPE_INSTANT);

            yodalite_free(bt_command);
            caps_destroy(msg);
            cJSON_Delete(root);
            usleep(10*1000);
        }
    }while (1);

    return 0;
}
static void a2dpsource_parse_scan_results(cJSON *obj) 
{
    int choice = 0;
    cJSON *bt_cmd = NULL;
    cJSON *bt_res = NULL;
    cJSON *bt_dev = NULL;
    cJSON *child_obj = NULL;
    cJSON *bt_addr = NULL;
    cJSON *bt_name = NULL;
    cJSON *bt_completed = NULL;
    char *proto = "A2DPSOURCE";
    char *blue_cmd = "CONNECT";
    int i = 0;
    int ret = 0;
    int dev_count;
    cJSON *send_cmd;
    char *bt_command=NULL;
    caps_t msg;

    if (connecting != 1) {
        return ;
    }

    if((bt_cmd = cJSON_GetObjectItemCaseSensitive(obj, "action")) != NULL){
      if (bt_cmd->valuestring != NULL && strcmp(bt_cmd->valuestring,"discovery") == 0) {
        if((bt_res = cJSON_GetObjectItemCaseSensitive(obj, "results")) != NULL){
          if((bt_completed = cJSON_GetObjectItemCaseSensitive(bt_res,"is_completed")) != NULL){
            if(cJSON_IsBool(bt_completed) && bt_completed->valuestring == 0)
                       return;;

                 if((bt_dev = cJSON_GetObjectItemCaseSensitive(bt_res,"deviceList")) != NULL){
                    dev_count = cJSON_GetArraySize(bt_dev);
                    printf("length :: %d\n",dev_count);

                    for (i = 0; i < dev_count; i++) {

                        child_obj =  cJSON_GetArrayItem(bt_dev,i);
                        bt_addr = cJSON_GetObjectItemCaseSensitive(child_obj, "address");
                        bt_name = cJSON_GetObjectItemCaseSensitive(child_obj, "name");


                        if (cJSON_IsString(bt_addr) && (bt_addr->valuestring != NULL)){
                             printf("bt_addr[%d]:%s\n",i, bt_addr->valuestring);
                        }

                        if (cJSON_IsString(bt_name) && (bt_name->valuestring != NULL)){
                             printf("bt_name[%d]:%s\n",i, bt_name->valuestring);
                        }
                    }

                    while (1) {
                        choice = read_cmd(NULL);
                        if (choice == 99) {
                            break;
                        }

                        if (choice >= dev_count || choice < 0) {
                            printf("num is invalid\n");
                            continue;
                        }

                        child_obj =  cJSON_GetArrayItem(bt_dev,choice);
                        bt_addr = cJSON_GetObjectItemCaseSensitive(child_obj, "address");
                        bt_name = cJSON_GetObjectItemCaseSensitive(child_obj, "name");


                        if (cJSON_IsString(bt_addr) && (bt_addr->valuestring != NULL)){
                             printf("choice bt_addr:%s\n",bt_addr->valuestring);
                        }

                        if (cJSON_IsString(bt_name) && (bt_name->valuestring != NULL)){
                             printf("choice bt_name:%s\n",bt_name->valuestring);
                        }

                        send_cmd = cJSON_CreateObject();
                        cJSON_AddStringToObject(send_cmd,"proto",proto);
                        cJSON_AddStringToObject(send_cmd,"command",blue_cmd);
                        cJSON_AddItemToObject(send_cmd,"address",bt_addr);
                        cJSON_AddItemToObject(send_cmd,"name",bt_name);

                        bt_command = cJSON_Print(send_cmd);
                        printf("connect cmd :: %s\n",bt_command);

                        msg = caps_create();
                        caps_write_string(msg, (const char *)bt_command);
                        ret = flora_agent_post(agent, "bluetooth.a2dpsource.command", msg, FLORA_MSGTYPE_INSTANT);

                        yodalite_free(bt_command);
                        caps_destroy(msg);
                        cJSON_Delete(send_cmd);
                        break;
                    }
                    connecting = 0;
                }
            }
        }
     }  
  }
}

static int a2dpsource_event_handle(cJSON *obj)
{
    if(cJSON_IsInvalid(obj))
     return -1;
    a2dpsource_parse_scan_results(obj);
    return 0;
}

static int common_event_handle(cJSON* obj)
{
    cJSON *bt_event = NULL;
    cJSON *bt_res = NULL;
    cJSON *bt_dev = NULL;
    cJSON *child_obj = NULL;
    cJSON *bt_addr = NULL;
    cJSON *bt_name = NULL;
    int i, dev_count, choice;
    char *proto = "A2DPSINK";
    char *blue_cmd = "CONNECT";
    int ret;
    cJSON *send_cmd;
    caps_t msg;
    char *bt_command = NULL;

    if(cJSON_IsInvalid(obj))
     return -1;

    if((bt_event = cJSON_GetObjectItemCaseSensitive(obj, "action")) != NULL){
       if (bt_event->valuestring != NULL && strcmp(bt_event->valuestring, "pairedList") == 0) {
          if((bt_res = cJSON_GetObjectItemCaseSensitive(obj, "results")) != NULL){
            if((bt_dev = cJSON_GetObjectItemCaseSensitive(bt_res,"deviceList")) != NULL){
                    dev_count = cJSON_GetArraySize(bt_dev);
                    printf("length :: %d\n", dev_count);
                    for (i = 0; i < dev_count; i++) {

                        child_obj =  cJSON_GetArrayItem(bt_dev,i);
                        bt_addr = cJSON_GetObjectItemCaseSensitive(child_obj, "address");
                        bt_name = cJSON_GetObjectItemCaseSensitive(child_obj, "name");


                        if (cJSON_IsString(bt_addr) && (bt_addr->valuestring != NULL)){
                             printf("bt_addr[%d]:%s\n",i, bt_addr->valuestring);
                        }

                        if (cJSON_IsString(bt_name) && (bt_name->valuestring != NULL)){
                             printf("bt_name[%d]:%s\n",i, bt_name->valuestring);
                        }
                    }

                    while (dev_count > 0) {
                        choice = read_cmd(NULL);
                        if (choice == 99) {
                            break;
                        }

                        if (choice >= dev_count || choice < 0) {
                            printf("num is invalid\n");
                            continue;
                        }

                        child_obj =  cJSON_GetArrayItem(bt_dev,choice);

                        bt_addr = cJSON_GetObjectItemCaseSensitive(child_obj, "address");
                        bt_name = cJSON_GetObjectItemCaseSensitive(child_obj, "name");

                        if (cJSON_IsString(bt_addr) && (bt_addr->valuestring != NULL)){
                             printf("bt_addr[%d]:%s\n",i, bt_addr->valuestring);
                        }

                        if (cJSON_IsString(bt_name) && (bt_name->valuestring != NULL)){
                             printf("bt_name[%d]:%s\n",i, bt_name->valuestring);
                        }

                        send_cmd = cJSON_CreateObject();
                        cJSON_AddStringToObject(send_cmd,"proto",proto);
                        cJSON_AddStringToObject(send_cmd,"command",blue_cmd);

                        cJSON_AddItemToObject(send_cmd,"address",bt_addr);
                        cJSON_AddItemToObject(send_cmd,"name",bt_name);

                        bt_command = cJSON_Print(send_cmd);
                        printf("connect cmd :: %s\n",bt_command);

                        msg = caps_create();
                        caps_write_string(msg, (const char *)bt_command);

                        ret = flora_agent_post(agent, "bluetooth.a2dpsink.command", msg, FLORA_MSGTYPE_INSTANT);
 
                        yodalite_free(bt_command);
                        caps_destroy(msg);
                        cJSON_Delete(send_cmd);
                        break;
                    }
                    connecting = 0;
                }
            }
        }
    }
    return 0;
}

static void bt_service_event_callback(const char *name, uint32_t msgtype, caps_t msg, void *arg)
{

    const char *buf = NULL;
    cJSON *bt_event = NULL;

    caps_read_string(msg, &buf); 
    bt_event=cJSON_Parse(buf);

    if (strcmp(name, "bluetooth.common.event") == 0) {
        //printf("%s\n", buf);
        common_event_handle(bt_event);
    } else if (strcmp(name, "bluetooth.a2dpsource.event") == 0) {
        a2dpsource_event_handle(bt_event);
    } else if (strcmp(name, "bluetooth.ble.event") == 0) {
     //   printf("%s\n", buf);
    } else if (strcmp(name, "bluetooth.a2dpsink.event") == 0) {
        //printf("%s\n", buf);
    }

    cJSON_Delete(bt_event); 
}

static int bt_flora_init(void) 
{
    flora_agent_config_param_t param;

    memset(&param,0,sizeof(flora_agent_config_param_t));
    agent = flora_agent_create();

    param.sock_path  = "unix:/var/run/flora.sock";
    flora_agent_config(agent, FLORA_AGENT_CONFIG_URI, &param);
    param.value  = 1024;
    flora_agent_config(agent, FLORA_AGENT_CONFIG_BUFSIZE,&param);

    flora_agent_subscribe(agent, "bluetooth.common.event", bt_service_event_callback, NULL);
    flora_agent_subscribe(agent, "bluetooth.ble.event", bt_service_event_callback, NULL);
    flora_agent_subscribe(agent, "bluetooth.a2dpsink.event", bt_service_event_callback, NULL);
    flora_agent_subscribe(agent, "bluetooth.a2dpsource.event", bt_service_event_callback, NULL);
    flora_agent_subscribe(agent, "bluetooth.hfp.event", bt_service_event_callback, NULL);

    flora_agent_start(agent, 0);

    return 0;
}

static void bt_flora_exit(void)
{
  flora_agent_close(agent);
  flora_agent_delete(agent);
}

static int btmgr_test_cmd(int argc,int8_t * const argv[])
{
   int idx = 0;
 
   if(argc < 2){
      bluetooth_usage();
      return 0;
    }

    bt_flora_init();

    idx= atoi(argv[1]);

   switch(idx){
     case 1:
        ble_operation();
        break;
     case 2:
        a2dp_sink_operation();
        break;
     case 3:
        a2dp_source_operation();
        break;
     case 4:
        hfp_operation();
        break;
     case 88:
        test_operation();
        break;
     default:
       bluetooth_usage();
   }

   bt_flora_exit();

  return 0;
}

#define max_args    (2)
#define test_help  "blemgr_test num"

int cmd_btmgr_test(void)
{

  YODALITE_REG_CMD(btmgr_test,max_args,btmgr_test_cmd,test_help);
  return 0;
}

