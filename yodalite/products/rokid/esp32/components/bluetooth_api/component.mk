#
# Component Makefile
#
ifdef CONFIG_BT_ENABLED
COMPONENT_ADD_INCLUDEDIRS := include
COMPONENT_SRCDIRS := src
COMPONENT_OBJS := src/ble_server.o src/a2dp_sink.o src/bluetooth_common.o
endif

