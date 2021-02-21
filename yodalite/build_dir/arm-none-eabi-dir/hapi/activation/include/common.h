#ifndef __ACTIVATION_COMMON_H_
#define __ACTIVATION_COMMON_H_

#include <yodalite_autoconf.h>
#include <lib/property/properties.h>

#ifdef CONFIG_MEM_EXT_ENABLE
#include <lib/mem_ext/mem_ext.h>
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif


#define NETWORK_CONNECTED 1
#define NETWORK_DISCONNECTED 0
#define NETWORK_NONE 2
#define WIFI_AP_MODE 1

#define WIFI_NONE_MODE 1
#define WIFI_STA_MODE 2
#define WIFI_BLE_MODE 3

#define KEY_LENTH 64
#define INT_LENTH 32

struct properties_t {
    char deviceid[PROPERTY_VALUE_MAX];
    char deviceTypeId[PROPERTY_VALUE_MAX];
    char rokidseed[PROPERTY_VALUE_MAX];
    char key[KEY_LENTH];
    char secret[KEY_LENTH];
    char mac[INT_LENTH];
    char ip[INT_LENTH];
};

extern struct properties_t properties;

#endif /* __ACTIVATION_COMMON_H_ */
