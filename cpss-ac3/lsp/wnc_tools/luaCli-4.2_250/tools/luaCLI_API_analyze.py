#!/usr/bin/python
######################################################################################
#
# Utility to scan Lua code for CPSS API calls
#
# Usage: luaCLI_API_analyze.py [-?] options"
#   Options:"
#       -i gdb_type_info  an output of gdb command "info types"
#       -C path           Scan CPSS sources from different path
#       -A                Scan CPSS sources for APIs (from $cpssRoot)
#       -L path           Scan Lua CLI script files"
#       -l                Scan Lua CLI script files (from $luaCLI_path)
#       -o fname          Save function/type info to this file
#       -I fname          Load previous info to compare from this file
#
#
#   To create gdb type info file:
#       1. Compile Linux appDemo / appDemoSim with DEBUG_INFO=ON
#       2. Get type info using GDB:
#           $ gdb appDemoSim > type_info_file <<eof
#           > info types
#           > quit
#           > eof
#   
# Examples:
#   1. Get information about changes related to luaCLI scripts
#       python luaCLI_API_analyze.py -i type_info_file \
#           -C /home/serg/cpss \
#           -L /home/serg/luaCLI/cli \
#           -I /revision_4_100/saved_info_luaCLI \
#           -o /revision_4_101/saved_info_luaCLI
#
#   2. Get summary about changed types/APIs in CPSS<F11>
#       python luaCLI_API_analyze.py -i type_info_file \
#           -C /home/serg/cpss \
#           -I /revision_4_100/saved_info \
#           -o /revision_4_101/saved_info
#
# $Revision: 1$
#
######################################################################################

cpssRoot = "/home/serg/cpss"
luaCLI_path = '../cli'
saved_info_fname = 'old_cpss_saved_info.txt'
save_info_fname = 'cpss_saved_info.txt'

import re
import sys
import os, fnmatch, string, getopt,binascii
from scan_utils import *

all_APIs = dict()   # list of all used APIs
all_types = dict()  # list of all used types


# List of functions which calls for CPSS APIs
callers = [
    'cpssGenWrapper',
    'myGenWrapper',
    'cmdLuaCLI_registerCfunction',
    'cmdLuaCLI_callCfunction',
    'cpssPerDeviceParamGet',
    'cpssPerDeviceParamSet',
    'cpssPerPortParamGet',
    'cpssPerPortParamSet'
]

reCall = re.compile(r"\b("+"|".join(callers)+r")\b\s*\(\s*\"([^\s\"]+)\"\s*[,\)]", re.M | re.DOTALL)

integer_types = [ "int", "char", "short" ]

######################################################################
# parse_lua_file
#
# DESCRIPTION:
#       Parse Lua for calls to CPSS APIs
#
# INPUTS:
#       Lua file content
#
# RETURNS:
#       None
#
# COMMENTS:
#
#####################################################################
def parse_lua_file(src):
    #remove comment
    dst = ''
    while src != '':
        i1 = src.find('--')
        if i1 < 0:
            dst += src
            break
        i2 = src.find('"')
        if i2 >= 0 and i1 > i2: # handle quotes
            dst += src[:i2+1]
            src = src[i2+1:]
            backslash = False
            while src != '':
                dst += src[0]
                if src[0] == '"' and not backslash:
                    src = src[1:]
                    break
                backslash = src[0] == '\\'
                src = src[1:]
            continue
        # handle comment
        dst += src[:i1]
        src = src[i1:]
        if src.startswith('--[['):
            i1 = src.find(']]--')
            if i1 >= 0:
                src = src[i1+4:]
                continue
        i1 = src.find('\n')
        if i1 >= 0:
            src = src[i1:]
        else:
            src = ''
    # comments removed, now get list of APIs
    def handle_call(m):
        if m.group(2) not in all_APIs:
            all_APIs[m.group(2)] = dict()

    reCall.sub(handle_call, dst)


######################################################################
# make_api_crc
#
# DESCRIPTION:
#       Calculate crc for CPSS API
#
# INPUTS:
#       decl
#
# RETURNS:
#       crc32
#
# COMMENTS:
#
#####################################################################
def make_api_crc(decl):
    ser = list()
    ser.append(decl['type'])
    ser.append(decl['name'])
    ser.append('(')
    for param in decl['params']:
        for a in param:
            ser.append(a)
        ser.append(',')
    ser.append(')')
    return hex(binascii.crc32(" ".join(ser)) & 0xffffffff)


reName = re.compile(r"}\s*([a-zA-Z_]\w*);\r?\n?$")
reTypefefInt = re.compile(r'typedef .* (int|char|short) ([a-zA-Z_][a-zA-Z_0-9]*);\r?\n?$')
reStruct_field = re.compile(r"^\s*([a-zA-Z_]\w*)\s+([a-zA-Z_]\w*)(\s*\[(\d+)\])?;\r?\n?$")
reEnumItem = re.compile(r"\b([a-zA-Z_][a-zA-Z_0-9]*)\b(\s*=\s*\d+)?\s*,?")


