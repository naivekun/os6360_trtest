#!/bin/bash

#############################################################################
# SCRIPT:   doimage_cm3.sh
# USAGE:    ./doimage_cm3.sh [-d <core ID>] [-g gap] -i <input binary image> -o <output file>
#
# PURPOSE:  create bootable boot image for CM3 systems.
#           The binary image is prefixed with a 32byte header
#           with size and CRC
# Options:
#   -d core ID                      0-3
#   -c Customer header file         File to insert into the booton's gap.
#   -g Gap                          Gap size for customer_file.
#   -i Input file
#   -o Ouput file
#
#  Header version 0
#        field          size(bytes)  offset(bytes)
#    magic              U32       4    0
#    version            U8        1    4
#    hdrSize            U8        1    5
#    flags              U8        1    6
#    coreId             U8        1    7
#    imageOffset        U32       4    8
#    imageSize          U32       4    12
#    imageChksum        U32       4    16
#    rsvd1              U32       4    20
#    rsvd2              U32       4    24
#    rsvd3              U16       2    28
#    ext                U8        1    30
#    hdrChksum          U8        1    31
#############################################################################
#                      Script Starts Here                                   #
#############################################################################

set -e
# Any subsequent commands which fail will cause the shell script to exit immediately

function swap {
    result=$(( $(( $1 >> 24 )) + $(( ($1 & 0xFF0000) >> 8 )) + $(( ($1 & 0xFF00) << 8 )) + $(( ($1 & 0xFF) << 24)) ))
    printf "0x%x" $result
}

command -v crc32 >/dev/null 2>&1 ||
    { echo >&2 "This script requires installation of crc32.  Aborting."; exit 1; }

MAGIC=CM3I
IMAGE_HEADER_VERSION=0
LOG_FILE=outputLogFile.txt
HEADER_FILE=image_cm3_header.txt
HEADER_BIN_FILE=image_cm3_header.bin
IMAGE_HEADER_SIZE=32
GAP_FILE=/dev/zero

BUILD_PATH="$(dirname $0)"
core_id=0
bin_file_offset=$IMAGE_HEADER_SIZE

while getopts ":hg:c:d:i:o:" opt;
do
    case $opt in
    h) # help
        echo "Usage: ./doimage_cm3.sh [-d <core ID>] [-g <image offset>] -i <input binary image> -o <output file>"
        echo
        exit 0
        ;;
    i) # input binary
        i="$OPTARG"
        if [ $i == '' ] 
        then
            echo "Input file not specified1!"
            exit 1
        fi
        if [ ! -f $i ]
        then
            echo "Input file $i not found!"
            exit 1
        else
            BIN_FILE=$i
        fi
        ;;
    o) # output binary
        o="$OPTARG"
        if [ $o != '' ]
        then
            output_file=$o
            rm -rf $o
        else
            echo "Output file not specified!"
            exit 0
        fi
        ;;
    c) # customer header
        c="$OPTARG"
        if [ $c != '' ]
        then
            GAP_FILE=$c
        else
            echo "Customer header file not specified. Gap will be zeroed"
        fi
        ;;		
    g) # Offset to start of binary image
        g="$OPTARG"
        if [ $g != '' ]
        then
            bin_file_offset=$g
        else
            echo "Wrong gap size was provided"
            exit 0
        fi
        ;;
    d) # core ID
        d="$OPTARG"
        if [ $d -gt 3 ]
        then
            echo "Wrong core ID $d. Range is 0-3."
            exit 0
        else
            core_id=$d
        fi
        ;;
    esac
done

if [ -z $BIN_FILE ] 
then
    echo "Input file not specified!"
    exit 1
fi
if [ -z $output_file ] 
then
    echo "Output file not specified!"
    exit 1
fi


echo "Creating CM3 bootable image:"

# 1) get size of bin file
#-------------------------------------------------------------------------
bin_file_size=$(stat -c %s $BIN_FILE);

