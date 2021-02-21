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
 * @file FreeRTOS_POSIX_pthread.c
 * @brief Implementation of thread functions in pthread.h
 */

/* C standard library includes. */
#include <stddef.h>
#include <string.h>
#include <yodalite_autoconf.h>

/* FreeRTOS+POSIX includes. */
#include "FreeRTOS_POSIX.h"
#include "errno.h"
#include "pthread.h"

/**
 * @brief Thread attribute object.
 */
#define pthreadDETACH_STATE_MASK 0x8000
#define pthreadSCHED_PRIORITY_MASK 0x7FFF
#define pthreadDETACH_STATE_SHIFT 15
#define pthreadGET_SCHED_PRIORITY( var ) ( (var)  & (pthreadSCHED_PRIORITY_MASK) )
#define pthreadIS_JOINABLE( var ) ( ( (var) & (pthreadDETACH_STATE_MASK) ) == pthreadDETACH_STATE_MASK )
/**
 * @brief Thread object.
 */
typedef struct pthread_internal
{
    pthread_attr_internal_t xAttr;        /**< Thread attributes. */
    void * ( *pvStartRoutine )( void * ); /**< Application thread function. */
    void * xTaskArg;                      /**< Arguments for application thread function. */
    TaskHandle_t xTaskHandle;             /**< FreeRTOS task handle. */
   #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
    StaticSemaphore_t xJoinBarrier;       /**< Synchronizes the two callers of pthread_join. */
    StaticSemaphore_t xJoinMutex;         /**< Ensures that only one other thread may join this thread. */
   #else
    SemaphoreHandle_t xJoinBarrier; 
    SemaphoreHandle_t xJoinMutex;
   #endif
    void * xReturn;                       /**< Return value of pvStartRoutine. */
} pthread_internal_t;

/**
 * @brief Terminates the calling thread.
 *
 * For joinable threads, this function waits for pthread_join. Otherwise,
 * it deletes the thread and frees up resources used by the thread.
 *
 * @return This function does not return.
 */
static void prvExitThread( void );

/**
 * @brief Wrapper function for the user's thread routine.
 *
 * This function is executed as a FreeRTOS task function.
 * @param[in] pxArg A pointer to a pthread_internal_t.
 *
 * @return nothing
 */
static void prvRunThread( void * pxArg );

/**
 * @brief Default pthread_attr_t.
 */
static const pthread_attr_internal_t xDefaultThreadAttributes =
{
    .usStackSize  = CONFIG_OSAL_PTHREAD_STACK_SIZE,
    .usSchedPriorityDetachState = ( ( uint16_t ) tskIDLE_PRIORITY & pthreadSCHED_PRIORITY_MASK) | ( PTHREAD_CREATE_JOINABLE << pthreadDETACH_STATE_SHIFT ),
};

/*-----------------------------------------------------------*/

static void prvExitThread( void )
{
    pthread_internal_t * pxThread = ( pthread_internal_t * ) pthread_self();

    /* If this thread is joinable, wait for a call to pthread_join. */
    if( pthreadIS_JOINABLE( pxThread->xAttr.usSchedPriorityDetachState ) )
    {
        #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
        ( void ) xSemaphoreGive( ( SemaphoreHandle_t ) &pxThread->xJoinBarrier );   
		#else
        ( void ) xSemaphoreGive(pxThread->xJoinBarrier);   
        #endif

        /* Suspend until the call to pthread_join. The caller of pthread_join
         * will perform cleanup. */
        vTaskSuspend( NULL );
    }
    else
    {
        /* For a detached thread, perform cleanup of thread object. */

        #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
        ( void ) xSemaphoreGive( ( SemaphoreHandle_t ) &pxThread->xJoinBarrier );
        ( void ) xSemaphoreGive( ( SemaphoreHandle_t ) &pxThread->xJoinMutex );
        vSemaphoreDelete( ( SemaphoreHandle_t ) &pxThread->xJoinBarrier );
        vSemaphoreDelete( ( SemaphoreHandle_t ) &pxThread->xJoinMutex );
        #else

        ( void ) xSemaphoreGive(pxThread->xJoinBarrier);
        ( void ) xSemaphoreGive(pxThread->xJoinMutex );
        vSemaphoreDelete(pxThread->xJoinMutex);
        vSemaphoreDelete(pxThread->xJoinBarrier);
       #endif
        vPortFree(pxThread);
        vTaskDelete( NULL );
    }
}

