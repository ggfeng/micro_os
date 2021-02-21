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
 * @file FreeRTOS_POSIX_semaphore.c
 * @brief Implementation of functions in semaphore.h
 */

/* C standard library includes. */
#include <stddef.h>
#include <yodalite_autoconf.h>
/* FreeRTOS+POSIX includes. */
#include "FreeRTOS_POSIX.h"
#include "errno.h"
#include "semaphore.h"
#include "utils.h"


/*-----------------------------------------------------------*/
int freertos_sem_destroy( sem_t * sem )
{
    sem_internal_t * pxSem = ( sem_internal_t * ) (*sem);

    /* Free the resources in use by the semaphore. */
    #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
    vSemaphoreDelete( ( SemaphoreHandle_t ) &pxSem->xSemaphore );
   #else
    vSemaphoreDelete(pxSem->xSemaphore );
   #endif
	
   vPortFree(pxSem);

    return 0;
}

/*-----------------------------------------------------------*/

int freertos_sem_getvalue( sem_t * sem,
                  int * sval )
{
    #ifndef CONFIG_RTOS_VERSION_LOWER_THAN_8_2_3
    sem_internal_t * pxSem = ( sem_internal_t * ) ( sem );
    /* Get the semaphore count using the FreeRTOS API. */
    *sval = ( int ) uxSemaphoreGetCount( ( SemaphoreHandle_t ) &pxSem->xSemaphore );
    #endif

    return 0;
}

/*-----------------------------------------------------------*/

int freertos_sem_init( sem_t * sem,
              int pshared,
              unsigned value )
{
    int iStatus = 0;
    //sem_internal_t * pxSem =  ( sem_internal_t * ) ( sem );
    sem_internal_t * pxSem=NULL;

    /* Silence warnings about unused parameters. */
    ( void ) pshared;

    pxSem = (sem_internal_t *) pvPortMalloc( sizeof(sem_internal_t));

    /* Check value parameter. */
    if( value > SEM_VALUE_MAX )
    {
        errno = EINVAL;
        iStatus = -1;
    }


    /* Create the FreeRTOS semaphore. This call will not fail because the
     * memory for the semaphore has already been allocated. */
    if( iStatus == 0 )
    {

        #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
             ( void ) xSemaphoreCreateCountingStatic( SEM_VALUE_MAX, value, &pxSem->xSemaphore );
		#else
         pxSem->xSemaphore =  xSemaphoreCreateCounting (SEM_VALUE_MAX, value);

		 if(pxSem->xSemaphore == NULL){
		   vPortFree(pxSem); 
           errno = EINVAL;
           iStatus = -1;
		 } 

		 if(iStatus == 0){
		   *sem = (sem_t)pxSem; 
		 }
        #endif
    }

    return iStatus;
}

/*-----------------------------------------------------------*/

int freertos_sem_post( sem_t * sem )
{
    sem_internal_t * pxSem = ( sem_internal_t * ) (*sem);

    /* Give the semaphore using the FreeRTOS API. */
    #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
      ( void ) xSemaphoreGive( ( SemaphoreHandle_t ) &pxSem->xSemaphore );
   #else
      xSemaphoreGive(pxSem->xSemaphore );
   #endif

    return 0;
}

/*-----------------------------------------------------------*/

int freertos_sem_timedwait( sem_t * sem,
                   const struct timespec * abstime )
{
    int iStatus = 0;
    sem_internal_t * pxSem = ( sem_internal_t * ) (*sem);
    TickType_t xDelay = portMAX_DELAY;

    if( abstime != NULL )
    {
        /* If the provided timespec is invalid, still attempt to take the
         * semaphore without blocking, per POSIX spec. */
        if( UTILS_ValidateTimespec( abstime ) == false )
        {
            xDelay = 0;
            iStatus = EINVAL;
        }
        else
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

            /* If abstime was in the past, still attempt to take the semaphore without
             * blocking, per POSIX spec. */
            if( iStatus == ETIMEDOUT )
            {
                xDelay = 0;
            }
        }
    }
    /* Take the semaphore using the FreeRTOS API. */
    #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
    if( xSemaphoreTake( ( SemaphoreHandle_t ) &pxSem->xSemaphore,xDelay ) != pdTRUE )
	#else
    if( xSemaphoreTake(pxSem->xSemaphore,xDelay ) != pdTRUE )
	#endif
    {
        if( iStatus == 0 )
        {
            errno = ETIMEDOUT;
        }
        else
        {
            errno = iStatus;
        }

        iStatus = -1;
    }
    else
    {
        iStatus = 0;
    }

    return iStatus;
}

/*-----------------------------------------------------------*/

int freertos_sem_trywait( sem_t * sem )
{
    int iStatus = 0;

    /* Setting an absolute timeout of 0 (i.e. in the past) will cause sem_timedwait
     * to not block. */
    struct timespec xTimeout = { 0 };

    iStatus = sem_timedwait( sem, &xTimeout );

    /* POSIX specifies that this function should set errno to EAGAIN and not
     * ETIMEDOUT. */
    if( ( iStatus == -1 ) && ( errno == ETIMEDOUT ) )
    {
        errno = EAGAIN;
    }

    return iStatus;
}

/*-----------------------------------------------------------*/

int freertos_sem_wait( sem_t * sem )
{
    return sem_timedwait( sem, NULL );
}

/*-----------------------------------------------------------*/
