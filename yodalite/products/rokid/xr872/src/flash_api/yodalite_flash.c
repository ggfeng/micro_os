#include <string.h>
#include <stdio.h>
#include "yodalite_autoconf.h"
#include "driver/chip/hal_flash.h"
#include "hardware/pal_flash.h"

#define FLASH_DEVICE_NUM  0
#define FLASH_SECT_SIZE (4096)
#define FLASH_CHIP_SIZE (0x200000)

static int xr87x_flash_open(tFLASH_DEV *pdev)
{
  if(strcmp(pdev->name,PAL_SPI_FLASH_0) == 0){
     pdev->flag = 1; 
     return  0;
  }

  return -1;
}

static int xr87x_flash_close(tFLASH_DEV *pdev)
{

  if(pdev->flag == 1){
     pdev->flag = 0;
     return 0;
  }

  return -1;
}

static int xr87x_flash_ioctl(tFLASH_DEV *pdev,int cmd,void *arg)
{

   if(pdev->flag == 0){
     printf("error:%s is not open\n",pdev->name);
     return -1;
   }

   switch(cmd){
   case PAL_FLASH_IOC_ERASE:
   {
    tERASE_INFO *erase =(tERASE_INFO*)arg;


   if (HAL_Flash_Open(FLASH_DEVICE_NUM, 3000) != HAL_OK) {
          printf("open %d fail\n",FLASH_DEVICE_NUM);
          return -1;
    }
  
    if(HAL_Flash_Erase(FLASH_DEVICE_NUM,FLASH_SECT_SIZE,erase->off,erase->size) != 0){
         printf("fatfs erase fail\n");
        HAL_Flash_Close(FLASH_DEVICE_NUM);
        return -1;
     }

    HAL_Flash_Close(FLASH_DEVICE_NUM);

      break;
   }
   case PAL_FLASH_IOC_INFO:
   {
        tFLASH_INFO *pInfo =(tFLASH_INFO*)arg;
        pInfo->sector = FLASH_SECT_SIZE;
        pInfo->block = FLASH_SECT_SIZE;
        pInfo->size = FLASH_CHIP_SIZE;
        break;
   }

   default:
        break;
 }
   return 0;
}

static int xr87x_flash_read(tFLASH_DEV *pdev,unsigned int addr,void*data,unsigned int size)
{

   if(pdev->flag == 0){
     printf("error:%s is not open\n",pdev->name);
     return -1;
   }

   if (HAL_Flash_Open(FLASH_DEVICE_NUM, 3000) != HAL_OK) {
          printf("open %d fail\n",FLASH_DEVICE_NUM);
          return -1;
     }

     if(HAL_Flash_Read(FLASH_DEVICE_NUM,addr,data,size) != 0){
        printf("fatfs flash read fail\n");
        HAL_Flash_Close(FLASH_DEVICE_NUM);
        return  -1;
    }

    HAL_Flash_Close(FLASH_DEVICE_NUM);

   return  0;
}


static int xr87x_flash_write(tFLASH_DEV *pdev,unsigned int addr,void*data,unsigned int size)
{
   if(pdev->flag == 0){
     printf("error:%s is not open\n",pdev->name);
     return -1;
   }

    if (HAL_Flash_Open(FLASH_DEVICE_NUM, 3000) != HAL_OK) {
       printf("open %d fail\n",FLASH_DEVICE_NUM);
       return -1;
    }


    if(HAL_Flash_Write(FLASH_DEVICE_NUM,addr,data,size) != 0){
        printf("fatfs flash write fail\n");
        HAL_Flash_Close(FLASH_DEVICE_NUM);
        return -1;
    }

    HAL_Flash_Close(FLASH_DEVICE_NUM);

    return  0;
}


struct flash_lapi yodalite_flash = {
	.yodalite_flash_open =  xr87x_flash_open,
	.yodalite_flash_close = xr87x_flash_close,
	.yodalite_flash_read =  xr87x_flash_read,
	.yodalite_flash_write = xr87x_flash_write,
	.yodalite_flash_ioctl = xr87x_flash_ioctl,
};
