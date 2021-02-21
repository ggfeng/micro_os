#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yodalite_autoconf.h>
#include <hapi/nv.h>
#include <lib/shell/shell.h>

#ifdef CONFIG_MEM_EXT_ENABLE
#include <lib/mem_ext/mem_ext.h>
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

enum{
 eSN_RD=0,
 eSN_WR,
 eSEED_RD,
 eSEED_WR,
 eID_RD,
 eID_WR,
 eEXIT,
};

const char *cmd_tbl[]= {
 "AT+Sn?",
 "AT+Sn=",
 "AT+Seed?",
 "AT+Seed=",
 "AT+Id?",
 "AT+Id=",
 "exit",
};

int cmd2idx(char *cmd){
  int i;
  for(i=0;i<sizeof(cmd_tbl)/sizeof(cmd_tbl[0]);i++){
   if(strstr(cmd,cmd_tbl[i]))
     return i;
  }
  return -1;
}


int read_cmd(char **cmd)
{
    *cmd = readline("ftm#");
    return 0; 
}

int write_cmd(const char*head,const char *cmd)
{
  if(head)
    printf("%s%s",head,cmd);
  else
    printf("%s",cmd);
 
    printf("\n");
  return 0;
}

void ftm_usage(void)
{
  
   printf("Usage:\n");
   printf("      %s\n",cmd_tbl[0]);
   printf("      %s%s\n",cmd_tbl[1],"sn");
   printf("      %s\n",cmd_tbl[2]);
   printf("      %s%s\n",cmd_tbl[3],"seed");
   printf("      %s\n",cmd_tbl[4]);
   printf("      %s%s\n",cmd_tbl[5],"devicetypeid");
   printf("      %s\n",cmd_tbl[6]);
}

static int ftm_read_sn(void)
{
  char *pBuf;

  if((pBuf =(char*) yodalite_malloc(MSG_LENGTH)) == NULL){
    printf("error:yodalite_malloc %d bytes fail\n",CONFIG_KEY_PART_SIZE);
    return -1;
  }

  if(read_sn(pBuf) == 0){
     write_cmd(cmd_tbl[1],pBuf);
   //printf("seed:%s\n",pBuf);
  }else{
   // printf("read sn fail\n");
     write_cmd(NULL,"ERROR");
  }
  yodalite_free(pBuf);
  return 0;
}

static int ftm_read_seed(void)
{
  char *pBuf;

  if((pBuf =(char*) yodalite_malloc(MSG_LENGTH)) == NULL){
    printf("error:yodalite_malloc %d bytes fail\n",CONFIG_KEY_PART_SIZE);
    return -1;
  }

  if(read_seed(pBuf) == 0){
     write_cmd(cmd_tbl[3],pBuf);
   //printf("seed:%s\n",pBuf);
  }else{
  //  printf("read seed fail\n");
     write_cmd(NULL,"ERROR");
  }
  yodalite_free(pBuf);
  return 0;
}

static int ftm_read_devicetypeid(void)
{
  char *pBuf;

  if((pBuf =(char*) yodalite_malloc(MSG_LENGTH)) == NULL){
    printf("error:yodalite_malloc %d bytes fail\n",CONFIG_KEY_PART_SIZE);
    return -1;
  }
  if(read_devicetypeid(pBuf) == 0){
     write_cmd(cmd_tbl[5],pBuf);
   //printf("seed:%s\n",pBuf);
  }else{
  //  printf("read devicetypeid fail\n");
     write_cmd(NULL,"ERROR");
  }
  yodalite_free(pBuf);
  return 0;
}


static int ftm_write_sn(char*data)
{
  if(write_sn(data+strlen(cmd_tbl[1])) == 0){
     write_cmd(NULL,"OK");
  }else{
   // printf("write sn fail\n");
     write_cmd(NULL,"ERROR");
  }

  return 0;
}

static int ftm_write_seed(char*data)
{
  if(write_seed(data+strlen(cmd_tbl[3])) == 0){
     write_cmd(NULL,"OK");
  }else{
   // printf("write seed fail\n");
     write_cmd(NULL,"ERROR");
  }

  return 0;
}

static int ftm_write_devicetypeid(char*data)
{

  if(write_devicetypeid(data+strlen(cmd_tbl[5])) == 0){
     write_cmd(NULL,"OK");
  }else{
   //printf("write devicetypid fail\n");
     write_cmd(NULL,"ERROR");
  }

  return 0;
}


static int ftm_cmd_process(int argc,int8_t *const argv[])
{
   int idx = -1;
   char *cmd = NULL;

     ftm_usage();
   while(1){
     read_cmd(&cmd);  

     if(cmd)
        idx=cmd2idx(cmd);

     switch (idx){
       case eSN_RD: 
         ftm_read_sn();
            break;
       case eSN_WR: 
         ftm_write_sn(cmd);
            break;
       case eSEED_RD: 
         ftm_read_seed();
            break;
       case eSEED_WR: 
         ftm_write_seed(cmd);
            break;
       case eID_RD: 
         ftm_read_devicetypeid();
            break;
       case eID_WR: 
         ftm_write_devicetypeid(cmd);
            break;
       case eEXIT: 
            return 0;
       default:
           ftm_usage();
    }
  }
  return 0;
}

#define max_args      1
#define ftm_help        "factory"

int cmd_ftm(void)
{
  YODALITE_REG_CMD(ftm,max_args,ftm_cmd_process,ftm_help);
  return 0;
}
