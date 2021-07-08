#!/bin/bash

#############################################################################												
# SCRIPT:  build_extended_image.sh
# USAGE:   ./build_extended_image.sh
# PURPOSE: The script create and image file with booton at start, followed by
#          super_image.file (it's optional to insert a vendor specific file
#          into the booton's "gap").
#############################################################################	


#############################################################################
#                      Script Starts Here                                   #
#############################################################################

set -e
# Any subsequent commands which fail will cause the shell script to exit immediately

SUPER_IMAGE_OFFSET=0x40000 #256K
EXTENDED_IMAGE_FILE=extended_image.bin
ZERO_FILL_FILE=zero_fill.bin

BOOTON_BIN_FILE_TMP=booton.bin.tmp
BOOTON_BIN_FILE=booton.bin
EXTENDED_IMAGE_NO_HDR=extended_image.noheader

FDOIMAGECM3=0         # Flag indicating to prepare CM3 image file
DOIMAGE=.             # "doimage" utility path
DOIMAGE_CM3="$(dirname $0)"  # "doimage" utility path
IMAGE_CPU=$1
CORE_ID=$3

# Check existence of the first parameter
if [ "x$IMAGE_CPU" == "xMSYS" ]; then
	echo "Do MSYS image"
else
	FDOIMAGECM3=1
	echo "Do CM3 image"
fi

if [ "$GAP_SIZE" != "" ]
then
BOOTON_HDR_GAP_SIZE=$(printf "%d\n" $GAP_SIZE)
GAP_SIZE=$((BOOTON_GAP_FILE_SIZE + $GAP_SIZE + 4095))
MOD=$((GAP_SIZE%4096))
    GAP_SIZE=$(($GAP_SIZE-$MOD));
GAP_SIZE=$(printf "0x%x" $((GAP_SIZE)))
else
GAP_SIZE=0x20
fi

EXEC_OFFSET=$(printf "0x%x" $((0xD4000000 + $GAP_SIZE + $2)) )
[ ! -f $BOOTON_NOHDR_BIN_FILE ] && { echo "$BOOTON_NOHDR_BIN_FILE file not found"; exit 1; }

if [ "$BOOTON_GAP_FILE" != "" ]
then
    [ ! -f $BOOTON_GAP_FILE ] && { echo "$BOOTON_GAP_FILE file not found"; exit 1; }
fi

# Create booton.bin with doimage header
if [ $FDOIMAGECM3 -eq 0 ]; then
	${DOIMAGE}/doimage -T flash -D 0xFFFFFFFF -E $EXEC_OFFSET -S $GAP_SIZE $BOOTON_NOHDR_BIN_FILE $BOOTON_BIN_FILE
else
	${DOIMAGE_CM3}/doimage_cm3.sh -g $GAP_SIZE -i $BOOTON_NOHDR_BIN_FILE -o $BOOTON_BIN_FILE -d $CORE_ID
fi

[ ! -f $BOOTON_BIN_FILE ] && { echo "$BOOTON_BIN_FILE file not found"; exit 1; }

BOOTON_FILE_SIZE=$(stat -c %s ${BOOTON_BIN_FILE})
ZERO_FILE_SIZE=$(($SUPER_IMAGE_OFFSET - $BOOTON_FILE_SIZE))


if [[ ! $ZERO_FILE_SIZE -gt 0 ]]
then
    echo FILE_SIZE=$BOOTON_FILE_SIZE
    echo "Gap + Customer_File + Booton exceed 256K"
    exit
fi

if [ "$BOOTON_GAP_FILE" != "" ]
then
    dd if=${BOOTON_BIN_FILE} of=${BOOTON_BIN_FILE_TMP} bs=1 count=${BOOTON_HDR_GAP_SIZE}
    dd if=${BOOTON_GAP_FILE} of=${BOOTON_BIN_FILE_TMP} bs=1 seek=${BOOTON_HDR_GAP_SIZE}
    BOOTON_SKIP_AND_SEEK=$(($BOOTON_HDR_GAP_SIZE + $BOOTON_GAP_FILE_SIZE))
    BOOTON_LEFT_BYTES=$(($BOOTON_FILE_SIZE - $BOOTON_SKIP_AND_SEEK))
    dd if=${BOOTON_BIN_FILE} of=${BOOTON_BIN_FILE_TMP} bs=1 count=${BOOTON_LEFT_BYTES} seek=${BOOTON_SKIP_AND_SEEK} skip=${BOOTON_SKIP_AND_SEEK}
else
    cp ${BOOTON_BIN_FILE} ${BOOTON_BIN_FILE_TMP}
fi

dd if=/dev/zero of=${ZERO_FILL_FILE} bs=1 count=${ZERO_FILE_SIZE}

cat $BOOTON_BIN_FILE_TMP $ZERO_FILL_FILE $HEADER_BIN_FILE > $EXTENDED_IMAGE_FILE

rm $BOOTON_BIN_FILE_TMP
rm $BOOTON_BIN_FILE
rm $ZERO_FILL_FILE

echo -e "\nExtended Image created"

return
