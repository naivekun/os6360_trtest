#eepromer  Makefile

CC	= arm-marvell-linux-gnueabi-gcc
AR	= arm-marvell-linux-gnueabi-ar


CFLAGS = -O2 -I../include -Wall

all:  eepromer eeprom

eepromer:  eepromer.o

eeprom:  eeprom.o

clean:
	rm -rf *~ *.o eepromer eeprom
