MODULE := libfreertos.a

SRC =	list.c              \
	queue.c             \
	tasks.c             \
	timers.c            \
	croutine.c          \
	event_groups.c      \
	stream_buffer.c

include $(COMMON_DIR)/freertos.mk
CFLAGS  += -I$(PRJ_DIR)/lib/FreeRTOS/include

include $(COMMON_DIR)/common.mk
