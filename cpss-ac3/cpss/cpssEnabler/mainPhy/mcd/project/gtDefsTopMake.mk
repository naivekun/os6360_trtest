####################################################################
# gtDefsTopMake.mk
#
# DESCRIPTION:
#       Common CPSS make definitions
#       This is a collection of settings required to build
#       CPSS components
#
# DEPENDENCIES:
#       None
#
# FILE REVISION NUMBER:
#       $Revision: 7 $
####################################################################

####################################################################
#   Common Definitions
####################################################################

ifeq (, $(wildcard $(USER_BASE)/tools/build/toolkit/$(TOOLKIT_MAKE_RULES)Defs.mk))
$(error "Make defs (build/toolkit/$(TOOLKIT_MAKE_RULES)Defs.mk) not found)")
endif
include $(USER_BASE)/tools/build/toolkit/$(TOOLKIT_MAKE_RULES)Defs.mk

#
# Variables defined per toolkit/target
#
# CFLAGS1       - OS specific defs, warninfs control
# CFLAGS2       - C dialect options, code generation options
# CFLAGS_OPT    - code optimization flags,
# CDEBUG        - defined in */gtTopMake.
#                 Usually CDEBUG=-DDEBUG_XXX -DDEBUG_YYY
#
# OS_CFLAGS     - OS specific flags. Used for include paths when build vxWorks
#
# LFLAGS        - ??? never defined
# CPU_FLAGS     - CPU specific parameters (target, endian, defines, etc)
#                 Should be defined in cpu/${CPU}.sh, toolkit/${TOOLKIT}Defs.mk
# DBGFLAGS      - Debug flags. Usually should include no optiomization flags
#                 Defined and used in toolkit/${TOOLKIT}Targets.mk
#
#
# LDFLAGS       - Linker flags, system libraries like -lpthread -lrt, etc
#
#
# Project specific flags, usually defined in XXXX/gtTopMake
#
# USER_BASE     - The CPSS top directory
# CUSTOM_BASE   - The top directory for customer projects.
#                 Has the same structure as CPSS:
#                     $CUSTOM_BASE/Project1/gtTopMake
#                     $CUSTOM_BASE/Project2/gtTopMake
# AD_CFLAGS     - Application defined flags (optional). Will be added to
#                 compiler command line.
#                 Can be defined in gtTopMake, gtBuild
# INCLUDE_PATH  - Application defined include flags (optional). Will be added
#                 to compiler command line.
# CDEBUG        - Debug related proprocessor directives (optional).
#                 For example $(FD)GT_MAC_DEBUG
#                 Can be defined in XXXX/gtTopMake
# CFLAGS        - Defined in XXXX/gtTopMake. Usually
#                 CFLAGS =  $(CFLAGS1) $(CFLAGS2) $(CFLAGS_OPT) \
#                           $(OS_DEFINE) $(CPU_FLAGS) $(AD_CFLAGS) $(CDEBUG)
# PROJ_NAME     - The name of project. Used as LIB_NAME for 'onelib' target
# PROJECT_DEFS  - The list of preprocessor defs to be defined
# PROJECT_PATH  - Include directives $(FIS)$(SW_ROOT)/mainPpDrv/h$(FIE) ...
# BASE_PATH     - include path for current project
#                 Defined as $(FIS)$(SW_ROOT)/$(PROJ_NAME)/h$(FIE)
#                 Usually added to PROJECT_PATH
#
#
# LIB_NAME      - The library name in *$PROJECT/libs/$CPU_DIR/
#
#
# SUBDIRS       - Subrirectories to build.
#                 Default all suibdirs with gtBuild
# KERNELSUBDIRS - Subdirectories with kernel modules.
#                 Default all with Makefile
# C_FILE_LIST   - List of files to compile
#                 Default all .c files
# C_EXCLUDE_FILE_LIST
#               - List of files to be excluded.
#                 Default none
#

export PROJ_BASE = $(dir $(firstword \
    $(wildcard $(USER_BASE)/$(PROJ_NAME)/gtTopMake) \
    $(if $(CUSTOM_BASE),$(wildcard $(CUSTOM_BASE)/$(PROJ_NAME)/gtTopMake),) \
))
SW_ROOT = $(subst \,/,$(USER_BASE))


# the PROJ_MAKE used in build rules in gtTargetsTopMake.mk
# it is the default value that mathces for all libraries
# except the Dune library
# For the dune library it is overridden in gtTopMake file

export PROJ_MAKE = $(wildcard $(SW_ROOT)/$(PROJ_NAME)/gtTopMake $(PROJ_BASE)/gtTopMake)

.PHONY: do_it_all
do_it_all: all


.SUFFIXES:
.SUFFIXES: .c .dep .$(OBJ) .h .$(ASM) .$(LIBEXT)

# by default make it little endian
ifeq ($(ENDIAN), )
ENDIAN = LE
endif

OS_INCLUDE =
ifndef COMPILATION_ROOT
  COMPILATION_ROOT = $(SW_ROOT)
endif

COMPILATION_ROOT_FIX = $(subst \,/,$(COMPILATION_ROOT))

#current project
BASE_PATH = $(FIS)$(SW_ROOT)/$(PROJ_NAME)/h$(FIE)

LIB_DIR = $(COMPILATION_ROOT_FIX)/$(PROJ_NAME)/libs/$(CPU_DIR)
LIB_DIR_FIX = $(subst /,\,$(LIB_DIR))

SCRAP_DIR = $(COMPILATION_ROOT_FIX)/$(PROJ_NAME)/objTemp/$(CPU_DIR)

ARFLAGS =  crus

