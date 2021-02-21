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
 * @file FreeRTOS_POSIX_pthread_cond.c
 * @brief Implementation of condition variable functions in pthread.h
 */

/* C standard library includes. */
#include <yodalite_autoconf.h>
#include <limits.h>

/* FreeRTOS+POSIX includes. */
#include "FreeRTOS_POSIX.h"
#include "errno.h"
#include "pthread.h"
#include "utils.h"

//#if CONFIG_ESP32 == 1
//PRIVILEGED_DATA static portMUX_TYPE condMutex = portMUX_INITIALIZER_UNLOCKED;
//#endif
/**
 * @brief Initialize a PTHREAD_COND_INITIALIZER cond.
 *
 * PTHREAD_COND_INITIALIZER sets a flag for a cond to be initialized later.
 * This function performs the initialization.
 * @param[in] pxCond The cond to initialize.
 *
 * @return nothing
 */
static void prvInitializeStaticCond( pthread_cond_internal_t * pxCond );

/*-----------------------------------------------------------*/

static void prvInitializeStaticCond( pthread_cond_internal_t * pxCond )
{
	int iStatus;
    /* Check if the condition variable needs to be initialized. */
    if( pxCond->xIsInitialized == pdFALSE )
    {
        /* Cond initialization must be in a critical section to prevent two threads
         * from initializing it at the same time. */
//        #if CONFIG_ESP32 == 1
//        taskENTER_CRITICAL(&condMutex);
//        #else
        taskENTER_CRITICAL();
//        #endif
        

        /* Check again that the cond is still uninitialized, i.e. it wasn't
         * initialized while this function was waiting to enter the critical
         * section. */
        if( pxCond->xIsInitialized == pdFALSE )
        {
            /* Set the members of the cond. The semaphore create calls will never fail
             * when their arguments aren't NULL. */
            #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
            pxCond->xIsInitialized = pdTRUE;
            ( void ) xSemaphoreCreateMutexStatic( &pxCond->xCondMutex );
            ( void ) xSemaphoreCreateCountingStatic( INT_MAX, 0U, &pxCond->xCondWaitSemaphore );
            #else

             pxCond->xCondMutex = xSemaphoreCreateMutex();
             if(pxCond->xCondMutex  != NULL){
                if((pxCond->xCondWaitSemaphore = xSemaphoreCreateCounting(INT_MAX, 0U)) == NULL){
			       vSemaphoreDelete(pxCond->xCondMutex);
                   iStatus = EAGAIN;
				}
			 }else{
                iStatus = EAGAIN;
			 }

			 if(iStatus ==0){
                pxCond->xIsInitialized = pdTRUE;
			 }
            #endif
            pxCond->iWaitingThreads = 0;
        }

        /* Exit the critical section. */
//        #if CONFIG_ESP32
//        taskEXIT_CRITICAL(&condMutex);
//        #else
        taskEXIT_CRITICAL();
//        #endif
    }
}

/*-----------------------------------------------------------*/

int freertos_pthread_cond_broadcast( pthread_cond_t * cond )
{
    int i = 0;
    pthread_cond_internal_t * pxCond = ( pthread_cond_internal_t * ) (*cond );

    /* If the cond is uninitialized, perform initialization. */
    prvInitializeStaticCond( pxCond );

    /* Lock xCondMutex to protect access to iWaitingThreads.
     * This call will never fail because it blocks forever. */
    #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
    ( void ) xSemaphoreTake( ( SemaphoreHandle_t ) &pxCond->xCondMutex, portMAX_DELAY );
    #else
    ( void ) xSemaphoreTake(pxCond->xCondMutex, portMAX_DELAY );
    #endif

    /* Unblock all threads waiting on this condition variable. */
    for( i = 0; i < pxCond->iWaitingThreads; i++ )
    {
      #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
        ( void ) xSemaphoreGive( ( SemaphoreHandle_t ) &pxCond->xCondWaitSemaphore );
      #else
        ( void ) xSemaphoreGive(pxCond->xCondWaitSemaphore );
	  #endif
    }

    /* All threads were unblocked, set waiting threads to 0. */
    pxCond->iWaitingThreads = 0;

    /* Release xCondMutex. */
     #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
    ( void ) xSemaphoreGive( ( SemaphoreHandle_t ) &pxCond->xCondMutex );
    #else
    ( void ) xSemaphoreGive(pxCond->xCondMutex);
    #endif

    return 0;
}

