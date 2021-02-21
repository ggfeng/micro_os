#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yodalite_autoconf.h>
#include <lib/property/properties.h>
#include <lib/shell/shell.h>


static int property_init_cmd(int argc,int8_t *const argv[])
{
    property_init();
    return 0;
}

static int property_deinit_cmd(int argc,int8_t *const argv[])
{
  property_deinit();
  return 0;
}

static int property_print_cmd(int argc,int8_t *const argv[])
{
  property_print_list();
  return 0;
}

static int property_get_cmd(int argc,int8_t *const argv[])
{
  char *key;
  char *default_value = "0";
  char value[PROPERTY_VALUE_MAX] = {0};

  if(argc < 2){
     printf("property_get key default\n");
     return 0;
  }

  key = argv[1];

  if(argc >= 3)
     default_value = argv[2];

  if(property_get(key,value,default_value)>=0){
     printf("%s:%s\n",key,value);
  }else{
     printf("error:property_get fail\n");
  }

  return 0;
}

static int property_set_cmd(int argc,int8_t *const argv[])
{
  char *key;
  char *value;

  if(argc < 3){
     printf("property_set key value\n");
     return 0;
  }

  key = argv[1];
  value = argv[2];

  if(property_set(key,value)==0){
     printf("%s:%s\n",key,value);
  }else{
     printf("error:property_set fail\n");
  }

  return 0;
}

static int property_remove_cmd(int argc,int8_t *const argv[])
{
  char *key;
  char *value;

  if(argc < 2){
     printf("property_remove key \n");
     return 0;
  }

  key = argv[1];

  if(property_remove(key)==0){
     printf("%s:%s\n",key);
  }else{
     printf("error:property_set fail\n");
  }

  return 0;
}

#define max_init_args      (1)
#define property_init_help      "property_init"

int cmd_property_init(void)
{

  YODALITE_REG_CMD(property_init,max_init_args,property_init_cmd,property_init_help);

  return 0;
}

#define max_deinit_args      (1)
#define property_deinit_help "property_deinit"

int cmd_property_deinit(void)
{

  YODALITE_REG_CMD(property_deinit,max_deinit_args,property_deinit_cmd,property_deinit_help);

  return 0;
}


//static int property_print_list(int argc,int8_t *const argv[])
//
#define max_print_args      (1)
#define property_print_help   "property_print"

int cmd_property_print(void)
{

  YODALITE_REG_CMD(property_print,max_print_args,property_print_cmd,property_print_help);

  return 0;
}

#define max_get_args      (3)
#define property_get_help "property_get key default"

int cmd_property_get(void)
{

  YODALITE_REG_CMD(property_get,max_get_args,property_get_cmd,property_get_help);

  return 0;
}

#define max_set_args      (3)
#define property_set_help "property_set key value"

int cmd_property_set(void)
{

  YODALITE_REG_CMD(property_set,max_set_args,property_set_cmd,property_set_help);

  return 0;
}

#define max_remove_args      (2)
#define property_remove_help "property_remove key"

int cmd_property_remove(void)
{

  YODALITE_REG_CMD(property_remove,max_remove_args,property_remove_cmd,property_remove_help);

  return 0;
}
