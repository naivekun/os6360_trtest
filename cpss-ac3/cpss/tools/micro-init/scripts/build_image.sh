#!/bin/bash

#############################################################################
# SCRIPT:   build_image.sh
# USAGE:    ./build_image.sh [-v <IMAGE_VERSION>] [-e <booton_msys/cm3.noheader>] [-m <CPU type>] [-c <gap File>] [-g <gap size>] <txt File>
#           (txt File is mandatory and must be last)
# PURPOSE:  build image header file and all files headers.
# INPUT:    text file with these sections:
#           path    name    type    execution_copy_offset     execution_offset    file_offset_in_image    use_bmp
# Options:
#   -v Version          set version to header
#                       when unset, IMAGE_VERSION=V1.0
#   -e Extended         supply booton_msys/cm3.noheader. this option create booton_msys/cm3.bin (including "gap", if supplied).
#   -c Customer         File to insert into the booton's gap.
#   -d core ID          0-3
#   -g Gap              gap size for customer_file.
#   -m CPU type         specify the CPU type CM3 or MSYS.
#   -i Image            specify first or second image [1,2] to create,
#                       1 - means offset 0, 2 - offset 0x200000
#                       when unset, the 1-st image will be created (relevant for MSYS only).
#
#  build header version 1
#         type           size(bytes)  offset(bytes)
#    Header magic       U32       4         0
#    Header size        U16       2         4
#    Header version     U16       2         6
#    Header CRC         U32       4         8
#    Header size        U32       4         12
#    Image size         U32       4         16
#    Image CRC          U32       4         20
#    Image version      U8[32]    32        24
#    Image Date + Time  U8[32]    32        56
#    Num of files       U8[4]     4         88
#
#############################################################################
#                      Script Starts Here                                   #
#############################################################################

set -e
# Any subsequent commands which fail will cause the shell script to exit immediately

command -v crc32 >/dev/null 2>&1 ||
    { echo >&2 "Micro-init packing script needed crc32 but it's not installed.  Aborting."; exit 1; }

TXT_FILE=${!#}
IMAGE_VERSION=V1.0
IMAGE_CPU=''
IMAGE_OFFSET=0
BOOTON_GAP_FILE_SIZE=0
BUILD_PATH="$(dirname $0)"
CORE_ID=0

while getopts ":v:e:g:m:c:d:i:" opt;
do
    case $opt in
    v) # Set Version
        v="$OPTARG"
        if [ $v != '' ] && [ ${#v} -lt 33 ]
        then
            IMAGE_VERSION=$v
        else
            echo "Version string larger then 32 characters"
            exit 0
        fi
        ;;
    e) # Create inage with booton
        e="$OPTARG"
        if [ $e != '' ]
        then
            BOOTON_NOHDR_BIN_FILE=$e
            EXTENDED_IMAGE=yes
        else
            echo "No booton file supplied but (-e) enabled"
            exit 0
        fi
        ;;
    g) # Gap to customer file
        g="$OPTARG"
        if [ $g != '' ]
        then
            GAP_SIZE=$g
        else
            echo "Wrong gap size was supplied"
            exit 0
        fi
        ;;
    m) # Specify CPU type CM3
        m="$OPTARG"
        if [ $m != '' ]
        then
            echo "CPU type is $m"
            IMAGE_CPU=$m
        else
            echo "CPU type isn't specified"
	    exit 0
        fi
        ;;
    i) # Specify image layer offset
        i="$OPTARG"
        if [ $i != '' ]
        then
            if [ "$i" == "2" ] ; then
                IMAGE_OFFSET=0x200000
            elif [ "$i" == "1" ] ; then
                IMAGE_OFFSET=0x0
            else
                echo "Image isn't specified correctly"
		exit 0
            fi
        fi
        ;;
    c) # File to insert into booton's "gap"
        c="$OPTARG"
        if [ $c != '' ]
        then
            BOOTON_GAP_FILE=$c
            BOOTON_GAP_FILE_SIZE=$(stat -c %s ${BOOTON_GAP_FILE})
        else
            echo "No file supplied for booton's gap but insertion (-c) enabled"
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
            CORE_ID=$d
        fi
        ;;
    esac
done

echo "Image offset is $IMAGE_OFFSET"

MAGIC=init
IMAGE_HEADER_VERSION=1
CRC_MAGIC=0x78787878
CRC_MAGIC_TEXT=78787878

