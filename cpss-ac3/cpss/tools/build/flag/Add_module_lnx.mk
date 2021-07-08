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
        $($(PROJ_NAME)_CMD_CFLAGS)

$(PROJ_NAME)_INCLUDE_MD := $(addprefix -I,$($(PROJ_NAME)_PROJECT_PATH))
$(P_PROJ_NAME)_CFLAGS_CL := $(filter-out $(P_PROJ_NAME)_$(W_OPT) ,$($(P_PROJ_NAME)_CFLAGS_CL))



CPSS_SUBDIRS_LIST += $(P_PROJ_NAME)
CPSS_SUBDIRS_LIST_CAP := $(CPSS_SUBDIRS_LIST_CAP) $(PROJ_NAME_CAP)
include $(patsubst %,%/$(MAKEFILE),$(P_PROJ_NAME))
SRC := $(SRC) $($(PROJ_NAME_CAP)_SRC)

$(PROJ_NAME_CAP)_OBJ   := $(patsubst %.c,$(OUT_DIR)/%.o,$($(PROJ_NAME_CAP)_SRC))
$(PROJ_NAME_CAP)_LIB   := $(OUT_DIR)/$(P_PROJ_NAME)/libs/$(CPSS_ARCH)/$(P_PROJ_NAME).a
$(PROJ_NAME_CAP)_DIRS  := $(addprefix $(OUT_DIR)/,$(sort $(dir $($(PROJ_NAME_CAP)_SRC))))
$(eval $(P_PROJ_NAME)_CFLAGS_CL := $(sort $($(P_PROJ_NAME)_CFLAGS_CL) $($(P_PROJ_NAME)_AD_CFLAGS)))
$(P_PROJ_NAME)_FLAGS := $(filter-out $(addprefix $(FD),$(DEL_FLAGS)),$($(P_PROJ_NAME)_CFLAGS_CL) $($(P_PROJ_NAME)_INCLUDE_MD))

###############################################################################
#
# %.o - compile .c files to objects
###############################################################################
$(OUT_DIR)/$(P_PROJ_NAME)/%.o: P_PROJ_NAME := $(P_PROJ_NAME)
$(OUT_DIR)/$(P_PROJ_NAME)/%.o: $(P_PROJ_NAME)/%.c
#	$(call compile,$($(P_PROJ_NAME)_FLAGS),$(OUT_DIR)/$(P_PROJ_NAME))
ifneq ($(DEPENDENCY),DEP_NO)
# create dependency file
	$(CC) -M $($(P_PROJ_NAME)_FLAGS) -c $< | sed -e '1 s/^[^:]*: *//' -e 's/ *\\$$//' | tr ' \011' '\012\012' > $@.tmp
	echo "$@: \\" > $(basename $@).d
	sed -n 's/^[^/].*$$/ & \\/ p' $@.tmp >> $(basename $@).d
	echo "" >> $(basename $@).d
# add empty targets
	sed -n 's/^[^/].*$$/&:/ p' $@.tmp >> $(basename $@).d
	rm $@.tmp
endif
# print compilation msg
	$(if $(COMPILATION_TYPE),,@ $(ECHO) CC $<)
# compile
	$(CC) $($(P_PROJ_NAME)_FLAGS) -c $< -o $@

# create library with all module objects inside
$($(PROJ_NAME_CAP)_LIB): PROJ_NAME_CAP := $(PROJ_NAME_CAP)
$($(PROJ_NAME_CAP)_LIB): $($(PROJ_NAME_CAP)_OBJ)
	$(call make_lib)

ifeq (Y, $(MK_DBG))
        FOREACH_TEST := $(foreach CUR_FILE,$($(P_PROJ_NAME)_PROJECT_DEFS) ,$(info "::,$(CPU_BOARD),$(CPU_PRODUCT),$(P_PROJ_NAME)_PROJECT_DEFS,$(CUR_FILE),"))
        FOREACH_TEST := $(foreach CUR_FILE,$($(P_PROJ_NAME)_W_OPT)        ,$(info "::,$(CPU_BOARD),$(CPU_PRODUCT),$(P_PROJ_NAME)_W_OPT,$(CUR_FILE),"))
        FOREACH_TEST := $(foreach CUR_FILE,$($(P_PROJ_NAME)_AD_CFLAGS)    ,$(info "::,$(CPU_BOARD),$(CPU_PRODUCT),$(P_PROJ_NAME)_AD_CFLAGS,$(CUR_FILE),"))
        FOREACH_TEST := $(foreach CUR_FILE,$($(P_PROJ_NAME)_CFLAGS_CL)    ,$(info "::,$(CPU_BOARD),$(CPU_PRODUCT),$(P_PROJ_NAME)_CFLAGS_CL,$(CUR_FILE),"))
endif
