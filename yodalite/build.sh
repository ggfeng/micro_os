#!/bin/bash

set -o errexit

usage() {
cat <<USAGE

Usage:
    bash $0 -p <PRODUCT> -f <IMG_FOLDER> -n <IMG_NAME> [OPTIONS]
    eg: ./build.sh -p esp32_lyrat -n esp32_lyrat -f esp32_lyrat -r

Description:
    Builds Openwrt for given PRODUCT

OPTIONS:
    -d, --debug
        Enable debugging - change Debug/release image

    -c, --clean
        Make distclean

    -r, --remove
        Make clean

    -h, --help
        Display this help message

    -p, --product
        The product name (openwrt/configs/<PRODUCT>_defconfig, eg: leo_k18_universal_node)

    -f, --folder
        Image folder (eg:leo-k18-universal)

    -n, --name
        Image name  (eg:openwrt-leo-k18-universal)

    -j, --jobs
        Specifies the number of jobs to run simultaneously (Default: 8)

USAGE
}

# Set defaults
CLEAN_OUTPUT_PRODUCT_DIR=
REMOVE_OUTPUT_PRODUCT_DIR=
PACKAGES_TO_CLEAN=
PRODUCT=
RELEASE_VERSION=
ROOT_DIR=$(pwd)
DEBUG="false"
# Setup getopt.
long_opts="debug,clean,folder:,help,jobs:,product:,name:,module:,solid_filesystem:"
getopt_cmd=$(getopt -o dcrf:hj:p:n:m:s: --long "$long_opts" \
            -n $(basename $0) -- "$@") || \
            { echo -e "\nERROR: Getopt failed. Extra args\n"; usage; exit 1;}

eval set -- "$getopt_cmd"
while true; do
    case "$1" in
        -d|--debug) DEBUG="true";;
        -c|--clean) CLEAN_OUTPUT_PRODUCT_DIR="true";;
        -m|--module) PACKAGES_TO_CLEAN=$(echo $2 | tr "," "\n");;
        -r|--remove) REMOVE_OUTPUT_PRODUCT_DIR="true";;
        -f|--folder) IMG_FOLDER="$2";;
        -h|--help) usage; exit 0;;
        -j|--jobs) JOBS="$2"; shift;;
        -p|--product) PRODUCT="$2"; shift;;
        -n|--name) IMG_NAME="$2"; shift;;
        -s|--solid_filesystem) BUILD_ROOT_FILESYSTEM="$2"; shift;;
        --) shift; break;;
    esac
    shift
done

#============================================================
# prepare build enviroment
#===========================================================

if [ -z $PRODUCT ];then
	usage
	exit 1
else
	echo "PRODUCT=${PRODUCT}"
fi

#============================================================
# default build all modules, make all image and save stuff
#============================================================
SCRIPTS_DIR=${ROOT_DIR}/products/common/modules
echo "${JOBS}"

if [[ ${PRODUCT} =~ esp32 ]]; then
	echo "source ${SCRIPTS_DIR}/esp32.sh"
	source ${SCRIPTS_DIR}/esp32.sh ${PRODUCT} ${DEBUG} ${IMG_NAME} ${IMG_FOLDER} ${JOBS}
elif [[  ${PRODUCT} =~ stm32 ]]; then
	echo "source ${SCRIPTS_DIR}/stm32.sh"
	source ${SCRIPTS_DIR}/stm32.sh ${PRODUCT} ${DEBUG} ${IMG_NAME} ${IMG_FOLDER} ${JOBS}
elif [[  ${PRODUCT} =~ kenobi ]]; then
	echo "source ${SCRIPTS_DIR}/kenobi.sh"
	source ${SCRIPTS_DIR}/kenobi.sh ${PRODUCT} ${DEBUG} ${IMG_NAME} ${IMG_FOLDER} ${JOBS}
elif [[ ${PRODUCT} =~ luke ]]; then
	echo "source ${SCRIPTS_DIR}/luke.sh"
	source ${SCRIPTS_DIR}/luke.sh ${PRODUCT} ${DEBUG} ${IMG_NAME} ${IMG_FOLDER} ${JOBS}
elif [[ ${PRODUCT} =~ xr871 ]]; then
	echo "source ${SCRIPTS_DIR}/xr871.sh"
	source ${SCRIPTS_DIR}/xr871.sh ${PRODUCT} ${DEBUG} ${IMG_NAME} ${IMG_FOLDER} ${JOBS}
elif [[ ${PRODUCT} =~ xr872 ]]; then
	echo "source ${SCRIPTS_DIR}/xr872.sh"
	source ${SCRIPTS_DIR}/xr872.sh ${PRODUCT} ${DEBUG} ${IMG_NAME} ${IMG_FOLDER} ${JOBS}
else
	echo "${PRODUCT} is current unsupported"
fi

if [ "${CLEAN_OUTPUT_PRODUCT_DIR}" = "true" ]; then
	clean_output_product_dir
fi

enviroment ${PRODUCT}

if [ "${REMOVE_OUTPUT_PRODUCT_DIR}" = "true" ]; then
	remove_output_product_dir
fi

build_fullimage

build_otaimage

build_ftpfiles

if [[ ${PRODUCT} =~ esp32 || ${PRODUCT} =~ kenobi || ${PRODUCT} =~ xr871 || ${PRODUCT} =~ xr872 ]]; then
     rokid_package
fi

check_exit
