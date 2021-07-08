#!/bin/sh

##############################################################
#                                                            #
#           cr_jffs2_img_gda8548.sh  - Create JFFS2 image        #
#                                                            #
#          Written by Aharon Gadot <aharong@marvell.com>     #
#                        Oct 27, 2005                        #
#                                                            #
#          Building JFFS2 image from root filesystem         #
#                                                            #
##############################################################

echo ""

Usage() {
	echo "Usage: cr_jffs2_img_gda8548.sh <IMAGE_PATH> <ROOTFS_PATH> <FLASH_BLOCK_SIZE>."
    echo " Create JFFS2 image in derectory /tftpboot from root filesystem for GDA8548 CPU (little endian)"
	echo " <IMAGE_PATH> - where to save JFFS2 image."
    echo " Root fs path given by <ROOTFS_PATH>. Don't forget to copy uImage to filesystem root."
    echo ""
    echo "Example: "
	  echo " cr_jffs2_img_gda8548.sh /tftpboot /tftpboot/rootfs_gda8548 128"
    echo ""
	exit $1
}

if [ "x$1" = "x--help" ]; then
	Usage 0
fi
if [ "x$3" = "x" ]; then
	Usage 1
fi
image_path=$1
rootfs_path=$2
flash_block_size=$3

# check path where JFFS2 image will be placed
test -d $image_path/. || {
	echo "Creating durectory $image_path for JFFS2 image"
	mkdir $image_path
}
if [ \! -d $image_path/. ]; then
  echo -e "Destination path is missing."
  Usage 1
fi

if [ \! -d $rootfs_path/bin ]; then
    echo "No file system found at $rootfs_path. Exiting."
    exit 1
fi
echo "File system root: $rootfs_path"

# generate MKDEV.sh script
cat >$rootfs_path/dev/MKDEV.sh <<eof
#!/bin/bash
if [ \`whoami\` = "root" ]; then
   sudo=
else
   sudo=sudo
fi
eof
awk '/^\/dev\// {
    printf "$sudo mknod %-12s %s %3s %3s\n",substr($1,6),$2,$6,$7
}' $rootfs_path/../rootfs_deviceMap >> $rootfs_path/dev/MKDEV.sh

# create JFFS2 image using mkfs.jffs2 tool
mkfs.jffs2 --eraseblock=128KiB  -n -b -d $rootfs_path -o $image_path/jffs2_gda8548.image

echo ""
echo "Filesystem JFFS2 image $image_path/jffs2_gda8548.image created successfuly."
echo ""
echo "Use next commads from U-Boot to place it on flash:"
echo "-->erase [start] [end]"
echo "-->tftpboot [ram_address] jffs2_gda8548.image"
echo "-->cp.b [ram_address] [start] [size]"
echo ""





