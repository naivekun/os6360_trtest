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
# Makefile for Linux and uClinux
#
# DESCRIPTION:
#       This file contains rules for building PSS for
#           Linux and uClinux on the specified board
#           using the tool chain environment. (refer to LSP manual.)
#
#*******************************************************************************

ifneq (,$(findstring mips,$(sysroot)))
        # Found
        SYSROOT_FLAG = --sysroot=$(sysroot)
else
        # Not found
        SYSROOT_FLAG =
endif
$(info "SYSROOT_FLAG=$(SYSROOT_FLAG)")

######### Debug level selector #########
DEBUG_AND_OPTIMIZE_LEVEL  = -O0 -g
##DEBUG_AND_OPTIMIZE_LEVEL  = -O2

VERSION_DATE = $(shell date "+%Y-%m-%d %T")

#the compilation of the 'RTOS on SIM' should be distinguished from 'RTOS'(non simulation)
ifeq (EXISTS, $(RTOS_ON_SIM))
  TMP_POSTFIX=_SIM
else
  TMP_POSTFIX=
endif

## Only redefine make definitions below this point, or your definitions will
## be overwritten by the makefile stubs above.
## Should update the kernel
VENDOR    = Marvell
HOME_INITRD = $(DIST_HOME)/rootfs

APPDEMO_IMAGE_NAME=appDemo
APPDEMOSIM_IMAGE_NAME=appDemoSim
HIR_APPDEMO_IMAGE_NAME=hir_appDemo
HIR_APPDEMOSIM_IMAGE_NAME=hir_appDemoSim

RELEASE   = application

#specify extra components & include path
EXTRA_INCLUDE =
LIB_EXTRA =
MACH_EXTRA =

#specify components for cpss as lib
CPSS_LIB = cpssDrvLib.a

COMMON_LIBS_LB =
MACH_EXTRA_LB =
LIB_EXTRA_LB =

include $(USER_BASE)/cpssCommon.mk

# DO NOT CHANGE libraries order
# IT IS important for successfull link
LIB_EXTRA_LB += $(UTF_LIB) $(COMPILATION_ROOT_FIX)/$(CPSS_LIB)



#add simulation libraries - must be 3 times for link pass
ifeq (1, $(LINUX_SIM))

LIB_EXTRA += $(SIM_LIB)
LIB_EXTRA += $(SIM_LIB)
LIB_EXTRA += $(SIM_LIB)

endif


#specify files for cpss as lib
#MACH_EXTRA_LB += $(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/cpssEnabler.o
MACH_EXTRA_LB += $(CPSS_ENABLER_FILES) \
  $(UTIL_FILES) \
  $(GALTIS_FILES) \
  $(LUA_WRAPPER_FILES)

  # \
  #$(TM_API_FILES)

  ifeq (YES, $(INCLUDE_TM))
    MACH_EXTRA_LB += $(TM_API_FILES)
  endif



CFLAGS = $(CC_ARCH)
ifeq (D_ON,$(DEBUG_INFO))
  CFLAGS += $(DEBUG_AND_OPTIMIZE_LEVEL)
  VERSION_FLAGS += DEBUG
endif

#Set linker flags
LDFLAGS := $(LDFLAGS_CC)
ifeq (YES,$(IS_64BIT_OS))                          # new linux platform - 64 bit
   ifeq (32,$(WORDSIZE_REQUIRED))                  # build 32 bits on 64 bits platform
      CFLAGS2 += -m32
      LDFLAGS += -L$(XCOMP_ROOT_PATH)/usr/lib32 -m32
      # if $(GCC_VER <= 4.6.*)
      ifeq (, $(filter-out 3.% 4.0.% 4.1.% 4.2.% 4.3.% 4.4.% 4.5.% 4.6.%,$(GCC_VER)))
          LDFLAGS += -melf_i386
      endif
   else                                            # build 64 bits on 64 bits platform
      CFLAGS2 += -D__WORDSIZE=64
      LDFLAGS += -L$(XCOMP_ROOT_PATH)/usr/lib${LIB_1}
   endif
else
    ifneq (,$(findstring /usr/,$(XCOMP_ROOT_PATH)))
        # Found
        LDFLAGS += -L$(XCOMP_ROOT_PATH)lib
    else
        # Not found
        LDFLAGS += -L$(XCOMP_ROOT_PATH)/usr/lib
    endif


