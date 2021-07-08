#!/bin/bash

NAME=os6360
Rel=0

if [ $# -eq 1 ];
then
  if [ $1 -eq 1 ];
  then
    Rel=1
  fi
fi

#Current path
CUR_PATH=`pwd`

#set -x

#Clean old images
rm -rf output

LINUX_FOLDER=linux/uni/4.4/kernel/mvebu_v7_lsp_noprestera_defconfig
VER_FILE="cpss/common/h/cpss/common/sw_version.h"

TIME=`date +%Y%m%d%H%M`
SW_VER="OS6360-"$VER_NUM"-"$TIME
STR="#define SW_VER "'"'$SW_VER'"'

echo $STR > $VER_FILE
cat $VER_FILE

ORI_DTB=$LINUX_FOLDER/arch/arm/boot/dts/msys-ac3-rd.dtb
DTB=$LINUX_FOLDER/rootfs/boot/msys-ac3-rd.dtb

if [ -f "$DTB" ];
then
  rm -f $DTB
fi

if [ -f "$ORI_DTB" ];
then
  rm -f $ORI_DTB
fi

#LUA file system
if [ "$CMDFS_NIX" != "yes" ]; then
  #RAMFS
  cp -rf wnc_hw_test/cmd_scripts/* lsp/wnc_tools/luaCli-4.2_250/scripts
  cp -rf wnc_txeq lsp/wnc_tools/luaCli-4.2_250/scripts
else
  #cmdFS
  if [ -f lsp/wnc_tools/luaCli-4.2_250/scripts/packet_EMI.txt ]; then
    #Previous RAMFS
    pushd lsp/wnc_tools/luaCli-4.2_250/scripts
    rm -f packet_EMI.txt
    rm -rf `cat ../../../../wnc_hw_test/file_list`
    popd
  fi
fi

#Build application to call CPSS APIs directly, not through luaCli.
#IPC_TO_CPSS=
IPC_TO_CPSS="y"
WNC_CPSS_IPC_FOLDER="lsp/wnc_tools/wnc-ipc-to-cpss"
WNC_CPSS_IPC_APP="ipc_to_cpss"
pushd $WNC_CPSS_IPC_FOLDER
if [ "$IPC_TO_CPSS" == "y" ]; then
  export IPC_TO_CPSS="y"
  make clean;make
else
  make clean;rm -f $LINUX_FOLDER/rootfs/usr/bin/$WNC_CPSS_IPC_APP 
fi
popd

#Use Python to convert Lua scripts and cisco-like config file to C format.
pushd lsp/wnc_tools/luaCli-4.2_250
./build.sh
popd

#Build MTD utility
MTD_UTILITY=lsp/wnc_tools/mtd-utils/mtd-utils-1.5.2
pushd $MTD_UTILITY
./build.sh
popd

#Build Lattice CPLD online upgrade tool
VME_UTILITY=lsp/wnc_tools/ispvme
pushd $VME_UTILITY
make
popd

#Build fw_setenv and fw_printenv
FW_ENV=../u-boot_2/tools/env
pushd ../u-boot_2
make HOSTCC=${CROSS_COMPILE}gcc HOSTSTRIP=${CROSS_COMPILE}strip env
popd

#./build_cpss.sh ARMv7_LK_4_4 DX_ALL NOKERNEL CPSS_ENABLER_NOKM CUST
./build_cpss.sh ARMv7_LK_4_4 DX_ALL UTF_NO CPSS_ENABLER_NOKM CUST VERBOSE

ROOTFS_BOOT=$LINUX_FOLDER/rootfs/boot

if [ ! -d "$ROOTFS_BOOT" ];
then
  mkdir $ROOTFS_BOOT
fi

if [ ! -d "output" ];
then
  mkdir output
fi

VER_RF_FILE=$LINUX_FOLDER"/rootfs/usr/local/quaggaver"
WELCOME="lsp/wnc_tools/rootfs/etc/welcome"
WELCOME_ORI="lsp/wnc_tools/rootfs/etc/welcome_ori"

echo $SW_VER > $VER_RF_FILE

#telnet welcome
cat $WELCOME_ORI > $WELCOME
echo SW VERSION: $SW_VER >> $WELCOME


ORI_ZIMAGE=$LINUX_FOLDER/arch/arm/boot/zImage
ZIMAGE=$LINUX_FOLDER/rootfs/boot/zImage

DRAGONITE_FW=cpss/cpssEnabler/mainSysConfig/src/appDemo/dragonite/firmware/dragonite.bin
DRAGONITE_BIN=$LINUX_FOLDER/rootfs/usr/bin/dragonite.bin

#Copy rootfs
if [ $Rel -eq 1 ];
then
  UBI_FILE="output/ac3bxh-$NAME-"`date +%m`"-"`date +%d`".img"
else
  UBI_FILE="output/ac3bxh-$NAME.img"
fi

cp -f $ORI_ZIMAGE $ZIMAGE
cp -f $ORI_DTB $DTB
cp -f $DRAGONITE_FW $DRAGONITE_BIN
cp -rf lsp/wnc_tools/rootfs/* $LINUX_FOLDER/rootfs
cp $TOOLCHAIN/arm-marvell-linux-gnueabi/libc/lib/libgcc_s.so* $LINUX_FOLDER/rootfs/lib
cp $TOOLCHAIN/arm-marvell-linux-gnueabi/libc/usr/bin/* $LINUX_FOLDER/rootfs/usr/bin
cp $VME_UTILITY/vme $LINUX_FOLDER/rootfs/usr/bin
cp $MTD_UTILITY/flash_eraseall $LINUX_FOLDER/rootfs/usr/bin
cp $MTD_UTILITY/arm-marvell-linux-gnueabi/flash_erase $LINUX_FOLDER/rootfs/usr/bin
cp $MTD_UTILITY/arm-marvell-linux-gnueabi/nanddump $LINUX_FOLDER/rootfs/usr/bin
cp $MTD_UTILITY/arm-marvell-linux-gnueabi/nandwrite $LINUX_FOLDER/rootfs/usr/bin
cp $FW_ENV/fw_printenv $LINUX_FOLDER/rootfs/usr/bin
cp $FW_ENV/fw_printenv $LINUX_FOLDER/rootfs/usr/bin/fw_setenv
cp $FW_ENV/fw_env.config $LINUX_FOLDER/rootfs/etc
cp config*.txt $LINUX_FOLDER/rootfs/etc
cp mrvl_luash $LINUX_FOLDER/rootfs/usr/bin
cp wnc_hw_test/* $LINUX_FOLDER/rootfs/usr/bin
if [ "$CMDFS_NIX" == "yes" ]; then
  CMDFS_PATH=$LINUX_FOLDER/rootfs/cmdFS
  mkdir -p $CMDFS_PATH
  #Remove all and re-copy, to always sync with luaCli
  rm -rf $CMDFS_PATH/*
  cp -rf lsp/wnc_tools/luaCli-4.2_250/scripts/* $CMDFS_PATH
  rm -rf $CMDFS_PATH/px
  cp -rf cmdFS/* $CMDFS_PATH
  cp -rf wnc_hw_test/cmd_scripts/* $CMDFS_PATH
  cp -rf wnc_txeq $CMDFS_PATH
fi
if [ "$IPC_TO_CPSS" == "y" ]; then
  cp -f $WNC_CPSS_IPC_FOLDER/$WNC_CPSS_IPC_APP $LINUX_FOLDER/rootfs/usr/bin
fi

# Build i2c-tools-4.0. poe utility need this tool.
cd lsp/wnc_tools/i2c-tools-4.0
make
cd -

# Build poe utility
cd lsp/wnc_tools/poe
cp ../i2c-tools-4.0/tools/i2cbusses.* host_poe_hal/lib
cp ../i2c-tools-4.0/lib/libi2c.a host_poe_hal/lib
cp ../i2c-tools-4.0/lib/libi2c.so.0.1.0 host_poe_hal/lib
make
cd -
cp lsp/wnc_tools/poe/bin/poe $LINUX_FOLDER/rootfs/usr/bin

# Copy eeprom utilities
cp lsp/wnc_tools/eeprom/eeprom $LINUX_FOLDER/rootfs/usr/bin
cp lsp/wnc_tools/eeprom/eeprom_parser $LINUX_FOLDER/rootfs/usr/bin

# Copy version utilities
cp lsp/wnc_tools/version/version $LINUX_FOLDER/rootfs/usr/bin

# Copy speed utilities
cp lsp/wnc_tools/front_port_speed/speed $LINUX_FOLDER/rootfs/usr/bin
cp lsp/wnc_tools/front_port_speed/pinfo_cfg/* $LINUX_FOLDER/rootfs/etc

# Copy tmux utility
cp lsp/wnc_tools/tmux/tmux $LINUX_FOLDER/rootfs/usr/bin
cp -r lsp/wnc_tools/tmux/terminfo/ $LINUX_FOLDER/rootfs/usr/share

#Copy diag install tarball
DIAG_DIR=lsp/wnc_tools/diag/diag_code/project
if [ -d $DIAG_DIR ];
then
  echo "Copy diag tarball ..."
  cd $DIAG_DIR/release;./make_rel_install
  cd -
  mkdir -p $LINUX_FOLDER/rootfs/opt/diag
  cp -f $DIAG_DIR/release/wnc-diag-*.install  $LINUX_FOLDER/rootfs/opt/diag
fi

#Assign switch vendor for diag framework
mkdir -p $LINUX_FOLDER/rootfs/var/opt
echo "MARVELL" > $LINUX_FOLDER/rootfs/var/opt/switch_vendor

#Make ubifs image
mkfs.ubifs -F -m 4KiB -e 248KiB -c 3600 -r $LINUX_FOLDER/rootfs -x none -vv $UBI_FILE


#Make ubi image
UBINIZE_CFG="output/ubinize.cfg"

if [ $Rel -eq 1 ];
then
  UBI_BIN="output/ubi-ac3bxh-$NAME-"`date +%m`"-"`date +%d`".bin"
else
  UBI_BIN="output/ubi-ac3bxh-$NAME.bin"
fi

echo "[rootf]
mode=ubi
image=$UBI_FILE
vol_id=0
vol_type=dynamic
vol_size=900MiB
vol_alignment=1
vol_name=nand_rootfs
vol_flags=autoresize" > $UBINIZE_CFG

ubinize -m 4KiB -p 256KiB -s 4KiB -o $UBI_BIN  $UBINIZE_CFG

#AOS integration
FIT_FOR_AOS="output/Nadiag.img"
UBI_FLASH="output/Ndiag-$NAME.bin"
UBI_FLASH_BIN="output/ubi-Ndiag-$NAME.bin"
AOS_FIT="Nosa.img"
AOS_SW_LSM="software.lsm"

mv -f $ZIMAGE output/
mv -f $DTB output/msys-ac3-rd.dtb
rm -rf $ROOTFS_BOOT

pushd $LINUX_FOLDER/rootfs
find . | cpio -o -H newc | gzip -9 >../ramdisk.cpio.gz
cp ../ramdisk.cpio.gz $CUR_PATH/output
rm ../ramdisk.cpio.gz
popd

mkdir -p $LINUX_FOLDER/rootfs/flash/certified
mkdir -p $LINUX_FOLDER/rootfs/flash/working
mkdir -p $LINUX_FOLDER/rootfs/flash/diags

cp lsp/wnc_tools/wnc-its/wnc-diag.its output
mkimage -f output/wnc-diag.its $FIT_FOR_AOS
cp -f $FIT_FOR_AOS $LINUX_FOLDER/rootfs/flash/diags
cp -f $AOS_FIT $LINUX_FOLDER/rootfs/flash/certified
cp -f $AOS_FIT $LINUX_FOLDER/rootfs/flash/working
cp -f $AOS_SW_LSM $LINUX_FOLDER/rootfs/flash/certified
cp -f $AOS_SW_LSM $LINUX_FOLDER/rootfs/flash/working
touch $LINUX_FOLDER/rootfs/flash/RunDiagnostics
touch $LINUX_FOLDER/rootfs/flash/Rj45LoopbackStub

mkfs.ubifs -F -m 4KiB -e 248KiB -c 3600 -r $LINUX_FOLDER/rootfs/flash -x none -vv $UBI_FLASH

echo "[rootfs]
mode=ubi
image=$UBI_FLASH
vol_id=0
vol_type=dynamic
vol_size=900MiB
vol_alignment=1
vol_name=flash
vol_flags=autoresize" > $UBINIZE_CFG

ubinize -m 4KiB -p 256KiB -s 4KiB -o $UBI_FLASH_BIN  $UBINIZE_CFG
