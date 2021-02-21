#include <lib/fatfs/ff.h>
#include "common.h"

#define fd_t     FIL

static int config_open_file(char *file,fd_t*fd,uint8_t mode)
{
  FRESULT fr;

  fr = f_open(fd,file,mode);

  if (fr){
      BT_LOGE("#Err:f_open(%s) = %d\n",file,fr);
      return -1;
  }

  return 0;
}

static int config_close_file(fd_t*fd)
{
  FRESULT fr; 

  fr = f_close(fd);

  if (fr){
      printf("#Err:f_close() = %d\n",fr);
      return -1;
  }

  return 0;
}

static int config_read_file(fd_t *fd,uint8_t* buf,uint32_t size)
{
  FRESULT fr; 
  uint32_t bw;
  
  fr = f_read(fd,buf,size,&bw);

  if (fr ||  bw != size){
      BT_LOGE("f_read() %d bytes fr= %d\n",bw,fr);
      return -1;
  }
  return 0;
}

static int config_write_file(fd_t *fd,uint8_t* buf,uint32_t size)
{
  FRESULT fr; 
  uint32_t bw;
  fr = f_write(fd,buf,size,&bw);

  if (fr ||  bw != size){
      BT_LOGE("f_write() %d bytes fr= %d\n",bw,fr);
      return -1;
  }

  return 0;
}


int bd_strtoba(uint8_t *addr, const char *address) {
    int i;
    int len = strlen(address);
    char *dest = (char *)addr;
    char *begin = (char *)address;
    char *end_ptr;

    if (!address || !addr || len != 17) {
        BT_LOGE("faile to addr:%s, len:%d\n", address, len);
        return -1;
    }
    for (i = 0; i < 6; i++) {
        dest[i] = (char)strtoul(begin, &end_ptr, 16);
        if (!end_ptr) break;
        if (*end_ptr == '\0') break;
        if (*end_ptr != ':') {
            BT_LOGE("faile to addr:%s, len:%d, end_ptr: %c, %s\n", address, len, *end_ptr, end_ptr);
            return -1;
        }
        begin = end_ptr +1;
        end_ptr = NULL;
    }
    if (i != 5) {
        BT_LOGE("faile to addr:%s, len:%d\n", address, len);
        return -1;
    }
    BT_LOGV("%02X:%02X:%02X:%02X:%02X:%02X\n",
           dest[0], dest[1], dest[2], dest[3], dest[4], dest[5]);
    return 0;
}

int bt_autoconnect_sync(struct bt_autoconnect_config *config) {
    fd_t fd;
    char *buf = NULL;
    char bt_addr[18] = {0};
    int i = 0;
    cJSON * root;
    cJSON * array;
    cJSON * item[ROKIDOS_BT_AUTOCONNECT_DEVICES_MAX_NUM] = {0};

    if (config == NULL) {
        BT_LOGE("config is null\n");
        return -1;
    }
    root = cJSON_CreateObject();
    array = cJSON_CreateArray();

    cJSON_AddNumberToObject(root,"autoconnect_num",config->autoconnect_num);
    cJSON_AddNumberToObject(root,"autoconnect_mode",config->autoconnect_mode);

    for (i = 0; i < config->autoconnect_num; i++) {
        memset(bt_addr, 0, sizeof(bt_addr));

        item[i] = cJSON_CreateObject();
        BT_LOGI("index : %d name :: %s\n", i, config->dev[i].name);
        BT_LOGI("addr :: %02x:%02x:%02x:%02x:%02x:%02x\n",
               config->dev[i].addr[0], config->dev[i].addr[1],
               config->dev[i].addr[2], config->dev[i].addr[3],
               config->dev[i].addr[4], config->dev[i].addr[5]);

        sprintf(bt_addr, "%02x:%02x:%02x:%02x:%02x:%02x",
                config->dev[i].addr[0], config->dev[i].addr[1],
                config->dev[i].addr[2], config->dev[i].addr[3],
                config->dev[i].addr[4], config->dev[i].addr[5]);


        cJSON_AddStringToObject(item[i],"addr",bt_addr);
        cJSON_AddStringToObject(item[i],"name",config->dev[i].name);
        cJSON_AddItemToArray(array, item[i]);
    }

    cJSON_AddItemToObject(root,"devices",array);

    if(config_open_file(config->autoconnect_filename ,&fd,FA_CREATE_ALWAYS | FA_WRITE)){
     BT_LOGE("error:config_open_file %s fail\n",config->autoconnect_filename);
     cJSON_Delete(root);
     return -1;
   }

    buf = (char *)cJSON_Print(root);
    BT_LOGI("buf:: %s\n", buf);

    if(config_write_file(&fd,buf,strlen(buf))){
       BT_LOGE("error:config_write_file %s fail\n",config->autoconnect_filename);
     }

    config_close_file(&fd);
//   cJSON_Delete(array);
     yodalite_free(buf);
     cJSON_Delete(root);

    return 0;
}

