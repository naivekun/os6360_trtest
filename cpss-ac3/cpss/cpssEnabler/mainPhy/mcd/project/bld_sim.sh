#!/usr/bin/env bash

export APPX_PATH=`pwd`
OBJS=`pwd`/compilation_root
IMAGES=`pwd`/images
mkdir -p $IMAGES

bash -x $APPX_PATH/Build.sh \
	Linux i386 LinuxWM \
	NOOP NOOP APPX \
	$OBJS $IMAGES
