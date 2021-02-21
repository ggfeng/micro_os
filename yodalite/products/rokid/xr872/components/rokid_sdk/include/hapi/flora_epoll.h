#ifndef _FLORA_EPOLL_H_
#define _FLORA_EPOLL_H_

#include <yodalite_autoconf.h>
#include <inttypes.h>
#include <lib/bm/SH_BM_api.h>
#include <osal/semaphore.h>

typedef union epoll_data {
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

struct flora_epoll_event {
    uint32_t events; /* epoll 事件 */
    epoll_data_t data; /* 用户数据变量 */
    struct flora_epoll_event* next;
    struct flora_epoll_event* prev;
};

typedef void (*flora_epoll_callback_t)(void *arg);

struct flora_epoll {
    struct flora_epoll_event* event_head;
    struct flora_epoll_event* event_tail;
    struct flora_epoll_event* event_pop;
    flora_epoll_callback_t event_avail_cb;
    pthread_cond_t epoll_cond;
    pthread_mutex_t epoll_mutex;
};

typedef struct flora_epoll_event flora_epoll_event_t;
typedef struct flora_epoll flora_epoll_t;

extern flora_epoll_t* flora_epoll_create(int size);
extern void flora_epoll_delete(flora_epoll_t* pEpoll);
extern int flora_epoll_post(flora_epoll_t* epfd, flora_epoll_event_t *event);
extern int flora_epoll_wait(flora_epoll_t* epfd, flora_epoll_event_t *event, int timeout_ms);

/* Valid opcodes */
#define EPOLL_CTL_ADD 1     /*注册新的fd到epfd中*/
#define EPOLL_CTL_DEL 2     /*修改已经注册的fd的监听事件*/
#define EPOLL_CTL_MOD 3     /*从epfd中删除一个fd*/

/*
 * Request the handling of system wakeup events so as to prevent system suspends
 * from happening while those events are being processed.
 *
 * Assuming neither EPOLLET nor EPOLLONESHOT is set, system suspends will not be
 * re-allowed until epoll_wait is called again after consuming the wakeup
 * event(s).
 *
 * Requires CAP_BLOCK_SUSPEND
 */
#define EPOLLWAKEUP (1 << 29)

/* Set the One Shot behaviour for the target file descriptor */
#define EPOLLONESHOT (1 << 30)

/* Set the Edge Triggered behaviour for the target file descriptor */
#define EPOLLET (1 << 31)

#define EPOLL_LOG   printf

#define FLORA_EPOLL_SUCCESS 0
#define FLORA_EPOLL_ENOMEM  -1
#define FLORA_EPOLL_EINVAL  -2
#define FLORA_EPOLL_ETIMEOUT -4

#endif  /*_FLORA_EPOLL_H_*/
