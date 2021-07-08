

# add V=1 to command line for verbose output
Q := @
ifdef V
  ifeq ("$(origin V)$(V)", "command line1")
    Q :=
  endif
endif


ARCH ?= arm
CROSS_COMPILE ?= /swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi-

BUILD_MODE ?= DEBUG
BUILD_DIR ?= build_dir
#OUT_DIR = $(BUILD_DIR)
BASE_DIR = $(shell pwd)
OUT_DIR ?= $(BASE_DIR)/$(BUILD_DIR)
.PHONY: all
all: link

clean:
	@ rm -rf $(OUT_DIR)/$(ARCH)/

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
CFLAGS =
ifeq ("$(ARCH)" , "arm")
 CPSS_ARCH := ARMV7
else
 ifeq ("$(ARCH)", "x86")
  CPSS_ARCH := i386_64
  CFLAGS +=-D__WORDSIZE=64
  CFLAGS +=-DASIC_SIMULATION
 endif
endif

ECHO = echo

# Name of makefile to include in each sub-module
MAKEFILE := Makefile

MODULES := .


INCLUDE := -I$(BASE_DIR) -I$(BASE_DIR)/include -I$(BASE_DIR)/pps/

CFLAGS += -DCPU_LE $(DEBUG_CFLAGS) -g -O0 -Wall -DUNIX $(INCLUDE) -DMV_CPU_LE

# Function compile(flags, output_folder)
#
# Compile a .c file into .o file using the defined toolchain, including dependecy .d file.
# Object and dependency files will be located together in output_folder,
# under the relative path to the source.
#
# Example: $(call compile $(CFLAGS),/local/objects)
# Source file: foo/bar/baz.c
# Targets: /local/objects/foo/bar/baz.o
#          /local/objects/foo/bar/baz.d

define compile
@# Create ouput folder
	$(Q)$(MKDIR_P) $(dir $@)
@# create dependency file
	$(Q)$(CC) -M -c $1 $< -o $2/$*.d
@# Duplicate it
	$(Q)cp $2/$*.d $2/$*.tmp
@# Create empty rule per file in dependency rule to avoid "No rule to make target ..." Errors
	$(Q)sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/d' -e 's/$$/ :/' < $2/$*.d >> $2/$*.tmp
@# Change object filename to include full path
	$(Q)sed -e 's/$(subst .,\.,$(notdir $@))/$(subst /,\/,$@)/'  < $2/$*.tmp > $2/$*.d
@# Remove temporary file
	$(Q)rm $2/$*.tmp
@# print compilation msg
	@ $(ECHO) CC $<
@# compile
	$(Q)$(CC) -c $1 $< -o $@
endef

SRC := $(shell find . -name "*.c" -not -path "*.git*")
# $(info $(SRC))
OBJ := $(patsubst %.c,$(OUT_DIR)/$(ARCH)/%.o,$(SRC))
DEP := $(patsubst %.o,%.d,$(OBJ))

MKDIR_P = mkdir -p



LIBS :=

###############################################################################
#
# %.o - compile .c files to objects

$(OUT_DIR)/$(ARCH)/%.o:  %.c
	$(call compile,$(CFLAGS),$(OUT_DIR)/$(ARCH))

link: $(OBJ) $(LIBS)
	$(Q)$(MKDIR_P) $(OUT_DIR)/$(ARCH)/
	$(Q)$(ECHO) LD satr
	$(Q)$(LD) $(LDFLAGS) $(OBJ) $(LIBS) -o $(OUT_DIR)/$(ARCH)/satr


install: link
	$(Q)cp $(OUT_DIR)/$(ARCH)/satr $(TARGET_DIR)
	$(Q)$(CROSS_COMPILE)strip $(TARGET_DIR)/satr
