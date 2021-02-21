#!/bin/sh

if [ $# -lt 2 ] ;then
device="STM32F072CB"
file="yodalite_stm32.bin"
else
device=$1
file=$2
fi

addr="0x8000000"
echo "device:${device} file:${file} addr:${addr}"
./stm32_JLINK.sh ${device} ${file} ${addr}
