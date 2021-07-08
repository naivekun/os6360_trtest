WNC_DIR	:= wnc-tool-src

WNC_CFLAGS	:= -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual \
		   -Wcast-align -Wwrite-strings -Wnested-externs -Winline \
		   -W -Wundef -Wmissing-prototypes -Iinclude
ifeq ($(USE_STATIC_LIB),1)
WNC_LDFLAGS	:= $(LIB_DIR)/$(LIB_STLIBNAME)
else
WNC_LDFLAGS	:= -L$(LIB_DIR) -li2c
endif

WNC_TARGETS	:= wnc-tool

$(WNC_DIR)/wnc-tool: $(WNC_DIR)/wnc-tool.o $(WNC_DIR)/util.o $(WNC_DIR)/i2cbusses.o
	$(CC) $(LDFLAGS) -o $@ $^ $(WNC_LDFLAGS)

$(WNC_DIR)/wnc-tool.o: $(WNC_DIR)/wnc-tool.c $(WNC_DIR)/i2cbusses.h $(INCLUDE_DIR)/i2c/smbus.h
	$(CC) $(CFLAGS) $(WNC_CFLAGS) -c $< -o $@

$(WNC_DIR)/i2cbusses.o: $(WNC_DIR)/i2cbusses.c $(WNC_DIR)/i2cbusses.h
	$(CC) $(CFLAGS) $(WNC_CFLAGS) -c $< -o $@

$(WNC_DIR)/util.o: $(WNC_DIR)/util.c $(WNC_DIR)/util.h
	$(CC) $(CFLAGS) $(WNC_CFLAGS) -c $< -o $@

all-prog: $(addprefix $(WNC_DIR)/,$(WNC_TARGETS))

clean-prog:
	$(RM) $(addprefix $(WNC_DIR)/,*.o $(WNC_TARGETS))

all: all-lib clean-prog all-prog
#	cp $(WNC_DIR)/$(WNC_TARGETS) $(SDE_INSTALL)/bin
#	./$(WNC_TARGETS) 0

clean: clean-prog

