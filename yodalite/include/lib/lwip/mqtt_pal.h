#ifndef _MQTT_PAL_H_
#define _MQTT_PAL_H_

#define MQTT_PAL_HTONS(s) htons(s)
#define MQTT_PAL_NTOHS(s) ntohs(s)
#define MQTT_PAL_TIME() time(NULL)

#define MQTT_PAL_MUTEX_INIT(mtx_ptr) pthread_mutex_init(mtx_ptr, NULL)
#define MQTT_PAL_MUTEX_LOCK(mtx_ptr) pthread_mutex_lock(mtx_ptr)
#define MQTT_PAL_MUTEX_UNLOCK(mtx_ptr) pthread_mutex_unlock(mtx_ptr)


typedef time_t mqtt_pal_time_t;
typedef pthread_mutex_t mqtt_pal_mutex_t;
typedef int mqtt_pal_socket_handle;

extern void close_nb_socket(int fd);
extern int open_nb_socket(const char* addr, const char* port);
extern ssize_t mqtt_pal_sendall(mqtt_pal_socket_handle fd, const void* buf, size_t len, int flags);
extern ssize_t mqtt_pal_recvall(mqtt_pal_socket_handle fd, void* buf, size_t bufsz, int flags);

#endif