TMP_CAT_FILE=temp_cat_file.tmp
COMBINE_FILES=combine_files.txt
OUTPUT_FILE=outputPrintFile.txt
TMP_HEADER=tmp_header.txt
HEADER_FILE=image_header.txt
HEADER_BIN_FILE=image_header.bin

# IFS default set to: <space>, <comma>, <tab> separator's
# (needed for separated file values) - don't forget to unset IFS
NOTE=#
# CRC offset's in header
HEADER_CRC_OFFSET=8
IMAGE_CRC_OFFSET=20
# Image & file header size (in bytes)
IMAGE_HEADER_SIZE=92
FILE_HEADER_SIZE=48

if [ "x" == "x$IMAGE_CPU" ]
then
    echo "please specify target CPU"
    exit
fi

if [ -z $TXT_FILE ]
then
    echo "please insert text file"
    exit
fi

if [ -f $HEADER_FILE ]
then rm $HEADER_FILE
fi

if [ -f $HEADER_BIN_FILE ]
then rm $HEADER_BIN_FILE
fi

if [ -f $COMBINE_FILES ]
then rm $COMBINE_FILES
fi

touch $HEADER_FILE
touch $HEADER_BIN_FILE
touch $COMBINE_FILES

echo "Micro-init starts packing image, including files:"

# 1) get size of all files and and sum it up, also count number of all files
#-------------------------------------------------------------------------
[ ! -f $TXT_FILE ] && { echo "text file $TXT_FILE not found. Must be last parameter!"; exit 0; }
while read path name type cpy_offset exec_offset spi_offset hex_bmp || [ -n "$hex_bmp" ]
    do
        if [ "${path:0:1}" == "$NOTE" ]              # skip file notes <#>
            then continue;
        fi
        if  [ ! -z "$path" -a "$path"!=" " ];    # skip null or blank line <#>
            then
            # count number of files
            NUM_OF_FILES=$((NUM_OF_FILES+1))

            # sanity check - exit script when file list entered has wrong value
            if [ ! -f ${path} ]
            then
                echo "file " $NUM_OF_FILES ": wrong path ->" ${path}
                exit
            fi

            if [ ${type} -lt 0 ] || [ ${type} -gt 12 ]
            then
                echo "file " $NUM_OF_FILES ": wrong type (only 0-12) ->" ${type}
                exit
            fi

            if [[ ${hex_bmp} -gt 0xFFFFFFFF ]]
            then
                echo "file " $NUM_OF_FILES ": wrong bitmap ->"  ${hex_bmp}
                exit
            fi

            # Concatenae all files
            cat $COMBINE_FILES ${path} > $TMP_CAT_FILE
            mv $TMP_CAT_FILE $COMBINE_FILES

            # sum all files sizes
            echo File ${NUM_OF_FILES} path: ${path}
            FILE_SIZE=$(stat -c %s ${path})
            TOTAL_FILES_SIZE=$((TOTAL_FILES_SIZE + FILE_SIZE))
        fi

done < $TXT_FILE

TOTAL_HEADER_SIZE=$((IMAGE_HEADER_SIZE+(NUM_OF_FILES*FILE_HEADER_SIZE)))

# 2) add image magic to header file
#------------------------------------------
echo Image magic        : ${MAGIC} > ${OUTPUT_FILE}
source $BUILD_PATH/header_add_string_field.sh $HEADER_FILE 4 ${MAGIC}


# 3) add header size to header file
#------------------------------------------
echo Header size    : ${IMAGE_HEADER_SIZE} >> ${OUTPUT_FILE}
source $BUILD_PATH/header_add_int_field.sh $HEADER_FILE 2 ${IMAGE_HEADER_SIZE}


# 4) add header version to header file
#------------------------------------------
echo Header version     : ${IMAGE_HEADER_VERSION} >> ${OUTPUT_FILE}
source $BUILD_PATH/header_add_int_field.sh $HEADER_FILE 2 ${IMAGE_HEADER_VERSION}


# 5) add empty header crc for calculation
#------------------------------------------
source $BUILD_PATH/header_add_int_field.sh $HEADER_FILE 4 ${CRC_MAGIC}

# 6) add total header size with files headers to header file
#------------------------------------------
echo total header size    : ${TOTAL_HEADER_SIZE} >> ${OUTPUT_FILE}
source $BUILD_PATH/header_add_int_field.sh $HEADER_FILE 4 ${TOTAL_HEADER_SIZE}


