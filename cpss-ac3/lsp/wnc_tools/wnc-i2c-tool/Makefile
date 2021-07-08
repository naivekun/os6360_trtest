# I2C tools for Linux
#
# Copyright (C) 2007-2012  Jean Delvare <jdelvare@suse.de>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

DESTDIR	=
prefix=/mnt/ac3/quagga/cpss-ac3/lsp/wnc_tools/wnc-i2c-tool
bindir	= $(prefix)/bin
sbindir	= $(prefix)/sbin
mandir	= $(prefix)/share/man
man8dir	= $(mandir)/man8
incdir	= $(prefix)/include
libdir	= $(prefix)/lib

INSTALL		:= install
INSTALL_DATA	:= $(INSTALL) -m 644
INSTALL_DIR	:= $(INSTALL) -m 755 -d
INSTALL_PROGRAM	:= $(INSTALL) -m 755
LN		:= ln -sf
RM		:= rm -f

CC	= arm-marvell-linux-gnueabi-gcc
AR	= arm-marvell-linux-gnueabi-ar

CFLAGS		?= -O2
# When debugging, use the following instead
CFLAGS		:= -O -g
CFLAGS		+= -Wall
SOCFLAGS	:= -fpic -D_REENTRANT $(CFLAGS)

USE_STATIC_LIB ?= 1
BUILD_STATIC_LIB ?= 1
ifeq ($(USE_STATIC_LIB),1)
BUILD_STATIC_LIB := 1
endif


.PHONY: all strip clean install uninstall

all:

SRCDIRS	:= include lib wnc-tool-src 
include $(SRCDIRS:%=%/Module.mk)

