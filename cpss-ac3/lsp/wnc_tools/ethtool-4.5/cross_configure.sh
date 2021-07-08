#!/bin/sh
ARM_TOOLCHAIN_DIR="/opt/armv7-marvell-linux-gnueabi-softfp-4.6.4_64K_i686_20160226/bin"
export CROSS_COMPILE="${ARM_TOOLCHAIN_DIR}/arm-marvell-linux-gnueabi-"
export AR="${CROSS_COMPILE}ar"
export AS="${CROSS_COMPILE}as"
export LD="${CROSS_COMPILE}ld"
export NM="${CROSS_COMPILE}nm"
export CC="${CROSS_COMPILE}gcc"

export FC="${CROSS_COMPILE}gfortran "
export RANLIB="${CROSS_COMPILE}ranlib"
export STRIP="${CROSS_COMPILE}strip"
export OBJCOPY="${CROSS_COMPILE}objcopy"  
export CFLAGS="-Os -pipe -O2 -static"

./configure --target=arm-linux --host=arm-linux