/*-----------------------------------------------------------*/

static void prvRunThread( void * pxArg )
{
    pthread_internal_t * pxThread = ( pthread_internal_t * ) pxArg;

    /* Run the thread routine. */
    pxThread->xReturn = pxThread->pvStartRoutine( ( void * ) pxThread->xTaskArg );

    /* Exit once finished. This function does not return. */
    prvExitThread();
}

/*-----------------------------------------------------------*/

int freertos_pthread_attr_destroy( pthread_attr_t * attr )
{
    int iStatus = 0;

    if( attr == NULL )
    {
        iStatus = EINVAL;
    }

    return iStatus;
}

/*-----------------------------------------------------------*/

int freertos_pthread_attr_getdetachstate( const pthread_attr_t * attr,
                                 int * detachstate )
{
    pthread_attr_internal_t * pxAttr = ( pthread_attr_internal_t * ) ( attr );

   if ( pthreadIS_JOINABLE( pxAttr->usSchedPriorityDetachState) )
   {
       *detachstate = PTHREAD_CREATE_JOINABLE;
   }
   else
   {
       *detachstate = PTHREAD_CREATE_DETACHED;
   }

    return 0;
}

/*-----------------------------------------------------------*/

int freertos_pthread_attr_getschedparam( const pthread_attr_t * attr,
                                struct sched_param * param )
{
    pthread_attr_internal_t * pxAttr = ( pthread_attr_internal_t * ) ( attr );

    param->sched_priority = ( int ) ( pthreadGET_SCHED_PRIORITY( pxAttr->usSchedPriorityDetachState) );

    return 0;
}

/*-----------------------------------------------------------*/

int freertos_pthread_attr_getstacksize( const pthread_attr_t * attr,
                               size_t * stacksize )
{
    pthread_attr_internal_t * pxAttr = ( pthread_attr_internal_t * ) ( attr );

    *stacksize = ( size_t ) pxAttr->usStackSize;

    return 0;
}

/*-----------------------------------------------------------*/

int freertos_pthread_attr_init( pthread_attr_t * attr )
{
    int iStatus = 0;

    /* Check if the attribute is NULL. */
    if( attr == NULL )
    {
        iStatus = EINVAL;
    }

    /* Copy the default values into the new thread attributes object. */
    if( iStatus == 0 )
    {
        *( ( pthread_attr_internal_t * ) ( attr ) ) = xDefaultThreadAttributes;
    }

    return iStatus;
}

/*-----------------------------------------------------------*/

int freertos_pthread_attr_setdetachstate( pthread_attr_t * attr,
                                 int detachstate )
{
    int iStatus = 0;
    pthread_attr_internal_t * pxAttr = ( pthread_attr_internal_t * ) ( attr );

    if( ( detachstate != PTHREAD_CREATE_DETACHED ) && ( detachstate != PTHREAD_CREATE_JOINABLE ) )
    {
        iStatus = EINVAL;
    }
    else
    {
        /* clear and then set msb bit to detachstate) */
        pxAttr->usSchedPriorityDetachState &= ~pthreadDETACH_STATE_MASK;
        pxAttr->usSchedPriorityDetachState |= ( (uint16_t) detachstate << pthreadDETACH_STATE_SHIFT );
    }

    return iStatus;
}

/*-----------------------------------------------------------*/

int freertos_pthread_attr_setschedparam( pthread_attr_t * attr,
                                const struct sched_param * param )
{
    int iStatus = 0;
    pthread_attr_internal_t * pxAttr = ( pthread_attr_internal_t * ) ( attr );

    /* Check for NULL param. */
    if( param == NULL )
    {
        iStatus = EINVAL;
    }

    /* Ensure that param.sched_priority is valid. */
    if( ( iStatus == 0 ) &&
        ( ( param->sched_priority > sched_get_priority_max( SCHED_OTHER ) ) ||
          ( param->sched_priority < 0 ) ) )
    {
        iStatus = ENOTSUP;
    }

    /* Set the sched_param. */
    if( iStatus == 0 )
    {
        /* clear and then set  15 LSB to schedule priority) */
        pxAttr->usSchedPriorityDetachState &= ~pthreadSCHED_PRIORITY_MASK;
        pxAttr->usSchedPriorityDetachState |= ( ( uint16_t) param->sched_priority );
    }

    return iStatus;
}

