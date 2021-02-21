
#include <string.h>
#include <stdio.h>
#include "yodalite_autoconf.h"
#include "driver/chip/hal_flash.h"

#include "fatfs.h"

#define FLASH_DEVICE_NUM  0

static BYTE g_drv = -1;
static int g_register_flag = 0;

static DSTATUS ff_raw_initialize (BYTE pdrv)
{
//    printf("%s->%d\n",__func__,__LINE__);
    return 0;
}

static DSTATUS ff_raw_status (BYTE pdrv)
{
 //   printf("%s->%d\n",__func__,__LINE__);
    return 0;
}

static DRESULT ff_raw_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
//    printf("ff_raw_read - pdrv=%i, sector=%i, count=%i,buff:%p\n", (unsigned int)pdrv, (unsigned int)sector, (unsigned int)count,buff);
   
    if((sector+count)*CONFIG_FATFS_SECTOR_SIZE > CONFIG_FATFS_FLASH_SIZE){
        printf("fatfs %x > %x\n",(unsigned int)((sector+count)*CONFIG_FATFS_SECTOR_SIZE),(unsigned int)CONFIG_FATFS_FLASH_SIZE);
        return RES_ERROR;
     }


     if (HAL_Flash_Open(FLASH_DEVICE_NUM, 3000) != HAL_OK) {
          printf("open %d fail\n",FLASH_DEVICE_NUM);
          return -1;
      }
   
     if(HAL_Flash_Read(FLASH_DEVICE_NUM,CONFIG_FATFS_FLASH_BASE+sector*CONFIG_FATFS_SECTOR_SIZE,buff,count*CONFIG_FATFS_SECTOR_SIZE) != 0){
        printf("fatfs flash read fail\n");
        HAL_Flash_Close(FLASH_DEVICE_NUM);
        return RES_ERROR;
    }

    HAL_Flash_Close(FLASH_DEVICE_NUM);

    return RES_OK;
}


static DRESULT ff_raw_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
 //   printf("ff_raw_read - pdrv=%i, sector=%i, count=%i,buff:%p\n", (unsigned int)pdrv, (unsigned int)sector, (unsigned int)count,buff);
   
    if((sector+count)*CONFIG_FATFS_SECTOR_SIZE > CONFIG_FATFS_FLASH_SIZE){
        printf("fatfs %x > %x\n",(unsigned int)((sector+count)*CONFIG_FATFS_SECTOR_SIZE),(unsigned int)CONFIG_FATFS_FLASH_SIZE);
        return RES_ERROR;
     }

     if (HAL_Flash_Open(FLASH_DEVICE_NUM, 3000) != HAL_OK) {
          printf("open %d fail\n",FLASH_DEVICE_NUM);
          return -1;
     }
  
    if(HAL_Flash_Erase(FLASH_DEVICE_NUM,CONFIG_FATFS_SECTOR_SIZE,CONFIG_FATFS_FLASH_BASE+sector*CONFIG_FATFS_SECTOR_SIZE,count) != 0){
         printf("fatfs erase fail\n");
        HAL_Flash_Close(FLASH_DEVICE_NUM);
        return RES_ERROR;
     }
   
     if(HAL_Flash_Write(FLASH_DEVICE_NUM,CONFIG_FATFS_FLASH_BASE+sector*CONFIG_FATFS_SECTOR_SIZE,buff,count*CONFIG_FATFS_SECTOR_SIZE) != 0){
        printf("fatfs flash write fail\n");
        HAL_Flash_Close(FLASH_DEVICE_NUM);
        return RES_ERROR;
    }

    HAL_Flash_Close(FLASH_DEVICE_NUM);

    return RES_OK;
}

static DRESULT ff_raw_ioctl (BYTE pdrv, BYTE cmd, void *buff)
{
  //  printf("%s->%d,cmd:%d,part->size:0x%x,%x,%x\n",__func__,__LINE__,cmd,pdrv,CONFIG_FATFS_FLASH_SIZE,CONFIG_FATFS_SECTOR_SIZE);

    switch (cmd) {
        case CTRL_SYNC:
            return RES_OK;
        case GET_SECTOR_COUNT:
            *((DWORD *) buff) = CONFIG_FATFS_FLASH_SIZE/CONFIG_FATFS_SECTOR_SIZE;
            return RES_OK;
        case GET_SECTOR_SIZE:
            *((WORD *) buff) = CONFIG_FATFS_SECTOR_SIZE;
            return RES_OK;
        case GET_BLOCK_SIZE:
            *((WORD *) buff) = CONFIG_FATFS_SECTOR_SIZE;
            return RES_OK;
    }
    return RES_ERROR;
}


static int  ff_diskio_register_raw_partition(BYTE pdrv)
{
    if (pdrv >= FF_VOLUMES) {
        return -1;
    }
    static const ff_diskio_impl_t raw_impl = {
        .init = ff_raw_initialize,
        .status = ff_raw_status,
        .read = ff_raw_read,
        .write = ff_raw_write,
        .ioctl = ff_raw_ioctl
    };

    ff_diskio_register(pdrv, &raw_impl);

    return 0;
}

int spiflash_fatfs_uninit(void)
{

   if(g_register_flag ==1)
   {
     ff_diskio_unregister(g_drv);
     g_register_flag = 0;
   }

   return 0;
}


int spiflash_fatfs_init(void)
{
   BYTE pdrv;
   int result;

   if(g_register_flag  == 0)
   {
      pdrv = 0xFF;

      if (ff_diskio_get_drive(&pdrv) != 0) {
        printf("the maximum count of volumes is already mounted");
        return -1;
      }

    result = ff_diskio_register_raw_partition(pdrv);
    if (result != 0) {
        printf("ff_diskio_register_raw_partition failed pdrv=%i, error - 0x(%x)", pdrv, result);
        goto fail;
    }

     g_drv = pdrv; 
     g_register_flag =1;
   }

    return 0;
fail:
    ff_diskio_unregister(pdrv);
    return result;
}
