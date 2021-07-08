####################################################################
# gtTargetsTopMake.mk
#
# DESCRIPTION:
#       Common CPSS Target rules
#       This is a collection of rules required to build
#       CPSS components
#
# DEPENDENCIES:
#       None
#
# FILE REVISION NUMBER:
#       $Revision: 8 $
####################################################################

ifeq (YES, $(QUIET))
	QUIET_CALL=@
else
	QUIET_CALL=
endif

AD_CFLAGS += $(addprefix $(FD),$(sort $(PROJECT_DEFS)))
OS_DEFINE += $(addprefix $(FD),$(sort $(OS_DEFS)))

ifeq (D_ON,$(DEBUG_INFO))
   CFLAGS += $(FD)DEBUG
endif

#
# add the H files of includes , to find what C files to compile
# a change in H file force all C files that has include to it to compile
#
# this section was pulled out from the gtDefsTopMake.mk file in order
# to allow the change of value $(DEP_FILES) after calling gtDefsTopMake.mk
# (before calling gtTargetsTopMake.mk)
#
ifneq (,$(DEP_FILES))
  # MAKECMDGOALS is not one of dep clean_p mkld
  ifeq (, $(filter _$(MAKECMDGOALS),_dep _clean_p _mkld))
    include $(DEP_FILES)
  endif
endif

all: dep touch compile
	+$(QUIET_CALL)set -e;$(DO_LOOP);$(DO_KERNELLOOP)

ifeq (, $(wildcard $(USER_BASE)/tools/build/toolkit/$(TOOLKIT_MAKE_RULES)Targets.mk))
$(error "Make defs (build/toolkit/$(TOOLKIT_MAKE_RULES)Targets.mk) not found)")
endif
include $(USER_BASE)/tools/build/toolkit/$(TOOLKIT_MAKE_RULES)Targets.mk

# removed Level0 conditionned directories create
# build_dir dependence added instead 
dep: build_dir $(DEP_FILE_LIST)
	+$(QUIET_CALL)set -e;$(DO_LOOP);$(DO_KERNELLOOP)

lib: localToLib
	+$(QUIET_CALL)$(DO_LOOP);$(DO_KERNELLOOP)

localToLib: compile
	+$(QUIET_CALL)$(DO_LOOP);$(DO_KERNELLOOP)
ifneq (0, $(words $(OBJ_FILES)))
	$(call AR_template,$(LIB_DIR)/$(LIB_NAME_FIX),$(OBJ_FILES))
endif
ifneq (0, $(words $(ASM_OBJ_FILES)))
	$(call AR_template,$(LIB_DIR)/$(LIB_NAME_FIX),$(ASM_OBJ_FILES))
endif

onelib: localToOneLib
	+$(QUIET_CALL)$(DO_LOOP);$(DO_KERNELLOOP)

localToOneLib: compile
ifneq (0, $(words $(OBJ_FILES)))
	$(call AR_template,$(LIB_DIR)/$(PROJ_NAME).$(LIBEXT),$(OBJ_FILES))
endif
ifneq (0, $(words $(ASM_OBJ_FILES)))
	$(call AR_template,$(LIB_DIR)/$(PROJ_NAME).$(LIBEXT),$(ASM_OBJ_FILES))
endif

#lib_full is command needed to do both "one lib and all libs"
#but with better efficient then separated commands --> for VC
lib_full: localToLib_full
	+$(QUIET_CALL)$(DO_LOOP);$(DO_KERNELLOOP)

localToLib_full:
ifneq (0, $(words $(OBJ_FILES)))
	$(call AR_template,$(LIB_DIR)/$(LIB_NAME_FIX),$(OBJ_FILES))
endif
ifneq (0, $(words $(ASM_OBJ_FILES)))
	$(call AR_template,$(LIB_DIR)/$(LIB_NAME_FIX),$(ASM_OBJ_FILES))
endif


compile: $(OBJ_FILE_LIST) $(ASM_OBJ_FILE_LIST)
		+@$(QUIET_CALL)set -e;$(DO_LOOP)

local: $(OBJ_FILE_LIST) $(ASM_OBJ_FILE_LIST) localToLib localToOneLib


qac_full: qac_dir dep qac qac_rep

qac: $(QAC_FILE_LIST)
	$(QUIET_CALL)set -e;$(DO_LOOP)

qac_rep: $(QAC_REP_FILE_LIST)
	$(QUIET_CALL)set -e;$(DO_LOOP)

touch:
ifneq (0,$(words $(TOUCH_FILE_LIST)))
	$(TOUCH) $(addprefix $(SCRAP_DIR)/,$(notdir $(TOUCH_FILE_LIST:.c=.o)))
	$(RM) $(addprefix $(SCRAP_DIR)/,$(notdir $(TOUCH_FILE_LIST:.c=.o)))
endif

PARTIAL_BUILD_FLAGS = -r

$(DEP_FILE_LIST) : %.dep : %.c
	$(CC) $(CFLAGS) $(INCLUDE_PATH) $(PROJECT_PATH) -M -o $(SCRAP_DIR)/$@ $(filter %/$(notdir $<),$(C_FILE_LIST_AFTER_EXCLUDE))

clean_loop:
	$(QUIET_CALL)$(DO_LOOP);$(DO_KERNELLOOP)
ifneq (0,$(words $(DEP_FILES)))
	$(RM) $(DEP_FILES)
endif
ifneq (0,$(words $(OBJ_FILES)))
	$(RM) $(OBJ_FILES)
endif
ifneq (0,$(words $(ASM_OBJ_FILES)))
	$(RM) $(ASM_OBJ_FILES)
endif
ifneq (0,$(words $(wildcard $(LIB_DIR)/$(LIB_NAME_FIX))))
	$(RM) $(LIB_DIR)/$(LIB_NAME_FIX)
endif

.PHONY: clean_p
clean_p: clean_loop
ifneq (0,$(words $(wildcard $(LIB_DIR)/$(ARCH_NAME).map)))
	$(RM) $(LIB_DIR)/$(ARCH_NAME).map
endif

# used "-p" flag: create all the path without "alredy exist" warning
build_dir: $(SCRAP_DIR) $(LIB_DIR)

$(SCRAP_DIR):
	- @ $(MKDIR) -p $(SCRAP_DIR)

$(LIB_DIR):
	- @ $(MKDIR) -p $(LIB_DIR)


