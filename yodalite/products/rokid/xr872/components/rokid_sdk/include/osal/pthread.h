/*
 * Amazon FreeRTOS+POSIX V1.0.3
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/**
 * @file pthread.h
 * @brief Threads.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/pthread.h.html
 */

#ifndef _FREERTOS_POSIX_PTHREAD_H_
#define _FREERTOS_POSIX_PTHREAD_H_

#include <yodalite_autoconf.h>
/* FreeRTOS+POSIX includes. POSIX states that this header shall make symbols
 * defined in sched.h and time.h visible. */
#ifdef CONFIG_OSAL_PTHREAD
#include "sys/types.h"
#include "sched.h"
#include "time.h"


/**
 * @defgroup pthread detach state.
 */
/**@{ */
#undef PTHREAD_CREATE_DETACHED
#undef PTHREAD_CREATE_JOINABLE
#define PTHREAD_CREATE_DETACHED    0       /**< Detached. */
#define PTHREAD_CREATE_JOINABLE    1       /**< Joinable (default). */
/**@} */

/**
 * @brief Returned to a single thread after a successful pthread_barrier_wait.
 *
 * POSIX specifies that this value should be distinct from any other value returned
 * by pthread_barrier_wait, so it's defined as negative to distinguish it from the
 * errnos, which are positive.
 */
#undef PTHREAD_BARRIER_SERIAL_THREAD
#define PTHREAD_BARRIER_SERIAL_THREAD    ( -2 )

/**
 * @defgroup Mutex types.
 */
/**@{ */
#ifndef PTHREAD_MUTEX_NORMAL
    #define PTHREAD_MUTEX_NORMAL        0                        /**< Non-robust, deadlock on relock, does not remember owner. */
#endif
#ifndef PTHREAD_MUTEX_ERRORCHECK
    #define PTHREAD_MUTEX_ERRORCHECK    1                        /**< Non-robust, error on relock,  remembers owner. */
#endif
#ifndef PTHREAD_MUTEX_RECURSIVE
    #define PTHREAD_MUTEX_RECURSIVE     2                        /**< Non-robust, recursive relock, remembers owner. */
#endif
#ifndef PTHREAD_MUTEX_DEFAULT
    #define PTHREAD_MUTEX_DEFAULT       PTHREAD_MUTEX_NORMAL     /**< PTHREAD_MUTEX_NORMAL (default). */
#endif
/**@} */

/**
 * @defgroup Compile-time initializers.
 */
/**@{ */

#if posixconfigENABLE_PTHREAD_COND_T == 1

    #undef  PTHREAD_COND_INITIALIZER
    #define PTHREAD_COND_INITIALIZER         FREERTOS_POSIX_COND_INITIALIZER  /**< pthread_cond_t. */
#else
    /**
     * pthread cond initializer place holder for compilation to go through
     * for the ports that don't define PTHREAD_COND_INITIALIZER (for example: esp)
     */
    #define PTHREAD_COND_INITIALIZER  ((pthread_cond_t) 0xFFFFFFFF)
#endif

#if posixconfigENABLE_PTHREAD_MUTEX_T == 1
    #undef PTHREAD_MUTEX_INITIALIZER
    #define PTHREAD_MUTEX_INITIALIZER    FREERTOS_POSIX_MUTEX_INITIALIZER /**< pthread_mutex_t. */
#endif

