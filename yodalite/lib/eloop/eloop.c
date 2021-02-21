#include <lib/eloop/eloop.h>

#include <yodalite_autoconf.h>

#ifdef CONFIG_MEM_EXT_ENABLE
#include <lib/mem_ext/mem_ext.h>
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

static void timer_timeout_hander(union sigval sv)
{
   eloop_ctx * eloop = sv.sival_ptr;

//   pthread_detach(pthread_self()); 

   if(eloop->cb)
      eloop->cb(eloop->ctx);
}

eloop_id_t eloop_timer_add(task_cb_t cb,void *ctx,unsigned int expires_ms,unsigned int period_ms)
{
  eloop_id_t ptr = NULL;
  struct sigevent se,*pSe = &se;

  memset(pSe,0,sizeof(struct sigevent));

  eloop_ctx*eloop = yodalite_malloc(sizeof(*eloop));

   if (!eloop)
        return NULL;

   eloop->cb = cb;
   eloop->ctx = ctx;
   eloop->timeout_ms = expires_ms;
   eloop->period_ms = period_ms;
 
//if (pthread_attr_init(&eloop->attr))
 //    return NULL;

// if(pthread_attr_setdetachstate(&eloop->attr,PTHREAD_CREATE_DETACHED))
  //  return NULL;

  pSe->sigev_notify = SIGEV_THREAD;
  pSe->sigev_notify_function = timer_timeout_hander;
//  pSe->sigev_notify_attributes = &eloop->attr;
  pSe->sigev_value.sival_ptr = eloop;

  if(timer_create(CLOCK_MONOTONIC,pSe, &eloop->timer)==0)
  {
     ptr = (eloop_id_t)eloop; 
  }

  return ptr;
}

void eloop_timer_delete(eloop_id_t timer_id)
{
    eloop_ctx*eloop = (eloop_ctx*)timer_id;

    timer_delete(eloop->timer);
 //   pthread_attr_destroy(&eloop->attr);
    yodalite_free(eloop);
}


int eloop_timer_start(eloop_id_t timer_id)
{
    eloop_ctx *eloop = (eloop_ctx *)timer_id;

    struct itimerspec its = {
        .it_value = {
            .tv_sec = eloop->timeout_ms / 1e3,
            .tv_nsec = (eloop->timeout_ms % 1000) * 1e6,
        },
        .it_interval = {
            .tv_sec = eloop->period_ms / 1e3,
            .tv_nsec = (eloop->period_ms % 1000) * 1e6,
        },
    };

    return timer_settime(eloop->timer, 0, &its, NULL);
}   
    
int eloop_timer_stop(eloop_id_t timer_id)
{
    eloop_ctx *eloop = (eloop_ctx*)timer_id;
    struct itimerspec its = {0};

    return timer_settime(eloop->timer, 0, &its, NULL);
}


