#!/usr/bin/python

#
#    Convert binary file to text representation of bits
#    each line represents a single 32bit word by 32 chars of 0 / 1,
#    from MSB (left) to LSB (right).
#    Padded with zeros until requested number of words
#
# usage:
#    python bin2rom.py input_file, # of lines
#
#
# $Revision: 1$

import sys, os, string

pad_line = '00000000000000000000000000000000'
byte_arr = list(pad_line) #split 32 chars of string to list

# Prepare output filename
if sys.argv[1].endswith(".bin"):
    outfilename = sys.argv[1][:-4]+".rom"

# Open input file in binary mode, output in text mode
binary_file = open(sys.argv[1], 'rb')
rom_file = open(outfilename, 'wt')

# Requested number of lines
lines = int(sys.argv[2])

# Read 4 bytes from input binary file
bytes_read = binary_file.read(4)

# Loop on all bytes of input file
while (1):
    index = 31
    for b in bytes_read: # bytes_read is string object
        num = ord(b)    # Convert char to number
        for i in range(0,8): # write 8 bits to line
            if (num & 0x01):
                byte_arr[index] = '1'
            else:
                byte_arr[index] = '0'
            index = index - 1
            num = num >> 1

    # in case less than 4 bytes read, pad with zeros
    while (index > 0):
        byte_arr[index] = '0'
        index = index -1

    # write line with 32 bit charactes to ouput file
    rom_file.write("%s\n" % "".join(byte_arr))
    lines = lines -1

    # if less than 4 bytes, break from loop
    if (len(bytes_read) < 4):
        break

    bytes_read = binary_file.read(4)

# Pad with lines of 0x00000000 until number of requested lines
while (lines > 0):
    rom_file.write("%s\n" % pad_line)
    lines = lines -1

# Close files
binary_file.close()
rom_file.close()