/*-----------------------------------------------------------*/

int freertos_pthread_cond_destroy( pthread_cond_t * cond )
{
    pthread_cond_internal_t * pxCond = ( pthread_cond_internal_t * ) (*cond);

    /* Free all resources in use by the cond. */
    #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
    vSemaphoreDelete( ( SemaphoreHandle_t ) &pxCond->xCondMutex );
    vSemaphoreDelete( ( SemaphoreHandle_t ) &pxCond->xCondWaitSemaphore );
    #else
    vSemaphoreDelete(pxCond->xCondMutex );
    vSemaphoreDelete(pxCond->xCondWaitSemaphore);
    #endif

	vPortFree(pxCond);
    return 0;
}

/*-----------------------------------------------------------*/

int freertos_pthread_cond_init( pthread_cond_t * cond,
                       const pthread_condattr_t * attr )
{
    int iStatus = 0;

    pthread_cond_internal_t * pxCond = NULL;

    pxCond  = (pthread_cond_internal_t*) pvPortMalloc(sizeof(pthread_cond_internal_t));

    /* Silence warnings about unused parameters. */
    ( void ) attr;

    if( pxCond == NULL )
    {
        iStatus = ENOMEM;
    }

    if( iStatus == 0 )
    {
        /* Set the members of the cond. The semaphore create calls will never fail
         * when their arguments aren't NULL. */
         #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
             pxCond->xIsInitialized = pdTRUE;
            ( void ) xSemaphoreCreateMutexStatic( &pxCond->xCondMutex );
            ( void ) xSemaphoreCreateCountingStatic( INT_MAX, 0U, &pxCond->xCondWaitSemaphore );
         #else
             pxCond->xCondMutex = xSemaphoreCreateMutex();
             if(pxCond->xCondMutex  != NULL){
                if((pxCond->xCondWaitSemaphore = xSemaphoreCreateCounting(INT_MAX, 0U)) == NULL){
			       vSemaphoreDelete(pxCond->xCondMutex);
				   vPortFree(pxCond);
                   iStatus = EAGAIN;

				}
			 }else{
                iStatus = EAGAIN;
				vPortFree(pxCond);
			 }

			 if(iStatus ==0){
                pxCond->xIsInitialized = pdTRUE;
			    *cond = ( pthread_cond_t)pxCond;
			 }
        #endif

        pxCond->iWaitingThreads = 0;
    }

    return iStatus;
}

/*-----------------------------------------------------------*/

int freertos_pthread_cond_signal( pthread_cond_t * cond )
{
    pthread_cond_internal_t * pxCond = ( pthread_cond_internal_t*) (*cond);

    /* If the cond is uninitialized, perform initialization. */
    prvInitializeStaticCond( pxCond );

    /* Check that at least one thread is waiting for a signal. */
    if( pxCond->iWaitingThreads > 0 )
    {
        /* Lock xCondMutex to protect access to iWaitingThreads.
         * This call will never fail because it blocks forever. */
        #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
        ( void ) xSemaphoreTake( ( SemaphoreHandle_t ) &pxCond->xCondMutex, portMAX_DELAY );
        #esle
        ( void ) xSemaphoreTake(pxCond->xCondMutex, portMAX_DELAY);
        #endif

        /* Check again that at least one thread is waiting for a signal after
         * taking xCondMutex. If so, unblock it. */
        if( pxCond->iWaitingThreads > 0 )
        {
          #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
            ( void ) xSemaphoreGive( ( SemaphoreHandle_t ) &pxCond->xCondWaitSemaphore );
		  #else
            ( void ) xSemaphoreGive(pxCond->xCondWaitSemaphore );
		  #endif

            /* Decrease the number of waiting threads. */
            pxCond->iWaitingThreads--;
        }

        /* Release xCondMutex. */
         #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
        ( void ) xSemaphoreGive(( SemaphoreHandle_t)&pxCond->xCondMutex );
         #else
        ( void ) xSemaphoreGive(pxCond->xCondMutex);
         #endif
    }

    return 0;
}