# 7) add image size to header file
#------------------------------------------
# image size include all files + header size
TOTAL_IMAGE_SIZE=$(($TOTAL_FILES_SIZE + TOTAL_HEADER_SIZE))
echo Total image size        : $TOTAL_IMAGE_SIZE >> ${OUTPUT_FILE}
source $BUILD_PATH/header_add_int_field.sh $HEADER_FILE 4 $TOTAL_IMAGE_SIZE


# 8) add  empty image crc for calculation
#------------------------------------------
source $BUILD_PATH/header_add_int_field.sh $HEADER_FILE 4 ${CRC_MAGIC}


# 9) add image version to header file
#------------------------------------------
echo Image version        : ${IMAGE_VERSION} >> ${OUTPUT_FILE}
source $BUILD_PATH/header_add_string_field.sh $HEADER_FILE 32 ${IMAGE_VERSION}


# 10) add image date and time to header file
#------------------------------------------
# %F = full date, %T = %H:%M:%S
TIME=$(date '+%F_%T')
echo Image time        : $TIME >> ${OUTPUT_FILE}
source $BUILD_PATH/header_add_string_field.sh $HEADER_FILE 32 ${TIME}


# 11) add NUM_OF_FILES to header file
#-----------------------------------
echo number of files    : $NUM_OF_FILES >> ${OUTPUT_FILE}
source $BUILD_PATH/header_add_int_field.sh $HEADER_FILE 4 ${NUM_OF_FILES}

# 12) add headers for all files
#-----------------------------------
source $BUILD_PATH/build_all_files.sh ${TXT_FILE}

echo "Full Header created successfully!"

# 13) calculate crc on the combine file
#-------------------------------------
# the CRC is calculated on all the image file (header + all files)
IMAGE_CRC=$(crc32 $HEADER_BIN_FILE)
echo -e "\nImage CRC : ${IMAGE_CRC}" >> ${OUTPUT_FILE}

# 14) Replace the image crc in the header txt file
#------------------------------------------
# find next occurrence of crc empty place
# when read from text file - each byte repesent by two chars - therefore multiple by 2
IMAGE_CRC_CHAR=$((IMAGE_CRC_OFFSET*2))
echo $(sed -e "s/\(.\{$IMAGE_CRC_CHAR\}\)${CRC_MAGIC_TEXT}/\1$IMAGE_CRC/" $HEADER_FILE) > $HEADER_FILE

# 15) convert header to binary
#----------------------------
xxd -r -p $HEADER_FILE > $HEADER_BIN_FILE

# 16) calculate crc only on the header
#-------------------------------------
HEADER_CRC=$(crc32 $HEADER_BIN_FILE)
echo Header CRC : ${HEADER_CRC} >> ${OUTPUT_FILE}

# 17) Replace the header crc in the header txt file
# when read from text file - each byte repesent by two chars - therefore multiple by 2
#------------------------------------------
HEADER_CRC_CHAR=$(($HEADER_CRC_OFFSET*2))
echo $(sed -e "s/\(.\{$HEADER_CRC_CHAR\}\)${CRC_MAGIC_TEXT}/\1$HEADER_CRC/" $HEADER_FILE) > $HEADER_FILE


# 18) convert header to binary
#----------------------------
xxd -r -p $HEADER_FILE > $HEADER_BIN_FILE

# 19) combine headers and files to one file
#------------------------------------------------
cat $HEADER_BIN_FILE $COMBINE_FILES > $TMP_CAT_FILE
mv  $TMP_CAT_FILE $HEADER_BIN_FILE

# 20) If "-e" option was used create an extended image
#-----------------------------------------------------
if [ "$EXTENDED_IMAGE" == "yes" ]
then
	echo "Target CPU # $IMAGE_CPU #"
    source $BUILD_PATH/build_extended_image.sh $IMAGE_CPU $IMAGE_OFFSET $CORE_ID
    EXTENDED_IMAGE_FILE_SIZE=$(stat -c %s ${EXTENDED_IMAGE_FILE})
    EXTENDED_WITHOUT_HDR_SIZE=$(($EXTENDED_IMAGE_FILE_SIZE - $GAP_SIZE))
    GAP_SIZE=$(printf "%d\n" $GAP_SIZE)
    dd if=${EXTENDED_IMAGE_FILE} of=${EXTENDED_IMAGE_NO_HDR} bs=1 skip=$GAP_SIZE
fi

# delete unnecessary files
rm $FILE_HEADER
rm $COMBINE_FILES
rm $TMP_HEADER
rm $OUTPUT_FILE
rm $HEADER_FILE
unset IFS

# rename image_header.bin to super_image.bin
mv image_header.bin super_image.bin

echo Build Image Done

exit