/*-----------------------------------------------------------*/

int freertos_pthread_attr_setstacksize( pthread_attr_t * attr,
                               size_t stacksize )
{
    int iStatus = 0;
    pthread_attr_internal_t * pxAttr = ( pthread_attr_internal_t * ) ( attr );

    if( stacksize < PTHREAD_STACK_MIN )
    {
        iStatus = EINVAL;
    }
    else
    {
        pxAttr->usStackSize = ( uint16_t ) stacksize;
    }

    return iStatus;
}

/*-----------------------------------------------------------*/

int freertos_pthread_create( pthread_t * thread,
                    const pthread_attr_t * attr,
                    void *( *startroutine )( void * ),
                    void * arg )
{
    int iStatus = 0;
    pthread_internal_t * pxThread = NULL;
    struct sched_param xSchedParam  = { .sched_priority = tskIDLE_PRIORITY };

#if CONFIG_OSAL_PTHREAD_WITH_DIFF_NAME == 1
	static char pthread_cnts = 0;
	char name[16] = {0};
#endif

    /* Allocate memory for new thread object. */
    pxThread = ( pthread_internal_t * ) pvPortMalloc( sizeof( pthread_internal_t ) );

    if( pxThread == NULL )
    {
        /* No memory. */
        iStatus = EAGAIN;
    }

    if( iStatus == 0 )
    {
        /* No attributes given, use default attributes. */
        if( attr == NULL )
        {
            pxThread->xAttr = xDefaultThreadAttributes;
        }
        /* Otherwise, use provided attributes. */
        else
        {
            pxThread->xAttr = *( ( pthread_attr_internal_t * ) ( attr ) );
        }

        /* Get priority from attributes */
        xSchedParam.sched_priority =  ( int ) pthreadGET_SCHED_PRIORITY( pxThread->xAttr.usSchedPriorityDetachState );

        /* Set argument and start routine. */
        pxThread->xTaskArg = arg;
        pxThread->pvStartRoutine = startroutine;

        /* If this thread is joinable, create the synchronization mechanisms for
         * pthread_join. */

        if( pthreadIS_JOINABLE( pxThread->xAttr.usSchedPriorityDetachState ) )
        {
            /* These calls will not fail when their arguments aren't NULL. */
        #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
            ( void ) xsemaphorecreatemutexstatic( &pxthread->xjoinmutex );
            ( void ) xsemaphorecreatebinarystatic( &pxthread->xjoinbarrier );
        #else
            if((pxThread->xJoinMutex = xSemaphoreCreateMutex()) != NULL){
			
               if((pxThread->xJoinBarrier = xSemaphoreCreateBinary()) == NULL){
				 vSemaphoreDelete(pxThread->xJoinMutex);
                 iStatus = EAGAIN;
		         vPortFree(pxThread);	
			   }
			}else{
               iStatus = EAGAIN;
		       vPortFree(pxThread);	
			}
        #endif
        }
    }

    if( iStatus == 0 )
    {
        /* Suspend all tasks to create a critical section. This ensures that
         * the new thread doesn't exit before a tag is assigned. */
        vTaskSuspendAll();

      #if CONFIG_OSAL_PTHREAD_WITH_DIFF_NAME == 1
        pthread_cnts ++;
		snprintf(name,16,"pthread%d",pthread_cnts);
	  #endif

        /* Create the FreeRTOS task that will run the pthread. */
        if( xTaskCreate( prvRunThread,
      #if CONFIG_OSAL_PTHREAD_WITH_DIFF_NAME == 1
                         name,
	  #else
                         posixconfigPTHREAD_TASK_NAME,
	  #endif
                         ( uint16_t ) ( pxThread->xAttr.usStackSize / sizeof( StackType_t ) ),
                         ( void * ) pxThread,
                         xSchedParam.sched_priority,
                         &pxThread->xTaskHandle ) != pdPASS )
        {
            /* Task creation failed, no memory. */

           #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
           ( void ) xSemaphoreGive( ( SemaphoreHandle_t ) &pxThread->xJoinBarrier );
           ( void ) xSemaphoreGive( ( SemaphoreHandle_t ) &pxThread->xJoinMutex );
           vSemaphoreDelete( ( SemaphoreHandle_t ) &pxThread->xJoinBarrier );
           vSemaphoreDelete( ( SemaphoreHandle_t ) &pxThread->xJoinMutex );
           #else
           ( void ) xSemaphoreGive(pxThread->xJoinBarrier);
           ( void ) xSemaphoreGive(pxThread->xJoinMutex );
           vSemaphoreDelete(pxThread->xJoinMutex);
           vSemaphoreDelete(pxThread->xJoinBarrier);
          #endif
           vPortFree(pxThread);
           iStatus = EAGAIN;
        }
        else
        {
            /* Store the pointer to the thread object in the task tag. */
            vTaskSetApplicationTaskTag( pxThread->xTaskHandle, ( TaskHookFunction_t ) pxThread );

            /* Set the thread object for the user. */
            *thread = ( pthread_t ) pxThread;

			printf("=================================================================\n");
           #if CONFIG_OSAL_PTHREAD_WITH_DIFF_NAME == 1
	        printf("   create task(%s) id(0x%x) stack(%d) priory(%d)   \n",name,pxThread,pxThread->xAttr.usStackSize,xSchedParam.sched_priority);
          #else
	        printf("   create task(%s) id(0x%x) stack(%d) priory(%d)   \n",posixconfigPTHREAD_TASK_NAME,pxThread,pxThread->xAttr.usStackSize,xSchedParam.sched_priority);
          #endif
			printf("=================================================================\n");
        }

        /* End the critical section. */
        xTaskResumeAll();
    }

    return iStatus;
}

