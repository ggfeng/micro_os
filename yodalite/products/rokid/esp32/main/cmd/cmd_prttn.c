#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_partition.h"
#include "yodalite_autoconf.h"
#include "lib/shell/shell.h"

#if (CONFIG_PRTTN_INFO_SHOW_ENABLE ==1)

#define PARTITION_NAME   "fatfs"


static int show_prttn_info(int argc,char *const argv[])
{

    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);

    for (; it != NULL; it = esp_partition_next(it)) {

        const esp_partition_t *p = esp_partition_get(it);

        if (p != NULL) {
           printf("addr:0x%08x;size:0x%08x;label:%s\n",p->address,p->size,p->label);
        } else {
        printf("Partition error: can't find partition name:%s\n", PARTITION_NAME);
       }
    }

    esp_partition_iterator_release(it);

    it = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);

    for (; it != NULL; it = esp_partition_next(it)) {

        const esp_partition_t *p = esp_partition_get(it);

        if (p != NULL) {
           printf("addr:0x%08x;size:%08x;label:%s\n",p->address,p->size,p->label);
        } else {
        printf("Partition error: can't find partition name:%s\n", PARTITION_NAME);
       }
    }
    esp_partition_iterator_release(it);
    
    return 0;
}

#define max_args   (1)
#define partition_help  "show partition info"

int cmd_show_prttn_info(void)
{
    YODALITE_REG_CMD(info_prttn,max_args,show_prttn_info,partition_help);

    return 0;
}

#endif