all_typedefs = dict() # [type] =
                      #     type => "struct" || "enum" || "int"
                      #     data => collected lines
                      #     crc  => crc string
                      #     usedby => list of structures which use this one
######################################################################
# scan_type_info_from_gdb
#
# DESCRIPTION:
#       Scan type info data
#
# INPUTS:
#       gdb_type_info   - the file splitted in lines
#
# RETURNS:
#       all_typedefs filled
#
# COMMENTS:
#
#####################################################################
def scan_type_info_from_gdb(gdb_type_info):
    def line_type(line):
        if line.startswith("File "):
            return "file"
        if line.startswith("typedef enum {"):
            return "enum"
        if line.startswith("typedef struct {"):
            return "struct"
        if reTypefefInt.match(line) != None:
            return "int"
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
        global all_typedefs
        m = reName.search(collected[-1])
        if m != None:
            name = m.group(1)
            if ltype == "struct" or ltype == "enum":
                if name not in all_typedefs:
                    all_typedefs[name] = dict()
                    all_typedefs[name]['usedby'] = list()
                all_typedefs[name]['type'] = ltype
                all_typedefs[name]['data'] = collected
                all_typedefs[name]['crc'] = hex(binascii.crc32("\n".join(collected)) & 0xffffffff)


    status = "none"
    collected = list()
    for line in gdb_type_info:
        ltype = line_type(line)
        if ( status == "enum" or status == "struct" ) and ltype == "":
            collected.append(line)
            continue
        if status == "enum" or status == "struct":
            # get name from lines
            apply_decl(collected)
            collected = list()
            status = ""
        if ltype == "enum" or ltype == "struct":
            status = ltype
            collected.append(line)
            continue
        if ltype == "int":
            m = reTypefefInt.match(line)
            if m.group(2) not in all_typedefs:
                all_typedefs[m.group(2)] = dict()
            all_typedefs[m.group(2)]['type'] = "int"

######################################################################
# type_classify
#
# DESCRIPTION:
#       return the type class:
#       BOOL | NUMBER | ENUM | SCTRUCT
#
# INPUTS:
#      t        - Type name 
#
# RETURNS:
#       BOOL | NUMBER | ENUM | SCTRUCT | UNKNOWN
#
#####################################################################
def type_classify(t):
    global all_typedefs
    global integer_types
    if t == "bool" or t == "GT_BOOL":
        return "BOOL"
    if t in integer_types:
        return "NUMBER"
    if t in all_typedefs:
        if all_typedefs[t]['type'] == "enum":
            return "ENUM"
        if all_typedefs[t]['type'] == "struct":
            return "STRUCT"
        if all_typedefs[t]['type'] == "int":
            return "NUMBER"
    return "UNKNOWN"

def Usage():
    print "Usage: luaCLI_API_analyze.py [-?] options"
    print "Options:"
    print "    -i gdb_type_info  an output of gdb command \"info types\""
    print "    -C path           Scan CPSS sources from different path"
    print "    -A                Scan CPSS sources for APIs (from "+cpssRoot+")"
    print "    -L path           Scan Lua CLI script files"
    print "    -l                Scan Lua CLI script files (from "+luaCLI_path+")"
    print "    -o fname          Save function/type info to this file"
    print "    -I fname          Load previous info to compare from this file"
    sys.exit(0)

def Notify_changes(msg):
    print msg