/*-----------------------------------------------------------*/

int freertos_pthread_getschedparam( pthread_t thread,
                           int * policy,
                           struct sched_param * param )
{
    int iStatus = 0;
    pthread_internal_t * pxThread = ( pthread_internal_t * ) thread;

    *policy = SCHED_OTHER;
    param->sched_priority = ( int ) pthreadGET_SCHED_PRIORITY( pxThread->xAttr.usSchedPriorityDetachState );

    return iStatus;
}

/*-----------------------------------------------------------*/

int freertos_pthread_equal( pthread_t t1,
                   pthread_t t2 )
{
    int iStatus = 0;

    /* Compare the thread IDs. */
    if( ( t1 != NULL ) && ( t2 != NULL ) )
    {
        iStatus = ( t1 == t2 );
    }

    return iStatus;
}

/*-----------------------------------------------------------*/

void freertos_pthread_exit( void * value_ptr )
{
    pthread_internal_t * pxThread = ( pthread_internal_t * ) pthread_self();

    /* Set the return value. */
    pxThread->xReturn = value_ptr;

    /* Exit this thread. */
    prvExitThread();
}


int freertos_pthread_detach(pthread_t pthread)
{
    int iStatus = 0;
    pthread_internal_t * pxThread = ( pthread_internal_t * )pthread;

    #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
    if(xSemaphoreTake((SemaphoreHandle_t ) &pxThread->xJoinMutex, 0 ) != pdPASS )
	#else
    if(xSemaphoreTake(pxThread->xJoinMutex, 0) != pdPASS )
	#endif
    {
            /* Another thread has already joined the requested thread, which would
             * cause this thread to wait forever. */
            iStatus = EDEADLK;
    }

    /* Attempting to join the calling thread would cause a deadlock. */
    if( iStatus == 0 )
    {
//        if( pthread_equal(pthread_self(), pthread ) != 0 )
//        {
//            iStatus = EDEADLK;
//        }

        /* clear and then set msb bit to detachstate) */
        pxThread->xAttr.usSchedPriorityDetachState &= ~pthreadDETACH_STATE_MASK;
        pxThread->xAttr.usSchedPriorityDetachState |= ( (uint16_t) PTHREAD_CREATE_DETACHED<< pthreadDETACH_STATE_SHIFT );
    }
   
    #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
    ( void ) xSemaphoreGive( ( SemaphoreHandle_t ) &pxThread->xJoinMutex );
    #else
    ( void ) xSemaphoreGive(pxThread->xJoinMutex);
    #endif

    return iStatus;
}
/*-----------------------------------------------------------*/