# 2) add image magic to header file
#------------------------------------------
echo Image magic        : ${MAGIC} > ${LOG_FILE}
source $BUILD_PATH/header_add_string_field.sh $HEADER_FILE 4 ${MAGIC}

# 3) add header version to header file
#------------------------------------------
echo Header version     : ${IMAGE_HEADER_VERSION} >> ${LOG_FILE}
source $BUILD_PATH/header_add_int_field.sh $HEADER_FILE 1 ${IMAGE_HEADER_VERSION}

# 4) add header size to header file
#------------------------------------------
echo Header size    : ${IMAGE_HEADER_SIZE} >> ${LOG_FILE}
source $BUILD_PATH/header_add_int_field.sh $HEADER_FILE 1 ${IMAGE_HEADER_SIZE}

# 5) add flags field to header file
#------------------------------------------
echo Flags    : 0 >> ${LOG_FILE}
source $BUILD_PATH/header_add_int_field.sh $HEADER_FILE 1 0

# 6) add core ID field to header file
#------------------------------------------
echo Core ID    : ${core_id} >> ${LOG_FILE}
source $BUILD_PATH/header_add_int_field.sh $HEADER_FILE 1 $core_id

# 7) add image offset field to header file
#------------------------------------------
echo Binary image offset    : ${bin_file_offset} >> ${LOG_FILE}
tmp_swap=`swap $bin_file_offset`
source $BUILD_PATH/header_add_int_field.sh $HEADER_FILE 4 $tmp_swap

# 8) add image size to header file
#------------------------------------------
echo Binary image size    : ${bin_file_size} >> ${LOG_FILE}
tmp_swap=`swap $bin_file_size`
source $BUILD_PATH/header_add_int_field.sh $HEADER_FILE 4 $tmp_swap

# 9) calculate crc on the bin file
#-------------------------------------
bin_file_crc32=0x$(crc32 $BIN_FILE)
echo Binary image crc    : ${bin_file_crc32} >> ${LOG_FILE}
tmp_swap=`swap $bin_file_crc32`
source $BUILD_PATH/header_add_int_field.sh $HEADER_FILE 4 $tmp_swap

# 10) add rsvd fields to header file
#------------------------------------------
echo rsvd1    : 0 >> ${LOG_FILE}
source $BUILD_PATH/header_add_int_field.sh $HEADER_FILE 4 0
echo rsvd2    : 0 >> ${LOG_FILE}
source $BUILD_PATH/header_add_int_field.sh $HEADER_FILE 4 0
echo rsvd3    : 0 >> ${LOG_FILE}
source $BUILD_PATH/header_add_int_field.sh $HEADER_FILE 2 0

# 13) add ext field to header file
#------------------------------------------
echo Ext    : 0 >> ${LOG_FILE}
source $BUILD_PATH/header_add_int_field.sh $HEADER_FILE 1 0

# 14) calculate header checksum
#-------------------------------------
xxd -r -p $HEADER_FILE > $HEADER_BIN_FILE
hdr_file_crc=0x$($BUILD_PATH/sum8.sh $HEADER_BIN_FILE)
echo Header checksum    : ${hdr_file_crc} >> ${LOG_FILE}
source $BUILD_PATH/header_add_int_field.sh $HEADER_FILE 1 $hdr_file_crc

# 15) convert header to binary
#----------------------------
xxd -r -p $HEADER_FILE > $HEADER_BIN_FILE

# 16) combine header with bin file
#------------------------------------------------
dd if=$HEADER_BIN_FILE of=$output_file bs=32 count=1
dd if=$GAP_FILE of=$output_file bs=1 count=$(($bin_file_offset - $IMAGE_HEADER_SIZE)) seek=$IMAGE_HEADER_SIZE
dd if=$BIN_FILE of=$output_file bs=1 count=$bin_file_size seek=$(printf "%d" $bin_file_offset)

# delete unnecessary files
rm $LOG_FILE
rm $HEADER_FILE
rm $HEADER_BIN_FILE
unset IFS

echo Build Image Done

exit
