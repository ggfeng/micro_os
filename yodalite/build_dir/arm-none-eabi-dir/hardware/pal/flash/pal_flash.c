//#include <yodalite_autoconf.h>

#ifndef CONFIG_LIBC_ENABLE
#include <stdio.h>
#include <string.h>
#else
#include <lib/libc/yodalite_libc.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <hardware/platform.h>
#include <hardware/pal_flash.h>

static struct flash_lapi pal_flash_lapi = {0};
static tFLASH_DEV pal_flash_tbl[] = { {PAL_SPI_FLASH_0,eSPI,0},};

int pal_flash_init(struct flash_lapi *platform_flash_lapi)
{
    if(!platform_flash_lapi)
        return -1;

    pal_flash_lapi.yodalite_flash_open  = platform_flash_lapi->yodalite_flash_open;
    pal_flash_lapi.yodalite_flash_close = platform_flash_lapi->yodalite_flash_close;
    pal_flash_lapi.yodalite_flash_read  = platform_flash_lapi->yodalite_flash_read;
    pal_flash_lapi.yodalite_flash_write = platform_flash_lapi->yodalite_flash_write;
    pal_flash_lapi.yodalite_flash_ioctl = platform_flash_lapi->yodalite_flash_ioctl;
   pal_flash_lapi.yodalite_flash_inherent_erase = platform_flash_lapi->yodalite_flash_inherent_erase;
   pal_flash_lapi.yodalite_flash_inherent_read = platform_flash_lapi->yodalite_flash_inherent_read;
   pal_flash_lapi.yodalite_flash_inherent_write = platform_flash_lapi->yodalite_flash_inherent_write;

    return 0;
}

static int flash_ioctl(int fd,int cmd,void *arg)
{
  int ret =0; 

  tFLASH_DEV *pdev= &pal_flash_tbl[fd];

 if(_IOC_TYPE(cmd) != PAL_FLASH_IOC_MAGIC){
   printf("error:magic %c != %c\n",_IOC_TYPE(cmd),PAL_FLASH_IOC_MAGIC);
   return -1;
  }

 if(_IOC_NR(cmd) > PAL_FLASH_MAX_NR){
   printf("error:max cmd %d > %d\n",_IOC_NR(cmd),PAL_FLASH_MAX_NR);
   return -1;
 }

 if(pal_flash_lapi.yodalite_flash_ioctl)
     ret = pal_flash_lapi.yodalite_flash_ioctl(pdev,cmd,arg);

  return ret;
}

int flash_getInfo(int fd,tFLASH_INFO *pinfo)
{
   return flash_ioctl(fd,PAL_FLASH_IOC_INFO,(void*)pinfo);
}

int flash_erase(int fd,unsigned int off,unsigned int size)
{
   tERASE_INFO erase;
   tFLASH_INFO info;

   if(flash_getInfo(fd,&info)){
     printf("error:flash_getInfo()\n");
     return -1;
   }

   if((off &(info.block-1)) || 
     (size&(info.block-1)) || 
     (off + size >info.size)) {
     printf("error:erase off or size\n");
     return -1;
   }

   erase.off = off;
   erase.size = size;

   return flash_ioctl(fd,PAL_FLASH_IOC_ERASE,(void*)&erase);
}

int flash_read(int fd,unsigned int off,void *data, unsigned int size)
{
  int ret = 0;
  tFLASH_DEV *pdev= &pal_flash_tbl[fd];

  if(pal_flash_lapi.yodalite_flash_read)
     ret = pal_flash_lapi.yodalite_flash_read(pdev,off,data,size);

  return ret;
}

int flash_write(int fd,unsigned int off,void *data, unsigned int size)
{
  int ret = 0;
  tFLASH_DEV *pdev= &pal_flash_tbl[fd];

  if(pal_flash_lapi.yodalite_flash_write)
     ret = pal_flash_lapi.yodalite_flash_write(pdev,off,data,size);

    return ret;
}

int flash_open(char *name)
{
  int i;
  tFLASH_DEV *pdev = &pal_flash_tbl[0];
  int num = sizeof(pal_flash_tbl)/sizeof(pal_flash_tbl[0]);

  for(i=0;i<num;i++){
     if(strcmp(pdev->name,name)==0){
      if(pal_flash_lapi.yodalite_flash_open){
        if(pal_flash_lapi.yodalite_flash_open(pdev)){
           pdev ++;
           continue;
        }
      }
      return i;
     }
     pdev ++;
   }
  return -1;
}

int flash_close(int fd)
{
  int ret=0;
  tFLASH_DEV*pdev= &pal_flash_tbl[fd];

 if(pal_flash_lapi.yodalite_flash_close)
     ret = pal_flash_lapi.yodalite_flash_close(pdev);

  return ret;
}

int flash_inherent_erase(unsigned char Sector)
{
	int ret=0;
	if(pal_flash_lapi.yodalite_flash_inherent_erase)
	  ret = pal_flash_lapi.yodalite_flash_inherent_erase(Sector);
	return ret;
}

int flash_inherent_read(unsigned long int addr,unsigned char *data,unsigned short int size)
{
	int ret = 0;
	if(pal_flash_lapi.yodalite_flash_inherent_read)
	    ret = pal_flash_lapi.yodalite_flash_inherent_read(addr,data,size);
	return ret;}

int flash_inherent_write(unsigned long int addr,unsigned char *data,unsigned short int size)
{
	int ret = 0;
	if(pal_flash_lapi.yodalite_flash_inherent_write)
		ret = pal_flash_lapi.yodalite_flash_inherent_write(addr,data,size);
	return ret;
	
}
