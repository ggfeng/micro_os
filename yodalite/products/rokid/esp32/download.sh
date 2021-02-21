#!/bin/sh

if [ $# -lt 1 ];then
ttyDev=/dev/ttyUSB0
else
ttyDev=$1
fi

echo "used tty device:${ttyDev}"

./esptool.py --chip esp32 --port ${ttyDev} --baud 921600 --before default_reset --after hard_reset write_flash -u --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 ./bootloader.bin 0x10000 ./yodalite_esp32.bin 0x8000 ./partitions_yodalite.bin

./idf_monitor.py --port ${ttyDev} --baud 115200 ./yodalite_esp32.elf