/**@} */
#undef pthread_detach
#undef pthread_attr_destroy
#undef pthread_attr_getdetachstate
#undef pthread_attr_getschedparam
#undef pthread_attr_getstacksize
#undef pthread_attr_init
#undef pthread_attr_setdetachstate
#undef pthread_attr_setschedparam
#undef pthread_attr_setstacksize
#undef pthread_barrier_destroy
#undef pthread_barrier_init
#undef pthread_barrier_wait
#undef pthread_create
#undef pthread_cond_broadcast
#undef pthread_cond_destroy
#undef pthread_cond_init
#undef pthread_cond_signal
#undef pthread_cond_timedwait
#undef pthread_cond_wait
#undef pthread_equal
#undef pthread_exit
#undef pthread_getschedparam
#undef pthread_join
#undef pthread_mutex_destroy
#undef pthread_mutex_init
#undef pthread_mutex_lock
#undef pthread_mutex_timedlock
#undef pthread_mutex_trylock
#undef pthread_mutex_unlock
#undef pthread_mutexattr_destroy
#undef pthread_mutexattr_gettype
#undef pthread_mutexattr_init
#undef pthread_mutexattr_settype
#undef pthread_self
#undef pthread_setschedparam
/**
 * @brief Destroy the thread attributes object.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_destroy.html
 */
int freertos_pthread_attr_destroy( pthread_attr_t * attr );

/**
 * @brief Get detachstate attribute.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_getdetachstate.html
 */
int freertos_pthread_attr_getdetachstate( const pthread_attr_t * attr,
                                 int * detachstate );

/**
 * @brief Get schedparam attribute.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_getschedparam.html
 */
int freertos_pthread_attr_getschedparam( const pthread_attr_t * attr,
                                struct sched_param * param );

/**
 * @brief Get stacksize attribute.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_getstacksize.html
 */
int freertos_pthread_attr_getstacksize( const pthread_attr_t * attr,
                               size_t * stacksize );

/**
 * @brief Initialize the thread attributes object.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_init.html
 *
 * @note Currently, only stack size, sched_param, and detach state attributes
 * are supported.
 */
int freertos_pthread_attr_init( pthread_attr_t * attr );

/**
 * @brief Set detachstate attribute.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_setdetachstate.html
 */
int freertos_pthread_attr_setdetachstate( pthread_attr_t * attr,
                                 int detachstate );

/**
 * @brief Set schedparam attribute.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_setschedparam.html
 */
int freertos_pthread_attr_setschedparam( pthread_attr_t * attr,
                                const struct sched_param * param );

/**
 * @brief Set stacksize attribute.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_setstacksize.html
 */
int freertos_pthread_attr_setstacksize( pthread_attr_t * attr,
                               size_t stacksize );

/**
 * @brief Destroy a barrier object.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_barrier_destroy.html
 */
int freertos_pthread_barrier_destroy( pthread_barrier_t * barrier );

/**
 * @brief Initialize a barrier object.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_barrier_init.html
 *
 * @note attr is ignored. count may be at most 8 when configUSE_16_BIT_TICKS is 1;
 * it may be at most 24 otherwise.
 */
int freertos_pthread_barrier_init( pthread_barrier_t * barrier,
                          const pthread_barrierattr_t * attr,
                          unsigned count );

/**
 * @brief Synchronize at a barrier.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_barrier_wait.html
 */
int freertos_pthread_barrier_wait( pthread_barrier_t * barrier );

/**
 * @brief Thread creation.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_create.html
 */
int freertos_pthread_create( pthread_t * thread,
                    const pthread_attr_t * attr,
                    void *( *startroutine )( void * ),
                    void * arg );

/**
 * @brief Broadcast a condition.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_cond_broadcast.html
 */
int freertos_pthread_cond_broadcast( pthread_cond_t * cond );

/**
 * @brief Destroy condition variables.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_cond_destroy.html
 */
int freertos_pthread_cond_destroy( pthread_cond_t * cond );

/**
 * @brief Initialize condition variables.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_cond_init.html
 *
 * @note attr is ignored.
 */
int freertos_pthread_cond_init( pthread_cond_t * cond,
                       const pthread_condattr_t * attr );

/**
 * @brief Signal a condition.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_cond_signal.html
 */
int freertos_pthread_cond_signal( pthread_cond_t * cond );

/**
 * @brief Wait on a condition with a timeout.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_cond_timedwait.html
 */
int freertos_pthread_cond_timedwait( pthread_cond_t * cond,
                            pthread_mutex_t * mutex,
                            const struct timespec * abstime );

