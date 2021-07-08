#!/bin/bash

#############################################################################												
# SCRIPT:  build_all_files.sh
# USAGE:   ./build_all_files.sh <txt File>
# PURPOSE: Build headers for all files reading from text file input, calculate CRC and create bin file

# $1 - file_list.txt									
#	1) build headers for all files
#                                  type    size(bytes)  offset(bytes)
#           Header size             U16        2             2     
#           Header version          U16        2             4     
#           Header CRC              U32        4             8     
#           File size               U32        4             12    
#           File name               U8[16]     16            28    
#           type                    U32        4             32    
#           execution copy offset   U32        4             36    
#           execution offset        U32        4             40    
#           file offset in image    U32        4             44    
#           use bmp                 U32        4             48    
# 
#  	2) calculate crc32							
#	3) create new bin file which include both txt file and header								
#############################################################################	


#############################################################################
#                      Script Starts Here                                   #
#############################################################################


set -e
# Any subsequent commands which fail will cause the shell script to exit immediately 

FILE_HEADER=build_all_files.txt
CRC_BYTE=8
FILE_OFFSET_IN_IMAGE=$((IMAGE_HEADER_SIZE+(NUM_OF_FILES*FILE_HEADER_SIZE)))

##### read text file ######
[ ! -f $TXT_FILE ] && { echo "$text file not found"; exit 0; }
while read path name type cpy_offset exec_offset spi_offset hex_bmp || [ -n "$hex_bmp" ]
    do
        if [ "${path:0:1}" == "$NOTE" ]              # skip file notes <#>
            then continue;
        fi
        if  [ ! -z "$path" -a "$path"!=" " ];	# skip null or blank line <#>	
            then
            # 1) build file header: version, crc & size
            #-----------------------------------
            source $BUILD_PATH/build_header_file.sh ${path}


            # 2) add file name to header 
            #-----------------------------------
            echo name	: "$name" >> ${OUTPUT_FILE}
            source $BUILD_PATH/header_add_string_field.sh $FILE_HEADER 16 ${name}


            # 3) add file type to header 
            #-----------------------------------
            echo type : "$type" >> ${OUTPUT_FILE}
            source $BUILD_PATH/header_add_int_field.sh $FILE_HEADER 4 $type


            # 4) add file "copy offset" to header 
            #-----------------------------------
            echo copy offset : "$cpy_offset" >> ${OUTPUT_FILE}
            source $BUILD_PATH/header_add_int_field.sh $FILE_HEADER 4 $cpy_offset


            # 5) add file "execution offset" to header 
            #-----------------------------------
            echo execution offset : "$exec_offset" >> ${OUTPUT_FILE}
            source $BUILD_PATH/header_add_int_field.sh $FILE_HEADER 4 $exec_offset


            # 6) add file "spi offset" to header 
            #-----------------------------------

            if [ $spi_offset == 0 ]
            then
                spi_offset=$FILE_OFFSET_IN_IMAGE
            fi

            FILE_SIZE=$(stat -c %s ${path})
            # count iteration number of files
         
            FILE_OFFSET_IN_IMAGE=$((FILE_OFFSET_IN_IMAGE+FILE_SIZE))
                
            echo file offset in image : "$spi_offset" >> ${OUTPUT_FILE}
            source $BUILD_PATH/header_add_int_field.sh $FILE_HEADER 4 $spi_offset


            # 7) add file bitmap to header 
            #-----------------------------------
            echo bitmap :   "$hex_bmp" >> ${OUTPUT_FILE}
            #    ./hex2binary.sh $hex_bmp
            source $BUILD_PATH/header_add_int_field.sh $FILE_HEADER 4 $hex_bmp

            # 8) convert file header to binary
            #----------------------------
            xxd -r -p $FILE_HEADER > $HEADER_BIN_FILE

            # 9) calculate crc file
            #-------------------------------------
            CRC=$(crc32 $HEADER_BIN_FILE)
            echo "crc="${CRC} >> ${OUTPUT_FILE}

            # 10) Replace the crc in the header txt file search from the 16 char
            #------------------------------------------
            echo $(sed -e "s/\(.\{$CRC_BYTE\}\)${CRC_MAGIC_TEXT}/\1$CRC/" $FILE_HEADER) > $FILE_HEADER

            # 11) combine file header to image header
            #----------------------------------------
            cat $HEADER_FILE $FILE_HEADER > $TMP_HEADER
            cp  $TMP_HEADER $HEADER_FILE
        fi
done < $TXT_FILE
 
# 12) convert all headers to binary file
#-----------------------------------------
echo -e "\nConvert all file headers to binary file"
xxd -r -p $HEADER_FILE > $HEADER_BIN_FILE


# 13) combine headers and files to one file
#------------------------------------------------
# Catenae all files
cat $HEADER_BIN_FILE $COMBINE_FILES > $TMP_CAT_FILE
cp  $TMP_CAT_FILE $HEADER_BIN_FILE

return
