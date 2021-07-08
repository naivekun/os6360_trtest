#!/usr/bin/env bash
#
# A script to build mvKernelExt.ko manually (example
#
# The target is MSYS/ARMADA385/ARMADA380, Linux 3.10
#
# Edit next line and set $DIST to kernel's source path

MV7SFT_PATH=/swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/armv7-marvell-linux-gnueabi-softfp_i686/bin
PATH=$MV7SFT_PATH:$PATH

DIST=/nfs/pt/home/sergeish/linux/uni/3.10/kernel/mvebu_lsp_defconfig
export ARCH=arm
export CROSS_COMPILE=arm-marvell-linux-gnueabi-


make -C $DIST M=`pwd` modules
