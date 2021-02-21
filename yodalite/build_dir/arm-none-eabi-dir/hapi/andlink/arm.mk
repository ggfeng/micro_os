LOCAL_PATH := $(call my-dir)
$(warning dongxicheng $(LOCAL_PATH))
$(warning dongxicheng $(my-dir))
include $(CLEAR_VARS)

MAJOR_VERSION := $(shell echo $(PLATFORM_VERSION) | cut -f1 -d.)
$(warning CURR PLATFORM VERSION MAJOR: $(MAJOR_VERSION))
VERSION_6_0 := $(shell test $(MAJOR_VERSION) -eq 6 && echo true)
VERSION_LT_6_0 := $(shell test $(MAJOR_VERSION) -lt 6 && echo true)

LOCAL_SRC_FILES := \
		async.c \
		coap_list.c \
		encode.c \
		mobileclientcoap.c \
		net.c \
		pdu.c \
		str.c \
		uri.c \
		block.c \
		debug.c \
		hashkey.c \
		mobileservercoap.c \
		option.c \
		resource.c \
		subscribe.c


LOCAL_SHARED_LIBRARIES := \
		libandroid_runtime \
		libbinder \
		libcutils \
		liblog \
		libhardware \
		libutils \
		libnativehelper \
		libz \
		libdl \
		libeasyr2

LOCAL_C_INCLUDES += \
		robot/easyr2/include \
		robot/external/easyrobot/include \
		robot/base/core/native/include \
		$(LOCAL_PATH)

LOCAL_C_INCLUDES += external/libcxx/include
LOCAL_C_INCLUDES += bionic

ifeq ($(VERSION_LT_6_0), true)
LOCAL_C_INCLUDES += external/stlport/stlport
endif

ARCH_ARM = yes
HAVE_ARMV6 = yes
HAVE_ARMV6T2 = yes
HAVE_ARMVFP = yes
LOCAL_ARM_MODE := arm



LOCAL_CFLAGS += -Wall -Wextra -Wno-unused-parameter -fno-rtti -fno-exceptions -std=gnu++0x -fPIC

LOCAL_CFLAGS += -DWITH_POSIX

EXTRA_CFLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=neon"
EXTRA_LDFLAGS="-Wl,--fix-cortex-a8 "

ABI = "armeabi-v7a"


LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := libandlink
LOCAL_MODULE_TARGET_ARCH := arm
$(warning dongxicheng $(BUILD_SHARED_LIBRARY))
include $(BUILD_SHARED_LIBRARY)
