#!/bin/bash
#
# source by build.sh # ./esp32.sh ${PRODUCT} ${DEBUG} ${IMG_NAME} ${IMG_FOLDER}
#
echo ">>>>>>> esp32.sh"

VERSION_NODE=$(date +%Y%m%d)
PRODUCT=$1
DEBUG=$2
IMG_NAME=$3
IMG_FOLDER=$4
JOBS=$5
CMD="-j${JOBS}"

#echo "$# $0 $1 $2 $3 $4 $5"
ROOT_DIR=$(pwd)
OUT_DIR=
RELEASE_VERSION=

ROKID_BUILD_DIR=$ROOT_DIR
PRODUCT_BUILD_DIR=$ROOT_DIR/products/rokid/${PRODUCT}
ROKID_LIB_DIR=$PRODUCT_BUILD_DIR/components/rokid_sdk

#CONFIG_FILE=`find $CONFIGS_DIR |grep ${PRODUCT}_defconfig`
CONFIG_FILE=${PRODUCT_BUILD_DIR}/configs/${PRODUCT}_defconfig
CONFIG_BIN_DIR=`grep -r CONFIG_BINARY_FOLDER  $CONFIG_FILE \
       |awk -F \" '{print $2}' \
       |sed 's/$(TOPDIR)\///g'`

export ADF_PATH=$ROOT_DIR/../vendor/esp32/esp-adf
export PATH=$ROOT_DIR/toolchains/xtensa-esp32-elf/bin:$PATH

echo "CONFIG_FILE=$CONFIG_FILE"
echo "CONFIG_BIN_DIR=$CONFIG_BIN_DIR"



if [ -z $IMG_NAME ]; then
	IMG_NAME=$(echo ${PRODUCT}|sed 's/_/-/g')
fi

echo "IMG_NAME=$IMG_NAME"

if [ -z $IMG_FOLDER ]; then
	OUT_DIR="${ROOT_DIR}/output/${CONFIG_BIN_DIR}"
else
	OUT_DIR="${ROOT_DIR}/output/${IMG_FOLDER}"
fi

echo "OUT_DIR=${OUT_DIR}"

check_exit()
{
	echo "$?"
	if [ $? != 0 ]; then
		exit $?
	fi
}

remove_output_product_dir()
{
	echo "Now is make clean ^_^"
	echo "Please waiting......................."
	pushd ${ROOT_DIR}
	make clean
	popd

	pushd ${PRODUCT_BUILD_DIR}
	make clean
	popd
}

clean_output_product_dir()
{
	echo "Now is distclean -_-"
	echo "Please waiting........................"
	pushd ${ROOT_DIR}
	make distclean
	popd
	pushd ${PRODUCT_BUILD_DIR}
#	make clean
        rm -rf ${PRODUCT_BUILD_DIR}/build
        rm -rf ${ROKID_LIB_DIR}/include
        rm -rf ${ROKID_LIB_DIR}/lib
        rm -rf $PRODUCT_BUILD_DIR/sdkconfig*

	popd
}

enviroment()
{

	pushd ${ROOT_DIR}/include/osal/sys

	if [ -e FreeRTOS_POSIX_portable.h ];then
		rm -rf FreeRTOS_POSIX_portable.h
	fi
       ln -s FreeRTOS_POSIX_portable_esp32.h FreeRTOS_POSIX_portable.h 
	popd
	pushd ${ROOT_DIR}

	echo "cp -f $CONFIG_FILE ${ROKID_BUILD_DIR}/.config"

	cp -f ${CONFIG_FILE} ${ROKID_BUILD_DIR}/.config
	cp -f ${CONFIG_FILE}.vendor $PRODUCT_BUILD_DIR/sdkconfig

	if [[ $DEBUG = "true" ]]; then
		CONFIGFLAG=1
	else
		CONFIGFLAG=2
	fi

	echo "source  ${ROOT_DIR}/envsetup.sh $1 ${CONFIGFLAG}"

	source ${ROOT_DIR}/envsetup.sh $1 ${CONFIGFLAG}

	make defconfig
	popd
}

#============================================================
# default build all modules, make all image and save stuff
#============================================================
#```
build_fullimage()
{
        echo "build_fullimage"
	echo -e "\nINFO: Build full packages\n"

	pushd ${ROOT_DIR}
        make defconfig && make
	popd

	if [ ! -d ${OUT_DIR}/full_images ];then
		mkdir -p ${OUT_DIR}/full_images
	else
		rm -rf ${OUT_DIR}/full_images/*
	fi

        mkdir -p ${ROKID_LIB_DIR}/lib
        mkdir -p ${ROKID_LIB_DIR}/include
```
        cp -rf ${OUT_DIR}/../*.a       ${ROKID_LIB_DIR}/lib/
	cp -rf ${ROOT_DIR}/include/*            ${ROKID_LIB_DIR}/include/
	cp -rf ${ROOT_DIR}/include/generated/* ${ROKID_LIB_DIR}/include/
```
	pushd ${PRODUCT_BUILD_DIR}

	echo "--${PRODUCT_BUILD_DIR}--"

        make defconfig
        make  ${CMD}
#	RELEASE_VERSION=$(awk -F "=" '$1=="ro.build.version.release"{print $2}' ${ROOT_DIR}/build/version.txt)
#	echo RELEASE_VERSION $RELEASE_VERSION
	popd
}

build_otaimage()
{
 echo "build otaimag"
}

rokid_package()
{

```
    cp -rf ${PRODUCT_BUILD_DIR}/build/bootloader/bootloader.bin ${OUT_DIR}/
    cp -rf ${PRODUCT_BUILD_DIR}/build/*.bin ${OUT_DIR}
    cp -rf ${PRODUCT_BUILD_DIR}/build/*.elf ${OUT_DIR}
    cp -rf ${PRODUCT_BUILD_DIR}/download.sh ${OUT_DIR}
    cp -rf ${PRODUCT_BUILD_DIR}/esptool.py ${OUT_DIR}
    cp -rf ${PRODUCT_BUILD_DIR}/idf_monitor.py ${OUT_DIR}
    cp -rf ${ROOT_DIR}/toolchains/xtensa-esp32-elf/bin/xtensa-esp32-elf-addr2line ${OUT_DIR}

    cp -rf ${PRODUCT_BUILD_DIR}/build/bootloader/bootloader.bin ${OUT_DIR}/full_images
    cp -rf ${PRODUCT_BUILD_DIR}/build/*.bin ${OUT_DIR}/full_images
    cp -rf ${PRODUCT_BUILD_DIR}/build/*.elf ${OUT_DIR}/full_images
    cp -rf ${PRODUCT_BUILD_DIR}/download.sh ${OUT_DIR}/full_images
    cp -rf ${PRODUCT_BUILD_DIR}/esptool.py ${OUT_DIR}/full_images
    cp -rf ${PRODUCT_BUILD_DIR}/idf_monitor.py ${OUT_DIR}/full_images
    cp -rf ${ROOT_DIR}/toolchains/xtensa-esp32-elf/bin/xtensa-esp32-elf-addr2line ${OUT_DIR}/full_images
```
 echo "rokid_package"
}

build_ftpfiles()
{
  echo "build_ftpfiles"
}

