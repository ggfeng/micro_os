#!/bin/bash
#
# source by build.sh # ./esp32.sh ${PRODUCT} ${DEBUG} ${IMG_NAME} ${IMG_FOLDER}
#
echo ">>>>>>> xr871.sh"

VERSION_NODE=$(date +%Y%m%d)
PRODUCT=$1
DEBUG=$2
IMG_NAME=$3
IMG_FOLDER=$4
CMD="V=s"

#echo "$# $0 $1 $2 $3 $4 $5"
ROOT_DIR=$(pwd)
OUT_DIR=
RELEASE_VERSION=

ROKID_BUILD_DIR=$ROOT_DIR
VENDOR_DIR=${ROOT_DIR}/../vendor/xr872
PRODUCT_BUILD_DIR=$ROOT_DIR/products/rokid/${PRODUCT}
ROKID_LIB_DIR=$PRODUCT_BUILD_DIR/components/rokid_sdk

#CONFIG_FILE=`find $CONFIGS_DIR |grep ${PRODUCT}_defconfig`

CONFIG_PRODUCT_CONFIG=${PRODUCT_BUILD_DIR}/configs/xr872_product_config
CONFIG_FILE=${PRODUCT_BUILD_DIR}/configs/${PRODUCT}_defconfig
CONFIG_BIN_DIR=`grep -r CONFIG_BINARY_FOLDER  $CONFIG_FILE \
       |awk -F \" '{print $2}' \
       |sed 's/$(TOPDIR)\///g'`

#export CC_DIR=$ROOT_DIR/toolchains/gcc-arm-none-eabi-4_9-2015q2/bin
#echo "CC_DIR=$CC_DIR"
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

	pushd ${ROOT_DIR}/include

	if [ -d arpa ];then
		rm -rf arpa
    fi
	popd


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
	make project_clean 
#        rm -rf ${PRODUCT_BUILD_DIR}/components

	popd
}

enviroment()
{
	pushd ${ROOT_DIR}/include

	if [ -d arpa ];then
		rm -rf arpa
	#	mkdir -p arpa
	#	cd arpa && ln -s ../../../vendor/xr872/include/net/lwip-2.0.3/lwip/inet.h inet.h 
    fi

	mkdir -p arpa

	cd arpa && ln -s ../../../vendor/xr872/include/net/lwip-2.0.3/lwip/inet.h inet.h 
	popd

	pushd ${ROOT_DIR}/include/osal/sys

	if [ -e FreeRTOS_POSIX_portable.h ];then
       rm -rf FreeRTOS_POSIX_portable.h
	fi
  
     ln -s FreeRTOS_POSIX_port_xr872.h FreeRTOS_POSIX_portable.h 

	popd

	pushd ${ROOT_DIR}/../vendor/xr872/include/net/mbedtls

	if [ ! -f net_sockets.h ];then 
       ln -s net.h  net_sockets.h
	fi
	popd


	pushd ${ROOT_DIR}

	echo "cp -f $CONFIG_FILE ${ROKID_BUILD_DIR}/.config"

	cp -f $CONFIG_FILE ${ROKID_BUILD_DIR}/.config
        cp -f $CONFIG_PRODUCT_CONFIG  $VENDOR_DIR/.config


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
        make defconfig
	make ${CMD}
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
	cp -rf ${ROOT_DIR}/include/*   ${ROKID_LIB_DIR}/include/
	cp -rf ${ROOT_DIR}/include/generated/* ${ROKID_LIB_DIR}/include/
```
	pushd ${PRODUCT_BUILD_DIR}

	echo "--${PRODUCT_BUILD_DIR}--"

        make project_build 
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
    cp -rf ${PRODUCT_BUILD_DIR}/image/xr872/*.img ${OUT_DIR}
    cp -rf ${PRODUCT_BUILD_DIR}/settings.ini ${OUT_DIR}
    cp -rf ${VENDOR_DIR}/tools/phoenixMC ${OUT_DIR}/download.sh
    chmod +x ${OUT_DIR}/download.sh

    cp -rf ${PRODUCT_BUILD_DIR}/image/xr872/*img ${OUT_DIR}/full_images
    cp -rf ${PRODUCT_BUILD_DIR}/settings.ini ${OUT_DIR}/full_images
    cp -rf ${VENDOR_DIR}/tools/phoenixMC ${OUT_DIR}/full_images/download.sh
    chmod +x ${OUT_DIR}/full_images/download.sh

```
 echo "rokid_package"
}

build_ftpfiles()
{
  echo "build_ftpfiles"
}