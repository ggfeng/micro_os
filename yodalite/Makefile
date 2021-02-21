PHONY += menuconfig silentoldconfig clean distclean

Q            := @
CONF_DIR     :=./config
Kconfig      := Kconfig

rm-distclean += include/generated include/config .config* output 

export PRJ_DIR := $(shell pwd)
export TOOLCHAINS   = $(PRJ_DIR)/toolchains
export OUT_DIR := $(PRJ_DIR)/output
export LIB_DIR := $(OUT_DIR)/libs
export INC_DIR := $(OUT_DIR)/include
export SRC_ROOT := $(PRJ_DIR)/src
export CFLAGS += -I$(PRJ_DIR)/include
export C_INCLUDE_PATH +=:$(PRJ_DIR)/include/generated:$(PRJ_DIR)/include

export COMMON_DIR   := $(PRJ_DIR)/common

ifeq ($(quiet),silent_)
silent := -s
endif
 
-include $(PRJ_DIR)/include/config/auto.conf
-include $(COMMON_DIR)/env.mk

export 

CROSS_COMPILE = $(CONFIG_TOOLPREFIX)
AS      = $(CROSS_COMPILE)as
LD      = $(CROSS_COMPILE)ld
CC      = $(CROSS_COMPILE)gcc
CPP     = $(CC) -E
AR      = $(CROSS_COMPILE)ar
NM      = $(CROSS_COMPILE)nm
LDR     = $(CROSS_COMPILE)ldr
STRIP       = $(CROSS_COMPILE)strip
OBJCOPY     = $(CROSS_COMPILE)objcopy
OBJDUMP     = $(CROSS_COMPILE)objdump

BUILD_DIR   = $(PRJ_DIR)/build_dir/$(CROSS_COMPILE)dir

lib-y  += init
lib-$(CONFIG_AIAL_ENABLE)  += aial
lib-$(CONFIG_APP_ENABLE)   += app
lib-$(CONFIG_HAPI_ENABLE)  += hapi
lib-$(CONFIG_HARDWARE_ENABLE) += hardware
lib-$(CONFIG_LIBS_ENABLE)     += lib
lib-$(CONFIG_OSAL_ENABLE)     += osal
lib-$(CONFIG_PRODUCTS_ENABLE) += products
#lib-$(CONFIG_TOOLS_ENABLE) += tools

ifeq ($(CONFIG_STM32F407ZG),y)
-include $(COMMON_DIR)/stm32f407_env.mk 
endif

ifeq ($(CONFIG_STM32F072CBY),y)
-include $(COMMON_DIR)/stm32f072_env.mk
endif

ifeq ($(CONFIG_STM32F411CEYx),y)
-include $(COMMON_DIR)/stm32f411_env.mk
endif

ifeq ($(CONFIG_XR871GT),y)
-include $(COMMON_DIR)/xr871_env.mk
endif

ifeq ($(CONFIG_XR872AT),y)
-include $(COMMON_DIR)/xr872_env.mk
endif

ifeq ($(CONFIG_ESP32),y)
-include $(COMMON_DIR)/esp32_env.mk
endif

#CFLAGS += -Werror

SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)


ifeq ($(CONFIG_SDK_BUILD_EXE),y)

TARGET       :=rokid-litos-app

ALL: $(TARGET)

$(TARGET):$(lib-y) $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(wildcard $(LIB_DIR)/*.a) $(LDFLAGS)
	$(Q)mv $(TARGET) $(OUT_DIR) 
else

TARGET       :=librokid-litos.a

ALL: $(TARGET)

$(TARGET):subdir $(OBJ) EXE_O
	echo $(OBJ)
	$(AR) -crs $(TARGET) $(OBJ) $(shell ls $(LIB_DIR)/*.o) $(LDFLAGS)
	$(Q)mv $(TARGET) $(OUT_DIR) 

EXE_O:
	$(foreach file,$(shell ls $(LIB_DIR)/*.a),cd $(LIB_DIR) && $(AR) -x $(file);)

endif

subdir: build_prepare
	$(foreach  dir,$(lib-y),$(MAKE) -C $(BUILD_DIR)/$(dir) || exit "$$?";)

build_prepare:
	mkdir -p $(BUILD_DIR)
	$(foreach  dir,$(lib-y),cp -fr $(dir) $(BUILD_DIR);)
	#$(foreach  dir,$(lib-y),cp -fpr $(dir) $(BUILD_DIR);)

menuconfig : $(CONF_DIR)/mconf $(CONF_DIR)/conf
	$(Q)$< $(Kconfig)
	$(Q)$(MAKE) silentoldconfig

$(CONF_DIR)/mconf:
	$(Q)$(MAKE) -C $(CONF_DIR) mconf

$(CONF_DIR)/conf:
	$(Q)$(MAKE) -C $(CONF_DIR) conf

silentoldconfig: $(CONF_DIR)/conf
	$(Q)mkdir -p include/generated include/config output output/libs output/include
	$(Q)$< -s --silentoldconfig $(Kconfig)

defconfig:silentoldconfig

clean:
ifeq ($(shell test -e $(BUILD_DIR) && echo $$?),0)
	$(foreach file,$(lib-y),make -C $(BUILD_DIR)/$(file) clean;)
endif
	$(Q)rm -rf output/*.a output/$(TARGET) *.a output/libs/*
distclean:clean
	-rm -rf  $(rm-distclean) $(BUILD_DIR) output
	$(Q)$(MAKE)  -C $(CONF_DIR) clean
	$(Q)echo -e ""
	$(Q)echo -e ""
	$(Q)echo "-------Warning you need make menuconfig !!!!---------"
	$(Q)echo -e ""

.PHONY : ALL
