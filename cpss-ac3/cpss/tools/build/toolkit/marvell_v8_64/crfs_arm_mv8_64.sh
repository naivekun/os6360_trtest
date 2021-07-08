#!/bin/bash

##############################################################
#                                                            #
#          crfs_arm.sh  - Create Root File System            #
#                                                            #
#       Written by Sergei Shkonda <sergeish@marvell.com>     #
#                     Dec 29, 2016                           #
#                                                            #
#      Building root filesystem for embedded Linux system    #
#                                                            #
##############################################################
#
## functions
#
busybox_versions_supported="1.25.0"

usage_exit() {
	echo
	echo "Incorrect parameters"
	echo "Usage: crfs_arm_mv8_64.sh <ROOTFS_PATH> <MV8_64_PATH> <BUSYBOX_PATH>"
	echo
	echo "Examples: "
	echo
	echo
	echo " ./crfs_arm_mv8_64.sh /tftpboot/rootfs_arm-mv8_64 \\"
	echo "       /swtools/devtools/gnueabi/arm_le/armv7-marvell-linux-gnueabi-softfp \\"
	echo "       /swtools/devsources/root_fs/files/busybox "
	exit 1
}

#
## main
#

HERE=`dirname $0`
HERE="`cd \"$HERE\" 2>/dev/null && pwd || echo \"$HERE\"`"
echo ""
if [ -n "$1" ]
then
  if [ "$1" = "--help" ]
  then
	echo "Usage: crfs_arm_mv8_64.sh <ROOTFS_PATH> <MV8_64_PATH> <BUSYBOX_PATH>."
	echo " Create root filesystem for ARM CPU in path given by <ROOTFS_PATH>."
	echo " <BUSYBOX_PATH> must contain busybox-1.01.tar.bz2 or busybox-1.01.tar.gz file"
	echo " Busybox source can be downloaded from http://busybox.net/downloads/."
	echo " <MV8_64_PATH> - path where MV8_64 toolchain or newer package was installed."
	echo ""
	echo "Example: "
	echo " ./crfs_arm_mv8_64.sh /tftpboot/rootfs_arm-mv8_64 \\"
	echo "      /swtools/devtools/gnueabi/arm_le/armv7-marvell-linux-gnueabi-softfp \\"
	echo "      /swtools/devsources/root_fs/files/busybox "
	echo ""
	exit 0
  fi
fi

