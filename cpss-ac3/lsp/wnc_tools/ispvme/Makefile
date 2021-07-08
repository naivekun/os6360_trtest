OBJ = $(patsubst %.c,%.o,$(wildcard *.c))
CFLAGS = -Wall -g
CC = arm-marvell-linux-gnueabi-gcc
STRIP = arm-marvell-linux-gnueabi-strip
INC =
LIBS =

.PHONY: all clean

vme: ${OBJ}
	${CC} ${OBJ} ${CFLAGS} ${INC} ${LIBS} -o $@
	cp $@ $@_dbg
	${STRIP} $@

%.o: %.c vmopcode.h
	${CC} $< ${CFLAGS} ${INC} ${LIBS} -c

clean:
	@rm -rf *.o vme vme_dbg

