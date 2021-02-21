export CFLAGS += -mcpu=cortex-m4 -mthumb -mthumb-interwork -specs=nano.specs -specs=nosys.specs \
                 -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -std=gnu89 -g -ggdb3 -fverbose-asm        \
                -I$(PRJ_DIR)/lib/FreeRTOS/include   \
                -I$(PRJ_DIR)/lib/FreeRTOS/portable/GCC/ARM_CM4F

