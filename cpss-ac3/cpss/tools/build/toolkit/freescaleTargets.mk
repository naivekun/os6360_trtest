#
# Make target rules for ELDK
#

# template for creating library
# args
#   $1 lib name
#   $2 object list
define AR_template
	$(AR) $(ARFLAGS) $(1) $(2)
endef

$(OBJ_FILE_LIST) : %.o : %.c %.dep
	$(CC) $(CFLAGS) $(OS_CFLAGS) $(INCLUDE_PATH) $(PROJECT_PATH) $(DBGFLAGS) $(FC) $(FO) $(SCRAP_DIR)/$@ $(filter %/$(notdir $<),$(C_FILE_LIST_AFTER_EXCLUDE))
$(ASM_OBJ_FILE_LIST) : %.o : %.$(ASM)
	$(CC) $(CFLAGS) $(OS_CFLAGS) $(INCLUDE_PATH) $(PROJECT_PATH) $(FC) $(FO) $(SCRAP_DIR)/$@ $(filter %/$(notdir $<),$(ASM_FILE_LIST_AFTER_EXCLUDE))


