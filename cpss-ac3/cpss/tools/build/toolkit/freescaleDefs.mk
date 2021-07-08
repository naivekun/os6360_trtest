#
# Make definitions for ELDK toolkit
#

#start of include
FIS= -I
#end of include
FIE=
FD=-D
FO=-o
FC=-c
FG=-g
OBJ=o
ASM=s
LIBEXT=a

ifeq (BE, $(ENDIAN))
  _ENDIAN_FLAG=-EB
else
  _ENDIAN_FLAG=-EL
endif

LDFLAGS=$(_ENDIAN_FLAG) -r -X -N
export LDFLAGS
LDFLAGS_CC = $(XCOMP_ROOT_PATH)/lib/ld.so.1
BM_SHM_LDFLAGS_LD =$(_ENDIAN_FLAG) -ldl -Bsymbolic-functions
BM_SHM_LDFLAGS_CC=-Xlinker $(_ENDIAN_FLAG) -ldl -Xlinker -Bsymbolic-functions

CPU_FLAGS += -m32
ifneq (, $(wildcard $(DIST_HOME)/include/linux/version.h))
CPU_FLAGS += -I$(DIST_HOME)/include -I$(DIST_HOME)/arch/$(CPU_TOOL)/include
endif


CFLAGS1 += -B$(COMPILER_ROOT)

CFLAGS1 =  -Wall $(CC_ARCH) -DOS_TARGET=$(OS_RUN)

CFLAGS2 = -ansi -fno-builtin -funroll-loops

#Shared library approach requires Position-Independent Code
ifeq (1,$(SHARED_MEMORY))
  CFLAGS2 += -fPIC -DPIC
endif

ifeq (D_ON,$(DEBUG_INFO))
  CFLAGS_OPT = -g -O0
else
  CFLAGS_OPT = -O2
endif

BUILD_MAP_FILE= -Map 