endif
LDFLAGS += -lpthread -ldl -rdynamic -lrt -lm

ifeq (1, $(LINUX_SIM))
   VERSION_FLAGS += LINUX_SIM
   ifneq (EXISTS, $(RTOS_ON_SIM))
      ifeq (EXISTS, $(FORCE_GM_USE))
         LDFLAGS += -lstdc++
      endif
   endif
endif

# define object files get to cpss lib
OBJ_FILES_01 = $(shell find $(COMPILATION_ROOT_FIX)/common -type f -a -name '*.o' -a ! -name common.o)


ifeq (EXISTS, $(PX_FAMILY))
        VERSION_FLAGS += PX_FAMILY
        OBJ_FILES_02 = $(shell find $(COMPILATION_ROOT_FIX)/mainPxDrv -type f -a -name '*.o')
        OBJ_FILES_03 =
else

        OBJ_FILES_02 = $(shell find $(COMPILATION_ROOT_FIX)/mainPpDrv -type f -a -name '*.o')

  ifeq (YES, $(INCLUDE_TM))
    VERSION_FLAGS += INCLUDE_TM
    OBJ_FILES_03 = $(shell find $(COMPILATION_ROOT_FIX)/mainTmDrv -type f -a -name '*.o' -a ! -name mainTmDrv.o)
  else
    OBJ_FILES_03 =
  endif

endif


ifeq (YES, $(CPP_LINK))
    LINKER=$(GPP)
else
    LINKER=$(CC)
endif

#Set build all rules
ifeq (1, $(SHARED_MEMORY))
VERSION_FLAGS += SHARED
# Defines the specific declarations and targets for "shared memory" CPSS and libs
include sharedMemMakefile.mk
else
all : cpss application
endif

#Build cpss lib and appDemo
all_lb: cpss cpss_lib appDemo_lb

#build cpss lib only
cpss_lb: cpss cpss_lib

#-------------------------------------------------
HIR_CODE_BASE=
$(info DEBUG LINUX_SIM $(LINUX_SIM))
#LINUX_SIM may have 1, 0
#or might not have been defined also.
#So casting it to proper boolean LINUX_SIM

ifeq (1, $(LINUX_SIM))
LINUX_SIM_01 = 1
else
LINUX_SIM_01 = 0
endif

ifeq  (0, ${LINUX_SIM_01})
ifeq  (1, $(GHIR_TYPE))
        HIR_CODE_BASE=hirSysConfig/
        APP_NAME = hir_appDemo
else
        APP_NAME = appDemo
endif
endif

ifeq  (1, ${LINUX_SIM_01})
ifeq  (1, $(GHIR_TYPE))
        HIR_CODE_BASE=hirSysConfig/
        APP_NAME = hir_appDemoSim
else
        APP_NAME = appDemoSim
endif
endif
#-----------------------------------------------------
ifneq (1, $(LINUX_SIM))
application : appDemo
else
application : appDemoSim
endif
#-----------------------------------------------------
#Build CPSS
cpss :
	echo " PREFIX - ${COMP_PREFIX}"
	$(MAKE) -f presteraTopMake full

cpss_lib:
	echo "Building cpss_lib"
	$(AR) crus $(COMPILATION_ROOT_FIX)/$(CPSS_LIB) $(OBJ_FILES_02)
	$(AR) rcs $(COMPILATION_ROOT_FIX)/$(CPSS_LIB) $(OBJ_FILES_01)

cpss_blob:
	echo "Building cpss_blob"
	$(LD) -r $(BUILD_MAP_FILE) $(COMPILATION_ROOT_FIX)/cpss_blob.map $(FO) $(COMPILATION_ROOT_FIX)/cpss_blob.o $(OBJ_FILES_01) $(OBJ_FILES_02) $(OBJ_FILES_03)
	$(NM) $(COMPILATION_ROOT_FIX)/cpss_blob.o \
    | sed -n 's/^ *U //p' | grep -v -e '^__aeabi_' -e '^memcpy$$' -e '^memset$$' \
    | sort -u > $(COMPILATION_ROOT_FIX)/unresolved_symbols.txt
	test \! -s $(COMPILATION_ROOT_FIX)/unresolved_symbols.txt
#--------------------------------------------------------------------------------------------------------------------------------------------------------------
#Build appDemo
appDemo_pre_link: $(COMPILATION_ROOT_FIX)/version_info.o
ifneq (silent,$(COMPILATION_TYPE))
	echo " begin appDemo ${APP_NAME}"
