#
# This sharedMakefile.mk is used ONLY for Shared Memory approach.
# It is inluded into Makefile on SHARED_MEMORY=1
#

#
#	Variables declaration part which are used for libcpss.so/libhelper.so
#

ifneq (,$(findstring mips,$(sysroot)))
        # Found
        SYSROOT_FLAG = --sysroot=$(sysroot)
else
        # Not found
        SYSROOT_FLAG =
endif
$(info "SYSROOT_FLAG=$(SYSROOT_FLAG)")


CPSS_CPSS_LIB_OBJETCTS=$(FPGA_FILES) $(UTIL_FILES) $(COMMON_LIBS) $(FA_FILES) $(XBAR_FILES) $(TM_API_FILES)


#	Cannot use $(PP_LIBS) 'cause mainPp*.$(LIBEXT) already include *FdbHash.$(LIBEXT)...
ifneq ($(NO_PP_USED),NOT_USED)
ifeq (EXISTS, $(CHX_FAMILY))
	CPSS_CPSS_LIB_OBJETCTS += $(COMPILATION_ROOT_FIX)/mainPpDrv/libs/$(CPU_DIR)/mainPpDrv.$(LIBEXT)
endif
ifeq (EXISTS, $(PX_FAMILY))
	CPSS_CPSS_LIB_OBJETCTS += $(COMPILATION_ROOT_FIX)/mainPxDrv/libs/$(CPU_DIR)/mainPxDrv.$(LIBEXT)
endif
endif

