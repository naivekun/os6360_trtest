
CROSS_COMPILE ?= /swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi-
CC = $(CROSS_COMPILE)gcc

TOOL=cm3_tool

FILES = \
  cm3_tool.c \
  i2c_lib.c  \
  mvTwsi.c

OBJS = $(patsubst %.c,%.o,$(FILES))

all:
	$(CC) -c cm3_tool.c -I . -O0 -g -o cm3_tool.o
	$(CC) -c i2c_lib.c -I . -O0 -g -o i2c_lib.o
	$(CC) -c mvTwsi.c -I . -O0 -g -o mvTwsi.o
	$(CC) $(OBJS) -o $(TOOL)
