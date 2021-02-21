#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hardware/pal_flash.h>
#include <hapi/nv.h>

#include <yodalite_autoconf.h>
#ifdef CONFIG_MEM_EXT_ENABLE
#include <lib/mem_ext/mem_ext.h>
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

#define MSG_FLAG_HEAD "YODALITE:"

#define SEED_OFFSET                 (MSG_LENGTH * 0)
#define SN_OFFSET                   (MSG_LENGTH * 1)
#define DEVICE_TYPEID_OFFSET        (MSG_LENGTH * 2)


static int read_key_partition(unsigned int off,char *buf,unsigned int size)
{
  char *pBuf=NULL;
  int fd = -1;

  if((fd=flash_open(CONFIG_KEY_PART_FLASH_NAME))== -1){
    printf("error:open %s fail\n",CONFIG_KEY_PART_FLASH_NAME);
    return -1;
  }

  if(buf == NULL || off > CONFIG_KEY_PART_SIZE || 
    off + size > CONFIG_KEY_PART_SIZE){
    printf("error:off:%x,size:%x,base:%x,size:%x\n",off,size,CONFIG_KEY_PART_ADDR,CONFIG_KEY_PART_SIZE);
    flash_close(fd);
    return -1;
  }

  if((pBuf = (char*)yodalite_malloc(CONFIG_KEY_PART_SIZE)) == NULL){
    printf("error:yodalite_malloc %d bytes fail\n",CONFIG_KEY_PART_SIZE);
    flash_close(fd);
    return -1;
  }

  if(flash_read(fd,CONFIG_KEY_PART_ADDR,pBuf,CONFIG_KEY_PART_SIZE)){
    printf("error:flash_read at 0x%x %d bytes fail\n",CONFIG_KEY_PART_ADDR,CONFIG_KEY_PART_SIZE);
    yodalite_free(pBuf);
    flash_close(fd);
    return -1;
  }

  memcpy(buf,pBuf+off,size); 
  yodalite_free(pBuf);
  flash_close(fd);

  return 0;
}

static int write_key_partition(unsigned int off,char *buf,unsigned int size)
{
  char *pBuf=NULL;
  int fd = -1;

  if(buf == NULL || off > CONFIG_KEY_PART_SIZE || 
    off+size > CONFIG_KEY_PART_SIZE){
    printf("error:off:%x,size:%x,base:%x,size:%x\n",off,size,CONFIG_KEY_PART_ADDR,CONFIG_KEY_PART_SIZE);
    return -1;
  }

  if((pBuf =(char*)yodalite_malloc(CONFIG_KEY_PART_SIZE)) == NULL){
    printf("error:yodalite_malloc %d bytes fail\n",CONFIG_KEY_PART_SIZE);
    return -1;
  }

  if(read_key_partition(0,pBuf,CONFIG_KEY_PART_SIZE)){
     yodalite_free(pBuf);
     printf("error:read_key_partition at:0x%x %d bytes fail\n",0,CONFIG_KEY_PART_SIZE);
     return -1;
  }

  if((fd=flash_open(CONFIG_KEY_PART_FLASH_NAME))== -1){
    printf("error:open %s fail\n",CONFIG_KEY_PART_FLASH_NAME);
    yodalite_free(pBuf);
    return -1;
  }

  if(flash_erase(fd,CONFIG_KEY_PART_ADDR,CONFIG_KEY_PART_SIZE)){
     printf("error:flash_erase at 0x%x %d bytes fail\n",CONFIG_KEY_PART_ADDR,CONFIG_KEY_PART_SIZE);
     yodalite_free(pBuf);
     flash_close(fd);
     return -1;
  }

  memcpy(pBuf+off,buf,size);

  if(flash_write(fd,CONFIG_KEY_PART_ADDR,pBuf,CONFIG_KEY_PART_SIZE)){
    printf("error:flash_write at 0x%x %d bytes fail\n",CONFIG_KEY_PART_ADDR,CONFIG_KEY_PART_SIZE);
    yodalite_free(pBuf);
    flash_close(fd);
    return -1;
  }

  yodalite_free(pBuf);
  flash_close(fd);
  return 0; 
}


static int nv_safe_read(unsigned int off,char *buf)
{
  char *h;
  char *pBuf;
  int iret= -1;

  if((pBuf =(char*) yodalite_malloc(MSG_LENGTH)) == NULL){
    printf("error:yodalite_malloc %d bytes fail\n",CONFIG_KEY_PART_SIZE);
    return -1;
  }

  memset(pBuf,0,MSG_LENGTH);

  if(read_key_partition(off,pBuf,MSG_LENGTH)== 0){
     if((h = strstr(pBuf,MSG_FLAG_HEAD)) != NULL){
        strcpy(buf,h+strlen(MSG_FLAG_HEAD));
        iret = 0;
     }
  }

  yodalite_free(pBuf);

  return iret;
}

static int nv_safe_write(unsigned int off,char *buf)
{
  char *pBuf;
  int iret = -1;

  if((pBuf =(char*) yodalite_malloc(MSG_LENGTH)) == NULL){
    printf("error:yodalite_malloc %d bytes fail\n",CONFIG_KEY_PART_SIZE);
    return -1;
  }

  memset(pBuf,0,MSG_LENGTH);
  snprintf(pBuf,MSG_LENGTH,"%s%s",MSG_FLAG_HEAD,buf);
  iret =  write_key_partition(off,pBuf,MSG_LENGTH);
  yodalite_free(pBuf);

  return iret; 
}

int read_seed(char *buf)
{
  return nv_safe_read(SEED_OFFSET,buf);
}

int write_seed(char *buf)
{
  return nv_safe_write(SEED_OFFSET,buf);
}

int read_sn(char *buf)
{
  return nv_safe_read(SN_OFFSET,buf);
}

int write_sn(char *buf)
{
  return nv_safe_write(SN_OFFSET,buf);
}

int read_devicetypeid(char *buf)
{
  return nv_safe_read(DEVICE_TYPEID_OFFSET,buf);
}

int write_devicetypeid(char *buf)
{
  return nv_safe_write(DEVICE_TYPEID_OFFSET,buf);
}
