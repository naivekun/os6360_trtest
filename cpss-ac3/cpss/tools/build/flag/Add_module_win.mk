######################################################
### add_module
###
### parameters:
###    PROJ_NAME
###
######################################################

P_PROJ_NAME := $(PROJ_NAME)
PROJ_NAME_CAP := $(call uppercase,$(PROJ_NAME))
$(P_PROJ_NAME)_BASE_PATH := $(P_PROJ_NAME)/h

include $(USER_BASE)/tools/build/flag/_$(P_PROJ_NAME)_TopMakeDef

$(PROJ_NAME)_CFLAGS_CL := \
        $(CFLAGS) \
        $(CFLAGS1) \
        $(CFLAGS2) \
        $($(PROJ_NAME)_CFLAGS2) \
        $($(PROJ_NAME)_PROJECT_DEFS) \
        $(CFLAGS_OPT) \
        $(OS_DEFINE) \
        $(CPU_FLAGS) \
        $(LFLAGS) \
        $(AD_CFLAGS) \
        $($(PROJ_NAME)_AD_CFLAGS) \
        $($(PROJ_NAME)_CDEBUG) \
        $($(PROJ_NAME)_CMD_CFLAGS) \
		$($(PROJ_NAME)_W_OPT)

$(PROJ_NAME)_INCLUDE_CL := $(addprefix /I ,$($(PROJ_NAME)_PROJECT_PATH))
$(PROJ_NAME)_INCLUDE_MD := $(addprefix -I,$($(PROJ_NAME)_PROJECT_PATH))

$(PROJ_NAME)_MDFLAGS    := $(subst /D,-D, $(filter /D%, $($(PROJ_NAME)_CFLAGS_CL)))
$(PROJ_NAME)_MDFLAGS    += -D_MSC_VER=1600

$(P_PROJ_NAME)_CFLAGS_CL := $(filter-out $(P_PROJ_NAME)_$(W_OPT) ,$($(P_PROJ_NAME)_CFLAGS_CL))
#$(warning "cpssEnabler_CFLAGS_CL=$(cpssEnabler_CFLAGS_CL)")






CPSS_SUBDIRS_LIST += $(P_PROJ_NAME)
CPSS_SUBDIRS_LIST_CAP := $(CPSS_SUBDIRS_LIST_CAP) $(PROJ_NAME_CAP)
include $(patsubst %,%/$(MAKEFILE),$(P_PROJ_NAME))
SRC := $(SRC) $($(PROJ_NAME_CAP)_SRC)

#$(PROJ_NAME_CAP)_DEP  := $($(PROJ_NAME_CAP)_OBJ:.obj=.dep)
$(PROJ_NAME_CAP)_OBJ   := $(patsubst %.c,$(OUT_DIR)/%.obj,$($(PROJ_NAME_CAP)_SRC))
$(PROJ_NAME_CAP)_LIB   := $(OUT_DIR)/$(P_PROJ_NAME)/libs/$(CPSS_ARCH)/$(P_PROJ_NAME).lib
$(PROJ_NAME_CAP)_DIRS  := $(addprefix $(OUT_DIR)/,$(sort $(dir $($(PROJ_NAME_CAP)_SRC))))
$(eval $(P_PROJ_NAME)_CFLAGS_CL := $(sort $(filter-out $(addprefix $(FD),$(DEL_FLAGS)),$($(P_PROJ_NAME)_CFLAGS_CL) $($(P_PROJ_NAME)_AD_CFLAGS))))
$(P_PROJ_NAME)_FLAGS := $($(P_PROJ_NAME)_CFLAGS_CL) $($(P_PROJ_NAME)_INCLUDE_MD)



$(OUT_DIR)/$(P_PROJ_NAME)/%.obj: P_PROJ_NAME := $(P_PROJ_NAME)
$(OUT_DIR)/$(P_PROJ_NAME)/%.obj: $(P_PROJ_NAME)/%.c
	$(call make_depend,$($(P_PROJ_NAME)_MDFLAGS),$($(P_PROJ_NAME)_INCLUDE_MD))
	$(CXX) $($(P_PROJ_NAME)_CFLAGS_CL) $(INCLUDE_VC10_CL) $($(P_PROJ_NAME)_INCLUDE_CL) /c /Fo$@ $< $(if $(COMPILATION_TYPE),> nul,)


# create module .lib
$($(PROJ_NAME_CAP)_LIB): $($(PROJ_NAME_CAP)_OBJ)
	$(call make_lib)

ifeq (Y, $(MK_DBG))
        FOREACH_TEST := $(foreach CUR_FILE,$($(P_PROJ_PATH)_PROJECT_DEFS) ,$(info "::,$(CPU_BOARD),$(CPU_PRODUCT),$(P_PROJ_PATH)_PROJECT_DEFS,$(CUR_FILE),"))
        FOREACH_TEST := $(foreach CUR_FILE,$($(P_PROJ_PATH)_W_OPT)        ,$(info "::,$(CPU_BOARD),$(CPU_PRODUCT),$(P_PROJ_PATH)_W_OPT,$(CUR_FILE),"))
        FOREACH_TEST := $(foreach CUR_FILE,$($(P_PROJ_PATH)_AD_CFLAGS)    ,$(info "::,$(CPU_BOARD),$(CPU_PRODUCT),$(P_PROJ_PATH)_AD_CFLAGS,$(CUR_FILE),"))
        FOREACH_TEST := $(foreach CUR_FILE,$($(P_PROJ_PATH)_CFLAGS_CL)    ,$(info "::,$(CPU_BOARD),$(CPU_PRODUCT),$(P_PROJ_PATH)_CFLAGS_CL,$(CUR_FILE),"))
endif
