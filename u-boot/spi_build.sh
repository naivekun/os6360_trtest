#!/bin/bash
DATE=`date +%m-%d`
VER="bxh-$DATE" 
./build.pl -f spi -i nand:spi -v $VER -b ac3s_rd -c -m 3

