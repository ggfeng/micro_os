ESP32_PRJ_DIR := $(shell pwd)
export ADF_PATH:= $(ESP32_PRJ_DIR)/../../../../vendor/esp32/esp-adf
export PATH  := $(PATH):$(ESP32_PRJ_DIR)/../../../toolchains/xtensa-esp32-elf/bin

PROJECT_NAME :=yodalite_esp32
EXCLUDE_COMPONENTS +=json
EXTRA_COMPONENT_DIRS :=vendorapi
EXTRA_COMPONENT_DIRS +=ailib

include $(ADF_PATH)/project.mk