int bt_autoconnect_get_index(struct bt_autoconnect_config *config, struct bt_autoconnect_device *dev) {
    int i = 0;

    for (i = 0; i < config->autoconnect_num; i++) {
    #if 0
        BT_LOGV("config addr :: %02x:%02x:%02x:%02x:%02x:%02x\n",
               config->dev[i].addr[0], config->dev[i].addr[1],
               config->dev[i].addr[2], config->dev[i].addr[3],
               config->dev[i].addr[4], config->dev[i].addr[5]);

        BT_LOGV("addr :: %02x:%02x:%02x:%02x:%02x:%02x\n",
               dev->addr[0], dev->addr[1],
               dev->addr[2], dev->addr[3],
               dev->addr[4], dev->addr[5]);
    #endif
        if (memcmp(config->dev[i].addr, dev->addr, sizeof(config->dev[i].addr)) == 0) {
            return i;
        }
    }

    return -1;
}

int bt_autoconnect_update(struct bt_autoconnect_config *config, struct bt_autoconnect_device *dev) {
    int i = 0;
    int index = 0;

    index = bt_autoconnect_get_index(config, dev);
    if (index < 0) {
        if (config->autoconnect_num < 1) {
            return -1;
        }

        for (i = config->autoconnect_num - 1; i > 0; i--) {
            memcpy(&config->dev[i], &config->dev[i - 1], sizeof(struct bt_autoconnect_device));
        }

        memcpy(&config->dev[0], dev, sizeof(struct bt_autoconnect_device));

        if (config->autoconnect_linknum < config->autoconnect_num) {
            config->autoconnect_linknum++;
        }
    } else {
        for (i = index; i > 0; i--) {
            memcpy(&config->dev[i], &config->dev[i - 1], sizeof(struct bt_autoconnect_device));
        }

        memcpy(&config->dev[0], dev, sizeof(struct bt_autoconnect_device));
    }

    return 0;
}

int bt_autoconnect_remove(struct bt_autoconnect_config *config, struct bt_autoconnect_device *dev) {
    int i = 0;
    int index = 0;

    index = bt_autoconnect_get_index(config, dev);
    if (index >= 0) {
        for (i = index; i < config->autoconnect_num - 1; i++) {
            memcpy(&config->dev[i], &config->dev[i + 1], sizeof(struct bt_autoconnect_device));
        }
        memset(&config->dev[config->autoconnect_num - 1], 0x0, sizeof(struct bt_autoconnect_device));
        if (config->autoconnect_linknum > 0) {
            config->autoconnect_linknum--;
        }
    }

    return index >= 0 ? 0 : -1;
}

static int bt_config_newinit(struct bt_autoconnect_config *config) {
    config->autoconnect_mode = ROKIDOS_BT_AUTOCONNECT_MODE;
    // config->autoconnect_num = ROKIDOS_BT_AUTOCONNECT_NUM;
    config->autoconnect_linknum = 0;

    config->dev = yodalite_calloc(config->autoconnect_num, sizeof(struct bt_autoconnect_device));
    if (config->dev == NULL) {
        BT_LOGE("yodalite_calloc error\n");
        return -1;
    }

    return 0;
}

