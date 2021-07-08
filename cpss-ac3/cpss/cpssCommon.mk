####################################################################
#   Common Definitions required to Build/link project
####################################################################

ifndef COMPILATION_ROOT
	COMPILATION_ROOT = $(USER_BASE)
endif
COMPILATION_ROOT_FIX = $(subst \,/,$(COMPILATION_ROOT))

ifeq (, $(wildcard $(USER_BASE)/tools/build/toolkit/$(TOOLKIT_MAKE_RULES)Defs.mk))
$(error "Make defs (build/toolkit/$(TOOLKIT_MAKE_RULES)Defs.mk) not found)")
endif
include $(USER_BASE)/tools/build/toolkit/$(TOOLKIT_MAKE_RULES)Defs.mk

ifeq (win32,$(OS_RUN))
	LIBOBJ=lib
else
	LIBOBJ=a
endif

### handle the PP files ###
ifeq (EXISTS, $(PX_FAMILY))
    PP_LIBS =$(wildcard $(COMPILATION_ROOT_FIX)/mainPxDrv/libs/$(CPU_DIR)/mainPx*.$(LIBEXT))
endif

ifeq (EXISTS, $(CHX_FAMILY))
ifeq (EXISTS, $(PX_FAMILY))
    PP_LIBS +=$(wildcard $(COMPILATION_ROOT_FIX)/mainPpDrv/libs/$(CPU_DIR)/mainPp*.$(LIBEXT))
else
    PP_LIBS =$(wildcard $(COMPILATION_ROOT_FIX)/mainPpDrv/libs/$(CPU_DIR)/mainPp*.$(LIBEXT))
endif
endif

ifneq (win32,$(OS_RUN))
PP_LIBS+=$(wildcard $(COMPILATION_ROOT_FIX)/mainPpDrv/libs/$(CPU_DIR)/*FdbHash.$(LIBEXT))
PP_LIBS+=$(wildcard $(COMPILATION_ROOT_FIX)/mainPpDrv/libs/$(CPU_DIR)/*IpLpmEngine.$(LIBEXT))
endif

ifeq (RHODES, $(FPGA_TYPE))
    FPGA_FILES = $(wildcard $(COMPILATION_ROOT_FIX)/mainRhodesDrv/libs/$(CPU_DIR)/mainRhodesDrv.$(LIBEXT))
endif


UTF_LIB=$(wildcard $(COMPILATION_ROOT_FIX)/mainUT/libs/$(CPU_DIR)/mainU*.$(LIBEXT))

UTIL_FILES = $(wildcard $(COMPILATION_ROOT_FIX)/mainExtUtils/libs/$(CPU_DIR)/mainExtUtils.$(LIBOBJ))


ifeq ($(NO_PP_USED),NOT_USED)
	PP_LIBS =
endif

### handle the COMMON files ###
COMMON_LIBS = $(COMPILATION_ROOT_FIX)/common/libs/$(CPU_DIR)/common.$(LIBOBJ)

### handle the FA files ###
ifeq (PRESTERA, $(FA_VENDOR))
	FA_FILES = $(COMPILATION_ROOT_FIX)/mainFaDrv/libs/$(CPU_DIR)/mainFaDrv.$(LIBOBJ)
endif

### handle the XBAR files ###
ifeq (PRESTERA, $(XBAR_VENDOR))
	XBAR_FILES = $(COMPILATION_ROOT_FIX)/mainXbarDrv/libs/$(CPU_DIR)/mainXbarDrv.$(LIBOBJ)
endif

### handle the Galtis files ###
ifeq (EXCLUDE_LIB, $(EXCLUDE_GALTIS))
GALTIS_FILES =
else
GALTIS_FILES = $(COMPILATION_ROOT_FIX)/mainGaltisWrapper/libs/$(CPU_DIR)/mainGaltisWrapper.$(LIBOBJ)
endif

### handle the Lua wrappers files ###
ifeq (yes, $(CMD_LUA_CLI))
LUA_WRAPPER_FILES = $(COMPILATION_ROOT_FIX)/mainLuaWrapper/libs/$(CPU_DIR)/mainLuaWrapper.$(LIBOBJ)
else
LUA_WRAPPER_FILES =
endif

### handle TM API files
TM_API_FILES =

ifeq (EXISTS, $(CHX_FAMILY))
    TM_API_FILES += $(wildcard $(COMPILATION_ROOT_FIX)/mainTmDrv/libs/$(CPU_DIR)/mainTmDrv.$(LIBOBJ))
endif

### handle the Cpss Enabler files ###
CPSS_ENABLER_FILES = $(COMPILATION_ROOT_FIX)/cpssEnabler/libs/$(CPU_DIR)/cpssEnabler.$(LIBOBJ)

### reference code components
REFCODE_FILES += $(wildcard $(COMPILATION_ROOT_FIX)/referenceHal/libs/$(CPU_DIR)/referenceHal.$(LIBOBJ))
REFCODE_FILES += $(wildcard $(COMPILATION_ROOT_FIX)/embeddedCommands/libs/$(CPU_DIR)/embeddedCommands.$(LIBOBJ))

#For simulation only
include $(USER_BASE)/simulationCommon.mk


ifneq (YES, $(INCLUDE_UTF))
  UTF_LIB  =
endif


# DO NOT CHANGE libraries order
# IT IS important for successfull link
LIB_EXTRA += $(UTF_LIB) $(PP_LIBS)

#currently nothing to add as "extra libraries"
MACH_EXTRA += $(CPSS_ENABLER_FILES) \
	$(COMMON_LIBS) \
	$(UTIL_FILES) \
	$(XBAR_FILES) \
	$(GALTIS_FILES) \
	$(LUA_WRAPPER_FILES) \
	$(FA_FILES) \
	$(REFCODE_FILES)

ifeq (YES, $(INCLUDE_TM))
	MACH_EXTRA += $(TM_API_FILES)
endif

ifeq (1, $(LINUX_SIM))
	MACH_EXTRA +=$(SIM_LIB)
endif

ifeq (RHODES, $(FPGA_TYPE))
	MACH_EXTRA +=   $(FPGA_FILES)
endif
