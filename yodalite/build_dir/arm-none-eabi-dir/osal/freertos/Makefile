MODULE := libposix.a

SRC = FreeRTOS_POSIX_utils.c 

ifeq ($(CONFIG_OSAL_PTHREAD),y)
SRC +=FreeRTOS_POSIX_pthread.c
endif


ifeq ($(CONFIG_OSAL_PTHREAD_MUTEX),y)
SRC += FreeRTOS_POSIX_pthread_mutex.c
endif

ifeq ($(CONFIG_OSAL_PTHREAD_BARRIER),y)
SRC += FreeRTOS_POSIX_pthread_barrier.c
endif

ifeq ($(CONFIG_OSAL_PTHREAD_COND),y)
SRC += FreeRTOS_POSIX_pthread_cond.c
endif

ifeq ($(CONFIG_OSAL_CLOCK),y)
SRC += FreeRTOS_POSIX_clock.c
endif

ifeq ($(CONFIG_OSAL_UNISTD),y)
SRC += FreeRTOS_POSIX_unistd.c
endif

ifeq ($(CONFIG_OSAL_MQ),y)
SRC += FreeRTOS_POSIX_mqueue.c
endif

ifeq ($(CONFIG_OSAL_SEMAPHORE),y)
SRC += FreeRTOS_POSIX_semaphore.c
endif

ifeq ($(CONFIG_OSAL_TIMER),y)
SRC += FreeRTOS_POSIX_timer.c
endif

ifeq ($(CONFIG_OSAL_SCHED),y)
SRC += FreeRTOS_POSIX_sched.c
endif

CFLAGS  += -I$(PRJ_DIR)/include/osal
CFLAGS  += -I$(PRJ_DIR)/include/osal/sys
CFLAGS  += -I$(PRJ_DIR)/osal/freertos/include
CFLAGS  += -I$(PRJ_DIR)/osal/freertso/include/portable
vpath %.c  \
  $(PRJ_DIR)/osal/freertos/source

include $(COMMON_DIR)/common.mk

