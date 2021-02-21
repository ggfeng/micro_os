#ifndef _ELOOP_H_
#define _ELOOP_H_
#include <string.h>
#include <stdlib.h>
#include <osal/pthread.h>
#include <osal/signal.h>
#include <osal/time.h>

typedef int* eloop_id_t;

typedef void (*task_cb_t)(void *ctx);

typedef struct {
   void *ctx;
   task_cb_t cb;
   timer_t timer;
   unsigned int timeout_ms;
   unsigned int  period_ms;
   pthread_attr_t attr;
}eloop_ctx;

extern int eloop_timer_stop(eloop_id_t timer_id);
extern int eloop_timer_start(eloop_id_t timer_id);
extern void eloop_timer_delete(eloop_id_t timer_id);
extern eloop_id_t eloop_timer_add(task_cb_t cb,void *ctx,unsigned int expires_ms,unsigned int period_ms);

#endif
