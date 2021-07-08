OBJ = $(patsubst %.c,%.o,$(wildcard *.c))
CFLAGS = -Wall -g
CC = arm-marvell-linux-gnueabi-gcc
STRIP = arm-marvell-linux-gnueabi-strip
INC =
LIBS =

.PHONY: all clean

ipc_to_cpss: ${OBJ}
	${CC} ${OBJ} ${CFLAGS} ${INC} ${LIBS} -o $@

%.o: %.c 
	${CC} $< ${CFLAGS} ${INC} ${LIBS} -c

clean:
	@rm -rf *.o ipc_to_cpss
