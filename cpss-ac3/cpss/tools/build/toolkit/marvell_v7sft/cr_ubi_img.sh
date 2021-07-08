#!/usr/bin/env bash

usage() {
	echo "Usage: $0 <kernel_build_dir> <arch> <kernel_ver> <rootfs> <output_dir>"
	exit
}

DIST=$1
ARCH=$2
VERS=$3
ROOTFS=$4
OUTPUT_DIR=$5

# Check parameters
test -z "$DIST" -o -z "$ARCH" -o -z "$ROOTFS" -o -z "$OUTPUT_DIR" && usage
test -d $DIST/. -a -d $ROOTFS/. -a -d $OUTPUT_DIR/. -a -d $DIST/arch/$ARCH/boot || usage

if [ ! -f $DIST/arch/$ARCH/boot/zImage ]; then
	echo "zImage not found in $DIST/arch/$ARCH/boot"
	exit 1
fi

TEMP_ROOT=$OUTPUT_DIR/temp_dir_rootfs.$$
mkdir -p $TEMP_ROOT
cd $ROOTFS
find . -depth |cpio -dump $TEMP_ROOT
mkdir $TEMP_ROOT/boot
cp $DIST/arch/$ARCH/boot/zImage $TEMP_ROOT/boot
find $DIST/arch/$ARCH/boot/dts -type f -a -name '*.dtb' -exec cp '{}' $TEMP_ROOT/boot \;

# TODO: WA
# copy additional DTBs
bin_dtb=/swdev/fileril103/Objects/cpss/bin_history/DTB

if [ "$VERS" = "3.14.33" -a -d $bin_dtb/ARMADA_390_NOKM ]; then
        VER_PARM=3_14
	cp $bin_dtb/ARMADA_390_NOKM/*.dtb $TEMP_ROOT/boot
fi

if [ "$VERS" = "3.10.70" -a -d $bin_dtb/ARMADA_385_NOKM ]; then
        VER_PARM=3_10
	cp $bin_dtb/ARMADA_385_NOKM/*.dtb $TEMP_ROOT/boot
fi

if [ "$VERS" = "4.4.8" -a -d $bin_dtb/ARMv7_LK_4_4 ]; then
        VER_PARM=4_4
	cp $bin_dtb/ARMv7_LK_4_4/*.dtb $TEMP_ROOT/boot
fi

opt=""
if [ -f $ROOTFS/../rootfs_deviceMap ]; then
	opt="-D $ROOTFS/../rootfs_deviceMap"
fi

echo ls -l $TEMP_ROOT/usr/bin/satr
     ls -l $TEMP_ROOT/usr/bin/satr 

echo ls -l $TEMP_ROOT/boot
     ls -l $TEMP_ROOT/boot


echo "BOARD_FAMILY=${BOARD_FAMILY}"
FAMILY_PARM=""

#if [ "${BOARD_FAMILY}" = "DX_ALL" -o "${BOARD_FAMILY}" = "DX" ];then
#  FAMILY_PARM=""
#fi 

if [ "${BOARD_FAMILY}" = "DXPX" ];then
      FAMILY_PARM="_DXPX"
      echo "DXDX FAMILY_PARM=${FAMILY_PARM}"
else
   if [ "${BOARD_FAMILY}" = "PX_ALL" -o "${BOARD_FAMILY}" = "PX" ];then
         #FAMILY_PARM="_${BOARD_FAMILY:0:2}"
         FAMILY_PARM="_PX"
         echo "PX_ALL FAMILY_PARM=${FAMILY_PARM}"
   fi 
fi 

echo "FAMILY_PARM=${FAMILY_PARM}"

PRODUCT_PARM=""
case ${PRODUCT_TYPE} in

  CPSS_ENABLER_NOKM )
       PRODUCT_PARM="_nokm"
  ;;
  CPSS_ENABLER )
       PRODUCT_PARM=""
  ;;
  CPSS_ENABLER_SHARED )
       PRODUCT_PARM="_shar"
  ;;
  CPSS_ENABLER_SHARED_NOKM )
       PRODUCT_PARM="_shar_nokm"
  ;;
esac
FLASH_IMAGE_NAME="arm7_${VER_PARM}${FAMILY_PARM}${PRODUCT_PARM}_ubifs"
echo "FLASH_IMAGE_NAME=${FLASH_IMAGE_NAME}"

#mkfs.ubifs -m 4KiB -e 248KiB -c 4000 -r $TEMP_ROOT $opt -x none $OUTPUT_DIR/ARMv7_${VERS}_256.img
#mkfs.ubifs -m 4KiB -e 504KiB -c 2000 -r $TEMP_ROOT $opt -x none $OUTPUT_DIR/ARMv7_${VERS}_512.img

echo mkfs.ubifs -m 4KiB -e 248KiB -c 4000 -r $TEMP_ROOT $opt -x none -v $OUTPUT_DIR/${FLASH_IMAGE_NAME}_256.img
     mkfs.ubifs -m 4KiB -e 248KiB -c 4000 -r $TEMP_ROOT $opt -x none -v $OUTPUT_DIR/${FLASH_IMAGE_NAME}_256.img
echo mkfs.ubifs -m 4KiB -e 504KiB -c 2000 -r $TEMP_ROOT $opt -x none -v $OUTPUT_DIR/${FLASH_IMAGE_NAME}_512.img
     mkfs.ubifs -m 4KiB -e 504KiB -c 2000 -r $TEMP_ROOT $opt -x none -v $OUTPUT_DIR/${FLASH_IMAGE_NAME}_512.img


rm -rf $TEMP_ROOT
cat >$OUTPUT_DIR/README_${FLASH_IMAGE_NAME}.txt <<eof
Burning image to flash instructions
===================================

Note: the commands below are commands for u-boot


1. Define and create UBI partition over the NAND device.
    set mtdids nand0=armada-nand
    set mtdparts mtdparts=armada-nand:80m(boot)ro,-(rootfs)
    set mtdparts_linux 'mtdparts=armada-nand:80m(boot)ro,-(rootfs);pxa3xx_nand-0:80m(boot)ro,-(rootfs)'
    set ubiboot_mount_ubi 'ubi part rootfs; ubifsmount nand_rootfs'
    saveenv

    /* BEGIN Aldrin-RD ONLY */
    mw 0xf1018008 0x55000000; mw 0xf101800c 0x05050051
    /* END Aldrin-RD ONLY */

    nand erase.part rootfs; ubi part rootfs; ubi create nand_rootfs

