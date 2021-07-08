#!/bin/bash

#############################################################################												
# SCRIPT:  build_header_file.sh
# USAGE:   ./build_header_file.sh <file Path>
# PURPOSE: The script get txt file and do the following

# $1 - txt file									
#	build a header version 1
#                        type   size(bytes)  offset(bytes)  
#	Header size       U16       2             2       
#       Header version    U16       2             4       
#       Header CRC        U32       4             8       				
#       File size         U32       4             12								
#############################################################################	


#############################################################################
#                      Script Starts Here                                   #
#############################################################################

set -e
# Any subsequent commands which fail will cause the shell script to exit immediately 

TEXT_FILE=$1

HEADER_VERSION=1
CRC=0

if [ -f $FILE_HEADER ] 
then rm $FILE_HEADER
fi

echo -e "\n"file	: ${TEXT_FILE} >> ${OUTPUT_FILE}

# 1) add header size to header file
#----------------------------------
echo File header size 	: ${FILE_HEADER_SIZE} >> ${OUTPUT_FILE}
source $BUILD_PATH/header_add_int_field.sh $FILE_HEADER 2 ${FILE_HEADER_SIZE}


# 2) add header version to header file
#-------------------------------------
echo Header version : ${HEADER_VERSION} >> ${OUTPUT_FILE}
source $BUILD_PATH/header_add_int_field.sh $FILE_HEADER 2 ${HEADER_VERSION}


# 3) add empty crc for calculation
#---------------------------------
source $BUILD_PATH/header_add_int_field.sh $FILE_HEADER 4 ${CRC_MAGIC}

# 4) add file size to header file (without header)
#---------------------------------
FILE_SIZE=$(stat -c %s ${TEXT_FILE})
echo file size	: ${FILE_SIZE} >> ${OUTPUT_FILE}
source $BUILD_PATH/header_add_int_field.sh $FILE_HEADER 4 ${FILE_SIZE}

return
