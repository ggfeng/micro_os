/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/* ESP-IDF port Copyright 2016 Espressif Systems (Shanghai) PTE LTD      */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#include <yodalite_autoconf.h>

#ifndef CONFIG_LIBC_ENABLE
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#else
#include <lib/libc/yodalite_libc.h>
#endif

#include <lib/fatfs/diskio.h>		/* FtFs lower layer API */
#include <lib/fatfs/ffconf.h>
#include <lib/fatfs/ff.h>

#ifdef CONFIG_MEM_EXT_ENABLE
#include <lib/mem_ext/mem_ext.h>
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

static ff_diskio_impl_t * s_impls[FF_VOLUMES] = { NULL };

#if FF_MULTI_PARTITION		/* Multiple partition configuration */
PARTITION VolToPart[] = {
    {0, 0},    /* Logical drive 0 ==> Physical drive 0, auto detection */
    {1, 0}     /* Logical drive 1 ==> Physical drive 1, auto detection */
};
#endif

int ff_diskio_get_drive(BYTE* out_pdrv)
{
    BYTE i;
    for(i=0; i<FF_VOLUMES; i++) {
        if (!s_impls[i]) {
            *out_pdrv = i;
            return 0;
        }
    }
    return -1;
}

void ff_diskio_register(BYTE pdrv, const ff_diskio_impl_t* discio_impl)
{

    if(pdrv >=  FF_VOLUMES){
       printf("#Err:pdrv(%d) < FF_VOLUMES(%d)\n",pdrv,FF_VOLUMES);
       return ;
    }

    if (s_impls[pdrv]) {
        ff_diskio_impl_t* im = s_impls[pdrv];
        s_impls[pdrv] = NULL;
        yodalite_free(im);
    }

    if (!discio_impl) {
       return;
    }

    ff_diskio_impl_t * impl = (ff_diskio_impl_t *)yodalite_malloc(sizeof(ff_diskio_impl_t));

    if(impl == NULL){
      printf("#Err:yodalite_malloc fail!\n");
      return; 
    }

    memcpy(impl, discio_impl, sizeof(ff_diskio_impl_t));

    s_impls[pdrv] = impl;

//    printf("%s:init:%p,read:%p,impl->init:%p,impl->read:%p,s_impls[%d]->init:%p,s_impls[%d]->reda:%p\n",__func__,discio_impl->init,discio_impl->read,impl->init,impl->read,pdrv,s_impls[pdrv]->init,pdrv,s_impls[pdrv]->read);
}

DSTATUS ff_disk_initialize (BYTE pdrv)
{
    if(s_impls[pdrv] == NULL)
      return  STA_NOINIT;

    return s_impls[pdrv]->init(pdrv);
}
DSTATUS ff_disk_status (BYTE pdrv)
{
    if(s_impls[pdrv] == NULL)
      return  STA_NOINIT;

    return s_impls[pdrv]->status(pdrv);
}
DRESULT ff_disk_read (BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
    if(s_impls[pdrv] == NULL)
      return  STA_NOINIT;

 return s_impls[pdrv]->read(pdrv, buff, sector, count);

}
DRESULT ff_disk_write (BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
    if(s_impls[pdrv] == NULL)
      return  STA_NOINIT;

    return s_impls[pdrv]->write(pdrv, buff, sector, count);
}

DRESULT ff_disk_ioctl (BYTE pdrv, BYTE cmd, void* buff)
{
    if(s_impls[pdrv] == NULL)
      return  STA_NOINIT;

    return s_impls[pdrv]->ioctl(pdrv, cmd, buff);
}

DWORD get_fattime(void)
{
#ifndef CONFIG_LIBC_ENABLE
    time_t t = time(NULL);
    struct tm tmr;
    localtime_r(&t, &tmr);
    int year = tmr.tm_year < 80 ? 0 : tmr.tm_year - 80;
    return    ((DWORD)(year) << 25)
            | ((DWORD)(tmr.tm_mon + 1) << 21)
            | ((DWORD)tmr.tm_mday << 16)
            | (WORD)(tmr.tm_hour << 11)
            | (WORD)(tmr.tm_min << 5)
            | (WORD)(tmr.tm_sec >> 1);
#else 
   return 0;
#endif


}