static int bt_config_parse(struct bt_autoconnect_config *config, cJSON*root) {
    cJSON *bt_autoconnect_mode = NULL;
    cJSON *bt_autoconnect_devices = NULL;
    cJSON *bt_dev = NULL;
    cJSON *addr = NULL;
    cJSON *name = NULL;
    int device_num = 0;
    int i = 0;
    char *bt_addr = NULL;
    char *bt_name = NULL;
    char zero_addr[6] = {0};

    if((bt_autoconnect_mode = cJSON_GetObjectItemCaseSensitive(root,"autoconnect_mode")) != NULL){
        config->autoconnect_mode = ROKIDOS_BT_AUTOCONNECT_MODE;
    } else {
        config->autoconnect_mode = ROKIDOS_BT_AUTOCONNECT_MODE;
    }

    if (config->autoconnect_num >= ROKIDOS_BT_AUTOCONNECT_DEVICES_MAX_NUM) {
        config->autoconnect_num = ROKIDOS_BT_AUTOCONNECT_DEVICES_MAX_NUM;
    }

    config->dev = yodalite_calloc(config->autoconnect_num, sizeof(struct bt_autoconnect_device));
    if (config->dev == NULL) {
        BT_LOGE("yodalite_calloc error\n");
        return -1;
    }
    if((bt_autoconnect_devices = cJSON_GetObjectItemCaseSensitive(root,"devices")) != NULL){
        device_num=cJSON_GetArraySize(bt_autoconnect_devices);
        BT_LOGI("device num :: %d\n", device_num);
        for (i = 0; i < device_num; i++) {
            bt_dev = cJSON_GetArrayItem(bt_autoconnect_devices,i);
            if (bt_dev != NULL) {
                BT_LOGI("index num :: %d\n", i);
                if((addr = cJSON_GetObjectItemCaseSensitive(bt_dev,"addr")) != NULL){
                   if (cJSON_IsString(addr) && (addr->valuestring != NULL)){
                             bt_addr = (char *)addr->valuestring;
                             BT_LOGI("addr %s\n", bt_addr);
                        }


                    if (bd_strtoba(config->dev[i].addr, bt_addr) != 0) {
                        BT_LOGW("error addr , init to zero");
                        memset(config->dev[i].addr, 0, sizeof(config->dev[i].addr));
                        continue;
                    }

                    if (memcmp(config->dev[i].addr, zero_addr, sizeof(zero_addr)) != 0) {
                        config->autoconnect_linknum++;
                    }

                } else {
                    BT_LOGW("no addr");
                    continue;
                }

                if((name = cJSON_GetObjectItemCaseSensitive(bt_dev,"name")) != NULL){
                   if (cJSON_IsString(name) && (name->valuestring != NULL)){
                       bt_name = (char *)name->valuestring;
                       BT_LOGI("bt_name%s\n",bt_name);
                       strncpy(config->dev[i].name, bt_name, strlen(bt_name));
                    }

                } else {
                    BT_LOGW("no name");
                }
            }
        }
    }

    return 0;
}

int bt_autoconnect_config_init(struct bt_autoconnect_config *config) {
    fd_t fd;
    int ret = 0;
    char buf[1024] = {0};
    cJSON *bt_config = NULL;

    if(config_open_file(config->autoconnect_filename,&fd,FA_READ)){
        BT_LOGW("%s is not exist\n", config->autoconnect_filename);
        goto new_init;
    } else {

          ret = config_read_file(&fd,buf,sizeof(buf)-1);
          config_close_file(&fd);
        if (ret < 0) {
            BT_LOGW("%s is not ok, newinit\n", config->autoconnect_filename);
            goto new_init;
        } else {
            BT_LOGI("%s\n", buf);
            bt_config = cJSON_Parse(buf);
            if (cJSON_IsInvalid(bt_config)) {
                BT_LOGW("%s is not right\n", config->autoconnect_filename);
                cJSON_Delete(bt_config);
                goto new_init;
            } else {
                ret = bt_config_parse(config, bt_config);
            }
            cJSON_Delete(bt_config);
        }
    }
    return ret;

new_init:
    return bt_config_newinit(config);
}


void bt_autconnect_config_uninit(struct bt_autoconnect_config *config) {
    if (config->dev) {
        free(config->dev);
        config->dev = NULL;
    }
}