#
#  Target name
#
ARCH_NAME = $(PROJ_NAME)
EXE_NAME  = temp

vpath %.$(OBJ)  $(SCRAP_DIR)
vpath %.dep  $(SCRAP_DIR)
vpath %.c.err  $(QAC_OUT_DIR)
vpath %.rep  $(QAC_OUT_DIR)

unexport LIB_NAME
ifndef LIB_NAME
 LIB_NAME_FIX = $(notdir $(CURDIR)).$(LIBEXT)
else
 ifeq (,$(suffix $(LIB_NAME)))
  LIB_NAME_FIX = $(LIB_NAME).$(LIBEXT)
 else
  LIB_NAME_FIX = $(subst .lib,.$(LIBEXT),$(LIB_NAME))
 endif
endif

unexport EXE_OBJ_LIST
ifndef EXE_OBJ_LIST
EXE_OBJ_LIST = $(wildcard $(SCRAP_DIR)/*.$(OBJ))
endif

unexport SUBDIRS
unexport KERNELSUBDIRS
ifndef SUBDIRS
 SUBDIRS = $(patsubst  %/,%,$(dir $(wildcard */gtBuild)))
 KERNELSUBDIRS_TMP = $(patsubst  %/,%,$(dir $(wildcard */Makefile)))
 KERNELSUBDIRS = $(filter-out $(SUBDIRS) , $(KERNELSUBDIRS_TMP))
endif
ifeq ($(SUBDIRS) ,NONE)
 SUBDIRS =
 KERNELSUBDIRS =
endif

unexport EXCLUDE_SUBDIRS

################################################################
# Conditional rebuild:
#   If variable BUILD_ONLY defined then it is interpretead as
#   list of directories. For example:
#      cpssEnabler mainPpDrv/src/cpssDriver/pp/hardware/pci mainPpDrv/src/cpssDriver/pp/hardware/smi
#   It means that project will be build/rebuilt for cpssEnabler.
#   Then 'pci' and 'smi' will be rebuild in mainPpDrv/src/cpssDriver/pp/hardware
################################################################
ifdef BUILD_ONLY
  SUBDIRS_FILTER=\
        $(sort \
            $(foreach \
                SUBDIRS_FILTER_O, \
                $(subst $(CURDIR)/,,$(filter $(CURDIR)/%, \
                    $(addprefix $(USER_BASE_REAL)/,$(subst \,/,$(BUILD_ONLY)))) \
                ), \
                $(firstword $(subst /, ,$(SUBDIRS_FILTER_O))) \
            ) \
        )
  ifneq ($(SUBDIRS_FILTER),)
    SUBDIRS:=$(filter $(notdir $(SUBDIRS_FILTER)),$(SUBDIRS))
  endif
endif

SUBDIRS_FIX = $(subst \,/,$(SUBDIRS))
EXCLUDE_SUBDIRS_FIX = $(subst \,/,$(EXCLUDE_SUBDIRS))
KERNELSUBDIRS_FIX = $(subst \,/,$(KERNELSUBDIRS))

unexport C_FILE_LIST
ifndef C_FILE_LIST
C_FILE_LIST = $(wildcard *.c)
endif

ifeq ($(C_EXCLUDE_FILE_LIST) ,ALL)
C_EXCLUDE_FILE_LIST = $(wildcard *.c)
endif

C_FILE_LIST_FIX = $(foreach file,$(C_FILE_LIST),$(dir $(file))$(notdir $(file)))
C_EXCLUDE_FILE_LIST_FIX = $(foreach file,$(C_EXCLUDE_FILE_LIST),$(dir $(file))$(notdir $(file)))
C_FILE_LIST_AFTER_EXCLUDE = $(filter-out $(C_EXCLUDE_FILE_LIST_FIX),$(C_FILE_LIST_FIX))

SUBDIRS_AFTER_EXCLUDE = $(filter-out $(EXCLUDE_SUBDIRS_FIX),$(SUBDIRS_FIX))

vpath %.c $(patsubst %,%:,$(dir $(filter %.c,$(C_FILE_LIST_AFTER_EXCLUDE))))


OBJ_FILE_LIST = $(notdir $(C_FILE_LIST_AFTER_EXCLUDE:.c=.$(OBJ)))
DEP_FILE_LIST = $(notdir $(C_FILE_LIST_AFTER_EXCLUDE:.c=.dep))

DEP_FILES = $(addprefix $(SCRAP_DIR)/,$(DEP_FILE_LIST))
OBJ_FILES = $(addprefix $(SCRAP_DIR)/,$(OBJ_FILE_LIST))
OBJ_FILES_FIX = $(subst /,\,$(OBJ_FILES))


unexport LIB_FILE_LIST
ifndef LIB_FILE_LIST
LIB_FILE_LIST = $(wildcard $(LIB_DIR)/*.$(LIBEXT))
endif

unexport EXE_OBJ_LIST
ifndef EXE_OBJ_LIST
EXE_OBJ_LIST = $(wildcard $(SCRAP_DIR)/*.$(OBJ))
endif



DO_LOOP = for i in $(SUBDIRS_AFTER_EXCLUDE); do $(MAKE) -f gtBuild -C $$i $@; done
DO_KERNELLOOP = for i in $(KERNELSUBDIRS_FIX); do $(MAKE) -f Makefile  -C $$i; done

####################################################################
#   Common Definitions
####################################################################
CFLAGS1 += $(FD)CPU=$(CPU) $(FD)_$(OS_RUN) $(FD)$(CPU_FAMILY) $(FD)CPU_$(ENDIAN) $(DEBUG_TYPE)



