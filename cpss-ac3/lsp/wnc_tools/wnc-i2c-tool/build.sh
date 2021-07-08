#!/bin/bash

#set -x

PWD=`pwd`
sed -i "s#^prefix=.*#prefix=$PWD#g" Makefile

make 

cd $PWD

 