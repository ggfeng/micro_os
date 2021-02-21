CPU := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
export CFLAGS += $(CPU) -fno-common -fmessage-length=0                                   \
	-fno-exceptions -ffunction-sections -fdata-sections -fomit-frame-pointer         \
	-Wall -Wpointer-arith -MMD -MP -Os -DNDEBUG -Wl,--gc-sections --specs=nano.specs \
        -I$(PRJ_DIR)/products/rokid/xr872/src/main                                 \
        -I$(PRJ_DIR)/../vendor/xr872/project/common                                \
        -I$(PRJ_DIR)/../vendor/xr872/include/kernel/FreeRTOS                       \
        -I$(PRJ_DIR)/../vendor/xr872/include/kernel/FreeRTOS/portable/GCC/ARM_CM4F \
        -I$(PRJ_DIR)/../vendor/xr872/include                                       \
		-I$(PRJ_DIR)/../vendor/xr872/include/net                                   \
        -I$(PRJ_DIR)/../vendor/xr872/include/net/lwip-2.0.3/                       \
        -I$(PRJ_DIR)/lib/lwip/include/compat/posix                                 \
        -I$(PRJ_DIR)/products/rokid/xr872/src/vendorapi/include                    \
        -include prj_conf_opt.h

