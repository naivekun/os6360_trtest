#*******************************************************************************
#                   Copyright 2002, GALILEO TECHNOLOGY, LTD.                   *
# THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL.                      *
# NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
# OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
# DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
# THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
# IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
#                                                                              *
# MARVELL COMPRISES MARVELL TECHNOLOGY GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, *
# MARVELL INTERNATIONAL LTD. (MIL), MARVELL TECHNOLOGY, INC. (MTI), MARVELL    *
# SEMICONDUCTOR, INC. (MSI), MARVELL ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K.  *
# (MJKK), GALILEO TECHNOLOGY LTD. (GTL) ,GALILEO TECHNOLOGY, INC. (GTI). and   *
# RADLAN COMPUTER COMMUNICATIONS LTD. (RND).
#*******************************************************************************
# Makefile for Linux
#
# DESCRIPTION:
#
#*******************************************************************************

######### Debug level selector  #########
DEBUG_AND_OPTIMIZE_LEVEL	= -O0 -g

HOME_INITRD = $(DIST_HOME)/rootfs

ifndef COMPILATION_ROOT
	COMPILATION_ROOT = $(USER_BASE)
endif
COMPILATION_ROOT_FIX = $(subst \,/,$(COMPILATION_ROOT))

ifeq (, $(wildcard $(USER_BASE)/tools/build/toolkit/$(TOOLKIT_MAKE_RULES)Defs.mk))
$(error "Make defs (build/toolkit/$(TOOLKIT_MAKE_RULES)Defs.mk) not found)")
endif
include $(USER_BASE)/tools/build/toolkit/$(TOOLKIT_MAKE_RULES)Defs.mk


CFLAGS += $(CFLAGS1) $(CFLAGS2) $(CFLAGS_OPT) $(OS_DEFINE) $(CPU_FLAGS) $(LFLAGS) $(AD_CFLAGS) $(CDEBUG)
ifeq (D_ON,$(DEBUG_INFO))
  CFLAGS += $(DEBUG_AND_OPTIMIZE_LEVEL)
endif

ifeq (YES,$(IS_64BIT_OS))                          # new linux platform - 64 bit
  LDFLAGS = -L$(XCOMP_ROOT_PATH)/usr/lib${LIB_1} -lpthread -ldl -rdynamic
else                                               # build 32 bits on 32 bits platform
  LDFLAGS = -L$(XCOMP_ROOT_PATH)/usr/lib -lpthread -ldl -rdynamic
endif
#LDFLAGS += $(XCOMP_ROOT_PATH)${XCOMP_ROOT_PATH_1}/usr/lib${LIB_1}/librt.a
LDFLAGS += -lm


# define object files get to application
OBJ_FILES = $(COMPILATION_ROOT_FIX)/mcd/libs/$(CPU_DIR)/*.a

APPNAME=mcdSample
all : $(APPNAME)
$(APPNAME) :
	echo " begin $(APPNAME)"
	rm -f ${COMPILATION_ROOT_FIX}/$(APPNAME)
	$(CC) $(CFLAGS) $(LDFLAGS) -o ${COMPILATION_ROOT_FIX}/$(APPNAME) $(OBJ_FILES)
ifneq (,$(DIST_HOME))
ifneq (YES, $(APP_DEMO_ONLY))	
ifeq (linux, $(OS_RUN))
ifneq (NO, $(LINUX_BUILD_KERNEL))
	echo cp ${COMPILATION_ROOT_FIX}/$(APPNAME) $(HOME_INITRD)/usr/bin/$(APPNAME)
	cp ${COMPILATION_ROOT_FIX}/$(APPNAME) $(HOME_INITRD)/usr/bin/$(APPNAME)
	echo -e "\t"$(STRIP)" "$(HOME_INITRD)"/usr/bin/$(APPNAME)"
	$(STRIP) $(HOME_INITRD)/usr/bin/$(APPNAME)
	chmod 755 $(HOME_INITRD)/usr/bin/$(APPNAME)
endif
endif
endif
endif
