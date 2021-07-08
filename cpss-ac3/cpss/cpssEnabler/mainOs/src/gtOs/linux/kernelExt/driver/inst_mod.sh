#!/usr/bin/env bash
# parameters:

home_rootfs=$1
node_major=$2
node_minor=$3

init_sh=$home_rootfs/etc/init.sh

test -d $home_rootfs || exit 0

# copy module
unset MAKEFLAGS
KSUBD=$(make  -s -C $DIST_HOME kernelversion)
grep -q 'modules.*uname -r' $init_sh || KSUBD=.
test -d $home_rootfs/lib/modules/$KSUBD || mkdir -p $home_rootfs/lib/modules/$KSUBD
cp mvKernelExt.ko $home_rootfs/lib/modules/$KSUBD

# fix init.sh
if [ "$KSUBD" = "." -a -f "$init_sh" ]; then
	grep -q -e mvKernelExt -e module_f $init_sh ||
		sed -i \
			-e '/date/ i\' \
			-e ' test -f /lib/modules/mvKernelExt.ko &&\' \
			-e '    insmod /lib/modules/mvKernelExt.ko\' \
			-e '' \
			$init_sh
fi

# create node if it not exists yet
if [ -f $home_rootfs/../rootfs_deviceMap ]; then
	dm=$home_rootfs/../rootfs_deviceMap
	grep -q '^\/dev\/mvKernelExt' $dm >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		echo "/dev/mvKernelExt c 640 0 0 $node_major   $node_minor 0 0 -" >>$dm
	fi
fi
if [ -e $home_rootfs/dev/null ]; then
    cd $home_rootfs/dev
    if [ \! -c mvKernelExt ]
    then
        sudo mknod mvKernelExt c $node_major $node_minor
    fi
fi