if [ $# -ne 3 ]
then
	usage_exit
fi

dname=`dirname "$1"`
bname=`basename "$1"`
start_path="`cd \"$dname\" 2>/dev/null && pwd || echo \"$dname\"`/$bname"
if [ -e $1 ]
then
	echo "File system root: $start_path"
else
	echo "Creating file system root: $start_path"
	/bin/mkdir $start_path
fi

dname=`dirname "$2"`
bname=`basename "$2"`
mv8_64="`cd \"$dname\" 2>/dev/null && pwd || echo \"$dname\"`/$bname"
if [ -e $mv8_64 ]
then
	echo "MV8_64 path: $mv8_64"
else
	echo "MV8_64 not found in $2. Exiting..."
	echo ""
	exit 1
fi

dname=`dirname "$3"`
bname=`basename "$3"`
busybox_path="`cd \"$dname\" 2>/dev/null && pwd || echo \"$dname\"`/$bname"

user_path=$(pwd)
cd $start_path
bbversion=""
for ver in $busybox_versions_supported
do
	if [ -e $busybox_path/busybox-$ver.tar.bz2 ]
	then
		bbversion=$ver
		echo "Uncompressing Busybox tar.bz2 archive..."
		tar xjf $busybox_path/busybox-$bbversion.tar.bz2
		echo -e "\bDone.\n"
		break
	elif [ -e $busybox_path/busybox-$ver.tar.gz ]
	then
		bbversion=$ver
		echo "Uncompressing Busybox tar.gz archive..."
		tar xzf $busybox_path/busybox-$bbversion.tar.gz
		echo -e "\bDone.\n"
		break
	fi
done
if [ "$bbversion" = "" ]
then
	echo "Busybox $busybox_versions_supported archive not found at path $busybox_path. Exiting..."
	echo ""
	exit 1
fi

#create file structure
#---------------------
mkdir bin
mkdir sbin
mkdir root
mkdir home
mkdir home/user
mkdir mnt
mkdir mnt/flash
mkdir mnt/nfs
mkdir etc
mkdir proc
mkdir sys
mkdir tmp
mkdir lib
mkdir lib/modules
mkdir usr
mkdir usr/lib
mkdir usr/bin
mkdir usr/local
mkdir usr/sbin
mkdir usr/share
mkdir var
mkdir var/lib
mkdir var/lock
mkdir var/log
mkdir var/run
mkdir var/tmp

echo "Creating devices"
#create structure of devices
#---------------------------
cd $start_path
mkdir dev
cd dev

#create dev folders
mkdir pts
mkdir shm
mkdir net

cat >../../rootfs_deviceMap <<eof
#<path>       <type> <mode> <uid> <gid> <major> <minor> <start> <inc> <count>
/dev/loop0       b 640 0 0   7   0 0 0 -
/dev/loop1       b 640 0 0   7   1 0 0 -
/dev/ram0        b 640 0 0   1   0 0 0 -
/dev/ram1        b 640 0 0   1   1 0 0 -
/dev/mtdblock0   b 640 0 0  31   0 0 0 -
/dev/mtdblock1   b 640 0 0  31   1 0 0 -
/dev/mtdblock2   b 640 0 0  31   2 0 0 -
/dev/mem         c 640 0 0   1   1 0 0 -
/dev/null        c 640 0 0   1   3 0 0 -
/dev/zero        c 640 0 0   1   5 0 0 -
/dev/random      c 640 0 0   1   8 0 0 -
/dev/ptyp0       c 640 0 0   2   0 0 0 -
/dev/ptyp1       c 640 0 0   2   1 0 0 -
/dev/ptyp2       c 640 0 0   2   2 0 0 -
/dev/ttyp0       c 640 0 0   3   0 0 0 -
/dev/ttyp1       c 640 0 0   3   1 0 0 -
/dev/ttyp2       c 640 0 0   3   2 0 0 -
/dev/tty0        c 640 0 0   4   0 0 0 -
/dev/ttyS0       c 640 0 0   4  64 0 0 -
/dev/tty         c 640 0 0   5   0 0 0 -
/dev/console     c 640 0 0   5   1 0 0 -
/dev/ptmx        c 640 0 0   5   2 0 0 -
/dev/sda         b 640 0 0   8   0 0 0 -
/dev/sda1        b 640 0 0   8   1 0 0 -
/dev/sda2        b 640 0 0   8   2 0 0 -
/dev/sda3        b 640 0 0   8   3 0 0 -
/dev/net/tun     c 640 0 0  10 200 0 0 -
/dev/mtd0        c 640 0 0  90   0 0 0 -
/dev/mtd0ro      c 640 0 0  90   1 0 0 -
/dev/mtd1        c 640 0 0  90   2 0 0 -
/dev/mtd1ro      c 640 0 0  90   3 0 0 -
/dev/mtd2        c 640 0 0  90   4 0 0 -
/dev/mtd2ro      c 640 0 0  90   5 0 0 -
#/dev/mvROS       c 640 0 0 250   0 0 0 -
#/dev/mvPP        c 640 0 0 244   0 0 0 -
#/dev/mvKernelExt c 640 0 0 244   1 0 0 -
/dev/i2c-0       c 640 0 0  89   0 0 0 -
/dev/i2c-1       c 640 0 0  89   1 0 0 -
#/dev/dragonite   c 640 0 0  10 240 0 0 -
eof
# create links
ln -s ttyS0 ttys0
ln -s ttyS0 tty5
ln -s ttyp1 tty1
ln -s mtd0  mtd

echo "Building libraries"
# copy libraries
# --------------
cd $start_path

cd usr/bin
usr_bin_path=`pwd`

cd ../../lib
lib_path=`pwd`

mv8_64_libc_root=$mv8_64/aarch64-linux-gnu/libc
mv8_64_lib=$mv8_64_libc_root/lib
mv8_64_lib2=$mv8_64_lib
mv8_64_prefix=$mv8_64/bin/aarch64-marvell-linux-gnu-

cd  $mv8_64_lib
cp -d libc-* libc.so* libm*.so*  ld-* libcrypt*.so* libpthread*.so* libdl*.so* librt*.so* libSegFault*.so* $lib_path

if [ "$COPY_GDBSERVER" != "" -a -f $mv8_64_libc_root/usr/bin/gdbserver ]; then
	cp -p $mv8_64_libc_root/usr/bin/gdbserver $lib_path/../usr/bin
	${mv8_64_prefix}strip $lib_path/../usr/bin/gdbserver
fi

cd  $mv8_64_lib2
cp -d libgcc_s* $lib_path

# strip the libraries except libthread_db
${mv8_64_prefix}strip ${lib_path}/l* > /dev/null 2>&1

cp -d $mv8_64_lib/libthread_db*.so* $lib_path

cd $lib_path
rm -f *orig*

echo "Creating etc files"
# create init files
# -----------------
cd $start_path
cd etc

# creating passwd
echo -e "root::0:0:root:/root:/bin/sh\n\nuser::500:500:Linux User,,,:/home/user:/bin/sh\n" >./passwd

# creating group
echo -e "root:x:1:root\nuser:x:500:\n" >./group
# creating inittab
cat << EOF > ./inittab
# autoexec
::respawn:-/etc/init.sh

# Stuff to do when restarting the init process
::restart:-/sbin/init
EOF

# creating motd
cat << EOF > ./motd


Welcome to Embedded Linux
           _  _
          | ||_|
          | | _ ____  _   _  _  _
          | || |  _ \\| | | |\\ \\/ /
          | || | | | | |_| |/    \\
          |_||_|_| |_|\\____|\\_/\\_/

          On Marvell's ARMADAXP board

For further information on the Marvell products check:
http://www.marvell.com/

toolchain=mv8_64 little endian

Enjoy!

EOF


# creating welcome file for telnet
cat << EOF > ./welcome


Welcome to Embedded Linux Telnet
           _  _
          | ||_|
          | | _ ____  _   _  _  _
          | || |  _ \\| | | |\\ \\/ /
          | || | | | | |_| |/    \\
          |_||_|_| |_|\\____|\\_/\\_/

          On Marvell's ARM board

toolchain=mv8_64 little endian

For further information on the Marvell products check:
http://www.marvell.com/
EOF

# creating init.sh
current_date=$(date +%m%d%H%M%Y)
current_date1=$(date)


# creating README.txt
cat << EOF > ./README.txt

File system building information
--------------------------------

build_date = ${current_date1}

sdk_prefix = ${mv8_64}

lib        = \${sdk_prefix}/aarch64-linux-gnu/libc/lib

cmd_prefix = \${sdk_prefix}/bin/aarch64-marvell-linux-gnu-

gdbserver  = ${gdbserver_file}

EOF

# creating init.sh

cat << EOF > ./init.sh
#!/bin/sh
if test -e /proc/version
then
 echo
else

 hostname MARVELL_LINUX
 HOME=/root

 test -d /dev/shm || mkdir /dev/shm
 test -d /dev/pts || mkdir /dev/pts

 mount -t proc   proc /proc
 mount -t sysfs  none /sys
 mount -t tmpfs -n -o size=128M none /dev/shm
 mount -t devpts -o mode=0622   none /dev/pts

 /usr/sbin/telnetd -l /bin/sh -f /etc/welcome

 kernel_ver=\`uname -r\`
 for module_f in /lib/modules/\${kernel_ver%%-*}/*.ko /lib/modules/*.ko
 do
   #create modules.dep file to make modinfo command run
   touch \`dirname \$module_f\`/modules.dep
   if [ -e \$module_f ]
   then
     insmod \$module_f
   fi
 done

 # change kernel console logging to warning
 echo 4 >/proc/sys/kernel/printk

date $current_date

 # Start the network interface
 /sbin/ifconfig lo 127.0.0.1

 rm -f /tmp/tasks

fi

export LD_PRELOAD=/lib/libSegFault.so

# print logo
# clear
echo
echo
echo
echo
uname -nrsv
cat /etc/motd


IFACE=eth1
read MAC </sys/class/net/$IFACE/address

if [ "${MAC}" != "00:00:00:00:00:02" ]; then

    count=0
    addr=00
    while [ $count -le 3 ]
    do
      addr=$addr:`grep -m1 -aoE '[0-9abcdef]{2}' /dev/urandom | head -n1`
            count=$(( $count + 1 ))
    done
    addr=${addr}':20'
    echo ifconfig eth1 hw ether $addr
         ifconfig eth1 hw ether $addr
fi

if test -e /usr/bin/appDemo
then
  if test -e /usr/local/auto_run.txt
  then
   echo PTK nfs, not running appDemo
  else
    /usr/bin/appDemo
  fi
else
  echo /usr/bin/appDemo not found
fi

exec /bin/sh
EOF

chmod 777 init.sh

# creating fstab
cat << EOF > ./fstab
/dev/nfs                /                       nfs     defaults        0 0
none                    /proc                   proc    defaults        0 0
none                    /sys                    sysfs   defaults        0 0
none                    /dev/shm                tmpfs   size=64M        0 0
none                    /dev/pts                devpts  mode=0622       0 0
EOF

# creating .config file for busybox
cd $start_path/busybox-$bbversion/

if [ -f $HERE/busybox-$bbversion.patch ]; then
	patch -p1 < $HERE/busybox-$bbversion.patch
fi
cp $HERE/busybox-$bbversion.config ./.config

# make and install Busybox
echo -e "Compiling Busybox application. This process may take several minutes.\nPlease wait...\n\n"
if [ "$bbversion" = "1.01" -o "$bbversion" = "1.2.1" ]
then
	#make TARGET_ARCH=arm CROSS=$mv8_64_prefix PREFIX=../. all install >/dev/null 2>/dev/null
	make TARGET_ARCH=arm CROSS=$mv8_64_prefix PREFIX=../. include/config.h
	make TARGET_ARCH=arm CROSS=$mv8_64_prefix PREFIX=../. all install
else
	# 1.25+
	export HOST_EXTRACFLAGS="-Wno-strict-aliasing -Wno-unused-result -Wno-format-security"
	make CONFIG_PREFIX=../. oldconfig
	make CONFIG_PREFIX=../. all install
fi
if [ $? -ne 0 ]; then
    echo -e "\nFailed to build busybox.\n"
    exit 1
fi
ln -s ./bin/busybox $start_path/init

echo -e "Build extra tools..."
for f in $HERE/*.c
do
	test -f $f || continue
	echo "  compile /usr/bin/`basename $f .c`"
	${mv8_64_prefix}gcc -Wall -s $f -o $start_path/usr/bin/`basename $f .c`
done
# SatR
if [ -f "$CPSS_PATH/tools/satr/Makefile" ]; then
	mkdir satr
	echo "  compile satr..."
	make -C $CPSS_PATH/tools/satr V=0 \
			ARCH=arm CROSS_COMPILE=$mv8_64_prefix \
			OUT_DIR=`pwd`/satr \
			TARGET_DIR=$start_path/usr/bin \
			install
fi


echo -e "\nCompilation completed.\n"

# remove Busybox sources
cd ..
rm -rf busybox*

echo ""
echo "Filesystem created successfuly"
echo ""

