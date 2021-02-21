#ifndef  _PROPERTIES_H_
#define  _PROPERTIES_H_

#ifdef __cplusplus
 extern "C" {
#endif
#include <inttypes.h>

#define PROPERTY_KEY_MAX   48 
#define PROPERTY_VALUE_MAX 96 

extern int property_init(void);
extern int property_deinit(void);
extern int property_print_list(void);
extern int property_get(const char *key, char *value, const char *default_value);
extern int property_set(const char *key, const char *value);
extern int property_remove(const char *key);

#ifdef __cplusplus
 }
#endif

#endif