endif
	echo "my appDemo ${APP_DEMO} ${GHIR_TYPE} $(LINUX_SIM)"
	rm -f ${COMPILATION_ROOT_FIX}/${APP_NAME}

ifeq (YES,$(STATIC_LINKED_APPDEMO))
# Create symtable for static linked appDemo
appDemo_link = appDemo_link_static
appDemo_create_symtable: appDemo_pre_link
	echo "int __SymbolTable;int __SymbolTableLength; "> $(COMPILATION_ROOT_FIX)/symtable.c
	cd $(COMPILATION_ROOT_FIX) && \
$(LINKER) -o ${COMPILATION_ROOT_FIX}/${APP_NAME} $(CFLAGS) \
  -Xlinker --whole-archive \
    $(subst $(COMPILATION_ROOT_FIX)/,,$(sort $(MACH_EXTRA) $(LIB_EXTRA))) \
  -Xlinker --no-whole-archive \
  symtable.c $(LDFLAGS)
# generate map file
# 1. Generate list
# x86_64
	cd $(COMPILATION_ROOT_FIX) &&  \
$(NM) ${APP_NAME} |sed -n 's/^.* T \(.*\)$$/\1/p'|LANG=C sort -u >symtable.list && \
cat symtable.list |sed -n 's/^\(.*\)$$/int \1();/p' >symtable.c && \
echo "struct { const char *name; int (*funcptr)();} __SymbolTable[] = {" >>symtable.c && \
cat symtable.list |sed -n 's/^\(.*\)$$/ { "\1", \1 },/p' >>symtable.c && \
echo " { (void*)0L, (void*)0L }" >>symtable.c && \
echo "};" >>symtable.c && \
echo "int __SymbolTableLength = sizeof(__SymbolTable)/sizeof(__SymbolTable[0])-1;" >>symtable.c
	cd $(COMPILATION_ROOT_FIX) &&  \
$(CC) $(CFLAGS) -c -o symtable.o symtable.c 2>/dev/null

appDemo_link_static: appDemo_create_symtable
  cd $(COMPILATION_ROOT_FIX) && \
$(LINKER) -o ${COMPILATION_ROOT_FIX}/appDemo $(CFLAGS) -static \
  -Xlinker --whole-archive \
    $(subst $(COMPILATION_ROOT_FIX)/,,$(sort $(MACH_EXTRA) $(LIB_EXTRA))) \
  -Xlinker --no-whole-archive $(COMPILATION_ROOT_FIX)/version_info.o \
  symtable.c $(LDFLAGS)
endif

ifneq (YES,$(STATIC_LINKED_APPDEMO))
#dynamic linked appDemo
appDemo_link = appDemo_link_dynamic
appDemo_link_dynamic: appDemo_pre_link
	cd ${COMPILATION_ROOT_FIX} && \
$(LINKER) $(SYSROOT_FLAG) -o ${COMPILATION_ROOT_FIX}/${APP_NAME} $(CFLAGS) \
  -Wl,-Map=${COMPILATION_ROOT_FIX}/${APP_NAME}.map \
  -Xlinker --whole-archive \
    $(subst $(COMPILATION_ROOT_FIX)/,,$(sort $(MACH_EXTRA) $(LIB_EXTRA))) \
  -Xlinker --no-whole-archive $(COMPILATION_ROOT_FIX)/version_info.o \
  $(LDFLAGS)
endif

appDemo_link: $(appDemo_link)
ifeq (YES, $(LIST_UNRESOLVED_SYMBOLS))
  -$(NM) $(COMPILATION_ROOT_FIX)/${APP_NAME} \
    | sed -n 's/^ *U \([^@]*\)\(@@GLIB.*\)\?$$/\1/p' | grep -v -e '^__aeabi_'|sort -u > $(COMPILATION_ROOT_FIX)/unresolved_symbols.txt
endif

ifeq (linux, $(OS_RUN))
ifneq (YES, $(APP_DEMO_ONLY))
ifneq (NO, $(LINUX_BUILD_KERNEL))
appDemo_install = appDemo_install
appDemo_install: appDemo_link
	echo -e "cp ${COMPILATION_ROOT_FIX}/${APP_NAME} $(HOME_INITRD)/usr/bin/${APP_NAME}"
	cp ${COMPILATION_ROOT_FIX}/${APP_NAME} $(HOME_INITRD)/usr/bin/${APP_NAME}
