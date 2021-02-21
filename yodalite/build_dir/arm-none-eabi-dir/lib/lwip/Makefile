MODULE := libmqtt.a

ifeq ($(CONFIG_ESP32),y)
CFLAGS += -I$(PRJ_DIR)/include/lib/
CFLAGS += -I$(PRJ_DIR)/../vendor/esp32/esp-adf/esp-idf/components/lwip/lwip/src/include/lwip
CFLAGS += -I$(PRJ_DIR)/../vendor/esp32/esp-adf/esp-idf/components/lwip/include/lwip/lwip 
CFLAGS += -I$(PRJ_DIR)/../vendor/esp32/esp-adf/esp-idf/components/lwip/include/lwip/port 
CFLAGS += -I$(PRJ_DIR)/../vendor/esp32/esp-adf/esp-idf/components/lwip/include/lwip/posix 
CFLAGS += -I$(PRJ_DIR)/../vendor/esp32/esp-adf/esp-idf/components/lwip/port/esp32/include

# for mbedtls
CFLAGS += -I$(PRJ_DIR)/../vendor/esp32/esp-adf/esp-idf/components/mbedtls/mbedtls/include
CFLAGS += -I$(PRJ_DIR)/../vendor/esp32/esp-adf/esp-idf/components/esp-tls
CFLAGS += -I$(PRJ_DIR)/../vendor/esp32/esp-adf/esp-idf/components/lwip/port/esp32/include
CFLAGS += -I$(PRJ_DIR)/../vendor/esp32/esp-adf/esp-idf/components/mbedtls/port/include
endif

CFLAGS += -I$(PRJ_DIR)/include/lib/lwip

SRC := $(wildcard *.c)

include $(COMMON_DIR)/common.mk
