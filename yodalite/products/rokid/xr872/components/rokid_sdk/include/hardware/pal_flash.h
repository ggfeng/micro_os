#ifndef _PAL_FLASH_H_
#define _PAL_FLASH_H_

#include <hardware/ioctl.h>

#define PAL_FLASH_IOC_MAGIC 'f'
#define PAL_FLASH_IOC_ERASE _IOW(PAL_FLASH_IOC_MAGIC,0,sizeof(void*))
#define PAL_FLASH_IOC_INFO  _IOR(PAL_FLASH_IOC_MAGIC,1,sizeof(void*))
#define PAL_FLASH_MAX_NR    1

#define PAL_SPI_FLASH_0  "spiflash/0"

typedef enum
{
  eSPI,
  eNOR,
  eNAND,
}eTYPE;

typedef struct 
{
  unsigned int off;
  unsigned int size;
}tERASE_INFO;


typedef struct 
{
  char * name;
  eTYPE type;
  int   idx;
  int   flag;
}tFLASH_DEV;

typedef struct {
    unsigned int sector;
    unsigned int block;
    unsigned int size;
}tFLASH_INFO;


struct flash_lapi {
    int (*yodalite_flash_open)(tFLASH_DEV*pdev);
    int (*yodalite_flash_close)(tFLASH_DEV*pdev);
    int (*yodalite_flash_ioctl)(tFLASH_DEV*pdev,int cmd,void *arg);
    int (*yodalite_flash_read)(tFLASH_DEV*pdev,unsigned int off,void *data, unsigned int size);
    int (*yodalite_flash_write)(tFLASH_DEV*pdev,unsigned int off,void *data, unsigned int size);
    int (*yodalite_flash_inherent_erase)(unsigned char Sector);
    int (*yodalite_flash_inherent_read)(unsigned long int addr,unsigned char *data,unsigned short int size);
    int (*yodalite_flash_inherent_write)(unsigned long int addr,unsigned char *data,unsigned short int size);
};

extern int flash_open(char *name);
extern int flash_close(int fd);
extern int flash_getInfo(int fd,tFLASH_INFO *pinfo);
extern int pal_flash_init(struct flash_lapi *platform_flash_lapi);
extern int flash_erase(int fd,unsigned int off,unsigned int size);
extern int flash_write(int fd,unsigned int off,void *data, unsigned int size);
extern int flash_read(int fd,unsigned int off, void *data,unsigned int size);

#endif  /*_PAL_FLASH_H_*/

