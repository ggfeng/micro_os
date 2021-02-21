#
# Component Makefile
#

#COMPONENT_ADD_INCLUDEDIRS := port/include port/include/coap libcoap/include libcoap/include/coap2

COMPONENT_ADD_INCLUDEDIRS := include
COMPONENT_OBJS = pdu.o net.o debug.o encode.o uri.o coap_list.o resource.o hashkey.o str.o option.o async.o subscribe.o block.o mobileclientcoap.o mobileservercoap.o
COMPONENT_SRCDIRS := .

COMPONENT_SUBMODULES += libandlink

# Silence format truncation warning, until it is fixed upstream
mobileclientcoap.o: CFLAGS += -Wno-pointer-sign