/**
 * @brief Wait on a condition.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_cond_wait.html
 */
int freertos_pthread_cond_wait( pthread_cond_t * cond,
                       pthread_mutex_t * mutex );

/**
 * @brief Compare thread IDs.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_equal.html
 */
int freertos_pthread_equal( pthread_t t1,
                   pthread_t t2 );

/**
 * @brief Thread termination.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_exit.html
 */
void freertos_pthread_exit( void * value_ptr );

/**
 * @brief Dynamic thread scheduling parameters access.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_getschedparam.html
 *
 * @note policy is always set to SCHED_OTHER by this function.
 */
int freertos_pthread_getschedparam( pthread_t thread,
                           int * policy,
                           struct sched_param * param );

/**
 * @brief Wait for thread termination.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_join.html
 */
int freertos_pthread_join( pthread_t thread,
                  void ** retval );

/**
 * @brief Destroy a mutex.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_destroy.html
 */
int freertos_pthread_mutex_destroy( pthread_mutex_t * mutex );

/**
 * @brief Initialize a mutex.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_init.html
 */
int freertos_pthread_mutex_init( pthread_mutex_t * mutex,
                        const pthread_mutexattr_t * attr );

/**
 * @brief Lock a mutex.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_lock.html
 */
int freertos_pthread_mutex_lock( pthread_mutex_t * mutex );

/**
 * @brief Lock a mutex with timeout.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_timedlock.html
 */
int freertos_pthread_mutex_timedlock( pthread_mutex_t * mutex,
                             const struct timespec * abstime );

/**
 * @brief Attempt to lock a mutex. Fail immediately if mutex is already locked.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_trylock.html
 */
int freertos_pthread_mutex_trylock( pthread_mutex_t * mutex );

/**
 * @brief Unlock a mutex.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_unlock.html
 */
int freertos_pthread_mutex_unlock( pthread_mutex_t * mutex );

/**
 * @brief Destroy the mutex attributes object.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutexattr_destroy.html
 */
int freertos_pthread_mutexattr_destroy( pthread_mutexattr_t * attr );

/**
 * @brief Get the mutex type attribute.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutexattr_gettype.html
 */
int freertos_pthread_mutexattr_gettype( const pthread_mutexattr_t * attr,
                               int * type );

/**
 * @brief Initialize the mutex attributes object.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutexattr_init.html
 *
 * @note Currently, only the type attribute is supported.
 */
int freertos_pthread_mutexattr_init( pthread_mutexattr_t * attr );

/**
 * @brief Set the mutex type attribute.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutexattr_settype.html
 */
int freertos_pthread_mutexattr_settype( pthread_mutexattr_t * attr,
                               int type );

/**
 * @brief Get the calling thread ID.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_self.html
 */
pthread_t freertos_pthread_self( void );

/**
 * @brief Dynamic thread scheduling parameters access.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_setschedparam.html
 *
 * @note policy is ignored; only priority (param.sched_priority) may be changed.
 */
int freertos_pthread_setschedparam( pthread_t thread,
                           int policy,
                           const struct sched_param * param );

int freertos_pthread_detach(pthread_t thread);