2. Prepare u-boot to load from NAND device.
   Replace ZZZZ.dtb in ubiboot_load with proper DTB file name.
   To see list of .dtb files AFTER image is burn:
       run ubiboot_mount_ubi; ubifsls /boot

   /* For Aldrin-RD ONLY - use this command*/
   set ubiboot_bootargs 'mw 0xf1018008 0x55000000; mw 0xf101800c 0x05050051; setenv bootargs root=ubi0 rw ubi.mtd=1 rootfstype=ubifs ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}:${netdev}:off console=ttyS0,115200 ${othbootargs} ${mtdparts_linux}'
   /* For all other systems - use this command*/
   set ubiboot_bootargs 'setenv bootargs root=ubi0 rw ubi.mtd=1 rootfstype=ubifs ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}:${netdev}:off console=ttyS0,115200 ${othbootargs} ${mtdparts_linux}'

   set ubiboot_bootargs 'setenv bootargs root=ubi0 rw ubi.mtd=1 rootfstype=ubifs ip=\${ipaddr}:\${serverip}:\${gatewayip}:\${netmask}:\${hostname}:\${netdev}:off console=ttyS0,115200 \${othbootargs} \${mtdparts_linux}'
   set ubiboot_load 'run ubiboot_mount_ubi; ubifsload 0x2000000 /boot/zImage;ubifsload 0x1000000 /boot/ZZZZ.dtb'
   set ubiboot 'run ubiboot_bootargs; run ubiboot_load; bootz 0x2000000 - 0x1000000'
   saveenv

3. Load the UBI image onto the NAND partition.
   Don’t forget to load the correct image according to the NAND flash erase sector size

    tftpboot 0x2000000 <image_name>.img
    ubi write 0x2000000 nand_rootfs \$filesize

  * Optional - to test the file system:
    ubifsmount nand_rootfs; ubifsls




Update image
============

To update image just execute:
   nand erase.part rootfs; ubi part rootfs; ubi create nand_rootfs
   Then step #3


eof
