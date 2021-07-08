#!/usr/bin/env python

######################################################################
# cmdFS_pack_files.py
#
# DESCRIPTION:
#       This script packs files into one file.
#
#       Usage:
#       cmdFS_RAM_embed_files.py [-L] [-B] [-o fname] list_file
#       cmdFS_RAM_embed_files.py [-L] [-B] [-o fname] [-x fname] -D dir
#          -o fname    - output file. Default cmdFS_RAM_embedded_data.inc
#          -x fname    - exclude list file
#          -D          - the argument is a directory. All files from this
#                        directory will be embedded
#          -L          - Use little-endian in header
#          -B          - Use big-endian in header
#
#
# INPUTS:
#       listing file -  file with two columns:
#            1. host file name (relative or absolute)
#            2. cmdFS file name (use / as path sep)
#
#            # Sample file list for embedding into cmdFS_RAM
#            #
#            # source file name                              target name
#            # ----------------                              -----------
#            ../lua/luaCLI/scripts/cmdLuaCLIDefs.lua         cmdLuaCLIDefs.lua
#            ../lua/luaCLI/scripts/genwrapper_API.lua        genwrapper_API.lua
#            ../lua/luaCLI/sample/bit_wise_test.lua          test/bit_wise.lua
#
#       exclude list file - the list of cmdFS file names to be excluded
#            # Sample exclude file list for embedding into cmdFS_RAM
#            #
#            # target name
#            # -----------
#            test/bit_wise.lua
#
# OUTPUTS:
#       cmdFS_embedded_data.bin
#
#       Format:
#           struct {
#               uint32_t    data_offset;
#               uint32_t    data_len;
#               uint32_t    mtime;
#               char        fname[244];
#           } file_info[nfiles+1];
#           End of header:
#           file_info[nfiles].data_offset == 0
#
# RETURNS:
#       none
#
# COMMENTS:
#
# FILE REVISION NUMBER:
#       $Revision: $
#####################################################################

import os,sys,getopt,time,struct

def Usage():
    print "Usage:"
    print sys.argv[0]+" [-L] [-B] [-o fname] list_file"
    print sys.argv[0]+" [-L] [-B] [-o fname] [-x fname] -D dir"
    print "   -o fname    - output file. Default cmdFS_embedded_data.bin"
    print "   -x fname    - exclude list file"
    print "   -D          - the argument is a directory. All files from this"
    print "                 directory will be embedded"
    print "   -L          - Use little-endian in header"
    print "   -B          - Use big-endian in header"
    sys.exit(0)

# record format: offset(U32), len(U32), mtime(U32), name(char[244])
headerfmt = "L L L 244s"


if __name__ == '__main__':
    outfile_name = "cmdFS_embedded_data.bin"
    exclude_list = list()
    embed_entire_directory = False
    endian = "<" # LE
    opts, files = getopt.getopt(sys.argv[1:], "?o:x:DLB")
    for (opt,val) in opts:
        if opt == "-o":
            outfile_name = val
        elif opt == "-x":
            if os.access(val, os.R_OK):
                f = open(val,"r")
                for line in f.readlines():
                    if line.startswith("#"):
                        continue
                    exclude_list.append(line.split()[0])
                f.close()
            else:
                print >>sys.stderr, "exclude list file \""+val+"\" doesn't exists or not readable"
                sys.exit(1)
        elif opt == "-D":
            embed_entire_directory = True
        elif opt == "-L":
            endian = "<"
        elif opt == "-B":
            endian = ">"
        else:
            Usage()

    header_fmt = endian + headerfmt

    if len(files) < 1:
        Usage()

    if embed_entire_directory:
        list_lines = list()
        srcdir = files[0]
        for root, dirs, files in os.walk(srcdir):
            for filename in files:
                sourcename = os.path.join(root,filename)
                targetname = sourcename
                if targetname.startswith(srcdir+os.path.sep):
                    targetname = targetname[len(srcdir+os.path.sep):]
                # on Win32 replace path separator with /
                if os.path.sep != "/":
                    targetname = targetname.replace(os.path.sep, "/")
                list_lines.append(sourcename + " " + targetname);
    else:
        # use list file
        if files[0] == "-":
            f = sys.stdin
        else:
            if not os.access(files[0], os.R_OK):
                print >>sys.stderr, "list file \""+files[0]+"\" doesn't exists or not readable"
                sys.exit(1)
            f = open(files[0],"r")
        list_lines = f.readlines()
        f.close()

    # create output file
    of = open(outfile_name,"wb")
    print "opened output file "+outfile_name+"..."

    lst = dict()

    # iteract through list file
    for line in list_lines:
        line = line.strip()
        if line.startswith("#") or line == "":
            continue
        a = line.split()
        if len(a) < 2:
            continue
        if a[1] in exclude_list:
            continue
        # read source
        if not os.access(a[0], os.R_OK):
            print >>sys.stderr, "file \""+a[0]+"\" doesn't exists or not readable"
            continue
        if len(a[1]) > 243:
            print >>sys.stderr, "file name too long (>243 chars): "+a[1]
            continue

        f = open(a[0],"rb")
        data = f.read()
        f.close()
        
        entry = dict()
        entry["name"] = a[1]
        entry["data"] = data;
        entry["mtime"] = os.stat(a[0]).st_mtime
        lst[a[1]] = entry
        print "pack file "+a[0]+" with name \""+a[1]+"\""

    # now create header
    header = ""
    data = ""
    data_offset = struct.calcsize(header_fmt)*(len(lst)+1)
    for entry in lst.values():
        header += struct.pack(header_fmt, data_offset, len(entry["data"]), int(entry["mtime"]), entry["name"])
        data += entry["data"]
        data_offset += len(entry["data"])
        # add padding to 16 byte
        if (len(entry["data"]) % 16) > 0:
            padding = 16 - (len(entry["data"]) % 16)
            data += "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"[:padding]
            data_offset += padding
    # add empty entry at the end of directory
    header += struct.pack(header_fmt, 0, 0, 0, "")

    of.write(header)
    of.write(data)
    of.close()
