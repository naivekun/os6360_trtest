#!/bin/bash

NAME=os6360
LINUX_FOLDER=linux/uni/4.4/kernel/mvebu_v7_lsp_noprestera_defconfig
ROOTFS=$LINUX_FOLDER/rootfs
BUILDROOTFS=$PWD/../buildroot/buildroot-2015.11-16.08.0/output/target
PYTHON_VENV=$BUILDROOTFS/usr/lib/python3.4
NGINX=$BUILDROOTFS/etc/nginx
NGINX_HTML=$BUILDROOTFS/usr/html
CUR_PATH=`pwd`

#AOS integration
FIT_FOR_AOS="output/Nadiag.img"
UBI_FLASH="output/Ndiag-$NAME.bin"
UBI_FLASH_BIN="output/ubi-Ndiag-$NAME.bin"
AOS_FIT="Nosa.img"
AOS_SW_LSM="software.lsm"

if [ ! -f $ROOTFS/usr/bin/appDemo ]; then
  echo Please build cpss first!
  exit -1
fi

# Copy corresponding libraries/binaries/symlinks of Traffic Generator GUI to rootfs folder
COMMON_LIB_SRC_LIST=(
  './usr/lib/libcrypto.so.1.0.0'
  './lib/libgcc_s.so.1'
  './usr/lib/libgd.so.3'
  './usr/lib/libjpeg.so.9'
  './usr/lib/libncurses.so.5'
  './lib/libnsl.so.1'
  './lib/libnss_dns.so.2'
  './lib/libnss_files.so.2'
  './usr/lib/libpcre.so.1'
  './usr/lib/libpng16.so.16'
  './usr/lib/libpython3.4m.so.1.0'
  './usr/lib/libreadline.so.6'
  './lib/libresolv.so.2'
  './usr/lib/libssl.so.1.0.0'
  './lib/libutil.so.1'
  './usr/lib/libxml2.so.2'
  './usr/lib/libxslt.so.1'
  './usr/lib/libz.so.1'
)

PYTHON_SRC_LIST=(
  './usr/bin/python3.4'
  './usr/bin/python3.4m'
  './usr/lib/libpython3.4m.so.1.0'
  './usr/lib/python3.4/lib-dynload/_csv.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/math.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_sha512.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/readline.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_testbuffer.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_socket.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_struct.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/parser.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_sha1.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/cmath.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/spwd.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_heapq.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/time.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/syslog.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/binascii.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/grp.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/fcntl.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_ctypes.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/termios.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_ssl.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/audioop.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_md5.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/select.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/array.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_testimportmultiple.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_opcode.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_crypt.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/mmap.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_sha256.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_multiprocessing.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_ctypes_test.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_hashlib.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_random.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/unicodedata.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_lsprof.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_json.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_pickle.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_datetime.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/xxlimited.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_testcapi.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_multibytecodec.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/resource.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/zlib.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_posixsubprocess.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_bisect.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_bz2.cpython-34m.so'
  './usr/lib/python3.4/lib-dynload/_elementtree.cpython-34m.so'
  './usr/lib/libpython3.so'
  './usr/share/vim/vim74/ftplugin/python.vim'
  './usr/share/vim/vim74/syntax/python.vim'
  './usr/share/vim/vim74/indent/python.vim'
  './usr/share/vim/vim74/autoload/pythoncomplete.vim'
  './usr/share/vim/vim74/autoload/python3complete.vim'
)

PYTHON_SYMLINK_LIST=(
  './usr/bin/python3->python3.4'
  './usr/bin/python->python3'
  './usr/lib/libpython3.4m.so->libpython3.4m.so.1.0'
)

NGINX_SRC_LIST=(
  './usr/sbin/nginx'
  './etc/nginx/nginx.conf'
  './etc/nginx/nginx.conf.default'
  './etc/logrotate.d/nginx'
)

SQLITE_SRC_LIST=(
  './usr/bin/sqlite3'
  './usr/lib/libsqlite3.so.0.8.6'
)

SQLITE_SYMLINK_LIST=(
  './usr/lib/libsqlite3.so.0->libsqlite3.so.0.8.6'
  './usr/lib/libsqlite3.so->libsqlite3.so.0.8.6'
)

SSH_SRC_LIST=(
  './usr/bin/ssh-keygen'
  './usr/bin/ssh-keyscan'
  './usr/bin/ssh-agent'
  './usr/bin/ssh'
  './usr/bin/ssh-add'
  './usr/sbin/sshd'
  './usr/share/vim/vim74/ftplugin/sshconfig.vim'
  './usr/share/vim/vim74/syntax/sshconfig.vim'
  './usr/share/vim/vim74/syntax/sshdconfig.vim'
  './usr/libexec/ssh-keysign'
  './usr/libexec/ssh-pkcs11-helper'
  './etc/ssh/sshd_config'
  './etc/ssh/ssh_config'
)

