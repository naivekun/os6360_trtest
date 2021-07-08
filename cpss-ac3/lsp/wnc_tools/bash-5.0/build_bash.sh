#!/bin/sh

#Configure
CC=arm-linux-gnueabi-gcc ./configure --host=arm-linux --target=arm-linux --enable-static-link --enable-history --without-bash-malloc

#Update -static into CFLAGS
sed -i '/^CFLAGS =/ s/$/ -static/' Makefile

#Make bash
make

