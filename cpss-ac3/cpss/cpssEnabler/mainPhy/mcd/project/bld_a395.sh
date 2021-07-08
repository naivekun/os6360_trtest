#!/usr/bin/env bash

##  WORKING_DIRECTORY             currect directory (pwd)
##  LINUX_DIST_PATH               $WORKING_DIRECTORY/linux 
##  LSP_USER_BASE                 $WORKING_DIRECTORY/lsp 
##  APPX_PATH                     $WORKING_DIRECTORY/application  

# set environment variables
if [ -z "${WORKING_DIRECTORY}" ]
then
    export WORKING_DIRECTORY=`pwd`
fi 

# kernel will be built in this dir
if [ -z $LINUX_DIST_PATH ]
then
    export LINUX_DIST_PATH="$WORKING_DIRECTORY/linux"
fi 


# LSP. Build script will look $LSP_USER_BASE/uni/3.14/kernel
# set LSP_USER_BASE
if [ -z  $LSP_USER_BASE ]
then
    export LSP_USER_BASE="$WORKING_DIRECTORY/lsp"
fi 


# application path
# set APPX_PATH 
if [ -z  $APPX_PATH  ]
then
    export APPX_PATH="$WORKING_DIRECTORY"
fi 

# directory for objects
OBJS=$APPX_PATH/compilation_root
# directory for images
IMAGES=$APPX_PATH/images
mkdir -p $IMAGES

# uncomment to skip kernel build
export LINUX_BUILD_KERNEL=NO

# uncomment to build in 24 streams
export MAKEFLAGS=-j24

# comment next line for verbose output
#silent="-f -s"


# now build
bash $APPX_PATH/Build.sh \
	$silent \
	-R $IMAGES \
	-L \
	Linux ARMADA_390 Linux \
	NOOP NOOP APPX \
	$OBJS $IMAGES
