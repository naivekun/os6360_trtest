#!/bin/sh
PATH=/sbin:/bin:/usr/sbin:/usr/bin
#LSOCKET_TO_CPSS="/tmp/lsocket_to_cpss"

if test -e /proc/version; then
  echo
else
  hostname MARVELL_LINUX
  HOME=/root
  mount -t devtmpfs devtmpfs /dev
  test -d /dev/shm || mkdir /dev/shm
  test -d /dev/pts || mkdir /dev/pts
  
  mount -t proc   proc /proc
  mount -t sysfs  none /sys
  test -d /dev/shm || mkdir /dev/shm
  mount -t tmpfs  -n -o size=128M none /dev/shm
  mount -t devpts -o mode=0622    none /dev/pts
  
  if [ -f /bin/bash ]; then
    /usr/sbin/telnetd -l /bin/bash -f /etc/welcome
  else
    /usr/sbin/telnetd -l /bin/sh -f /etc/welcome
  fi
  
  kernel_ver=`uname -r`
  for module_f in /lib/modules/${kernel_ver%%-*}/*.ko /lib/modules/*.ko
  do
    if [ -e $module_f ]; then
      insmod $module_f
    fi
  done
  
  date 012008402020
  
  # Start the network interface
  /sbin/ifconfig lo 127.0.0.1
fi

#Create I2C device node
/usr/sbin/i2c_node

#Mount the diagnostic log partition
/usr/sbin/mnt_diag_log

#Identify SKU
/usr/sbin/identify_sku

# Create file for MFG usage
if [ -f /flash/Rj45LoopbackStub ]; then
  mkdir -p /usr/data
  touch /usr/data/mfg_mode
fi

#Start CPSS
ulimit -c unlimited
echo "/flash/diags/core.%e.%p.%t" > /proc/sys/kernel/core_pattern
dmesg -n 4
#/usr/bin/appDemo -daemon -config /etc/config.txt > /dev/null 2>&1

# NOTE that init_sdk needs to be called after
# identify_sku (need SKU name) and check /flash/diags/Rj45LoopbackStub for MFG mode
if test -e /usr/sbin/init_sdk
then
  /usr/sbin/init_sdk
else
  echo /usr/sbin/init_sdk not found
fi

#LSP_VERS
dmesg -s 32768 |grep -i LSP > /dev/shm/LSP_VERS.TXT
test -s /dev/shm/LSP_VERS.TXT || uname -a > /dev/shm/LSP_VERS.TXT

#Get post info
/usr/sbin/show_post

#Set GPIO for Lattice CPLD online upgrade function
/usr/sbin/jtag_config

#Initial diag
if [ -f /opt/diag/init_diag ]; then
  /opt/diag/init_diag
fi

#Record chip info
/usr/sbin/record_flash_info

#Init for traffic tester
/usr/sbin/init_trtest

# Enable SYS LED power switch
echo 1 > /sys/bus/i2c/devices/0-007f/sys_led_pwr_ctrl_en

# Disable watchdog
echo 0 > /sys/bus/i2c/devices/0-007f/wdt_rst_en

# Launch Nginx and SSHD
if [ -f /etc/init.d/S50nginx ]; then
  /etc/init.d/S50nginx start
fi

if [ -f /etc/init.d/S50sshd ]; then
  /etc/init.d/S50sshd start
fi

if [ -f /flash/diags/wnc.sh ]; then
  /flash/diags/wnc.sh
fi

cat /etc/welcome
