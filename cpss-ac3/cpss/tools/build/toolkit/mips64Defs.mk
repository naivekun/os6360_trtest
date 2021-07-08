#
# Make definitions for GNU based tools (mips64)
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
LDFLAGS=$(_ENDIAN_FLAG) -m elf64btsmip -r -X -N
BM_SHM_LDFLAGS_LD =$(_ENDIAN_FLAG) -m elf64btsmip -ldl -Bsymbolic-functions
BM_SHM_LDFLAGS_CC=-meb -mabi=64 -msoft-float -fpic -ldl -Xlinker -Bsymbolic-functions
export LDFLAGS

CFLAGS1 += -B$(COMPILER_ROOT)

#Add a C definition to distiguish between different OS (Added for FreeBSD)
ifdef OS_TARGET
  CFLAGS1 =  -Wall $(CC_ARCH) -D$(OS_TARGET) -DOS_TARGET=$(OS_TARGET) -DOS_TARGET_RELEASE=$(OS_TARGET_RELEASE)
else
  CFLAGS1 =  -Wall $(CC_ARCH) -DOS_TARGET=$(OS_RUN)
endif

ifeq (YES,$(SO_LIB))
  CFLAGS1 += -fPIC
endif

#CFLAGS2 = -ansi -pedantic -fvolatile -fno-builtin -funroll-loops
CFLAGS2 = -ansi -fno-builtin -funroll-loops
ifeq (3, $(GCCVER))
  CFLAGS2 += -fvolatile
endif
CFLAGS2 += -D__WORDSIZE=64

ifeq (1,$(SHARED_MEMORY))
  CFLAGS2 += -fPIC -DPIC
endif


ifeq (D_ON,$(DEBUG_INFO))
  CFLAGS_OPT = -g -O0
else
  CFLAGS_OPT = -O2
endif

BUILD_MAP_FILE= -Map 

