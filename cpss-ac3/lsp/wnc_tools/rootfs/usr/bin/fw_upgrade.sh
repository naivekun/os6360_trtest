#!/bin/sh

usage() {
	printf "Usage: fw_upgrade.sh [file|link]\n"
	printf "Example: fw_upgrade.sh upgrade.img\n"
	printf "         fw_upgrade.sh http://192.168.1.1/upgrade.img\n"
}

if [ $# -eq 0 ]; then
	usage
	exit 1
fi

folder_tmp="/tmp"

if [ -n "$(echo $1 | grep http)" ]; then
	tarball_file_name="/tmp/image"
	wget $1 -O $tarball_file_name
else
	tarball_file_name=$1
fi

# Uncompress tarball and check MD5
untar_files=$(tar xvf $tarball_file_name -C $folder_tmp)

for file in buildroot.img uboot.img
do
	echo -n "Check $file ... "
	if [ -f $folder_tmp/$file ]; then
		cksum_file="$(md5sum $folder_tmp/$file)"
		if [ "$cksum_file" == "$(cat $folder_tmp/check | grep $cksum_file)" ]; then
			echo "FAIL"
			rm -rf $untar_files
			exit 2
		else
			echo "PASS"
			upgrade=$(echo $upgrade" /tmp/"$file)
		fi
	else
		echo "Not found"
	fi
done

if [ -n "$upgrade" ]; then
	echo "Move $(echo $upgrade | sed 's/\/tmp\///g') to /root"
	mv $upgrade /root
else
	echo "Can't find any image. Please check."
	exit 3
fi

echo "Reboot system and execute upgrade procedure."
sync
reboot
