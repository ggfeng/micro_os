#ifndef MQTT_LOGIN_H_
#define MQTT_LOGIN_H_

typedef int(*mqtt_hook_t)(char*topic,char *msg);

extern int yodalite_mqtt_login(char *account_id,char *master_id,char *key,char *secret,mqtt_hook_t func);
extern int mqtt_login(void);

#endif

