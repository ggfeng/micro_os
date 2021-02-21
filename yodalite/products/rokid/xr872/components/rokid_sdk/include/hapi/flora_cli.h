#ifndef _FLORA_CLI_H_
#define _FLORA_CLI_H_

#include <yodalite_autoconf.h>
#include <inttypes.h>

#include <hapi/caps.h>
#include <hapi/flora_epoll.h>
#include <osal/time.h>

#define FLORA_DEBUG	1

#define FLORA_CLI_SUCCESS 0
// 认证过程失败(版本号不支持)
#define FLORA_CLI_EAUTH -1
// 参数不合法
#define FLORA_CLI_EINVAL -2
// 连接错误
#define FLORA_CLI_ECONN -3
// 'get'请求超时
#define FLORA_CLI_ETIMEOUT -4
// 'get'请求目标不存在
#define FLORA_CLI_ENEXISTS -5
// 客户端id已被占用
// uri '#' 字符后面的字符串是客户端id
#define FLORA_CLI_EDUPID -6
// 缓冲区大小不足
#define FLORA_CLI_EINSUFF_BUF -7
// 客户端主动关闭
#define FLORA_CLI_ECLOSED -8
// 在回调线程中调用call(阻塞模式)
#define FLORA_CLI_EDEADLOCK -9
// monitor模式下，不可调用subscribe/declare_method/post/call
#define FLORA_CLI_EMONITOR -10

#define FLORA_MSGTYPE_INSTANT 0
#define FLORA_MSGTYPE_PERSIST 1
#define FLORA_NUMBER_OF_MSGTYPE 2

#define FLORA_CLI_EPOLL_SIZE 3

#define FLORA_LOG(...)   \
    do {    \
        struct timespec ts;     \
        clock_gettime(CLOCK_REALTIME, &ts);     \
        printf("%d.%d:", ts.tv_sec, ts.tv_nsec);    \
        printf(__VA_ARGS__);    \
    }while(0)

#define FLORA_ASSERT(...)   \
    do {    \
        struct timespec ts;     \
        clock_gettime(CLOCK_REALTIME, &ts);     \
        printf("%d.%d:", ts.tv_sec, ts.tv_nsec);    \
        printf(__VA_ARGS__);    \
        if(FLORA_DEBUG)	\
            while(1);	\
    }while(0)

#define DEFAULT_ID "flora_default"

typedef struct {
  // FLORA_CLI_SUCCESS: 成功
  // 其它: 此请求服务端自定义错误码
  int32_t ret_code;
  caps_t data;
  // 回应请求的服务端自定义标识
  // 可能为空字串
  char *extra;
} flora_call_result;

typedef void (*flora_cli_disconnected_func_t)(void *arg);
// msgtype: INSTANT | PERSIST
// 'msg': 注意，必须在函数未返回时读取msg，函数返回后读取msg非法
typedef void (*flora_cli_recv_post_func_t)(const char *name, uint32_t msgtype,
                                           caps_t msg, void *arg);
typedef flora_call_result* flora_call_reply_t;

// 'call'请求的接收端
// 在此函数中填充'reply'变量，客户端将把'reply'数据发回给'call'请求发送端
typedef void (*flora_cli_recv_call_func_t)(const char *name, caps_t msg,
                                           void *arg, flora_call_result* reply);

typedef struct {
    void* recv_post;
    void* recv_call;
    void* disconnected;
} flora_cli_callback_arg_t;

typedef struct {
  flora_cli_recv_post_func_t recv_post;
  flora_cli_recv_call_func_t recv_call;
  flora_cli_disconnected_func_t disconnected;
} flora_cli_callback_t;

struct flora_message {
    const char* msg_name;
    void* persist_msg;
    uint32_t persistmsg_size;
    uint32_t msgtype;
    flora_call_result* msg_reply;
    struct flora_message* next;
    struct flora_message* prev;
};

#define FLORA_MUTEX_UNLOCK 0
#define FLORA_MUTEX_LOCK 1
#define FLORA_MUTEX_DEL 2
#define FLORA_MUTEX_RECYCLE 3
struct flora_mutex {
    pthread_mutex_t* mutex;
    int mutex_cnt;
    int flag;
    const char* channel_id;
    struct flora_mutex* next;
    struct flora_mutex* prev;
};
typedef struct flora_mutex flora_mutex_t;
typedef struct {
    flora_mutex_t *head;
    flora_mutex_t *tail;
} flora_mutex_queue_t;

#define FLORA_CHANNEL_IDLE 0
#define FLORA_CHANNEL_SCAN 1
struct flora_cli_channel {
    /*BM_FILLEDLIST flora_filledlist;*/
    struct flora_message *async_msg_head;
    struct flora_message *async_msg_tail;
    struct flora_message *persist_msg_head;
    struct flora_message *persist_msg_tail;
    const char* channel_id;
    int flag;
    flora_mutex_t *channel_mutex;
    pthread_t channel_thread;
    flora_epoll_t *channel_epoll;
    struct flora_cli_channel* next;
    struct flora_cli_channel* prev;
    struct flora_cli_sock* pSock;
    flora_cli_callback_t callback;
    flora_cli_callback_arg_t callback_arg;
    uint32_t msg_buf_size;
};
typedef struct {
    struct flora_cli_channel *head;
    struct flora_cli_channel *tail;
} flora_channel_queue_t;

struct flora_cli_sock {
    /*BM_FREEPOOL flora_freepool;*/
    const char* uri;
    struct flora_cli_sock* next;
    struct flora_cli_channel* pChannel_head;
    struct flora_cli_channel* pChannel_tail;
};
typedef struct flora_cli_channel flora_cli_channel_t;
typedef flora_cli_channel_t* flora_cli_t;
typedef struct flora_cli_sock flora_cli_sock_t;
typedef struct flora_message flora_message_t;

struct flora_thread_arg {
    flora_cli_channel_t* pChannel;
    flora_message_t* pMsg;
    caps_t msg;
    int* pTimeout;
    struct flora_thread_arg* next;
    struct flora_thread_arg* prev;
};
typedef struct flora_thread_arg flora_thread_arg_t;
typedef struct {
    flora_thread_arg_t *head;
    flora_thread_arg_t *tail;
} flora_thread_arg_queue_t;

extern void flora_call_reply_write_code(flora_call_reply_t reply, int32_t code);
extern void flora_call_reply_write_data(flora_call_reply_t reply, caps_t data);
extern void flora_call_reply_end(flora_call_reply_t reply);
extern int32_t flora_cli_subscribe(flora_cli_t handle, const char *name);
extern int32_t flora_cli_unsubscribe(flora_cli_t handle, const char *name);
extern int32_t flora_cli_declare_method(flora_cli_t handle, const char *name);
extern int32_t flora_cli_remove_method(flora_cli_t handle, const char *name);
extern int32_t flora_cli_post(flora_cli_t handle, const char *name, caps_t msg, uint32_t msgtype);
extern int32_t flora_cli_call(flora_cli_t handle, const char *name, caps_t msg, const char *target, flora_call_result *result, uint32_t timeout);
extern int32_t flora_cli_connect(const char *uri, flora_cli_callback_t *cb, flora_cli_callback_arg_t *arg, uint32_t msg_buf_size, flora_cli_t *result);
extern void flora_cli_delete(flora_cli_t handle);
extern void flora_cli_set_channel_id(flora_cli_t handle, const char *channel_id);

#endif /*_FLORA_CLI_H_*/
