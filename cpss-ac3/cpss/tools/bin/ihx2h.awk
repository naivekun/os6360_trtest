# Convert .ihx file to a record list suitable to be included as .h file in C compilation
# Usage:
#   1. Compile microcontroller code
#      ...
#   2. Generate table as .h file:
#      awk -f ihx2h.awk impCtrl.ihx > impCtrl.h
#
#   3. Include .h file within a table defined as:
#      struct {
#          short    length;
#          short    address;
#          char     *data;
#      } table[] = {
#      #include <impCtrl.h>
#      };
#
BEGIN {
    print "/***************************************************"
    print " * Automatically Generated - Don't edit manually - *"
#   print " * Filename: " FILENAME
#   print " * DateTime: " strftime("%y_%m_%d_%H_%M_%S")
    print " ***************************************************/"
}
/^\:......[^0]/ {
    # Ignore anything but data chunk
}
/^\:.......[^0]/ {
    # Ignore anything but data chunk
}
/^\:......00/ {
    # Data chunk
    line = $0
    len = length(line)
    chunkNum++
    chunkLen = substr(line,2,2)
    chunkAddr = substr(line,4,4)
    chunkFcs = substr(line,len-1,2)
    chunkList[chunkAddr] = (chunkLen chunkFcs)
    # print length, address and start of data
    print "/*" line "*/"
    print "{0x" chunkLen ", 0x" chunkAddr ","
    printf " \""
    for (i = 10; i < len-2; i+=2) {
        # print each character in a string
        printf "\\x" substr(line,i,2)
    }
    print "\"},"
}
END {
# 0. Prepare hex to dec conversion array
    toDec["0"] = 0
    toDec["1"] = 1
    toDec["2"] = 2
    toDec["3"] = 3
    toDec["4"] = 4
    toDec["5"] = 5
    toDec["6"] = 6
    toDec["7"] = 7
    toDec["8"] = 8
    toDec["9"] = 9
    toDec["a"] = 10
    toDec["b"] = 11
    toDec["c"] = 12
    toDec["d"] = 13
    toDec["e"] = 14
    toDec["f"] = 15
    toDec["A"] = 10
    toDec["B"] = 11
    toDec["C"] = 12
    toDec["D"] = 13
    toDec["E"] = 14
    toDec["F"] = 15
    toHex[ 0] = "0"
    toHex[ 1] = "1"
    toHex[ 2] = "2"
    toHex[ 3] = "3"
    toHex[ 4] = "4"
    toHex[ 5] = "5"
    toHex[ 6] = "6"
    toHex[ 7] = "7"
    toHex[ 8] = "8"
    toHex[ 9] = "9"
    toHex[10] = "A"
    toHex[11] = "B"
    toHex[12] = "C"
    toHex[13] = "D"
    toHex[14] = "E"
    toHex[15] = "F"
# 1. print chunk information
    chunkLen  = 4 * (chunkNum + 1)
    chunkLen += 4 * int((chunkNum + 62) / 63)
    chunkAddr = "EC00"
    # print length, address and start of data
    print "/**/"
    print "{" chunkLen ", 0x" chunkAddr ","
    # prepare Fcs related variables
    chunkFcsAddr = (16 * toDec[substr(chunkAddr,1,1)]) + toDec[substr(chunkAddr,2,1)]
    chunkFcsFcs = chunkFcsAddr
    chunkNum = 0
    for (chunkAddr in chunkList) {
        chunkNum++
        chunkLen = substr(chunkList[chunkAddr],1,2)
        chunkFcs = substr(chunkList[chunkAddr],3,2)
        # update Fcs
        chunkFcsFcs += (16 * toDec[substr(chunkAddr,1,1)]) + toDec[substr(chunkAddr,2,1)]
        chunkFcsFcs += (16 * toDec[substr(chunkAddr,3,1)]) + toDec[substr(chunkAddr,4,1)]
        chunkFcsFcs += (16 * toDec[substr(chunkLen,1,1)]) + toDec[substr(chunkLen,2,1)]
        chunkFcsFcs += (16 * toDec[substr(chunkFcs,1,1)]) + toDec[substr(chunkFcs,2,1)]
        # print each chunk's information
        print "/*" chunkAddr ":" chunkLen ":" chunkFcs "*/"
        print " \"\\x" substr(chunkAddr,3,2) "\\x" substr(chunkAddr,1,2) "\\x" chunkLen "\\x" chunkFcs "\""
        if ((chunkNum % 63) == 0) {
            # compute final Fcs on each block of 64 chunk informations
            chunkFcsFcs -= 4
            chunkFcsFcs %= 256
            chunkFcsFcs = (256 - chunkFcsFcs)
            print " \"\\x00\\x" toHex[int(chunkFcsAddr/16)] toHex[chunkFcsAddr%16] "\\xfc\\x" toHex[int(chunkFcsFcs/16)] toHex[chunkFcsFcs%16] "\""
            chunkFcsAddr++
            chunkFcsFcs = chunkFcsAddr
        }
    }
    if ((chunkNum % 63) != 0) {
        # compute final Fcs on each block of 64 chunk informations
        chunkFcsLen = 4 * (chunkNum % 63)
        chunkFcsFcs += chunkFcsLen
        chunkFcsFcs %= 256
        chunkFcsFcs = (256 - chunkFcsFcs)
        print " \"\\x00\\x" toHex[int(chunkFcsAddr/16)] toHex[chunkFcsAddr%16] "\\x" toHex[int(chunkFcsLen/16)] toHex[chunkFcsLen%16] "\\x" toHex[int(chunkFcsFcs/16)] toHex[chunkFcsFcs%16] "\""
        chunkFcsAddr++
        chunkFcsFcs = chunkFcsAddr
    }
    # print end of chunks information
    print "/*FFFF:00:00*/"
    print " \"\\xFF\\xFF\\x00\\x00\""
    print "},"
# 2. print end of array - zero length chunk
    print "/*:00000001FF*/"
    print "{0x00, 0x0000,"
    print " \"\"}"
    print "/***************************************************/"
}