int freertos_pthread_join( pthread_t pthread,
                  void ** retval )
{
    int iStatus = 0;
    pthread_internal_t * pxThread = ( pthread_internal_t * ) pthread;

    /* Make sure pthread is joinable. Otherwise, this function would block
     * forever waiting for an unjoinable thread. */
    if ( !pthreadIS_JOINABLE( pxThread->xAttr.usSchedPriorityDetachState ) )
    {
        iStatus = EDEADLK;
    }

    /* Only one thread may attempt to join another. Lock the join mutex
     * to prevent other threads from calling pthread_join on the same thread. */
    if( iStatus == 0 )
    {
        #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
        if( xSemaphoreTake( ( SemaphoreHandle_t ) &pxThread->xJoinMutex, 0 ) != pdPASS )
        #else
        if(xSemaphoreTake(pxThread->xJoinMutex, 0) != pdPASS)
		#endif
        {
            /* Another thread has already joined the requested thread, which would
             * cause this thread to wait forever. */
            iStatus = EDEADLK;
        }
    }

    /* Attempting to join the calling thread would cause a deadlock. */
    if( iStatus == 0 )
    {
        //if( pthread_equal( pthread_self(), pthread ) != 0 )
        if( pthread_equal( pthread_self(), pthread ) == 0 )
        {
            iStatus = EDEADLK;
        }
    }

    if( iStatus == 0 )
    {
        /* Wait for the joining thread to finish. Because this call waits forever,
         * it should never fail. */

        #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
        ( void ) xSemaphoreTake( ( SemaphoreHandle_t ) &pxThread->xJoinBarrier, portMAX_DELAY );
        #else
        ( void ) xSemaphoreTake(pxThread->xJoinBarrier, portMAX_DELAY );
       #endif

        /* Create a critical section to clean up the joined thread. */
        vTaskSuspendAll();

        #if (CONFIG_SUPPORT_STATIC_ALLOCATION == 1)
        /* Release xJoinBarrier and delete it. */
        ( void ) xSemaphoreGive( ( SemaphoreHandle_t ) &pxThread->xJoinBarrier );
        vSemaphoreDelete( ( SemaphoreHandle_t ) &pxThread->xJoinBarrier );

        /* Release xJoinMutex and delete it. */
        ( void ) xSemaphoreGive( ( SemaphoreHandle_t ) &pxThread->xJoinMutex );
        vSemaphoreDelete( ( SemaphoreHandle_t ) &pxThread->xJoinMutex );
       #else
        ( void ) xSemaphoreGive(pxThread->xJoinBarrier);
        vSemaphoreDelete(pxThread->xJoinBarrier );
        /* Release xJoinMutex and delete it. */
        ( void ) xSemaphoreGive(pxThread->xJoinMutex );
        vSemaphoreDelete(pxThread->xJoinMutex);

       #endif

        /* Delete the FreeRTOS task that ran the thread. */
        vTaskDelete( pxThread->xTaskHandle );

        /* Set the return value. */
        if( retval != NULL )
        {
            *retval = pxThread->xReturn;
        }

        /* Free the thread object. */
        vPortFree( pxThread );

        /* End the critical section. */
        xTaskResumeAll();
    }

    return iStatus;
}

/*-----------------------------------------------------------*/

pthread_t freertos_pthread_self( void )
{
    /* Return a reference to this pthread object, which is stored in the
     * FreeRTOS task tag. */
    return ( pthread_t ) xTaskGetApplicationTaskTag( NULL );
}

/*-----------------------------------------------------------*/

int freertos_pthread_setschedparam( pthread_t thread,
                           int policy,
                           const struct sched_param * param )
{
    int iStatus = 0;

    pthread_internal_t * pxThread = ( pthread_internal_t * ) thread;

    /* Silence compiler warnings about unused parameters. */
    ( void ) policy;

    /* Copy the given sched_param. */
    iStatus = pthread_attr_setschedparam( ( pthread_attr_t * ) &pxThread->xAttr, param );

    if ( iStatus == 0 )
    {
        /* Change the priority of the FreeRTOS task. */
        vTaskPrioritySet( pxThread->xTaskHandle, param->sched_priority );
    }

    return iStatus;
}

/*-----------------------------------------------------------*/