ifneq (silent,$(COMPILATION_TYPE))
	echo -e "\t"$(STRIP)" "$(HOME_INITRD)"/usr/bin/appDemo"
endif


  #Strip debug info only without debugging flag (we can use gdb on hardware)!
	$(STRIP) $(HOME_INITRD)/usr/bin/${APP_NAME}
	chmod 755 $(HOME_INITRD)/usr/bin/${APP_NAME}
	cp $(USER_BASE)/cpssEnabler/mainSysConfig/${HIR_CODE_BASE}src/appDemo/phy/88x3240/88X32xx-FW-R00-03-13.hdr $(HOME_INITRD)/88X32xx-FW.hdr
	cp $(USER_BASE)/cpssEnabler/mainSysConfig/${HIR_CODE_BASE}src/appDemo/phy/88x3240/88X33xx-Z2-FW.hdr $(HOME_INITRD)/88X33xx-Z2-FW.hdr
	cp $(USER_BASE)/cpssEnabler/mainSysConfig/${HIR_CODE_BASE}src/appDemo/phy/88x3240/e2010fw_0_3_7_0_10079.hdr $(HOME_INITRD)/e2010fw.hdr
	cp $(USER_BASE)/cpssEnabler/mainSysConfig/${HIR_CODE_BASE}src/appDemo/phy/88x3240/x3310fw_0_2_8_0_8850.hdr $(HOME_INITRD)/x3310fw_0_2_8_0_8850.hdr
	cp $(USER_BASE)/cpssEnabler/mainSysConfig/${HIR_CODE_BASE}src/appDemo/phy/88x3240/e21x0fw_8_0_0_36_8923.hdr $(HOME_INITRD)/e21x0fw_8_0_0_36_8923.hdr
	cp $(USER_BASE)/cpssEnabler/mainSysConfig/${HIR_CODE_BASE}src/appDemo/phy/88x3240/3240_0400_6270.bin $(HOME_INITRD)/3240_0400_6270.bin
	cp $(USER_BASE)/cpssEnabler/mainSysConfig/${HIR_CODE_BASE}src/appDemo/phy/88x3240/flashdownloadslave.bin $(HOME_INITRD)/flashdownloadslave.bin
	chmod 644 $(HOME_INITRD)/88X32xx-FW.hdr
	chmod 644 $(HOME_INITRD)/88X33xx-Z2-FW.hdr
	chmod 644 $(HOME_INITRD)/e2010fw.hdr
	chmod 644 $(HOME_INITRD)/x3310fw_0_2_8_0_8850.hdr
	chmod 644 $(HOME_INITRD)/e21x0fw_8_0_0_36_8923.hdr
	chmod 644 $(HOME_INITRD)/3240_0400_6270.bin
	chmod 644 $(HOME_INITRD)/flashdownloadslave.bin

# SatR
ifneq (, $(wildcard $(USER_BASE)/tools/satr/Makefile))
appDemo_install += satr_install
satr_install:
	@test -d $(COMPILATION_ROOT_FIX)/satr || mkdir -p $(COMPILATION_ROOT_FIX)/satr
	@$(MAKE) -C $(USER_BASE)/tools/satr V=0 \
      OUT_DIR=$(COMPILATION_ROOT_FIX)/satr \
      TARGET_DIR=$(HOME_INITRD)/usr/bin \
      install
endif
endif
endif
endif

appDemo : appDemo_link $(appDemo_install)

