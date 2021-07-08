#
# Make target definitions for Microsoft Visual C++ 10
#

##flags for makedepend external utility - used for both BC and VC
#MDFLAGS0 = $(subst /D,-D, $(CFLAGS))
#MDFLAGS1 = $(subst /W3,, $(MDFLAGS0))
#MDFLAGS2 = $(subst /Od,, $(MDFLAGS1))
#MDFLAGS3 = $(subst /Gm,, $(MDFLAGS2))
#MDFLAGS4 = $(subst /GX,, $(MDFLAGS3))
#MDFLAGS5 = $(subst /ZI,, $(MDFLAGS4))
#MDFLAGS6 = $(subst /nologo,, $(MDFLAGS5))
#MDFLAGS7 = $(subst /GZ,, $(MDFLAGS6))
#MDFLAGS8 = $(subst /RTC1,, $(MDFLAGS7))
#MDFLAGS9 = $(subst /EHsc,, $(MDFLAGS8))
#MDFLAGS = $(subst /MTd,, $(MDFLAGS9))
MDFLAGS = $(subst /D,-D, $(filter /D%, $(CFLAGS)))
MDFLAGS += -D_MSC_VER=1600

# template for creating library
# args
#   $1 lib name
#   $2 object list
define AR_template
	@ $(ECHO) Generating $(notdir $(1))...
    @ if [ -z "$$ROL_WINE_ENV" ]; then \
	    $(AR) /nologo /out:$(1) $(wildcard $(1)) $(2) /IGNORE:4006; \
      else \
	    $(AR) /nologo /out:$(1) $(wildcard $(1)) $(patsubst /%.obj, Z:/%.obj, $(2)) /IGNORE:4006; \
      fi;
endef

APP_DIR = $(subst \,/,$(PROJ_BSP_DIR))
ifdef ROL_WM_BUILD_SYSTEM
APP_DIR := $(LIB_DIR)
endif

# the dependencies tool not work in VC --> need a fix
# in the meantime remove the building of *.dep files
ifndef ROL_WINE_ENV
$(OBJ_FILE_LIST) : %.obj : %.c
else
$(OBJ_FILE_LIST) : %.obj : %.c $(SCRAP_DIR)/%.dep
endif
	$(CC) $(CFLAGS) $(OS_CFLAGS) $(INCLUDE_PATH) $(PROJECT_PATH) $(FC) /Fo$(SCRAP_DIR)/$@ /Fd$(APP_DIR)/vc100.pdb $(filter %/$(notdir $<),$(C_FILE_LIST_AFTER_EXCLUDE))
$(CXX_OBJ_FILE_LIST) : %.obj : %.cpp
	$(CXX) $(CXXFLAGS) $(OS_CFLAGS) $(INCLUDE_PATH) $(PROJECT_PATH) $(FC) /Fo$(SCRAP_DIR)/$@ /Fd$(APP_DIR)/vc100.pdb $(filter %/$(notdir $<),$(CXX_FILE_LIST_AFTER_EXCLUDE))


 QAC_PROJECT_PATH = $(subst /I ,-i , $(PROJECT_PATH))
# QAC_FLAGS = $(subst /D,-d , $(AD_CFLAGS))
 QAC_FLAGS_ = $(filter /D%, $(CFLAGS)) 
 QAC_FLAGS = $(subst /D,-d , $(QAC_FLAGS_)) 
 QAC_PERSONALITIES = $(subst \,/,$(USER_BASE))/tools/Prqa/personalities.via 