#define  pthread_detach               freertos_pthread_detach
#define  pthread_attr_destroy         freertos_pthread_attr_destroy
#define  pthread_attr_getdetachstate  freertos_pthread_attr_getdetachstate
#define  pthread_attr_getschedparam   freertos_pthread_attr_getschedparam
#define  pthread_attr_getstacksize    freertos_pthread_attr_getstacksize 
#define  pthread_attr_init            freertos_pthread_attr_init
#define  pthread_attr_setdetachstate  freertos_pthread_attr_setdetachstate
#define  pthread_attr_setschedparam   freertos_pthread_attr_setschedparam
#define  pthread_attr_setstacksize    freertos_pthread_attr_setstacksize
#define  pthread_barrier_destroy      freertos_pthread_barrier_destroy
#define  pthread_barrier_init         freertos_pthread_barrier_init
#define  pthread_barrier_wait         freertos_pthread_barrier_wait
#define  pthread_create               freertos_pthread_create
#define  pthread_cond_broadcast       freertos_pthread_cond_broadcast
#define  pthread_cond_destroy         freertos_pthread_cond_destroy
#define  pthread_cond_init            freertos_pthread_cond_init
#define  pthread_cond_signal          freertos_pthread_cond_signal
#define  pthread_cond_timedwait       freertos_pthread_cond_timedwait
#define  pthread_cond_wait            freertos_pthread_cond_wait
#define  pthread_equal                freertos_pthread_equal
#define  pthread_exit                 freertos_pthread_exit
#define  pthread_getschedparam        freertos_pthread_getschedparam
#define  pthread_join                 freertos_pthread_join
#define  pthread_mutex_destroy        freertos_pthread_mutex_destroy
#define  pthread_mutex_init           freertos_pthread_mutex_init
#define  pthread_mutex_lock           freertos_pthread_mutex_lock
#define  pthread_mutex_timedlock      freertos_pthread_mutex_timedlock
#define  pthread_mutex_trylock        freertos_pthread_mutex_trylock
#define  pthread_mutex_unlock         freertos_pthread_mutex_unlock
#define  pthread_mutexattr_destroy    freertos_pthread_mutexattr_destroy
#define  pthread_mutexattr_gettype    freertos_pthread_mutexattr_gettype
#define  pthread_mutexattr_init       freertos_pthread_mutexattr_init
#define  pthread_mutexattr_settype    freertos_pthread_mutexattr_settype
#define  pthread_self                 freertos_pthread_self
#define  pthread_setschedparam        freertos_pthread_setschedparam
#else
#include "sched.h"
#include "time.h"

/**
 * @defgroup pthread detach state.
 */
/**@{ */
#undef PTHREAD_CREATE_DETACHED
#undef PTHREAD_CREATE_JOINABLE
#define PTHREAD_CREATE_DETACHED    0       /**< Detached. */
#define PTHREAD_CREATE_JOINABLE    1       /**< Joinable (default). */
/**@} */

/**
 * @brief Returned to a single thread after a successful pthread_barrier_wait.
 *
 * POSIX specifies that this value should be distinct from any other value returned
 * by pthread_barrier_wait, so it's defined as negative to distinguish it from the
 * errnos, which are positive.
 */
#undef PTHREAD_BARRIER_SERIAL_THREAD
#define PTHREAD_BARRIER_SERIAL_THREAD    ( -2 )

#if !defined ( __timespec_defined) && !defined (_SYS__TIMESPEC_H_)
#define __timespec_defined
#define _SYS__TIMESPEC_H_
/* Time Value Specification Structures, P1003.1b-1993, p. 261 */

struct timespec {
  time_t  tv_sec;   /* Seconds */
  long    tv_nsec;  /* Nanoseconds */
};
#endif


/**
 * @defgroup Mutex types.
 */
/**@{ */
#ifndef PTHREAD_MUTEX_NORMAL
    #define PTHREAD_MUTEX_NORMAL        0                        /**< Non-robust, deadlock on relock, does not remember owner. */
#endif
#ifndef PTHREAD_MUTEX_ERRORCHECK
    #define PTHREAD_MUTEX_ERRORCHECK    1                        /**< Non-robust, error on relock,  remembers owner. */
#endif
#ifndef PTHREAD_MUTEX_RECURSIVE
    #define PTHREAD_MUTEX_RECURSIVE     2                        /**< Non-robust, recursive relock, remembers owner. */
#endif
#ifndef PTHREAD_MUTEX_DEFAULT
    #define PTHREAD_MUTEX_DEFAULT       PTHREAD_MUTEX_NORMAL     /**< PTHREAD_MUTEX_NORMAL (default). */
#endif
/**@} */

/**
 * @defgroup Compile-time initializers.
 */
