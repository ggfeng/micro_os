
#include <string.h>
#include <stdio.h>
#include "fatfs.h"

static int g_register_flag = 0;
const esp_partition_t* ff_raw_handles[FF_VOLUMES];

static DSTATUS ff_raw_initialize (BYTE pdrv)
{

//  printf("%s->%d\n",__func__,__LINE__);
    return 0;
}

static DSTATUS ff_raw_status (BYTE pdrv)
{
//  printf("%s->%d\n",__func__,__LINE__);
    return 0;
}

static DRESULT ff_raw_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
    const esp_partition_t* part = ff_raw_handles[pdrv];

   // printf("ff_raw_read - pdrv=%i, sector=%i, count=%i,buff:%p\n", (unsigned int)pdrv, (unsigned int)sector, (unsigned int)count,buff);

    esp_err_t err = esp_partition_read(part,sector*SPI_FLASH_SEC_SIZE,buff,count*SPI_FLASH_SEC_SIZE);

    if (err != ESP_OK) {
        printf("esp_partition_read failed (0x%x)", err);
        return RES_ERROR;
    }

    return RES_OK;
}


static DRESULT ff_raw_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
    esp_err_t err;

    const esp_partition_t* part = ff_raw_handles[pdrv];

    err = esp_partition_erase_range(part,sector*SPI_FLASH_SEC_SIZE,count*SPI_FLASH_SEC_SIZE);

    if(err != ESP_OK)
    {
        printf("esp_partition_erase_range failed (0x%x)", err);
        return RES_ERROR;
    } 

    err = esp_partition_write(part,sector*SPI_FLASH_SEC_SIZE,buff,count*SPI_FLASH_SEC_SIZE);
    if (err != ESP_OK) {
         printf("esp_partition_write failed (0x%x)", err);
        return RES_ERROR;
    }

//  printf("%s->%d\n",__func__,__LINE__);
    return RES_OK;
}

static DRESULT ff_raw_ioctl (BYTE pdrv, BYTE cmd, void *buff)
{
    const esp_partition_t* part = ff_raw_handles[pdrv];

   // printf("%s->%d,cmd:%d,part->size:0x%x\n",__func__,__LINE__,cmd,part->size);

    switch (cmd) {
        case CTRL_SYNC:
            return RES_OK;
        case GET_SECTOR_COUNT:
            *((DWORD *) buff) = part->size / SPI_FLASH_SEC_SIZE;
            return RES_OK;
        case GET_SECTOR_SIZE:
            *((WORD *) buff) = SPI_FLASH_SEC_SIZE;
            return RES_OK;
        case GET_BLOCK_SIZE:
            return RES_ERROR;
    }
    return RES_ERROR;
}


static esp_err_t ff_diskio_register_raw_partition(BYTE pdrv, const esp_partition_t* part_handle)
{
    if (pdrv >= FF_VOLUMES) {
        return ESP_ERR_INVALID_ARG;
    }
    static const ff_diskio_impl_t raw_impl = {
        .init = ff_raw_initialize,
        .status = ff_raw_status,
        .read = ff_raw_read,
        .write = ff_raw_write,
        .ioctl = ff_raw_ioctl
    };

    ff_diskio_register(pdrv, &raw_impl);

    ff_raw_handles[pdrv] = part_handle;

    return ESP_OK;

}


static BYTE ff_diskio_get_pdrv_raw(const esp_partition_t* part_handle)
{
    for (int i = 0; i < FF_VOLUMES; i++) {
        if (part_handle == ff_raw_handles[i]) {
            return i;
        }
    }
    return 0xff;
}

esp_err_t esp_spiflash_fatfs_uninit(const char* partition_label)
{
    const esp_partition_t *data_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA,
            ESP_PARTITION_SUBTYPE_DATA_FAT, partition_label);

    if (data_partition == NULL) {
        printf("Failed to find FATFS partition (type='data', subtype='fat', partition_label='%s'). Check the partition table.", partition_label);
        return ESP_ERR_NOT_FOUND;
    }

   if(g_register_flag ==1)
   {

     BYTE pdrv = ff_diskio_get_pdrv_raw(data_partition);

     if (pdrv == 0xff) {
        return ESP_ERR_INVALID_STATE;
     } 

     ff_raw_handles[pdrv] = NULL;
     ff_diskio_unregister(pdrv);

     g_register_flag = 0;
   }

   return 0;
}


esp_err_t esp_spiflash_fatfs_init(const char* partition_label)
{
    BYTE pdrv;
    esp_err_t result = ESP_OK;

    const esp_partition_t *data_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA,
            ESP_PARTITION_SUBTYPE_DATA_FAT, partition_label);

   if(g_register_flag  == 0)
   {
      if (data_partition == NULL) {
        printf("Failed to find FATFS partition (type='data', subtype='fat', partition_label='%s'). Check the partition table.", partition_label);
        return ESP_ERR_NOT_FOUND;
      }

      pdrv = 0xFF;

      if (ff_diskio_get_drive(&pdrv) != ESP_OK) {
        printf("the maximum count of volumes is already mounted");
        return ESP_ERR_NO_MEM;
      }

    result = ff_diskio_register_raw_partition(pdrv, data_partition);
    if (result != ESP_OK) {
        printf("ff_diskio_register_raw_partition failed pdrv=%i, error - 0x(%x)", pdrv, result);
        goto fail;
    }


     g_register_flag =1;
   }

    return 0;
fail:
    ff_diskio_unregister(pdrv);
    return result;
}
