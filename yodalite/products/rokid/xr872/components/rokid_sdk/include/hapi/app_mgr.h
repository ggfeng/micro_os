#ifndef _APP_MGR_H_
#define _APP_MGR_H_

#include <yodalite_autoconf.h>
#include <inttypes.h>
#include <osal/pthread.h>
#include <hapi/flora_agent.h>

#define MAX_TASK_STACK_DEPTH    (1)

#define APPMGR_LOG(...)  \
    do {    \
        struct timespec ts;     \
        clock_gettime(CLOCK_REALTIME, &ts);     \
        printf("%d.%d:", ts.tv_sec, ts.tv_nsec);    \
        printf(__VA_ARGS__);	\
    }while(0)

#define APPMGR_SOCK    "unix:/var/run/flora.sock"
#define APPMGR_PERSIST_MSG_SIZE    64
#define APP_CUT    "cut"
#define APP_SCENE    "scene"
#define APP_UNINTERRUPTABLE "uninterruptable"

typedef enum {
    IDLE = 0,
    RUNNING,
    SUSPEND,
    RESUME,
} app_status_t;

typedef void* (*app_func_t)(void* param);
typedef void (*app_intent_func_t)(uint32_t msgtype, caps_t msg, void *arg);
typedef void (*app_syscall_func_t)(caps_t msg, void *arg, flora_call_result* reply);

typedef struct {
    const char* task_name;
    app_intent_func_t task_func;
} app_intent_t;

typedef struct {
    const char* task_name;
    app_syscall_func_t task_func;
} app_syscall_t;

struct application {
    const char* name;
    const char* appId;
    const char* form;
    flora_agent_t app_itc;
    app_func_t onPrepare;
    app_func_t onStart;
    app_func_t onStop;
    flora_cli_recv_post_func_t intent_callback;
    void* intent_argument;
    app_intent_t* intentDeal;
    int intent_cnt;
    flora_cli_recv_call_func_t syscall_callback;
    void* syscall_argument;
    app_syscall_t* syscallDeal;
    int syscall_cnt;
    struct application* next;
    struct application* prev;
};
typedef struct application app_t;

struct app_index{
    const char* app_name;
    struct app_index* next;
    struct app_index* prev;
};
typedef struct app_index app_index_t;

typedef struct {
    app_t* head;
    app_t* tail;
} app_queue_t;

typedef struct {
    app_index_t* top;
    app_index_t* bot;
    int depth;
} app_stack_t;

#define app_create(_name, _id, _form, prepare_func, start_func, stop_func, intent_arg, intent_array, syscall_arg, syscall_array)    \
    void intent_cb##_name(const char *name, uint32_t msgtype, caps_t msg, void *arg)    \
    {   \
        int intent_cnt = sizeof(intent_array)/sizeof(app_intent_t);   \
        int i;  \
        for(i = 0; i<intent_cnt; i++) {   \
            if(!strcmp(intent_array[i].task_name, name)) {  \
                intent_array[i].task_func(msgtype, msg, arg);    \
                break;  \
            }   \
        }   \
    }   \
    void syscall_cb##_name(const char *name, caps_t msg, void *arg, flora_call_result* reply)   \
    {   \
        int syscall_cnt = sizeof(syscall_array)/sizeof(app_syscall_t);     \
        int i;  \
        for(i = 0; i<syscall_cnt; i++) {   \
            if(!strcmp(syscall_array[i].task_name, name)) {  \
                syscall_array[i].task_func(msg, arg, reply);    \
                break;  \
            }   \
        }   \
    }   \
    app_t app_##_name = {        \
        .name = #_name,        \
        .appId = #_id,            \
        .form = #_form,          \
        .app_itc = NULL,           \
        .onPrepare = prepare_func,      \
        .onStart = start_func,          \
        .onStop = stop_func,      \
        .intent_callback = intent_cb##_name,                            \
        .intent_argument = intent_arg,                           \
        .intentDeal = intent_array,                              \
        .intent_cnt = sizeof(intent_array)/sizeof(app_intent_t),   \
        .syscall_callback = syscall_cb##_name,                          \
        .syscall_argument = syscall_arg,                         \
        .syscallDeal = syscall_array,                            \
        .syscall_cnt = sizeof(syscall_array)/sizeof(app_syscall_t), \
        .next = NULL,                 \
        .prev = NULL,                 \
    }

extern int app_register(app_t *pApp);
extern app_t* app_pickup(const char* app_name);
extern app_t* app_select(const char* app_id);
extern int app_start(app_t *pApp);
extern int app_stop(void);

#endif  /*_APP_MGR_H_*/
