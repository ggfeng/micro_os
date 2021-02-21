#ifndef _FATFS_H_

#define _FATFS_H_

#include "lib/fatfs/diskio.h"
#include "lib/fatfs/ffconf.h"
#include "lib/fatfs/ff.h"

extern int  spiflash_fatfs_init(void);
extern int  spiflash_fatfs_uninit(void);

#endif

