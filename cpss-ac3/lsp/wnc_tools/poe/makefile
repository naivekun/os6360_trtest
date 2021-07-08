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
POE_API_PATH    = host_poe_api
POE_HAL_PATH    = host_poe_hal


SUB20_LIB_PATH       = $(POE_HAL_PATH)/lib
SUB20_LIB            = $(SUB20_LIB_PATH)/i2cbusses.a
OUTDIR               = ./bin


CC=arm-marvell-linux-gnueabi-gcc
STRIP = arm-marvell-linux-gnueabi-strip
#AR=ar
CFLAGS= -Wall -Wextra -g -fPIC -c -D__SUB_STATIC__
#CFLAGS_ARCH=-Wextra -Wall -Werror -g
#CFLAGS=-Wextra -Wall -g  -ansi  #-Werror
#ARFLAGS= cr
INCFLAGS=  -I./  \
	-I$(POE_HAL_PATH)/inc \
	-I$(POE_API_PATH)/inc \
	-I$(SUB20_LIB_PATH) \
	-I../libusb/libusb-win32-src/src

LD=$(CC)
LDFLAGS := -lpthread -L$(SUB20_LIB_PATH) -L/usr/local/lib -L../i2c-tools-4.0/lib -lm -ldl -lrt

#==========================================================================
# TARGETS
#==========================================================================

all:

# Clean
	$(MAKE) -C $(SUB20_LIB_PATH) clean
	$(MAKE) -C $(POE_API_PATH) clean
	$(MAKE) -C $(POE_HAL_PATH) clean
	-rm -rf $(OUTDIR)/*
	-rm -rf $(SUB20_LIB_PATH)/*.o


	# run make files
	$(MAKE) -C $(SUB20_LIB_PATH)
	$(MAKE) -C $(POE_API_PATH)
	$(MAKE) -C $(POE_HAL_PATH)

	-cp -f $(SUB20_LIB) $(OUTDIR)

	$(LD) -o $(OUTDIR)/poe 			\
	$(POE_API_PATH)/lib/host_poe_api_lib.a 	\
		  $(POE_HAL_PATH)/lib_hal/host_poe_hal_lib.a \
		  $(OUTDIR)/i2cbusses.a 	\
		  -ldl $(LDFLAGS)
	cp $(OUTDIR)/poe $(OUTDIR)/poe_dbg
	$(STRIP) $(OUTDIR)/poe
	-rm -rf $(OUTDIR)/*.o

#==========================================================================
# RULES
#==========================================================================

clean:
	$(MAKE) -C $(SUB20_LIB_PATH) clean
	$(MAKE) -C $(POE_API_PATH) clean
	$(MAKE) -C $(POE_HAL_PATH) clean
	-rm -rf $(OUTDIR)/*
	-rm -rf $(SUB20_LIB_PATH)/*.o
	rm -rf $(SUB20_LIB_PATH)/*.o
	-rm -rf $(POE_API_PATH)/*.o
	-rm -rf $(POE_HAL_PATH)/*.o