/*-----------------------------------------------------------*/

int freertos_pthread_cond_timedwait( pthread_cond_t * cond,
                            pthread_mutex_t * mutex,
                            const struct timespec * abstime )
{
    int iStatus = 0;
    pthread_cond_internal_t * pxCond = ( pthread_cond_internal_t * ) (*cond);
    TickType_t xDelay = portMAX_DELAY;

    /* If the cond is uninitialized, perform initialization. */
    prvInitializeStaticCond( pxCond );

    /* Convert abstime to a delay in TickType_t if provided. */
    if( abstime != NULL )
    {
        struct timespec xCurrentTime = { 0 };

        /* Get current time */
        if( clock_gettime( CLOCK_REALTIME, &xCurrentTime ) != 0 )
        {
            iStatus = EINVAL;
        }
        else
        {
            iStatus = UTILS_AbsoluteTimespecToDeltaTicks( abstime, &xCurrentTime, &xDelay );
        }
    }

    /* Increase the counter of threads blocking on condition variable, then
     * unlock mutex. */
    if( iStatus == 0 )
    {
         #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
        ( void ) xSemaphoreTake( ( SemaphoreHandle_t ) &pxCond->xCondMutex, portMAX_DELAY );
		 #else
        ( void ) xSemaphoreTake(pxCond->xCondMutex, portMAX_DELAY );
		 #endif
        pxCond->iWaitingThreads++;
        #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
        ( void ) xSemaphoreGive( ( SemaphoreHandle_t ) &pxCond->xCondMutex );
        #else
        ( void ) xSemaphoreGive(pxCond->xCondMutex);
        #endif

        iStatus = pthread_mutex_unlock( mutex );
    }

    /* Wait on the condition variable. */
    if( iStatus == 0 )
    {
        #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
        if( xSemaphoreTake( ( SemaphoreHandle_t ) &pxCond->xCondWaitSemaphore,xDelay ) == pdPASS )
		#else
        if( xSemaphoreTake(pxCond->xCondWaitSemaphore,xDelay) == pdPASS )
		#endif
        {
            /* When successful, relock mutex. */
            iStatus = pthread_mutex_lock( mutex );
        }
        else
        {
            /* Timeout. Relock mutex and decrement number of waiting threads. */
            iStatus = ETIMEDOUT;
            ( void ) pthread_mutex_lock( mutex );

           #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
            ( void ) xSemaphoreTake( ( SemaphoreHandle_t ) &pxCond->xCondMutex, portMAX_DELAY );
           #else
            ( void ) xSemaphoreTake(pxCond->xCondMutex, portMAX_DELAY );
		   #endif
            pxCond->iWaitingThreads--;
           #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
            ( void ) xSemaphoreGive( ( SemaphoreHandle_t ) &pxCond->xCondMutex );
           #else
            ( void ) xSemaphoreGive(pxCond->xCondMutex);
           #endif
        }
    }

    return iStatus;
}

/*-----------------------------------------------------------*/

int freertos_pthread_cond_wait( pthread_cond_t * cond,
                       pthread_mutex_t * mutex )
{
    return pthread_cond_timedwait( cond, mutex, NULL );
}
