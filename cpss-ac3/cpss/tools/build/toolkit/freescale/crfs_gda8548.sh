#!/bin/bash

##############################################################
#                                                            #
#              crfs_gda8548.sh  - Create Root File System    #
#                                                            #
#          Written by Aharon Gadot <aharong@marvell.com>     #
#                        Oct 26, 2005                        #
#                                                            #
#       Building root filesystem for embedded Linux system   #
#                                                            #
##############################################################

echo ""

busybox_versions_supported="1.25.0"

Usage()
{
    echo "Usage: crfs_gda8548.sh <ROOTFS_PATH> <FREESCALE_PATH> <ARCH> <BUSYBOX_PATH>."
    echo " Create root filesystem for PowerPC CPU given by <ARCH>"
    echo " in path given by <ROOTFS_PATH>."
    echo " <BUSYBOX_PATH> must contain busybox-1.01.tar.bz2 or busybox-1.01.tar.gz file"
    echo " Busybox source can be downloaded from http://busybox.net/downloads/."
    echo " Possible <ARCH> - 85xx"
    echo " <FREESCALE_PATH> - path where freescale package was installed, ends with freescale-2011.03"
    echo ""
    echo "Example: "
    echo " crfs_gda8548.sh /tftpboot/rootfs_gda8548 /swtools/devtools/powerpc/freescale-2011.03 85xx /swtools/devsources/root_fs/files/busybox"
    echo ""
	exit $1
}

if [ "$1" = "--help" ]; then
    Usage 0
fi

if [ "$4" = "" ]; then
	echo -e "No enough parameters\n"
	Usage 1
fi

start_path=$1
freescale_path=$2
arch=$3
busybox_path=$4
HERE=`dirname $0`
HERE="`cd \"$HERE\" 2>/dev/null && pwd || echo \"$HERE\"`"

if [ -e $start_path ]; then
    echo "File system root: $start_path"
else
    echo "Creating file system root: $start_path"
    /bin/mkdir $start_path || exit 1
fi
if [ ! -e $freescale_path/bin/powerpc-linux-gnu-gcc ]; then
	echo "freescale toolkit not found in ${freescale_path}. Exiting..."
	exit 1
fi


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
/dev/mtd0        c 640 0 0  90   0 0 0 -
/dev/mtd1        c 640 0 0  90   2 0 0 -
/dev/mvPP        c 640 0 0 254   0 0 0 -
eof

# make character devices

# create links
ln -s ttyS0 ttys0
ln -s ttyS0 tty5
ln -s ttyp1 tty1
ln -s mtd0  mtd

echo "Building libraries"
# copy libraries
# --------------
cd $user_path
cd $start_path
cd lib

freescale_lib=$freescale_path/powerpc-linux-gnu/libc/lib
freescale_prefix=$freescale_path/bin/powerpc-linux-gnu-
freescale_usr_lib=$freescale_path/powerpc-linux-gnu/libc/usr/lib

cp $freescale_lib/ld-2.13.so .
cp $freescale_lib/libc-2.13.so .
cp $freescale_lib/libdl-2.13.so .
cp $freescale_lib/libpthread-2.13.so .
cp $freescale_lib/libthread_db-1.0.so .
cp $freescale_lib/libcrypt-2.13.so .
cp $freescale_lib/librt-2.13.so .
cp $freescale_lib/libm-2.13.so .
cp $freescale_lib/libSegFault.so .

# strip all libraries
$freescale_prefix-strip l*

ln -s ld-2.13.so 			ld.so.1
ln -s libc-2.13.so  		libc.so.6
ln -s libdl-2.13.so 		libdl.so.2
ln -s libpthread-2.13.so 	libpthread.so.0
ln -s libthread_db-1.0.so 	libthread_db.so.1
ln -s libcrypt-2.13.so 		libcrypt.so.1
ln -s librt-2.13.so 		librt.so.1
ln -s libm-2.13.so 			libm.so.6

echo "Creating init files"
# create init files
# -----------------
cd $user_path
cd $start_path
cd etc

# creating passwd
echo -e "root::0:0:root:/root:/bin/sh\n\nuser::500:500:Linux User,,,:/home/user:/bin/sh\n" >./passwd

# creating group
echo -e "root:x:1:root\nuser:x:500:\n" >./group
# creating inittab
dash=""
test "$bbversion" = "1.25.0" && dash="-"
cat << EOF > ./inittab
# autoexec
::respawn:${dash}/etc/init.sh

# Stuff to do when restarting the init process
::restart:${dash}/sbin/init
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

          On Marvell's $arch board

For further information on the Marvell products check:
http://www.marvell.com/


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

          On Marvell's $arch board

For further information on the Marvell products check:
http://www.marvell.com/


Enjoy!
EOF

# creating init.sh
current_date=$(date +%m%d%H%M%Y)

cat << EOF > ./init.sh
#!/bin/sh
if test -e /proc/version
then
 echo
else

 hostname MARVELL_LINUX
 HOME=/root
 mount -t proc   proc /proc
 mount -t sysfs  none /sys
 mount -t tmpfs  -o size=128M none /dev/shm
 mount -t devpts -o mode=0622 none /dev/pts

 /usr/sbin/telnetd -l /bin/sh -f /etc/welcome

 for module_f in /lib/modules/*.ko
 do
   if [ -e \$module_f ]; then
     insmod \$module_f
   fi
 done

 date $current_date

 # Start the network interface
 /sbin/ifconfig lo 127.0.0.1

 rm -f /tmp/tasks

fi

export LD_PRELOAD=/lib/libSegFault.so
# print logo
#clear
uname -nrsv
cat /etc/motd

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

chmod 755 init.sh

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
export HOST_EXTRACFLAGS="-Wno-strict-aliasing -Wno-unused-result -Wno-format-security"
make CONFIG_PREFIX=../. oldconfig
make CONFIG_PREFIX=../. all install
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
	${freescale_prefix}gcc -Wall -s $f -o $start_path/usr/bin/`basename $f .c`
done
# TODO: SatR


echo -e "\nCompilation completed.\n"

# remove Busybox sources
cd ..
rm -rf busybox*

echo ""
echo "Filesystem created successfuly"
echo ""
