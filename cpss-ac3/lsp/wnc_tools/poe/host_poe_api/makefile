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

POE_API_PATH    = ../host_poe_api
POE_HAL_PATH    = ../host_poe_hal

OUTDIR=./bin
LIBDIR=./lib
AR_NAME=host_poe_api_lib.a
CC=arm-marvell-linux-gnueabi-gcc
AR=arm-marvell-linux-gnueabi-ar
CFLAGS_ARCH=-Wextra -Wall -Werror -g
CFLAGS=-Wextra -Wall -g  -ansi  #-Werror
ARFLAGS= cr
INCFLAGS=  -I$(POE_API_PATH)/inc -I$(POE_HAL_PATH)/inc
LD=$(CC)

#==========================================================================
# TARGETS
#==========================================================================

all:

	$(CC) $(CFLAGS) $(INCFLAGS) -c -o $(OUTDIR)/host_poe_communication.o $(POE_API_PATH)/src/host_poe_communication_.c
	$(AR) $(ARFLAGS) $(LIBDIR)/$(AR_NAME) $(OUTDIR)/host_poe_communication.o

	$(CC) $(CFLAGS) $(INCFLAGS) -c -o $(OUTDIR)/host_poe_client.o $(POE_API_PATH)/src/host_poe_client.c
	$(AR) $(ARFLAGS) $(LIBDIR)/$(AR_NAME) $(OUTDIR)/host_poe_client.o

	$(CC) $(CFLAGS) $(INCFLAGS) -c -o $(OUTDIR)/mscc_software_update.o $(POE_API_PATH)/src/mscc_software_update.c
	$(AR) $(ARFLAGS) $(LIBDIR)/$(AR_NAME) $(OUTDIR)/mscc_software_update.o

#==========================================================================
# RULES
#==========================================================================

clean:
	-rm -f $(LIBDIR)/*
	-rm -f $(OUTDIR)/*


