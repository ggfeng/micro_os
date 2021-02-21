#ifndef _FATFS_H_

#define _FATFS_H_

#include "lib/fatfs/diskio.h"
#include "lib/fatfs/ffconf.h"
#include "lib/fatfs/ff.h"
#include "esp_partition.h"

extern esp_err_t esp_spiflash_fatfs_init(const char* partition_label);
extern esp_err_t esp_spiflash_fatfs_uninit(const char* partition_label);

#endif

