# Put your stlink folder here so make burn will work.
#
ROOT_DIR:=../../../..
KENOBI_DIR:=$(ROOT_DIR)/vendor/kenobi
VENDOR_CODE:=Drivers Inc MDK-ARM Middlewares Src

SRCS:= $(wildcard main/*.c main/*.s vendorapi/*c vendorapi/*s)

# Library code
SRCS += stm32f0xx_hal_adc.c         \
	stm32f0xx_hal_adc_ex.c     \
	stm32f0xx_hal.c            \
	stm32f0xx_hal_cortex.c     \
	stm32f0xx_hal_dac.c        \
	stm32f0xx_hal_dac_ex.c     \
	stm32f0xx_hal_dma.c        \
	stm32f0xx_hal_flash.c      \
	stm32f0xx_hal_flash_ex.c   \
	stm32f0xx_hal_gpio.c       \
	stm32f0xx_hal_i2c.c        \
	stm32f0xx_hal_i2c_ex.c     \
	stm32f0xx_hal_i2s.c        \
	stm32f0xx_hal_pcd.c        \
	stm32f0xx_hal_pcd_ex.c     \
	stm32f0xx_hal_pwr.c        \
	stm32f0xx_hal_pwr_ex.c     \
	stm32f0xx_hal_rcc.c        \
	stm32f0xx_hal_rcc_ex.c     \
	stm32f0xx_hal_tim.c        \
	stm32f0xx_hal_tim_ex.c     \
	stm32f0xx_hal_uart.c       \
	stm32f0xx_hal_uart_ex.c

OUTPATH=build
PROJ_NAME=yodalite_stm32
ROKID_LIB_PATH=./components/rokid_sdk/lib
ROKID_LIB_INCLUDE=./components/rokid_sdk/include

###################################################
TOOLCHAINS=$(ROOT_DIR)/yodalite/toolchains

-include $(ROOT_DIR)/yodalite/build/include/config/auto.conf
-include $(ROOT_DIR)/yodalite/build/common/env.mk

CROSS_COMPILE = $(CONFIG_TOOLPREFIX)
export

CC=$(CROSS_COMPILE)gcc
SIZE=$(CROSS_COMPILE)size
OBJCOPY=$(CROSS_COMPILE)objcopy

# Choose debug or release...
CFLAGS = -g -O2           # Normal
#CFLAGS = -ggdb -O0       # RSW - for GDB debugging, disable optimizer

CFLAGS += -Wall -Tstm32f072cby.ld
CFLAGS += -DUSE_STDPERIPH_DRIVER

CFLAGS += -DUSE_HAL_DRIVER
CFLAGS += -DSTM32F072xB
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -nostartfiles -mcpu=cortex-m0
#CFLAGS += -msoft-float
CFLAGS += -specs=nosys.specs
CFLAGS += -specs=nano.specs
#CFLAGS += -nostartfiles
###################################################

vpath %.c  \
vendorapi	\
Src        \
MDK-ARM    \
Drivers/STM32F0xx_HAL_Driver/Src                           \
Middlewares/ST/STM32_USB_Device_Library/Class/AUDIO/Src    \
Middlewares/ST/STM32_USB_Device_Library/Class/Core/Src     \
Middlewares/ST/STM32_USB_Device_Library/Core/Src

# Includes
CFLAGS += -I./Inc
CFLAGS += -I./MDK-ARM/RTE/072-module
CFLAGS += -I./Drivers/STM32F0xx_HAL_Driver/Inc
CFLAGS += -I./Drivers/STM32F0xx_HAL_Driver/Inc/Legacy
CFLAGS += -I./Middlewares/ST/STM32_USB_Device_Library/Core/Inc
CFLAGS += -I./Middlewares/ST/STM32_USB_Device_Library/Class/Core/Inc
CFLAGS += -I./Middlewares/ST/STM32_USB_Device_Library/Class/AUDIO/Inc
CFLAGS += -I./Drivers/CMSIS/Device/ST/STM32F0xx/Include
CFLAGS += -I./Drivers/CMSIS/Include
CFLAGS += -I$(ROKID_LIB_INCLUDE)
CFLAGS += -I./vendorapi

###################################################

.PHONY: proj dir_tree clean distclean

all:  dir_tree proj

dir_tree:
	mkdir -p $(OUTPATH)

proj: $(OUTPATH)/$(PROJ_NAME).elf
	$(SIZE) $(OUTPATH)/$(PROJ_NAME).elf

$(OUTPATH)/$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ -L$(ROKID_LIB_PATH) -lrokid-litos
	$(OBJCOPY) -O ihex $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).bin
clean:
	rm -rf *.o *.lst
	rm -f $(OUTPATH)/$(PROJ_NAME).elf
	rm -f $(OUTPATH)/$(PROJ_NAME).hex
	rm -f $(OUTPATH)/$(PROJ_NAME).bin


distclean:clean
	-rm -rf $(VENDOR_CODE) $(OUTPATH) components

linkdir:
	-rm -rf $(VENDOR_CODE) $(OUTPATH)
	$(foreach dir,$(VENDOR_CODE),ln -s $(KENOBI_DIR)/$(dir) $(dir);)
