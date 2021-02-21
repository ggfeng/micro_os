#include <string.h>
#include <stdio.h>
#include "hardware/pal_flash.h"
#include "esp_partition.h"

static const esp_partition_t *esp32_find_partition(uint32_t addr,uint32_t size)
{
    const esp_partition_t *p = NULL;

    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);

    for (; it != NULL; it = esp_partition_next(it)) {

        p = esp_partition_get(it);

        if (p != NULL) {
           if(addr >= p->address && addr+size <= p->address + p->size){
    //        printf("addr:%08x prtt:%s:0x%08x -- 0x%08x\n",addr,p->label,p->address,p->address + p->size);
              esp_partition_iterator_release(it);
              return  p;
           }
        } 
    }

    esp_partition_iterator_release(it);

    it = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);

    for (; it != NULL; it = esp_partition_next(it)) {
        p = esp_partition_get(it);
        if (p != NULL) {
           if(addr >= p->address && addr+size <= p->address + p->size){
  //          printf("addr:%08x prtt:%s:0x%08x -- 0x%08x\n",addr,p->label,p->address,p->address + p->size);
              esp_partition_iterator_release(it);
              return  p;
           }
        } 
    }

    esp_partition_iterator_release(it);


    return NULL;
}

static int esp32_flash_open(tFLASH_DEV *pdev)
{
  if(strcmp(pdev->name,PAL_SPI_FLASH_0) == 0){
     pdev->flag = 1; 
     return  0;
  }

  return -1;
}

static int esp32_flash_close(tFLASH_DEV *pdev)
{

  if(pdev->flag == 1){
     pdev->flag = 0;
     return 0;
  }

  return -1;
}

static int esp32_flash_ioctl(tFLASH_DEV *pdev,int cmd,void *arg)
{

   if(pdev->flag == 0){
     printf("error:%s is not open\n",pdev->name);
     return -1;
   }

   switch(cmd){
   case PAL_FLASH_IOC_ERASE:
   {
        esp_err_t err;
        tERASE_INFO *erase =(tERASE_INFO*)arg;
        const esp_partition_t *part = esp32_find_partition(erase->off,erase->size);

        if(part == NULL){
          printf("error:esp32_find_partition failed\n");
          return -1;
        }

        err = esp_partition_erase_range(part,erase->off-part->address,erase->size);
        if(err != ESP_OK){
          printf("error:esp_partition_erase_range failed (0x%x)", err);
          return -1;
        } 
        break;
   }
   case PAL_FLASH_IOC_INFO:
   {
        tFLASH_INFO *pInfo =(tFLASH_INFO*)arg;
        pInfo->sector = SPI_FLASH_SEC_SIZE;
        pInfo->block = SPI_FLASH_SEC_SIZE;
  //    pInfo->size  = SPI_FLASH_0_SIZE; 
        pInfo->size = spi_flash_get_chip_size();
        break;
   }

   default:
        break;
 }
   return 0;
}

static int esp32_flash_read(tFLASH_DEV *pdev,unsigned int addr,void*data,unsigned int size)
{
   esp_err_t err;
   const esp_partition_t *part;

   if(pdev->flag == 0){
     printf("error:%s is not open\n",pdev->name);
     return -1;
   }

   part = esp32_find_partition(addr,size);

   if(part == NULL){
       printf("error:esp32_find_partition failed\n");
       return -1;
    }

   //printf("%s->%d:addr:%x,size:%x\n",__func__,__LINE__,addr-part->address,size);
    err = esp_partition_read(part,addr-part->address,data,size);

    if (err != ESP_OK) {
        printf("error:esp_partition_read failed (0x%x)", err);
        return -1;
    }

    return  0;
}


static int esp32_flash_write(tFLASH_DEV *pdev,unsigned int addr,void*data,unsigned int size)
{
   esp_err_t err;
   const esp_partition_t *part;

   if(pdev->flag == 0){
     printf("error:%s is not open\n",pdev->name);
     return -1;
   }

   part = esp32_find_partition(addr,size);

   if(part == NULL){
       printf("error:esp32_find_partition failed\n");
       return -1;
    }

   //printf("%s->%d:addr:%x,size:%x\n",__func__,__LINE__,addr-part->address,size);

    err = esp_partition_write(part,addr-part->address,data,size);
    if (err != ESP_OK){
        printf("error:esp_partition_read failed (0x%x)", err);
        return -1;
    }

   return  0;
}


struct flash_lapi yodalite_flash = {
	.yodalite_flash_open =  esp32_flash_open,
	.yodalite_flash_close = esp32_flash_close,
	.yodalite_flash_read =  esp32_flash_read,
	.yodalite_flash_write = esp32_flash_write,
	.yodalite_flash_ioctl = esp32_flash_ioctl,
};
