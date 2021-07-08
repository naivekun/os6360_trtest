#*************************************************************************
#
#             Copyright (c) 2008 by Microsemi Corp. Inc.
#
#   This software is copyrighted by, and is the sole property of Microsemi
#   Corp. All rights, title, ownership, or other interests in the
#   software  remain the property of Microsemi Corp. This software
#   may only be used in accordance with the corresponding license
#   agreement.  Any unauthorized use, duplication, transmission,
#   distribution, or disclosure of this software is expressly forbidden.
#
#   This Copyright notice may not be removed or modified without prior
#   written consent of Microsemi Corp.
#
#   Microsemi Corp. reserves the right to modify this software without
#   notice.
#
# ************************************************************************
#
#   File Revision: 1.0
#
# ************************************************************************
#
#   Description: make file for PoE API project.
#
#
#*************************************************************************

#==========================================================================
# CONSTANTS
#==========================================================================

POE_HAL_PATH    = ../host_poe_hal

OUTDIR=./bin
LIBDIR=./lib_hal
AR_NAME=host_poe_hal_lib.a
CC=arm-marvell-linux-gnueabi-gcc
AR=arm-marvell-linux-gnueabi-ar
CFLAGS_ARCH=-Wextra -Wall -Werror -g
CFLAGS=-Wextra -Wall -g  -ansi  #-Werror
ARFLAGS= cr
INCFLAGS=  -I$(POE_HAL_PATH)/inc -I$(POE_HAL_PATH)/lib
LD=$(CC)

#==========================================================================
# TARGETS
#==========================================================================

all:

	$(CC) $(CFLAGS) $(INCFLAGS) -c -o $(OUTDIR)/host_poe_comm_device.o $(POE_HAL_PATH)/src/host_poe_comm_device.c
	$(AR) $(ARFLAGS) $(LIBDIR)/$(AR_NAME) $(OUTDIR)/host_poe_comm_device.o

	$(CC) $(CFLAGS) $(INCFLAGS) -c -o $(OUTDIR)/host_poe_comm_os.o $(POE_HAL_PATH)/src/host_poe_comm_os.c
	$(AR) $(ARFLAGS) $(LIBDIR)/$(AR_NAME) $(OUTDIR)/host_poe_comm_os.o


#==========================================================================
# RULES
#==========================================================================

clean:
	-rm -f $(LIBDIR)/*
	-rm -f $(OUTDIR)/*


