#!/bin/bash

#set -x

PWD=`pwd`
sed -i "s#^PREFIX=.*#PREFIX=$PWD#g" Makefile

WITHOUT_XATTR=1 make

cd $PWD
