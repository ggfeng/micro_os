MODULE := libactivation.a

$(warning '==================cgl========')

ifeq ($(CONFIG_ANDLINK_ENABLE),y)
SRC := activation.c cloudapp.c cloudgw.c dispatcher.c http_parser.c httpsession.c login.c ./network/andlink/network.c
CFLAGS  += -I$(PRJ_DIR)/hapi/activation/network/andlink
CFLAGS  += -I$(PRJ_DIR)/hapi/andlink/include
else
SRC := $(wildcard *.c ./network/ble/*.c)
CFLAGS  += -I$(PRJ_DIR)/hapi/activation/network/ble
endif

CFLAGS  += -I$(PRJ_DIR)/hapi/activation/include
CFLAGS += -I$(PRJ_DIR)/include/lib
CFLAGS += -I$(PRJ_DIR)/include
CFLAGS += -I$(PRJ_DIR)/include/speech

CFLAGS  += -I$(PRJ_DIR)/hapi/netmgr/include
CFLAGS  += -I$(PRJ_DIR)/hapi/asound/include
CFLAGS += -DYODALITE

include $(COMMON_DIR)/common.mk