function install_file_or_symlink() {
  local target_file=$2
  local link_source=$3
  
  case "$1" in
    file)
      if [ ! -f $BUILDROOTFS/$target_file ]; then
        echo $BUILDROOTFS/$target_file does not exist, it cannot install!
      else
        mkdir -p `dirname $ROOTFS/$target_file`
        cp -v $BUILDROOTFS/$target_file $ROOTFS/$target_file
      fi
    ;;
    symlink)
      if [ ! -f `dirname $ROOTFS/$target_file`/$link_source ]; then
        echo `dirname $ROOTFS/$target_file`/$link_source does not exist, it cannot create a symlink!
      else
        (cd `dirname $ROOTFS/$target_file`;ln -svf $link_source `basename $target_file`)
      fi
    ;;
    *)
      echo "Invalid input type $1"
    ;;
  esac
}

# Copy common libraries of Traffic Generator GUI to rootfs
for target in ${COMMON_LIB_SRC_LIST[@]}; do
  install_file_or_symlink "file" $target
done

# Copy python binaries of Traffic Generator GUI to rootfs
for target in ${PYTHON_SRC_LIST[@]}; do
  install_file_or_symlink "file" $target
done

# Copy python symlinks of Traffic Generator GUI to rootfs
for target in ${PYTHON_SYMLINK_LIST[@]}; do
  sentence="${target//$'->'/' '}"
  stringarray=($sentence)
  install_file_or_symlink "symlink" ${stringarray[0]} ${stringarray[1]}
done

# Copy nginx binaries of Traffic Generator GUI to rootfs
for target in ${NGINX_SRC_LIST[@]}; do
  install_file_or_symlink "file" $target
done

# Copy sqlite binaries of Traffic Generator GUI to rootfs
for target in ${SQLITE_SRC_LIST[@]}; do
  install_file_or_symlink "file" $target
done

# Copy sqlite symlinks of Traffic Generator GUI to rootfs
for target in ${SQLITE_SYMLINK_LIST[@]}; do
  sentence="${target//$'->'/' '}"
  stringarray=($sentence)
  install_file_or_symlink "symlink" ${stringarray[0]} ${stringarray[1]}
done

# Copy ssh binaries of Traffic Generator GUI to rootfs
for target in ${SSH_SRC_LIST[@]}; do
  install_file_or_symlink "file" $target
done

# Copy Python venv and Nginx config/html
cp -rv $PYTHON_VENV $ROOTFS/usr/lib/
cp -rv $NGINX $ROOTFS/etc
cp -rv $NGINX_HTML $ROOTFS/usr

mkdir -p $ROOTFS/var/www
mkdir -p $ROOTFS/etc/init.d
mkdir -p $ROOTFS/var/log/nginx/
mkdir -p $ROOTFS/var/tmp/nginx/
mkdir -p $ROOTFS//var/empty

cp -v $BUILDROOTFS/etc/hostname $ROOTFS/etc/hostname
cp -v $BUILDROOTFS/etc/hosts $ROOTFS/etc/hosts
cp -v $BUILDROOTFS/etc/init.d/S50nginx $ROOTFS/etc/init.d/
cp -v $BUILDROOTFS/etc/init.d/S50sshd $ROOTFS/etc/init.d/

# create corresponding user/group
cat << EOF > $ROOTFS/etc/passwd
root:x:0:0:root:/root:/bin/sh
daemon:x:1:1:daemon:/usr/sbin:/bin/false
bin:x:2:2:bin:/bin:/bin/false
sys:x:3:3:sys:/dev:/bin/false
sync:x:4:100:sync:/bin:/bin/sync
mail:x:8:8:mail:/var/spool/mail:/bin/false
www-data:x:33:33:www-data:/var/www:/bin/false
operator:x:37:37:Operator:/var:/bin/false
nobody:x:99:99:nobody:/home:/bin/false
sshd:x:1000:1000:SSH drop priv user:/:/bin/sh
ftp:x:1001:1001:Anonymous FTP User:/home/ftp:/bin/false
EOF

cat << EOF > $ROOTFS/etc/shadow
root::10933:0:99999:7:::
daemon:*:10933:0:99999:7:::
bin:*:10933:0:99999:7:::
sys:*:10933:0:99999:7:::
sync:*:10933:0:99999:7:::
mail:*:10933:0:99999:7:::
www-data:*:10933:0:99999:7:::
operator:*:10933:0:99999:7:::
nobody:*:10933:0:99999:7:::
sshd:*:::::::
ftp:*:::::::
EOF

cat << EOF > $ROOTFS/etc/group
root:x:0:
daemon:x:1:
bin:x:2:
sys:x:3:
adm:x:4:
tty:x:5:
disk:x:6:
lp:x:7:
mail:x:8:
kmem:x:9:
wheel:x:10:root
cdrom:x:11:
dialout:x:18:
floppy:x:19:
video:x:28:
audio:x:29:
tape:x:32:
www-data:x:33:
operator:x:37:
utmp:x:43:
plugdev:x:46:
staff:x:50:
lock:x:54:
netdev:x:82:
nogroup:x:99:
users:x:100:
sshd:x:1000:
ftp:x:1001:
EOF

# Create a new ramdisk.cpio.gz
rm -rf $LINUX_FOLDER/rootfs/flash
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
cp -f $FIT_FOR_AOS output/Nadiag_GUI.img
cp -f $FIT_FOR_AOS $LINUX_FOLDER/rootfs/flash/diags

mkfs.ubifs -F -m 4KiB -e 248KiB -c 3600 -r $LINUX_FOLDER/rootfs/flash -x none -vv $UBI_FLASH

#Make ubi image
UBINIZE_CFG="output/ubinize.cfg"

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