#	Objects which are located in cpssEnabler but are the part of libcpss.so
CPSS_CPSS_LIB_OBJETCTS += $(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/mainExtDrv.$(LIBEXT)
CPSS_CPSS_LIB_OBJETCTS += $(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/shrMemIntraCPSSdata.$(LIBEXT)

CPSS_HELPER_LIB_OBJETCTS =  $(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/gtOs.$(LIBEXT)
CPSS_HELPER_LIB_OBJETCTS += $(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/gtStack.$(LIBEXT)
CPSS_HELPER_LIB_OBJETCTS += $(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/gtUtil.$(LIBEXT)
CPSS_HELPER_LIB_OBJETCTS += $(wildcard $(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/mainExtDrvShared.$(LIBEXT))

SCRIPTLD_ROOT=$${CPSS_PATH}/cpssEnabler/mainOs/src/gtOs/sharedMemory/scriptsld
ifeq (1, $(LINUX_SIM))
    ifeq (Linux, $(OS_TARGET))
		SCRIPTLD_CPSS_LIB=$(SCRIPTLD_ROOT)/linuxSim/libcpss_ld_script.$(CPU_DIR)
		SCRIPTLD_HELPER_LIB=$(SCRIPTLD_ROOT)/linuxSim/libhelper_ld_script.$(CPU_DIR)
    else
#		??? Not supported yet!
    endif
else
    ifeq (linux, $(OS_RUN))
		SCRIPTLD_CPSS_LIB=$(SCRIPTLD_ROOT)/$(OS_RUN)/libcpss_ld_script.$(CPU)
		SCRIPTLD_HELPER_LIB=$(SCRIPTLD_ROOT)/$(OS_RUN)/libhelper_ld_script.$(CPU)
    else
#		??? Not supported yet!
    endif
endif

BUILD_MAP_FILE=-Map
MAPFILE_CPSS_LIB=$(COMPILATION_ROOT_FIX)/libcpss.map
MAPFILE_HELPER_LIB=$(COMPILATION_ROOT_FIX)/libhelper.map

ifeq (1,$(LINUX_SIM))
#   For simulation COMPILER can reside in any unexpected place (/usr/local/bin/, ~/bin/, etc.)
#   So don't rely on this path, use ususal convention-defined.
    XCOMP_ROOT_PATH_FIX=

#   Use wrappers for ALL functions from libc which are used by simulation (libcpss.so shouldn't call libc directly!)
##    SIM_LIBC_FUNCS=access __assert_fail calloc __errno_location exit fclose fgets fopen fprintf free malloc \
##		memcmp memcpy memset printf qsort rand sprintf sscanf strcat strcmp strcpy strlen strncmp strncpy \
##		strrchr strstr strtok system vsprintf

#	Use wrappers ONLY for dynamic allocation to provide shared alorithm
    SIM_LIBC_FUNCS=calloc free malloc realloc

ifeq (EXISTS, $(FORCE_GM_USE))
	#LIB_EXTRA += $(USER_BASE)/simulation/libs/GM/Linux/PpInterfaceLibs/lionB/liblionB_gm.a
	SIM_EXTRA += $(USER_BASE)/simulation/libs/GM/Linux/PpInterfaceLibs/bobcat3/libBC3Gm_gm.a
	CPLUSPLUS_LIB_REQUIRED=yes
endif

    SIM_LIBC_WRAPPERS = $(foreach func,$(SIM_LIBC_FUNCS),--wrap $(func) )
ifneq (,$(LIBC_A))
	SIM_STATIC_LIB_STAT_MODULE=$(COMPILATION_ROOT_FIX)/stat.o
endif
else
#   For BM doesn't fix path
    XCOMP_ROOT_PATH_FIX=$(XCOMP_ROOT_PATH)
ifdef USE_GNUEABI
#	Some arch-specific routines are located into additional shared libraries
	BM_SHM_ADDITIONAL_ARCH_SO_DEPS=$(XCOMP_ROOT_PATH_FIX)/../lib/libgcc_s.so
endif
endif

# Note! We should implement more elaborate resolution of standard libraries names to accept newer version
# not less than current usable.
#ifneq (,$(wildcard $(XCOMP_ROOT_PATH_FIX)/lib/*linux*/libpthread.so.0))
#  HELPER_USE_VERSION_FROM_LIBS=$(wildcard $(XCOMP_ROOT_PATH_FIX)/lib/*linux*/libpthread.so.0) \
#        $(wildcard $(XCOMP_ROOT_PATH_FIX)/lib/*linux*/libc.so.6)
#else
#  HELPER_USE_VERSION_FROM_LIBS=$(XCOMP_ROOT_PATH_FIX)/lib/libpthread.so.0 $(XCOMP_ROOT_PATH_FIX)/lib/libc.so.6
#endif
HELPER_USE_VERSION_FROM_LIBS=-lc -lpthread

#
#	Variables declaration part which are used for CPSS Enabler
#
CPSS_ENABLER_OBJECTS =
ifneq (EXCLUDE_LIB,$(EXCLUDE_GALTIS))
CPSS_ENABLER_OBJECTS+=$(COMPILATION_ROOT_FIX)/mainGaltisWrapper/libs/$(CPU_DIR)/mainGaltisWrapper.$(LIBEXT)
endif
ifneq (yes, $(CMD_LUA_CLI))
CPSS_ENABLER_OBJECTS+=$(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/mainCmd.$(LIBEXT)
endif
#CPSS_ENABLER_OBJECTS+=$(wildcard $(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/luaCLI.$(LIBEXT))
CPSS_ENABLER_OBJECTS+=$(wildcard $(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/confi.$(LIBEXT))
CPSS_ENABLER_OBJECTS+=$(wildcard $(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/extPhyM.$(LIBEXT))
CPSS_ENABLER_OBJECTS+=$(wildcard $(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/mainPhy.$(LIBEXT))
CPSS_ENABLER_APP_DEMO_CPU_ETH_PORT_MANAGEOBJECTS+=$(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/mainSysConfig.$(LIBEXT)

#	Cannot use $(UTF_LIB) 'cause $(wildcard ) doesn't work for non-existent files
#	(they will be built only as a result of this make into 'cpss' target)
ifeq (YES,$(INCLUDE_UTF))
	CPSS_ENABLER_OBJECTS += $(COMPILATION_ROOT_FIX)/mainUT/libs/$(CPU_DIR)/mainUT.$(LIBEXT)
endif

ifeq (yes, $(CMD_LUA_CLI))
    CPSS_ENABLER_LUA_WRAPPERS = \
    		$(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/mainCmd.$(LIBEXT) \
    		$(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/luaCLI.$(LIBEXT) \
            $(COMPILATION_ROOT_FIX)/mainLuaWrapper/libs/$(CPU_DIR)/mainLuaWrapper.$(LIBEXT)
endif

ifdef APP_DEMO_CPU_ETH_PORT_MANAGE
CPSS_ENABLER_OBJECTS+=$(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/eventsEmulation.$(LIBEXT)
endif

#   Including some libraries several times to resolve depencies
ifneq (yes, $(CMD_LUA_CLI))
CPSS_ENABLER_OBJECTS+=$(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/mainCmd.$(LIBEXT)
endif
CPSS_ENABLER_OBJECTS+=$(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/mainSysConfig.$(LIBEXT)
ifneq (yes, $(CMD_LUA_CLI))
CPSS_ENABLER_OBJECTS+=$(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/mainCmd.$(LIBEXT)
endif
#CPSS_ENABLER_OBJECTS+=$(wildcard $(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/luaCLI.$(LIBEXT))
CPSS_ENABLER_OBJECTS+=$(wildcard $(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/confi.$(LIBEXT))
CPSS_ENABLER_OBJECTS+=$(wildcard $(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/extPhyM.$(LIBEXT))
CPSS_ENABLER_OBJECTS+=$(wildcard $(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/mainPhy.$(LIBEXT))
ifeq (YES,$(INCLUDE_UTF))
	CPSS_ENABLER_OBJECTS += $(COMPILATION_ROOT_FIX)/mainUT/libs/$(CPU_DIR)/mainUT.$(LIBEXT)
    CPSS_ENABLER_OBJECTS += $(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/mainSysConfig.$(LIBEXT)
endif
CPSS_ENABLER_OBJECTS += $(wildcard $(COMPILATION_ROOT_FIX)/referenceHal/libs/$(CPU_DIR)/referenceHal.$(LIBOBJ))
CPSS_ENABLER_OBJECTS += $(wildcard $(COMPILATION_ROOT_FIX)/embeddedCommands/libs/$(CPU_DIR)/embeddedCommands.$(LIBOBJ))



#	Clients shouldn't copy relocatable objects from .so libraries!
CFLAGS_NO_COPY_RELOC=-Xlinker -z -Xlinker nocopyreloc
ifeq (EXISTS, $(CHX_FAMILY))
CFLAGS_NO_COPY_RELOC+=-DCHX_FAMILY
endif

#   Variables for test clients (to be built as separate executables)
ifeq (YES, $(FAST_COMPILE))
SHLIBTESTCLIENTS_OBJ_DIR=shlibTestClients/src
else
SHLIBTESTCLIENTS_OBJ_DIR=shlibTestClients/objTemp/$(CPU_DIR)
endif
BUILD_SHMT_PATH=cpssEnabler/mainOs/src/gtOs/sharedMemory/testClients
SHMT_LDFLAGS=-lpthread -lrt -ldl -lm

ifdef APP_DEMO_CPU_ETH_PORT_MANAGE
SHMT_LDFLAGS+=-DAPP_DEMO_CPU_ETH_PORT_MANAGE
endif

ifeq (yes, $(CPLUSPLUS_LIB_REQUIRED))
#	Goden Module (ASIC simulation for PUMA, lion) uses C++, use stdc++ library
	SHMT_LDFLAGS+= -lstdc++
endif

ifeq (1,$(LINUX_SIM))
ifneq (,$(filter 3.%,$(GCC_VER)))
    LDSHLIB=$(LD) $(SYSROOT_FLAG) -shared $(LDFLAGS)
    XLINKER=
else
    LDSHLIB=$(CC) $(SYSROOT_FLAG) -shared $(CFLAGS2)
    XLINKER=-Xlinker
endif
endif

ifneq (,$(wildcard $(XCOMP_ROOT_PATH_FIX)/usr/lib/*-linux-*))
#  LDFLAGS := -L$(wildcard $(XCOMP_ROOT_PATH_FIX)/usr/lib/*-linux-*) $(LDFLAGS)
endif

#
# Following targets are used only for shared memory approach
#

# Note - shared Makefile has own 'all' target.
ifeq (EXISTS, $(CHX_FAMILY))
ifneq (, $(wildcard $(USER_BASE)/shlibTestClients/_Makefile))
    SHARED_APPS = fdbLearning
    ifneq (EXCLUDE_LIB,$(EXCLUDE_GALTIS))
        SHARED_APPS += RxTxProcess
    endif
endif
endif
ifeq (yes,$(LUACLI_STANDALONE))
SHARED_APPS += luaCLI
endif

##== all: =============================
all : cpss_shared application_shared $(SHARED_APPS)

ifneq (,$(wildcard $(XCOMP_ROOT_PATH_FIX)/usr/lib/*linux*/libc.a))
  LIBC_A = $(wildcard $(XCOMP_ROOT_PATH_FIX)/usr/lib/*linux*/libc.a)
else
  LIBC_A = $(wildcard $(XCOMP_ROOT_PATH_FIX)/usr/lib/libc.a)
endif
##== cpss_shared: =============================
# additional steps for CPSS - build DSOs
cpss_shared: $(COMPILATION_ROOT_FIX)/version_info.o 
#	Shared library version should also build .so libraries
	$(RM) -f $(COMPILATION_ROOT_FIX)/libcpss.so
ifeq (1,$(LINUX_SIM))
	cd $(COMPILATION_ROOT_FIX) && \
$(LDSHLIB) -fpic -fPIC \
	$(foreach word,$(SIM_LIBC_WRAPPERS),$(XLINKER) $(word)) \
	$(XLINKER) --whole-archive \
		$(subst $(COMPILATION_ROOT_FIX)/,, \
			$(CPSS_CPSS_LIB_OBJETCTS) \
			$(COMPILATION_ROOT_FIX)/simulation/libs/$(CPU_DIR)/simulation.$(LIBEXT) \
			$(SLAN_LIB)) \
	$(XLINKER) -no-whole-archive $(COMPILATION_ROOT_FIX)/version_info.o \
	$(XLINKER) $(BUILD_MAP_FILE) $(XLINKER) $(MAPFILE_CPSS_LIB) \
	-T $(SCRIPTLD_CPSS_LIB) \
	$(XLINKER) -soname $(XLINKER) libcpss.so \
	-o libcpss.so
	echo "libcpss.so created"
else
ifneq (,$(filter 3.%,$(GCC_VER)))
	# GCC_VER == 3.*
	$(LD) $(SYSROOT_FLAG) -shared $(BM_SHM_LDFLAGS_LD) --whole-archive $(CPSS_CPSS_LIB_OBJETCTS) -no-whole-archive	\
		$(BUILD_MAP_FILE) $(MAPFILE_CPSS_LIB) -T $(SCRIPTLD_CPSS_LIB)  \
		-soname libcpss.so \
		-o $(COMPILATION_ROOT_FIX)/libcpss.so $(BM_SHM_ADDITIONAL_ARCH_SO_DEPS)
else
	# GCC_VER != 3.*
	cd $(COMPILATION_ROOT_FIX) && \
$(CC) $(SYSROOT_FLAG) -shared $(CC_ARCH) $(BM_SHM_LDFLAGS_CC) \
	-Xlinker --whole-archive \
		$(subst $(COMPILATION_ROOT_FIX)/,, $(CPSS_CPSS_LIB_OBJETCTS)) \
	-Xlinker --no-whole-archive \
	$(addprefix -Xlinker , \
	    $(BUILD_MAP_FILE) $(MAPFILE_CPSS_LIB) \
		-T $(SCRIPTLD_CPSS_LIB)  \
	    -soname libcpss.so) \
	-o libcpss.so $(BM_SHM_ADDITIONAL_ARCH_SO_DEPS)
	echo "libcpss.so created"
endif
endif
	$(RM) -f $(COMPILATION_ROOT_FIX)/libhelper.so
ifeq (1,$(LINUX_SIM))
#	stat routine cannot be resolved dynamically for i386(used only for simulation), link it into .so object
#	Note - change dir to output directory to make file there
ifneq (,$(LIBC_A))
	cd $(COMPILATION_ROOT_FIX); $(AR) x $(LIBC_A) stat.o
endif
endif
#	link with libpthread.so to resolve correct version of glibc
#	(pthread_cond_init@@GLIBC_2.3.2 should be used)!
ifneq (,$(filter 3.%,$(GCC_VER)))
# GCC_VER == 3.*
	$(LD) $(SYSROOT_FLAG) -shared $(BM_SHM_LDFLAGS_LD) --whole-archive $(CPSS_HELPER_LIB_OBJETCTS) -no-whole-archive \
		$(SIM_STATIC_LIB_STAT_MODULE) $(HELPER_USE_VERSION_FROM_LIBS) \
		-T $(SCRIPTLD_HELPER_LIB) $(BUILD_MAP_FILE) $(MAPFILE_HELPER_LIB) \
		-soname libhelper.so -o $(COMPILATION_ROOT_FIX)/libhelper.so \
		$(BM_SHM_ADDITIONAL_ARCH_SO_DEPS)
else
# GCC_VER != 3.*
	cd $(COMPILATION_ROOT_FIX) && \
$(CC) $(SYSROOT_FLAG) -shared $(CC_ARCH) $(CFLAGS2) $(BM_SHM_LDFLAGS_CC) \
	-Xlinker --whole-archive \
		$(subst $(COMPILATION_ROOT_FIX)/,, $(CPSS_HELPER_LIB_OBJETCTS)) \
	-Xlinker --no-whole-archive \
	$(SIM_STATIC_LIB_STAT_MODULE) $(HELPER_USE_VERSION_FROM_LIBS) \
	-T $(SCRIPTLD_HELPER_LIB) \
	$(addprefix -Xlinker , \
		$(BUILD_MAP_FILE) $(MAPFILE_HELPER_LIB) \
	    -soname libhelper.so) \
	-o libhelper.so \
	$(BM_SHM_ADDITIONAL_ARCH_SO_DEPS)
	echo "libhelper.so created"
endif
#------------------------------

ifneq (1, $(LINUX_SIM))
##== application_shared: =============================
##== appDemo_shared: =============================
# If no simulation then we use BM target to build CPSS Enabler
application_shared : appDemo_shared 
appDemo_shared : cpss_shared $(COMPILATION_ROOT_FIX)/version_info.o
ifneq (CPSS_LIB_SHARED, $(PRODUCT_TYPE_FLAVOR))
	echo "begin appDemo for shared library"
	cd  ${COMPILATION_ROOT_FIX}
	rm -f ${COMPILATION_ROOT_FIX}/appDemo

#   Shared memory approach uses dynamic libraries libcpss.so and libhelper.so
	cd $(COMPILATION_ROOT_FIX) && \
$(CC) $(SYSROOT_FLAG) $(CC_ARCH) -o ${COMPILATION_ROOT_FIX}/appDemo $(CFLAGS) $(CFLAGS_NO_COPY_RELOC) \
	-Xlinker --whole-archive \
		$(subst $(COMPILATION_ROOT_FIX)/,, \
			$(sort $(CPSS_ENABLER_OBJECTS) $(CPSS_ENABLER_LUA_WRAPPERS))) \
	-Xlinker --no-whole-archive $(COMPILATION_ROOT_FIX)/version_info.o  \
	$(LDFLAGS) \
	libcpss.so \
	libhelper.so -lrt -lm \
	-Xlinker $(BUILD_MAP_FILE) -Xlinker appDemo.map
ifneq (NO, $(LINUX_BUILD_KERNEL))
#   Copy libraries to /usr/lib/ into image directory
	cp $(COMPILATION_ROOT_FIX)/libcpss.so $(HOME_INITRD)/usr/lib/
	cp $(COMPILATION_ROOT_FIX)/libhelper.so $(HOME_INITRD)/usr/lib/
	echo -e "cp ${COMPILATION_ROOT_FIX}/appDemo $(HOME_INITRD)/usr/bin/appDemo"
	cp ${COMPILATION_ROOT_FIX}/appDemo $(HOME_INITRD)/usr/bin/appDemo
	#echo -e "\t"$(STRIP)" "$(HOME_INITRD)"/usr/bin/appDemo"
	$(STRIP) $(HOME_INITRD)/usr/bin/appDemo
	chmod 777 $(HOME_INITRD)/usr/bin/appDemo
	$(STRIP) $(HOME_INITRD)/usr/lib/libcpss.so
	$(STRIP) $(HOME_INITRD)/usr/lib/libhelper.so
endif

# end of ifneq ((CPSS_LIB_SHARED, $(PRODUCT_TYPE_FLAVOR))
endif


# else of ifneq (1, $(LINUX_SIM))
else

##== application_shared: =============================
##== appDemo_shared: =============================
# If simulation is used then we use WM target to build CPSS Enabler
application_shared: appDemoSim_shared
appDemoSim_shared : cpss_shared
ifneq (CPSS_LIB_SHARED, $(PRODUCT_TYPE_FLAVOR))
#   Shared memory approach uses dynamic libraries libcpss.so and libhelper.so
	cd $(COMPILATION_ROOT_FIX) && \
$(CC) $(SYSROOT_FLAG) $(CFLAGS) $(CFLAGS_NO_COPY_RELOC) \
	-Xlinker --whole-archive \
		$(subst $(COMPILATION_ROOT_FIX)/,, \
			$(sort $(CPSS_ENABLER_OBJECTS) $(CPSS_ENABLER_LUA_WRAPPERS))) \
	-Xlinker --no-whole-archive \
	-Wl,-rpath,$(COMPILATION_ROOT_FIX) \
	-Xlinker $(BUILD_MAP_FILE) -Xlinker ${COMPILATION_ROOT_FIX}/appDemoSim.map \
	libcpss.so \
	libhelper.so \
	$(LDFLAGS) -o $(COMPILATION_ROOT_FIX)/$@
	echo "appDemoSim for shared library created"

# end of ifneq ((CPSS_LIB_SHARED, $(PRODUCT_TYPE_FLAVOR))
endif
# end of ifneq (1, $(LINUX_SIM))
endif
#------------------------------

ifeq ($(ENDIAN), )
ENDIAN = LE
endif
#== fdbLearning: =============================
fdbLearning: cpss_shared $(COMPILATION_ROOT_FIX)/$(SHLIBTESTCLIENTS_OBJ_DIR)/fdbLearning.o
ifneq (CPSS_LIB_SHARED, $(PRODUCT_TYPE_FLAVOR))
ifeq (1,$(INCLUDE_SHMT))
#   Target to build test clients for shared memory approach
#   Applicable ONLY for SHARED_MEMORY=1
#   Build both clients:
	cd $(COMPILATION_ROOT_FIX) && \
$(CC) $(SYSROOT_FLAG) $(CFLAGS) $(CFLAGS2) -g $(CFLAGS_NO_COPY_RELOC) \
	-Wl,-rpath,$(COMPILATION_ROOT_FIX) \
	$(SHLIBTESTCLIENTS_OBJ_DIR)/fdbLearning.o	\
	-Xlinker $(BUILD_MAP_FILE) -Xlinker ${COMPILATION_ROOT_FIX}/fdbLearning.map \
	libcpss.so \
	libhelper.so \
	-o fdbLearning $(SHMT_LDFLAGS)
    ifneq (1, $(LINUX_SIM))
    ifneq (NO, $(LINUX_BUILD_KERNEL))
		echo -e "cp ${COMPILATION_ROOT_FIX}/fdbLearning $(HOME_INITRD)/usr/bin/fdbLearning"
		cp ${COMPILATION_ROOT_FIX}/fdbLearning $(HOME_INITRD)/usr/bin/fdbLearning
		$(STRIP) $(HOME_INITRD)/usr/bin/fdbLearning
    endif
    endif

	echo "fdbLearning created"

	#-----------------------------------------------------------------------------------------

    ifneq (1, $(LINUX_SIM))
    ifneq (NO, $(LINUX_BUILD_KERNEL))
	# copy librt.so.1 as symlink
	( cd $(XCOMP_ROOT_PATH)/lib$(LIB_1) && cp -P librt.so.1 librt-2.*so $(HOME_INITRD)/lib )
	#cp -f $(XCOMP_ROOT_PATH)/lib$(LIB_1)/librt.so.1 $(HOME_INITRD)/lib
	#cp -f $(XCOMP_ROOT_PATH)/lib$(LIB_1)/librt-2.*so $(HOME_INITRD)/lib
    endif
    endif

#-----------------------------------------------------------------------------------------

else
	echo testClients NOT used in configuration use INCLUDE_SHMT to switch ON
endif

# end of ifneq ((CPSS_LIB_SHARED, $(PRODUCT_TYPE_FLAVOR))
endif

#== luaCLI: =============================
luaCLI: cpss_shared
ifneq (CPSS_LIB_SHARED, $(PRODUCT_TYPE_FLAVOR))
	cd $(COMPILATION_ROOT_FIX) && \
$(CC) $(SYSROOT_FLAG) $(CFLAGS) $(CFLAGS_NO_COPY_RELOC) \
	-Xlinker --whole-archive \
	    luaCLI_standalone/libs/$(CPU_DIR)/luaCLI_standalone.$(LIBEXT) \
	-Xlinker --no-whole-archive \
	-Wl,-rpath,$(COMPILATION_ROOT_FIX) \
	-Xlinker $(BUILD_MAP_FILE) -Xlinker ${COMPILATION_ROOT_FIX}/$@.map \
	libhelper.so \
	libcpss.so \
	$(LDFLAGS) -o $@ -lm
ifneq (1, $(LINUX_SIM))
ifneq (NO, $(LINUX_BUILD_KERNEL))
	echo -e "cp ${COMPILATION_ROOT_FIX}/luaCLI $(HOME_INITRD)/usr/bin/luaCLI"
	cp ${COMPILATION_ROOT_FIX}/luaCLI $(HOME_INITRD)/usr/bin/luaCLI
	$(STRIP) $(HOME_INITRD)/usr/bin/luaCLI
endif
endif
	echo "luaCLI created"
# end of ifneq ((CPSS_LIB_SHARED, $(PRODUCT_TYPE_FLAVOR))
endif

#------------------------------
#$(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/sharedMemoryUT.$(LIBEXT) 	\

EVENT_EMUL_LIB=
ifdef APP_DEMO_CPU_ETH_PORT_MANAGE
EVENT_EMUL_LIB=$(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/eventsEmulation.$(LIBEXT)
endif

#== RxTxProcess: =============================
RxTxProcess: $(COMPILATION_ROOT_FIX)/RxTxProcess
ifeq (EXISTS,$(CHX_FAMILY))
RXTX_PROCESS_FILE=RxTxProcess.o
endif

#== RxTxProcess: =============================
$(COMPILATION_ROOT_FIX)/RxTxProcess: cpss_shared $(COMPILATION_ROOT_FIX)/$(SHLIBTESTCLIENTS_OBJ_DIR)/$(RXTX_PROCESS_FILE)
ifneq (CPSS_LIB_SHARED, $(PRODUCT_TYPE_FLAVOR))
ifeq (1,$(INCLUDE_SHMT))
#   Target to build test clients for shared memory approach
#   Applicable ONLY for SHARED_MEMORY=1
#   Build both clients:
	cd $(COMPILATION_ROOT_FIX) && \
$(CC) $(SYSROOT_FLAG) $(CFLAGS) $(CFLAGS2)  $(CFLAGS_NO_COPY_RELOC) \
	-Wl,-rpath,$(COMPILATION_ROOT_FIX) \
	$(SHLIBTESTCLIENTS_OBJ_DIR)/$(RXTX_PROCESS_FILE)	\
	-Xlinker $(BUILD_MAP_FILE) -Xlinker $@.map \
	libcpss.so \
	libhelper.so \
	$(EVENT_EMUL_LIB) \
	-o $@ $(SHMT_LDFLAGS)
    ifneq (1, $(LINUX_SIM))
    ifneq (NO, $(LINUX_BUILD_KERNEL))
		echo -e "cp $@ $(HOME_INITRD)/usr/bin/RxTxProcess"
		cp $@ $(HOME_INITRD)/usr/bin/RxTxProcess
		$(STRIP) $(HOME_INITRD)/usr/bin/RxTxProcess
    endif
    endif
	echo "RxTxProcess created"
else
	echo testClients NOT used in configuration use INCLUDE_SHMT to switch ON
endif
# end of ifneq ((CPSS_LIB_SHARED, $(PRODUCT_TYPE_FLAVOR))
endif
