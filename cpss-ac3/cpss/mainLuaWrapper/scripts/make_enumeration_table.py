#!/usr/bin/env python
#------------------------------------------------------------------------------
#  File name: make_enumeration_table.py
#------------------------------------------------------------------------------
# Usage:
#   1. Compile Linux appDemo / appDemoSim with DEBUG_INFO=ON
#   2. Get type info using GDB:
#      $ gdb appDemoSim > type_info_file <<eof
#      > info types
#      > quit
#      > eof
#   3. Run script:
#      python make_enumeration_table.py \
#           -i type_info_file \
#           -o try.c \
#           CPSS_DRAM_FREQUENCY_ENT
#   4. Check generated file
#
#--------------------------------------------------------------------------
#  File version: 1 $
#--------------------------------------------------------------------------

import re, os, getopt, sys, time


integer_types = [ "int", "char", "short" ]
float_types = [ "float", "double" ]

reName = re.compile(r"}\s*([a-zA-Z_]\w*);\r?\n?$")
reTypefefInt = re.compile(r'typedef (.* )?(int|char|short) ([a-zA-Z_][a-zA-Z_0-9]*);\r?\n?$')
reStruct_field = re.compile(r"^\s*([a-zA-Z_]\w*)\s+([a-zA-Z_]\w*)(\s*\[(\d+|[a-zA-Z_][a-zA-Z_0-9]*)\])?;\r?\n?$")
reStruct_comprex_field = re.compile(r"^\s*(struct|union)\s*\{\s*\.\.\.\s*\}\s*([a-zA-Z_]\w*);$")
reStruct_comprex_field2 = re.compile(r"^\s*(struct|union)\s+[a-zA-Z_]\w*\s+([a-zA-Z_]\w*);$")
reEnumItem = re.compile(r"\b([a-zA-Z_][a-zA-Z_0-9]*)\b(\s*=\s*\d+)?\s*,?")

reStructEmbedStart = re.compile(r"\s*(struct|union)(\s+[a-zA-Z_]\w*)?\s*{", 0)
reStructEmbedMember = re.compile(r"\b([a-zA-Z_]\w*)\b\s*\*?\s*\b([a-zA-Z_]\w*)\s*;",0)
reStructEmbedEnd = re.compile(r"}\s*([a-zA-Z_]\w*)\s*;",0)

all_enums = dict()

includes = [ ]
include_map = { }

def scan_type_info_from_gdb(file_name):
    f = open(file_name,"r")
    lines = f.readlines()
    f.close()

    def line_type(line):
        if line.startswith("File "):
            return "file"
        if line.startswith("typedef enum {"):
            return "enum"
        if line.startswith("typedef struct {"):
            return "struct"
        if line.startswith("typedef union {"):
            return "union"
        if line.startswith("typedef "):
            return "none"
        if line.startswith("enum "):
            return "none"
        if line.startswith("struct "):
            return "none"
        if line.strip() == "":
            return "none"
        return ""

    def apply_decl(collected):
        if len(collected) == 0:
            return
        ltype = line_type(collected[0])
        global reName
        global all_enums
        m = reName.search(collected[-1])
        if m != None:
            name = m.group(1)
            if ltype == "enum" and name not in all_enums:
                all_enums[name] = collected


    status = "none"
    collected = list()
    for line in lines:
        ltype = line_type(line)
        if status in [ "enum", "struct", "union"]:
            if ltype == "":
                collected.append(line)
                continue
            if status != "enum" and line.strip() == "":
                continue
            # get name from lines
            apply_decl(collected)
            collected = list()
            status = ""
        if ltype in [ "enum", "struct", "union"]:
            status = ltype
            collected.append(line)
            continue
        if ltype == "int":
            continue





# list of all required types (with recursive ones)
decls = dict()
# list of wrappers
defs = dict()


# list of required types (from cmd line)
types_required = list()


def make_enum_table(t):
    # TODO
    result = 'struct { int val;const char *string } enumTable_'+t+'[] = {\n'
    for line in all_enums[t]:
        # get part of line in { }
        f = line.find('{')
        if f >= 0:
            line = line[f+1:]
        f = line.rfind('}')
        if f >= 0:
            line = line[:f] + ","

        p = 0
        while True:
            m = reEnumItem.search(line, p)
            if m == None:
                break
            result += '    { '+m.group(1)+', "'+m.group(1)+'" },\n'
            p = m.end(0)
    result += '    { 0, NULL }\n'
    result += '};\n\n'
    return result





if __name__ == '__main__':
    gdb_type_info = list()
    of = sys.stdout
    ofname = "<STDOUT>"
    is_file = False


    opts, types_required = getopt.getopt(sys.argv[1:], "i:o:I:?")
    for (opt,val) in opts:
        if opt == '-i':
            gdb_type_info.append(val)
        elif opt == '-o':
            of = open(val, "w")
            ofname = val
            is_file = True
        elif opt == '-I':
            if val not in includes:
                includes.append(val)
        elif opt == '-?':
            print "Usage: make_type_wrapper.py -i gdb_type_info [-o outfile] ENUM ENUM...."
            print "    -i gdb_type_info  an output of gdb command \"info types\""
            print "    -o outfile        Output file"
            print "    -I headerPath     Add #include <headerPath>"
            sys.exit(0)

    if len(gdb_type_info) < 1:
        print "*** gdb_type_info parameter required"
        sys.exit(0)

    for type_info_file in gdb_type_info:
        scan_type_info_from_gdb(type_info_file)

    for t in types_required[:]:
        if t in all_enums:
            #print "++ "+t+"=",all_enums[t]
            decls[t] = "enum"
            defs[t] = make_enum_table(t)
        else:
            print "-- "+t+" NOT FOUND"

    print >>of, "/*************************************************************************"
    print >>of, "* "+ofname
    print >>of, "*"
    print >>of, "*       WARNING!!! this is a generated file, please don't edit it manually"
    print >>of, "*       See COMMENTS for command line"
    print >>of, "*"
    print >>of, "* DESCRIPTION:"
    print >>of, "*       An enum to string tables" 
    print >>of, "*       It implements tables for the following enums:"
    for k in defs.keys():
        print >>of, "*           " + k
    print >>of, "*"
    print >>of, "* DEPENDENCIES:"
    print >>of, "*"
    print >>of, "* COMMENTS:"
    print >>of, "*       Generated at "+time.ctime()
    print >>of, "*       "+" ".join(sys.argv)
    print >>of, "*"
    print >>of, "* FILE REVISION NUMBER:"
    print >>of, "*       $Revision: 1 $"
    print >>of, "**************************************************************************/"
    for k in includes:
        print >>of, "#include <"+k+">"
    print >>of, ""

    print >>of, "\n/***** declarations ********/\n"
    for k, v in defs.items():
        print >>of, v
    if is_file:
        of.close()