#####################
### Main
#####################
if __name__ == '__main__':
    gdb_type_info = list()
    scan_cpss_api = False
    scan_lua_cli = False
    opts, rest = getopt.getopt(sys.argv[1:], "?AC:i:L:lI:o:")
    for (opt,val) in opts:
        if opt == '-?':
            Usage()
        elif opt == '-i':
            try:
                f = open(val)
                gdb_type_info = f.readlines()
                f.close()
            except:
                print "Failed to read "+val
        elif opt == "-A":
            scan_cpss_api = True
        elif opt == "-C":
            scan_cpss_api = True
            cpssRoot = val
        elif opt == "-l":
            scan_lua_cli = True
        elif opt == "-L":
            scan_lua_cli = True
            luaCLI_path = val
        elif opt == "-I":
            saved_info_fname = val
        elif opt == "-o":
            save_info_fname = val
        else:
            Usage()

    if scan_lua_cli:
        for fname in all_files(luaCLI_path, '*.lua'):
            try:
                f = open(fname)
                src = f.read()
                f.close()
                parse_lua_file(src)
            except:
                continue

    # load saved info
    # saved_info['function']['name'] = crc
    # saved_info['struct']['name'] = crc
    # saved_info['enum']['name'] = crc
    saved_info = dict()
    saved_info['function'] = dict()
    saved_info['struct'] = dict()
    saved_info['enum'] = dict()
    try:
        f = open(saved_info_fname)
        for l in f.readlines():
            if l.startswith("#"):
                continue
            ll = l.split()
            if len(ll) > 2 and ll[0] in ['enum','struct','function']:
                saved_info[ll[0]][ll[1]] = ll[2]
        f.close()
    except:
        pass


    if scan_cpss_api:
        for srcpath in all_files(cpssRoot,"*.h"):
            source = readfile(srcpath)
            for decl in scan_c_src(source):
                if 'name' not in decl:
                    continue
                name = decl['name']
                # ignore this function because there is _two_ declartions
                # in cpssEnabler/mainOs/h/gtOs/gtOsExc.h
                if name == "osExceptionHandling":
                    continue
                if name not in all_APIs:
                    if scan_lua_cli:
                        continue
                    all_APIs[name] = dict()
                # check cpss API is not changed for this function
                all_APIs[name]['decl'] = decl
                # new CRC
                all_APIs[name]['crc'] = make_api_crc(decl)
                # compare crc, add notice
                if name in saved_info['function']:
                    if saved_info['function'][name] != all_APIs[name]['crc']:
                        Notify_changes("*** "+name+"() API changed")
                
                # used types
                all_APIs[name]['types'] = list()
                for param in decl['params']:
                    ptype = param[1]
                    # handle pointers
                    if ptype.endswith("*"):
                        ptype = ptype[:-1].strip()
                    all_APIs[name]['types'].append(ptype)
                    if ptype not in all_types:
                        all_types[ptype] = dict()
                        all_types[ptype]['apis'] = list()
                    all_types[ptype]['apis'].append(name)

    # now scan type info file
    if len(gdb_type_info) > 0:
        scan_type_info_from_gdb(gdb_type_info)

    # create type usage tree
    all_used_types = all_types.keys()
    def scan_depend(type):
        global all_typedefs
        global all_used_types
        if type not in all_typedefs:
            # TODO: notify about missing type
            return
        if all_typedefs[type]['type'] == "int":
            # ignore integer types
            return
        if all_typedefs[type]['type'] == "enum":
            # no recursion in enums
            return
        # add used types
        for line in all_typedefs[type]['data'][1:-1]:
            m = reStruct_field.match(line)
            if m == None:
                #TODO  error = "unhandled line"
                continue
            ftype = m.group(1)
            fcls = type_classify(m.group(1)) 
            if fcls not in ["ENUM","STRUCT"]:
                continue
            if ftype not in all_typedefs:
                # TODO: Notify error no fcls ftype in gdb type info file
                continue
            # add reference to all_typedefs[ftype]['usedby']
            if type not in all_typedefs[ftype]['usedby']:
                all_typedefs[ftype]['usedby'].append(type)
            if ftype not in all_used_types:
                all_used_types.append(ftype)
                scan_depend(ftype)
        
    for type in all_used_types[:]:
        scan_depend(type)

    for type in all_used_types:
        if type not in all_typedefs:
            # TODO: notify about missing type
            continue
        tt = all_typedefs[type]['type']
        if tt not in ["enum", "struct"]:
            # ignore all but enum and structs
            continue
        # compare crc, add notice
        if type not in saved_info[tt]:
            # type was not used
            continue
        if saved_info[tt][type] == all_typedefs[type]['crc']:
            # crc is the same
            continue
        Notify_changes("*** "+tt+" "+type+" changed")
        affected_api = list()
        def collect_affected_API(type):
            global all_types
            global affected_api
            global all_typedefs
            if type in all_types:
                for a in all_types[type]['apis']:
                    if a not in affected_api:
                        affected_api.append(a)
            if 'usedby' not in all_typedefs[type]:
                return
            for u in all_typedefs[type]['usedby']:
                collect_affected_API(u)
        collect_affected_API(type)
        affected_api.sort()
        for a in affected_api:
            Notify_changes("    +++ "+a+"() affected")

        

    # save API info
    # save type info 
    try:
        f = open(save_info_fname, "w")
        for k,v in all_APIs.items():
            if 'crc' not in v:
                print "no description found for function "+k+"()",v
                continue
            print >>f, "function "+k+" "+v['crc']
        for k,v in all_typedefs.items():
            flg = False
            def type_used_by_cli(type):
                global all_types
                global all_typedefs
                if type in all_types:
                    if len(all_types[type]['apis']) > 0:
                        return True
                if 'usedby' not in all_typedefs[type]:
                    return False
                for u in all_typedefs[type]['usedby']:
                    if type_used_by_cli(u):
                        return True
                return False
            if not type_used_by_cli(k):
                continue
            if 'type' in v and v['type'] == 'int':
                continue
            if 'crc' not in v:
                print "no description found for type"+k,v
                continue
            print >>f, v['type']+" "+k+" "+v['crc']
        f.close()
    except:
        print "Failed to open output file: "+save_info_fname

                


    #print "\n".join(all_APIs)
