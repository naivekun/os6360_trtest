#!

export ARCH=arm

#PATH=$PATH:/Marvell/SWITCH/AC3/TOOLS/le/armv7-marvell-linux-gnueabi-softfp-4.6.4_64K_i686_20160226/bin


PATH=$PATH:/Marvell/A38x/TOOLs/marvell-gcc-5.2.1-16.02.0/armv7/le/armv7-marvell-linux-gnueabi-softfp-5.2.1_i686_20151110/bin
export CROSS_COMPILE_BH=arm-marvell-linux-gnueabi-
export CROSS_COMPILE=arm-marvell-linux-gnueabi-
timestamp=`date +"%H%M-%m%0e-%Y"`
BOARD_NAME=WNC-OS6360-P48
BOOTDEV=spi

make mrproper

./build.pl -f $BOOTDEV -i spi:nand -b ac3s_rd -m 3 
#./build.pl -f $BOOTDEV -i nand -b ac3s_rd  -m 3 -c

cscope -Rb &
#
#
mv u-boot-msys-ac3-*-$BOOTDEV-uart.bin  ./u-boot-msys-ac3-$BOARD_NAME-$timestamp-$BOOTDEV-uart.bin
mv u-boot-msys-ac3-*-$BOOTDEV-debug.bin ./u-boot-msys-ac3-$BOARD_NAME-$timestamp-$BOOTDEV-debug.bin
mv u-boot-msys-ac3-*-$BOOTDEV.bin       ./u-boot-msys-ac3-$BOARD_NAME-$timestamp-$BOOTDEV.bin
#cp u-boot-a38x-3-15-nand-uart.bin /tftpboot
#
cp ./u-boot-msys-ac3-$BOARD_NAME-$timestamp-$BOOTDEV-uart.bin /tftpboot
cp ./u-boot-msys-ac3-$BOARD_NAME-$timestamp-$BOOTDEV.bin      /tftpboot

echo '***********************************************'
ls u-boot-msys-ac3-$BOARD_NAME-$timestamp-$BOOTDEV.bin
echo '***********************************************'

aplay /usr/share/sounds/speech-dispatcher/test.wav