/**@{ */
    #undef PTHREAD_COND_INITIALIZER
#if posixconfigENABLE_PTHREAD_COND_T == 1

    #define PTHREAD_COND_INITIALIZER         FREERTOS_POSIX_COND_INITIALIZER  /**< pthread_cond_t. */
#else
    /**
     * pthread cond initializer place holder for compilation to go through
     * for the ports that don't define PTHREAD_COND_INITIALIZER (for example: esp)
     */
    #define PTHREAD_COND_INITIALIZER  ((pthread_cond_t) 0xFFFFFFFF)
#endif

#ifndef PTHREAD_MUTEX_INITIALIZER 
    #define PTHREAD_MUTEX_INITIALIZER    {0} /**< pthread_mutex_t. */
#endif

#ifndef pthread_barrier_t
#define pthread_barrier_t void*
#endif


/**
 * @brief Used to define a barrier attributes object.
 */

#ifndef pthread_barrierattr_t
#define pthread_barrierattr_t void *
#endif

int  pthread_detach(pthread_t thread);
int  pthread_attr_destroy( pthread_attr_t * attr );
int  pthread_attr_getdetachstate( const pthread_attr_t * attr,int * detachstate );
int  pthread_attr_getschedparam( const pthread_attr_t * attr,struct sched_param * param );
int  pthread_attr_getstacksize( const pthread_attr_t * attr,size_t * stacksize );
int  pthread_attr_init( pthread_attr_t * attr );
int  pthread_attr_setdetachstate( pthread_attr_t * attr,int detachstate );
int  pthread_attr_setschedparam( pthread_attr_t * attr,const struct sched_param * param );
int  pthread_attr_setstacksize( pthread_attr_t * attr,size_t stacksize );
int  pthread_barrier_destroy( pthread_barrier_t * barrier );
int  pthread_barrier_init( pthread_barrier_t * barrier,const pthread_barrierattr_t * attr,unsigned count );
int  pthread_barrier_wait( pthread_barrier_t * barrier );
int  pthread_create( pthread_t * thread,const pthread_attr_t * attr,void *( *startroutine )( void * ),void * arg );
int  pthread_cond_broadcast( pthread_cond_t * cond );
int  pthread_cond_destroy( pthread_cond_t * cond );
int  pthread_cond_init( pthread_cond_t * cond,const pthread_condattr_t * attr );
int  pthread_cond_signal( pthread_cond_t * cond );
int  pthread_cond_timedwait( pthread_cond_t * cond,pthread_mutex_t * mutex,const struct timespec * abstime );
int  pthread_cond_wait( pthread_cond_t * cond,pthread_mutex_t * mutex );
int  pthread_equal( pthread_t t1,pthread_t t2 );
void pthread_exit( void * value_ptr );
int  pthread_detach(pthread_t pthread);
int  pthread_getschedparam( pthread_t thread,int * policy,struct sched_param * param );
int  pthread_join( pthread_t thread,void ** retval );
int  pthread_mutex_destroy( pthread_mutex_t * mutex );
int  pthread_mutex_init( pthread_mutex_t * mutex,const pthread_mutexattr_t * attr );
int  pthread_mutex_lock( pthread_mutex_t * mutex );
int  pthread_mutex_timedlock( pthread_mutex_t * mutex,const struct timespec * abstime );
int  pthread_mutex_trylock( pthread_mutex_t * mutex );
int  pthread_mutex_unlock( pthread_mutex_t * mutex );
int  pthread_mutexattr_destroy( pthread_mutexattr_t * attr );
int  pthread_mutexattr_gettype( const pthread_mutexattr_t * attr,int * type );
int  pthread_mutexattr_init( pthread_mutexattr_t * attr );
int  pthread_mutexattr_settype( pthread_mutexattr_t * attr,int type );
pthread_t pthread_self( void );
int  pthread_setschedparam( pthread_t thread,int policy,const struct sched_param * param );
#endif
#endif
