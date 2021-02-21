ifeq ($(CONFIG_ARM_CM4F),y)
SRC += $(PRJ_DIR)/lib/FreeRTOS/portable/MemMang/heap_4.c
SRC += $(PRJ_DIR)/lib/FreeRTOS/portable/GCC/ARM_CM4F/port.c

CFLAGS  += -I$(PRJ_DIR)/lib/FreeRTOS/portable/GCC/ARM_CM4F
endif

ifeq ($(CONFIG_XR872AT),y)
SRC += $(PRJ_DIR)/../vendor/xr872/src/kernel/FreeRTOS/Source/portable/MemMang/heap_4.c
SRC += $(PRJ_DIR)/../vendor/xr872/src/kernel/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
CFLAGS  += -I$(PRJ_DIR)/../vendor/xr872/include/kernel/FreeRTOS/portable/GCC/ARM_CM4F
endif

