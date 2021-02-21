ifeq ($(CONFIG_XR871GT),y)
export PATH  := $(PATH):$(TOOLCHAINS)/gcc-arm-none-eabi-4_9-2015q2/bin
else
ifeq ($(CONFIG_XR872AT),y)
export PATH  := $(PATH):$(TOOLCHAINS)/gcc-arm-none-eabi-4_9-2015q2/bin
else
ifeq ($(CONFIG_ESP32), y)
export PATH  :=$(PATH):$(TOOLCHAINS)/xtensa-esp32-elf/bin
else
export PATH  := $(PATH):$(TOOLCHAINS)/gcc-arm-none-eabi-5_4-2016q3/bin
endif
endif
endif

