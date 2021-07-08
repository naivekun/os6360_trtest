#
# Make definitions for Microsoft Visual C++ 10
#

#start of include
FIS = /I "
#end of include
FIE = "
FD=/D
FO=/Fo
FC=/c
FG=/Zi
OBJ=obj
ASM=asm
LIBEXT=lib


OS_DEFS += _VISUAL

CFLAGS1 = /nologo /DWIN32 /D_WIN32 /DNDEBUG /D_MBCS /D_VISUALC /DIMAGE_HELP_SHELL /DAPPLICATION_SYMBOL_TABLE /D_CONSOLE /D_LIB
CFLAGS2 =
ifeq (64,$(WORDSIZE_REQUIRED))
 CFLAGS2 += /D__WORDSIZE=64
endif
CXXFLAGS2 = $(CFLAGS2)

CFLAGS_OPT := /Od /EHsc /RTC1 /MTd /D_CRT_SECURE_NO_WARNINGS /FC

ifneq (D_OFF,$(DEBUG_INFO))
    # CFLAGS_OPT += /ZI
    # CFLAGS_OPT += /Gm
    CFLAGS_OPT += /Z7
endif

ifneq (dep,$(MAKECMDGOALS))
### add this flag only on compilation and not on make depend to avoid next warnings:
### makedepend: warning:  cannot open "/Zp1"
  ifeq (simulation, $(PROJ_NAME))
    CFLAGS_OPT += /Zp1
  endif
endif

BUILD_MAP_FILE=/Fm