MINI_XCAT3_OBJECTS = $(wildcard $(COMPILATION_ROOT_FIX)/miniXcat3/libs/$(CPU_DIR)/*.a)
miniXcat3.bin:
	rm -f ${COMPILATION_ROOT_FIX}/miniXcat3.bin
	$(CC) -o ${COMPILATION_ROOT_FIX}/miniXcat3.bin $(CFLAGS) $(MINI_XCAT3_OBJECTS) $(LDFLAGS)
	$(CC) -static -o ${COMPILATION_ROOT_FIX}/miniXcat3.static $(CFLAGS) $(MINI_XCAT3_OBJECTS) $(LDFLAGS)
	#$(STRIP) ${COMPILATION_ROOT_FIX}/miniXcat3.bin
	#$(STRIP) ${COMPILATION_ROOT_FIX}/miniXcat3.static

#build appDemo using cpss lib
appDemo_lb :
	echo " begin appDemo_lb"
	cd  ${COMPILATION_ROOT_FIX}
	rm -f ${COMPILATION_ROOT_FIX}/appDemo
	$(CC) -o ${COMPILATION_ROOT_FIX}/appDemo $(CFLAGS) $(MACH_EXTRA_LB) $(LIB_EXTRA_LB) $(LDFLAGS)
	echo -e "cp ${COMPILATION_ROOT_FIX}/appDemo $(HOME_INITRD)/usr/bin/appDemo"
	cp ${COMPILATION_ROOT_FIX}/appDemo $(HOME_INITRD)/usr/bin/appDemo
	echo -e "\t"$(STRIP)" "$(HOME_INITRD)"/usr/bin/appDemo"
	$(STRIP) $(HOME_INITRD)/usr/bin/appDemo
	chmod 777 $(HOME_INITRD)/usr/bin/appDemo
#------------------------------------------------------------------------------------------------------------------------
# Build simulation image
appDemoSim : ${COMPILATION_ROOT_FIX}/version_info.o
	cd ${COMPILATION_ROOT_FIX} && \
	$(CC) -o ${COMPILATION_ROOT_FIX}/$(APP_NAME) $(CFLAGS) \
  -Xlinker --whole-archive \
    $(subst $(COMPILATION_ROOT_FIX)/,,$(sort $(MACH_EXTRA) $(LIB_EXTRA))) \
  -Xlinker --no-whole-archive $< \
 $(SIM_EXTRA) \
 $(LDFLAGS)

	$(info appDemoSim GT:${GHIR_TYPE} LS:$(LINUX_SIM) LSE=${LINUX_SIM_01} APP_NAME ${APP_NAME})

ifeq (YES, $(LIST_UNRESOLVED_SYMBOLS))
  -$(NM) $(COMPILATION_ROOT_FIX)/$(APP_NAME) \
    | sed -n 's/^ *U \([^@]*\)\(@@GLIB.*\)\?$$/\1/p' | sort -u > $(COMPILATION_ROOT_FIX)/unresolved_symbols.txt
endif


ifeq (EXISTS,$(RTOS_ON_SIM))
	cp ${COMPILATION_ROOT_FIX}/$(APP_NAME) $(HOME_INITRD)/usr/bin/appDemo
	chmod 777 $(HOME_INITRD)/usr/bin/appDemo
ifneq (D_ON, $(DEBUG_INFO))
	$(STRIP) $(HOME_INITRD)/usr/bin/appDemo
endif
endif
ifneq (silent,$(COMPILATION_TYPE))
	echo $(APP_NAME) "created"
endif

#------------------------------------------------------------------------------------------------------------------------
ifeq ($(ENDIAN), )
 	ENDIAN = LE
endif
#------------------------------------------------------------------------------------------------------------------------

CHECK_CPP_CFLAGS += $(CFLAGS1) $(CFLAGS2) $(CFLAGS_OPT) $(OS_DEFINE) $(CPU_FLAGS) $(LFLAGS) $(AD_CFLAGS) $(CDEBUG)
CHECK_CPP_CFLAGS += $(FD)CPU=$(CPU) $(FD)_$(OS_RUN) $(FD)$(CPU_FAMILY) $(FD)CPU_$(ENDIAN) $(DEBUG_TYPE)
CHECK_CPP_CFLAGS += $(OS_CFLAGS) $(DBGFLAGS)
CHECK_CPP_CFLAGS +=-Icommon/h -ImainPpDrv/h -ImainExtUtils/h
CHECK_CPP_CFLAGS += $(FIS)common/h/cpss/extServices/os
CHECK_CPP_CFLAGS += $(FIS)mainTmDrv/h
CHECK_CPP_CFLAGS += $(addprefix $(FD),$(sort $(PROJECT_DEFS)))
CHECK_CPP_CFLAGS += $(addprefix $(FD),$(sort $(OS_DEFS)))

#----------------------------------------------------------
#ifeq (EXISTS, $(CHX_FAMILY))
VERSION_FLAGS += CHX_FAMILY
CHECK_CPP_CFLAGS += $(FD)CHX_FAMILY
CHECK_CPP_CFLAGS += $(FD)CONFIG_DDR3
CHECK_CPP_CFLAGS += $(FIS)common/h
CHECK_CPP_CFLAGS += $(FIS)cpssEnabler/mainOs/h
CHECK_CPP_CFLAGS += $(FIS)mainPpDrv/src/cpss/generic/labServices/ddr/bap/h
CHECK_CPP_CFLAGS += $(FIS)mainPpDrv/src/cpss/generic/labServices/ddr/ddr3libv2/h/Driver
CHECK_CPP_CFLAGS += $(FIS)mainPpDrv/src/cpss/generic/labServices/ddr/ddr3libv2/h/Driver/ddr3
CHECK_CPP_CFLAGS += $(FIS)mainPpDrv/src/cpss/generic/labServices/ddr/ddr3libv2/h/Silicon
CHECK_CPP_CFLAGS += $(FIS)mainPpDrv/src/cpss/generic/labServices/h
CHECK_CPP_CFLAGS += $(FIS)common/src/cpss/common/labServices/port/gop/h
CHECK_CPP_CFLAGS += $(FIS)common/src/cpss/common/labServices/port/gop/h/port
CHECK_CPP_CFLAGS += $(FIS)mainTmDrv/h
CHECK_CPP_CFLAGS += $(FIS)mainTmDrv/h/core
CHECK_CPP_CFLAGS += $(FIS)mainTmDrv/h/platform
CHECK_CPP_CFLAGS += $(FIS)mainTmDrv/h/private
#endif

#----------------------------------------------------------
ifeq (1,$(LINUX_SIM))
 	CHECK_CPP_CFLAGS += $(addprefix $(FIS),$(sort $(SIM_INCLUDES)))
endif
#----------------------------------------------------------
checkCpp:
	echo "============================================================"
	echo "*** Check C++ errors/warning for common,mainPpDrv,mainTmDrv:"
	echo "============================================================"
	for f in $(COMPILATION_ROOT_FIX)/hdr*.cc; do \
		$(CXX) $(CHECK_CPP_CFLAGS) -c $$f -o $$f.o; done

#----------------------------------------------------------

%.o : %.c
	$(CC) $(CFLAGS) $(CFLAGS2) -c $< -o $@

.PHONY: $(COMPILATION_ROOT_FIX)/version_info.c
$(COMPILATION_ROOT_FIX)/version_info.c:
	@echo Creating version info...
	@echo const char *VERSION_DATE = '"$(VERSION_DATE)"'';'  >  $@
	@echo const char *VERSION_FLAGS = '"$(VERSION_FLAGS)"'';'  >> $@


clean_all:  clean
clean:
	rm -f appDemo*
	rm -f *.gdb
	find . -name '*~' -exec rm -f {} \;
	find . -name '*.o' -exec rm -f {} \;
	find . -name '*.$(LIBEXT)' -exec rm -f {} \;
	find . -name '*.dep' -exec rm -f {} \;
	find . -name '*.map' -exec rm -f {} \;

unix:
	find . -name '*.c' -exec dos2unix -q {} \;
	find . -name '*.cpp' -exec dos2unix -q {} \;
	find . -name '*.h' -exec dos2unix -q {} \;
	find . -name '*.C' -exec dos2unix -q {} \;
	find . -name '*.CPP' -exec dos2unix -q {} \;
	find . -name '*.H' -exec dos2unix -q {} \;
	find . -name Makefile -exec dos2unix -q {} \;
	find . -name '*.mk' -exec dos2unix -q {} \;
	find . -name presteraTopMake -exec dos2unix -q {} \;
	find . -name gtTopMake -exec dos2unix -q {} \;
	find . -name gtBuild -exec dos2unix -q {} \;

dos:
	find . -name '*.c' -exec unix2dos -q {} \;
	find . -name '*.cpp' -exec unix2dos -q {} \;
	find . -name '*.h' -exec unix2dos -q {} \;
	find . -name '*.C' -exec unix2dos -q {} \;
	find . -name '*.CPP' -exec unix2dos -q {} \;
	find . -name '*.H' -exec unix2dos -q {} \;
	find . -name Makefile -exec unix2dos -q {} \;
	find . -name '*.mk' -exec unix2dos -q {} \;
	find . -name presteraTopMake -exec unix2dos -q {} \;
	find . -name gtTopMake -exec unix2dos -q {} \;
	find . -name gtBuild -exec unix2dos -q {} \;

