#------------------------------------------------------------------------------
#  File name: gen_xml_funcdescr.py
#------------------------------------------------------------------------------
# Usage:
#TODO
#   1. Compile Linux appDemo / appDemoSim with DEBUG_INFO=ON
#   2. Get type info using GDB:
#      $ gdb appDemoSim > type_info_file <<eof
#      > info types
#      > quit
#      > eof
#   3. Run script:
#      python gen_xml_funcdescr.py \
#           -i type_info_file_dx \
#           -C /home/serg/tmp/Marvell/cpss \
#           -F dxCh \
#           -o cpssAPI.xml
#      or
#      python gen_xml_funcdescr.py \
#           -i type_info_file_dx \
#           -C /home/serg/tmp/Marvell/cpss \
#           -F dxCh \
#           -F px \
#           -S \
#           -o cpssAPI.xml
#      (This will generate cpssAPI_dxCh.xml and cpssAPI_common.xml)
#   4. Check generated file(s)
#
#  ------------------------------------------------
# variable              value
# -------------------------------------------------
#  CONF_XML_FUNCDESCR   path to Search dirs in JSON format
#                       default: cpss/mainLuaWrapper/scripts/xml_funcdescr.json 
# example:
#   "Search": {
#      "Search_list": [
#        "embeddedCommands/px/pxEC/h",
#        "embeddedCommands/dx/dxEC/h",
#        "mainPpDrv/h",
#        "mainPxDrv/h",
#        "common/h"
#      ]
#   }
#--------------------------------------------------
#  File version: 3 $
#--------------------------------------------------------------------------

import pprint
import json

pp = pprint.PrettyPrinter(indent=2,width=100)
import re, os, getopt, sys, time

cpssRoot = "/home/serg/tmp/Marvell/cpss"

integer_types = [ "int", "char", "short", "long" ]
float_types = [ "float", "double" ]
privateAPIStartsWith = ("prv", "pvr", "mv", "hws", "shmIpc", "gen")

reName = re.compile(r"}\s*([a-zA-Z_]\w*);\r?\n?$")
reTypedefInt = re.compile(r'typedef (signed |unsigned |short |long )*(int|char|short|long) ([a-zA-Z_][a-zA-Z_0-9]*);\r?\n?$')
reStruct_field = re.compile(r"^\s*([a-zA-Z_]\w*)\s+([a-zA-Z_]\w*)(\s*\[(\d+)\])?;\r?\n?$")
reStruct_comprex_field = re.compile(r"^\s*(struct|union)\s*\{\s*\.\.\.\s*\}\s*([a-zA-Z_]\w*);$")
reStruct_comprex_field2 = re.compile(r"^\s*(struct|union)\s+[a-zA-Z_]\w*\s+([a-zA-Z_]\w*);$")
reEnumItem = re.compile(r"\b([a-zA-Z_][a-zA-Z_0-9]*)\b(\s*=\s*\d+)?\s*,?")
_reArrayParameter = re.compile(r"(\w+)\[(\w*)\]")
_reCommentSection = re.compile(r'([A-Z\s]+):$')
_reParamDescr = re.compile(r' {0,12}([a-zA-Z_][a-zA-Z_0-9]*) +- +(.*)$')

reStructEmbedStart = re.compile(r"\s*(struct|union)(\s+[a-zA-Z_]\w*)?\s*{", 0)
reStructEmbedMember = re.compile(r"\b([a-zA-Z_]\w*)\b\s*\*?\s*\b([a-zA-Z_]\w*)\s*;",0)
reStructEmbedEnd = re.compile(r"}\s*([a-zA-Z_]\w*)\s*;",0)

all_structs = dict()
all_enums = dict()
all_defs = dict() # [type] = "struct" || "enum" || "int"
typeExcludeList = list()

#-------------------------------------------------------------------------------
def get_conf_json_all(data,dict_conf):

    #print "-----------------------------"
    for section in data:
        #print "section=", section
        if section == 'comment':
            continue

        dict_conf[section]=dict()
        for option in data[section]:
            if option != '':
                #str_opt=str(option)
                str_opt = option

                if option == 'comment':
                    continue

                #print section,"   option=", option

                #if type(data[section][option]) is str:
                if type(data[section][option]) != list: 
                    #print section,option,'type=',type(data[section][option])
                    val=data[section][option]
                    str_val=str(val)
                    if val != '':

                        dict_conf[section][str_opt]=dict()
                        if  str_opt in os.environ:
                            #print section,option + ': ', os.getenv(str_opt)

                            str_opt.replace('\\','/')
                            dict_conf[section][str_opt] = os.getenv(str_opt)
                        else:
                            #print section,str_opt + ': ',str_val
                            str_val.replace('\\','/')
                            dict_conf[section][str_opt]=str_val
                else:
                    #print '*** type is list: ', section,option,data[section][option]
                    dict_conf[section][option]=data[section][option]

#-------------------------------------------------------------------------------

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
        if reTypedefInt.match(line) != None:
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
        global all_structs
        global all_defs
        m = reName.search(collected[-1])
        if m != None:
            name = m.group(1)
            if name not in all_defs:
                all_defs[name] = ltype
                if ltype in ["struct", "union"]:
                    all_structs[name] = collected
                if ltype == "enum":
                    all_enums[name] = collected

    status = "none"
    collected = list()
    for line in lines:
        ltype = line_type(line)
        if status in ["enum", "struct", "union" ] and ltype == "":
            collected.append(line)
            continue
        if status in ["enum", "struct", "union" ]:
            # get name from lines
            apply_decl(collected)
            collected = list()
            status = ""
        if ltype in ["enum", "struct", "union" ]:
            status = ltype
            collected.append(line)
            continue
        if ltype == "int":
            m = reTypedefInt.match(line)
            all_defs[m.group(3)] = "int"





# list of wrappers
defs = dict()


def type_classify(t):
    global all_defs
    global integer_types
    if t == "bool" or t == "GT_BOOL":
        return "bool"
    if t in integer_types:
        return "int"
    if t in float_types:
        return "float"
    if t in all_defs:
        return all_defs[t]
    return "UNKNOWN"



# remove 
def lstrip_list(lines):
    min_indent = None
    for line in lines:
        if line.strip() == "":
            continue
        indent = len(line) - len(line.lstrip())
        if min_indent == None:
            min_indent = indent
        if min_indent > indent:
            min_indent = indent
    r = list()
    for line in lines:
        if line.strip() == "":
            r.append("")
            continue
        r.append(line[min_indent:])
    return "\n".join(r).rstrip().decode("windows-1250").encode("utf-8")




#####################
### Main
#####################

from scan_utils import *

#from xml.dom.minidom import getDOMImplementation
from xml.dom import minidom
xml_impl = minidom.getDOMImplementation()

def all_files_x(roots, patterns='*', single_level=False, yield_folders=False):
    retlist=list()
    for root in roots:
        for f in all_files(root, patterns, single_level, yield_folders):
            retlist.append(f)
    return retlist

def fix_comment(comment):
    # fix bad characters (cp1250 chars)
    comment = comment.replace('\x90','&nbsp')
    comment = comment.replace('\x91','`')
    comment = comment.replace('\x92','\'')
    comment = comment.replace('\x93','"')
    comment = comment.replace('\x94','"')
    comment = comment.replace('\x95','*')
    comment = comment.replace('\x96','-')
    comment = comment.replace('\x97','-')
    return comment

#-----------------------------------------------------
if __name__ == '__main__':


    gdb_type_info = list()
    of = sys.stdout
    ofname = "<STDOUT>"
    is_file = False
    families = list()
    add_descriptions = False
    add_comments = False
    bad_struct_fname = "<STDOUT>"
    manual_structs = dict()
    mapping = dict()
    structures_required = list()
    enums_required = list()
    banList = list()
    split_xml = False
    bad_functions = dict()
    manual_descr_file = None

    opts,rest = getopt.getopt(sys.argv[1:], "i:o:A:C:F:SDcB:M:m:X:?")
    for (opt,val) in opts:
        if opt == '-i':
            gdb_type_info.append(val)
        elif opt == '-o':
            ofname = val
            is_file = True
        elif opt == "-B":
            bad_struct_fname = val
        elif opt == "-M":
            manual_descr_file = val
        elif opt == "-A":
            #load file with list of banned API's
            
            # try to open input file
            try:
                banListFile = open(val, 'r')
            except IOError:
                print "Error: cannot open file: " + val + "\n"
                sys.exit(2)

            for line in banListFile.readlines():
                banList.append(line.strip())
            banListFile.close()

        elif opt == '-m':
            # load structure names mapping
            pf = open(val, "r")
            for line in pf.readlines():
                line = line.strip()
                if line.startswith("#"):
                    continue
                ll = line.split()
                if len(ll) == 2:
                    mapping[ll[0]] = ll[1]
            pf.close()

        elif opt == "-C":
            cpssRoot = val
        elif opt == "-F":
            families.append(val)
        elif opt == "-D":
            add_descriptions = True
        elif opt == "-c":
            add_comments = True
        elif opt == "-S":
            split_xml = True
        elif opt == "-X":
            pf = open(val, "r")
            for line in pf.readlines():
                if not line.startswith('#') and line.strip() != '':
                    #print "add to exclude '"+line.strip()+"'"
                    typeExcludeList.append(line.strip())
            pf.close

        elif opt == '-?':
            print "Usage: gen_xml_funcdescr.py -i gdb_type_info [-o outfile] [-r] [-p file] [-F family]"
            print "    -i gdb_type_info  an output of gdb command \"info types\""
            print "    -o outfile        Output file"
            print "    -C path           Scan CPSS sources from different path"
            print "    -F family         generate for given families (dxCh,exMxPm,etc)"
            print "    -B fname          Save unparsed structures to this file"
            print "    -M fname          Load manual structure definitions from this file"
            print "    -m map_file       field mapping file   STRUCT_NAME.fpath luaName"
            print "    -D                Add description fields"
            print "    -A fname          Load list of banned API's"
            print "    -X fname          Load list of unsupported types"
            print "    -S                Split XML into per-family files"
            sys.exit(0)

    #print "-------------------------------------------------------"
    #print '****** cpssRoot=',cpssRoot

    if  'CONF_XML_FUNCDESCR' in os.environ:
        #print 'in os.environ: CONF_XML_FUNCDESCR' + ': ', os.getenv('CONF_XML_FUNCDESCR')
        CONF_XML_FUNCDESCR = os.getenv('CONF_XML_FUNCDESCR')
    else:
        #pwd = os.path.abspath('')
        #print 'pwd =', pwd
        CONF_XML_FUNCDESCR = os.path.join(\
            cpssRoot, \
            'mainLuaWrapper', \
            'scripts', \
            'xml_funcdescr.json')

    print "CONF_XML_FUNCDESCR=",CONF_XML_FUNCDESCR  

    data = ''
    Search_list = list()
    if os.path.exists(CONF_XML_FUNCDESCR):
        with open(CONF_XML_FUNCDESCR) as f:
            data = json.load(f)

        dict_conf=dict()
        get_conf_json_all(data,dict_conf)

        #print '-- Search -- Search_list ---------------------------------------------------'
        for Path in dict_conf['Search']['Search_list']:
            if Path != '':
                #print 'Path =', Path
                Search_list.append(os.path.normpath(Path)) 
        #pprint (Search_list)
    else:
        print "Not Found CONF_XML_FUNCDESCR=",CONF_XML_FUNCDESCR  

    #print "-------------------------------------------------------"

    if manual_descr_file != None:
        mdoc = minidom.parse(manual_descr_file)
        top_elem = mdoc.documentElement
        for e in top_elem.getElementsByTagName("Structures"):
            for s in e.getElementsByTagName("Structure"):
                if s.hasAttribute("family"):
                    f = s.getAttribute("family").encode("ascii")
                    if f not in families:
                        continue
                ns = s.cloneNode(0)
                for m in s.getElementsByTagName("Member"):
                    t = m.getAttribute("type").encode("ascii")
                    if m.getAttribute("class") == "struct" and t not in all_defs:
                        if t not in structures_required:
                            structures_required.append(t)
                    if m.getAttribute("class") == "enum" and t not in enums_required:
                        enums_required.append(t)
                    ns.appendChild(m.cloneNode(0))
                t = ns.getAttribute("name").encode("ascii")
                manual_structs[t] = ns
                all_defs[t] = "struct"
                if ns.getAttribute("special") == "unused":
                    all_defs[t] = "unusable"
    if len(gdb_type_info) < 1:
        print "*** gdb_type_info parameter required"
        sys.exit(0)

    if split_xml:
        # In case of splitted XML create for each passed family and common
        # own XML
        if is_file == False:
            print "*** In split mode cannot output in stdout"
            sys.exit()
        else:
            xml_files = dict()
            famlist = ['common']
            famlist.extend(families)
            for fam in famlist:
                xml_files[fam] = dict()
                xml_files[fam]['doc'] = xml_impl.createDocument(None,'CpssFunctionPrototypes',None)

            # Create file descriptors
            if os.path.isdir(ofname):
                # In case if passed to -o value is a directory, make default 
                # file name
                for fam in famlist:
                    xml_files[fam]['handler'] = open(ofname + "/cpssAPI_%s.xml"%fam, "w")
            else:
                # Add suffix to file name base.ext -> base_${family}.ext
                path_parts = ofname.split('.')
                for fam in famlist:
                    if len(path_parts) == 2:
                        xml_files[fam]['handler'] = open(path_parts[0] + "_" + fam + "." + path_parts[1], "w")
                    else:
                        xml_files[fam]['handler'] = open(ofname + "_" + fam, "w")
            xml_files['none'] = xml_files.pop("common") #Rename dictionary key
    else:
        # No splitting
        if is_file == True:
            of = open(ofname, "w")
    
    doc = xml_impl.createDocument(None,'CpssFunctionPrototypes',None)

    for type_info_file in gdb_type_info:
        scan_type_info_from_gdb(type_info_file)

    # list of all declarations
    decls = dict()
    scanned_structs = dict()
    scanned_enums = dict()


    if len(Search_list) > 0:
        search_dirs = list()
        for sl in Search_list:
            search_dirs.append(os.path.join(cpssRoot,sl))
    else:
        search_dirs = [
            os.path.join(cpssRoot,"mainPpDrv","h"),
            os.path.join(cpssRoot,"mainPxDrv","h"),
            os.path.join(cpssRoot,"common","h")
        ]
    print '************************************************'
    print '****** search_dirs='
    pprint(search_dirs)
    print '************************************************'

    # scan CPSS header files for structures/unions prototypes
    for srcpath in all_files_x(search_dirs,"*.h"):
        # skip uneeded families
        family='none'
        for f in [ "dxCh", "px" ]:
            if srcpath.find(os.path.sep+f+os.path.sep) >= 0:
                family = f
        if family != 'none' and family not in families:
            continue
        # skip private headers
        if srcpath.find('private'+os.path.sep) >= 0:
            continue
        if srcpath.find(os.path.sep+'prv') >= 0:
            continue
        source = readfile(srcpath)
        for decl in scan_struct_descriptions(source):
            nm = decl["name"]
            if nm in scanned_structs:
                if 'comment' in decl and 'comment' not in scanned_structs[nm]:
                    scanned_structs[nm]['comment'] = decl['comment']
                continue
            if nm not in all_defs:
                all_defs[nm] = decl["type"]
            if decl["type"] not in ["struct", "union"]:
                if decl["type"] == "enum":
                    if nm not in scanned_enums:
                        scanned_enums[nm] = decl
                continue
            if 'body' not in decl:
                continue
            s = decl['body']
            pos = 0
            stack = [ [] ]
            while True:
                m1 = reStructEmbedStart.search(s,pos)
                m2 = reStructEmbedEnd.search(s,pos)
                if m1 == None and m2 == None:
                    nxt = len(s)
                elif m1 != None and m2 != None:
                    if m1.start(0) < m2.start(0):
                        nxt = m1.start(0)
                    else:
                        nxt = m2.start(0)
                elif m2 == None:
                    nxt = m1.start(0)
                else:
                    nxt = m2.start(0)
                sub = s[pos:nxt]
                pos1 = 0
                while True:
                    m3 = reStructEmbedMember.search(sub,pos1)
                    if m3 == None:
                        break
                    stack[-1].append([ m3.group(2), m3.group(1) ])
                    pos1 = m3.end(0)

                if m1 != None and m1.start(0) == nxt:
                    # start
                    if m1.group(1) == "union":
                        decl["has_unions"] = True
                    stack.append([])
                    pos = m1.end(0)
                elif m2 != None:
                    # end
                    if len(stack) < 2:
                        break # smth is wrong
                    for x in stack[-1]:
                        stack[-2].append([ m2.group(1)+"."+x[0], x[1] ])
                    del stack[len(stack)-1]
                    pos = m2.end(0)
                else:
                    break
            if len(stack) != 1:
                continue

            decl['flds'] = stack[-1]
            decl['family'] = family
            scanned_structs[nm] = decl


    # scan CPSS header files for function prototypes
    for srcpath in all_files_x(search_dirs, "*.h"):
        # skip uneeded families
        family='none'
        for f in [ "dxCh", "px" ]:
            if srcpath.find(os.path.sep+f+os.path.sep) >= 0:
                family = f
        if family != 'none' and family not in families:
            continue
        # skip private headers
        if srcpath.find('private'+os.path.sep) >= 0:
            continue
        if srcpath.find(os.path.sep+'prv') >= 0:
            continue
        if srcpath.startswith(cpssRoot+os.path.sep):
            srcpath_rel = srcpath[len(cpssRoot+os.path.sep):]
        else:
            srcpath_rel = srcpath
        source = readfile(srcpath)
        for decl in scan_c_src(source):
            if 'name' not in decl:
                continue
            name = decl['name']
            decl['srcpath'] = srcpath_rel
            # skip private functions
            if name.startswith(privateAPIStartsWith):
                continue
            # skip banned API's
            if name in banList:
                continue
            if decl['type'] not in [ "GT_STATUS", "void", "GT_VOID" ]:
                x = doc.createElement("Function")
                x.setAttribute("name", name)
                x.setAttribute("srcpath", decl['srcpath'])
                e = doc.createElement("Error")
                e.appendChild(doc.createTextNode('return type is not GT_STATUS/void'))
                x.appendChild(e)
                bad_functions[name] = x
                continue
            decl['param_descr'] = dict()
            if 'comment' in decl:
                # parse comment
                c = decl['comment']
                c = c.replace("\n*", "\n")
                c = c.replace("\n *", "\n")
                desc = None
                state = None
                lparam_name = None
                lparam_descr = None
                def complete_param_descr():
                    global decl
                    global lparam_name, lparam_descr
                    if lparam_name != None:
                        decl['param_descr'][lparam_name] = lstrip_list(lparam_descr)
                    lparam_name = None
                    lparam_descr = None

                for line in c.split("\n"):
                    line = line.rstrip()
                    v = line.replace("\t"," ").replace("  ", " ").strip()
                    m = _reCommentSection.match(v)
                    if m != None:
                        if state == "INPUTS" or state == "OUTPUTS":
                            complete_param_descr()
                        state = m.group(1).strip()
                        continue
                    if state == "DESCRIPTION":
                        if desc == None:
                            desc = list()
                        desc.append(line)
                    if state == "INPUTS" or state == "OUTPUTS":
                        m = _reParamDescr.match(line)
                        if m != None:
                            complete_param_descr()
                            lparam_name = m.group(1)
                            lparam_descr = list()
                            lparam_descr.append(m.group(2))
                        elif lparam_name != None:
                            lparam_descr.append(line.strip())
                            
                if desc != None:
                    decl['descr'] = lstrip_list(desc)
                    #print "descr='"+decl['descr']+"'"
                #print "param_descr=",decl['param_descr']
            skip = ''
            funcErr = False
            # handle parameters
            decl['aparams'] = list()
            for param in decl['params']:
                pdir = param[0]
                ptype = param[1]
                pname = param[2]
                # handle wrong parameter types
                if pdir not in [ "IN", "OUT", "INOUT" ]:
                    skip += 'Wrong parameter direction for '+pname+': "'+pdir+'"\n'
                    funcErr = True
                # handle pointers
                was_ptr = 0
                while ptype.endswith("*"):
                    ptype = ptype[:-1].strip()
                    was_ptr += 1
                while pname.startswith("*"):
                    pname = pname[1:].strip()
                    was_ptr += 1
                aparam = {'dir' : pdir, 'type': ptype, 'name' : pname }
                aparam['class'] = type_classify(ptype)
                if pname in decl['param_descr']:
                    aparam['descr'] = decl['param_descr'][pname]
                if pname.endswith(("Ptr","Prt")):
                    aparam['name'] = pname[:-3]
                # handle arrays
                m = _reArrayParameter.match(pname)
                if m != None:
                    skip += 'Parameter '+pname+' is array\n'
                    aparam['name'] = m.group(1)
                    if m.group(2).strip("0123456789") == "":
                        aparam['class'] += '['+m.group(2)+']'
                    else:
                        aparam['class'] += '[]'
                decl['aparams'].append(aparam)
                if ptype in typeExcludeList:
                    skip += 'Parameter '+pname+' of unsupported type '+ptype+'\n'
                    funcErr = True
                    break
                if pdir in [ "IN", "INOUT" ]:
                    if type_classify(ptype) not in ["bool", "int", "float", "enum", "struct"]:
                        #print "=> ptype='"+ptype+"' cl='"+type_classify(ptype)+"'",param
                        skip += 'Wrong class for parameter '+pname+': '+type_classify(ptype)+'\n'
                        funcErr = True
                        break
                if was_ptr > 1:
                    skip += 'Parameter '+pname+' is double pointer\n'
                    funcErr = True
                elif aparam['class'] == "struct" and was_ptr != 1:
                    skip += 'Parameter '+pname+' is structure by value\n'
                    funcErr = True
                    
            if funcErr:
                x = doc.createElement("Function")
                x.setAttribute("name", name)
                x.setAttribute("srcpath", decl['srcpath'])
                for err in skip.strip().split('\n'):
                    e = doc.createElement("Error")
                    e.appendChild(doc.createTextNode(err))
                    x.appendChild(e)
                bad_functions[name] = x
                continue
            if skip == '':
                x = doc.createElement("Function")
            else:
                x = doc.createElement("ExtFunction")
            x.setAttribute("name", name)
            x.setAttribute("type", decl['type'])
            x.setAttribute("srcpath", decl['srcpath'])
            # add comment
            if add_descriptions and 'descr' in decl:
                descr = doc.createElement("Description")
                descr.appendChild(doc.createTextNode(decl['descr']))
                x.appendChild(descr)
            if add_comments and 'comment' in decl:
                descr = doc.createElement("Comment")
                descr.appendChild(doc.createCDATASection(fix_comment(decl['comment'])))
                x.appendChild(descr)
            params = doc.createElement("Params")
            for p in decl['aparams']:
                param = doc.createElement("Param")
                param.setAttribute("dir", p['dir'])
                param.setAttribute("type", p['type'])
                param.setAttribute("name", p['name'])
                param.setAttribute("class", p['class'])
                if p['class'] in ["struct","struct[]"]:
                    if p['type'] not in structures_required:
                        structures_required.append(p['type'])
                if p['class'] in ["enum", "enum[]"] and p['type'] not in enums_required:
                        enums_required.append(p['type'])
                if p['type'] in manual_structs:
                    _m = manual_structs[p['type']]
                    if _m.hasAttribute("special"):
                        param.setAttribute("special", _m.getAttribute("special"))
                if add_descriptions and 'descr' in p:
                    descr = doc.createElement("Description")
                    descr.appendChild(doc.createTextNode(p['descr']))
                    param.appendChild(descr)
                params.appendChild(param)
            x.appendChild(params)
            x.setAttribute("family", family) #Family attribute will be deleted
            decls[name] = x

    #pp.pprint(scanned_structs)

    structs = dict()
    #TODO: add manual structs for external file
    for t,c in manual_structs.items():
        # Family attribute will be deleted in the resulting XMLs
        if t in scanned_structs.keys():
            c.setAttribute('family', scanned_structs[t]['family'])
        else:
            c.setAttribute('family', 'none') # Currently is only GT_U64
        structs[t] = c
    bad_structs = dict()

    while len(structures_required) > 0:
        for t in structures_required[:]:
            #TODO: skip manually implemented
            if t in structs or t in bad_structs:
                structures_required.remove(t)
                continue
            if t not in structures_required:
                continue
            if t not in all_structs and t not in scanned_structs:
                print "xxx: type="+t
                structures_required.remove(t)
                continue
            if t not in all_structs and t in scanned_structs:
                c = scanned_structs[t]
                x = doc.createElement("Structure")
                x.setAttribute("family", c['family'])
                x.setAttribute("name", t)
                if add_comments and 'comment' in c:
                    comment_elem = doc.createElement("Comment")
                    comment_elem.appendChild(doc.createCDATASection(fix_comment(c['comment'])))
                    x.appendChild(comment_elem)
                for fld in c['flds']:
                    fname = fld[0]
                    ftype = fld[1]
                    fcls = type_classify(ftype)
                    handled = True
                    if fcls in ["bool", "int", "float", "enum", "struct", "union"]:
                        member = doc.createElement("Member")
                        member.setAttribute("name", fname)
                        member.setAttribute("class", fcls)
                        member.setAttribute("type", ftype)
                        x.appendChild(member)
                        if fcls in ["struct","union"] and ftype not in structs and ftype not in bad_structs:
                            if ftype not in structures_required:
                                structures_required.append(ftype)
                        if fcls == "enum" and ftype not in enums_required:
                            enums_required.append(ftype)
                        if fcls == "union":
                            x.setAttribute("special","union")
                if c["type"] == "union":
                    x.setAttribute("special","UNION")
                structs[t] = x
                structures_required.remove(t)
            else:
                family = None
                if t in scanned_structs:
                    family = scanned_structs[t]['family']

                if family == None: #Heuristic to determine family by struct name
                    if t.startswith('CPSS_DXCH'):
                        family = "dxCh"
                    else:
                        family = "none"

                c = all_structs[t]
                if t not in structures_required:
                    continue
                #TODO: skip manually implemented
                if len(c) > 2:
                    x = doc.createElement("Structure")
                    x.setAttribute("name", t)
                    x.setAttribute("family", family) #Family attribute will be deleted
                    comment_elem = ''
                    if add_comments and t in scanned_structs:
                        if 'comment' in scanned_structs[t]:
                            comment_elem = doc.createElement("Comment")
                            comment = scanned_structs[t]['comment']
                            comment_elem.appendChild(doc.createCDATASection(fix_comment(comment)))
                    if comment_elem != '':
                        x.appendChild(comment_elem)
                    errors = False
                    for line in c[1:-1]:
                        line = line.strip()
                        m = reStruct_field.match(line)
                        error = ""
                        if m != None:
                            ftype = m.group(1)
                            fname = m.group(2)
                            # do mapping
                            if t+"."+fname in mapping:
                                fname = mapping[t+"."+fname]
                            fcls = type_classify(m.group(1)) 
                            if fcls in ["bool", "int", "float", "enum", "struct", "union"]:
                                member = doc.createElement("Member")
                                member.setAttribute("name", fname)
                                member.setAttribute("class", fcls)
                                member.setAttribute("type", ftype)
                                if m.group(4) != None:
                                    member.setAttribute("array", m.group(4))
                                x.appendChild(member)
                                if fcls in ["struct", "union"] and ftype not in structs and ftype not in bad_structs:
                                    if ftype not in structures_required:
                                        structures_required.append(ftype)
                                if fcls == "enum" and ftype not in enums_required:
                                    enums_required.append(ftype)
                                if fcls == "union":
                                    x.setAttribute("special","union")
                            else:
                                error = "cannot identify type class"
                        else:
                            handled = False
                            if t in scanned_structs:
                                m = reStruct_comprex_field.match(line)
                                if m == None:
                                    m = reStruct_comprex_field2.match(line)
                                if m != None:
                                    prefix = m.group(2)+"."
                                    for fld in scanned_structs[t]['flds']:
                                        if fld[0].startswith(prefix):
                                            fname = fld[0]
                                            ftype = fld[1]
                                            # do mapping
                                            if t+"."+fname in mapping:
                                                fname = prefix+mapping[t+"."+fname]
                                            fcls = type_classify(ftype)
                                            handled = True
                                            if fcls in ["bool", "int", "float", "enum", "struct", "union"]:
                                                member = doc.createElement("Member")
                                                member.setAttribute("name", fname)
                                                member.setAttribute("class", fcls)
                                                member.setAttribute("type", ftype)
                                                x.appendChild(member)
                                                if fcls in ["struct","union"] and ftype not in structs and ftype not in bad_structs:
                                                    if ftype not in structures_required:
                                                        structures_required.append(ftype)
                                                if fcls == "enum" and ftype not in enums_required:
                                                    enums_required.append(ftype)
                                                if fcls == "union":
                                                    x.setAttribute("special","union")
                            if handled and 'has_unions' in scanned_structs[t]:
                                x.setAttribute("special","union")

                            if not handled:
                                error = "unhandled line"
                        if error != "":
                            x.appendChild(doc.createComment(" error: "+error+" "+line.rstrip()+" "))
                            errors = True
                    #x.writexml(sys.stdout,"", " ", "\n")
                    if t in scanned_structs and scanned_structs[t]["type"] == "union":
                        x.setAttribute("special", "UNT")
                    if not errors:
                        structs[t] = x
                    else:
                        bad_structs[t] = x
                    structures_required.remove(t)

    enums = dict()
    for t in enums_required[:]:
        if t not in all_enums:
            continue
        x = doc.createElement("Enum")
        x.setAttribute("name", t)
        if add_comments and t in scanned_enums and 'comment' in scanned_enums[t]:
            descr = doc.createElement("Comment")
            descr.appendChild(doc.createCDATASection(fix_comment(scanned_enums[t]['comment'])))
            x.appendChild(descr)
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
                e = doc.createElement("Value")
                e.setAttribute("str", m.group(1))
                x.appendChild(e)
                p = m.end(0)
        enums[t] = x


    # Now sort nodes and save xml to file
    if split_xml == True:
        famlist = ['none']
        famlist.extend(families)
        #For each family invcluding 'none'
        for fam in famlist:
            # Add functions to resulting XML
            #   Calculate which structures belong to the family
            fam_funcs = list(map(lambda x: x[0], filter(lambda x: x[1].getAttribute('family') == fam, decls.iteritems())))
            fam_funcs.sort()
            curdoc = xml_files[fam]['doc']
            top_elem = curdoc.documentElement
            x = curdoc.createElement("Functions")
            for f in fam_funcs:
                x.appendChild(decls[f])
            # Write function count
            x.setAttribute("count", "%d"%len(fam_funcs))
            top_elem.appendChild(x)
            # Add structures to resulting XML
            struct_names = structs.keys()
            #   Calculate which structures belong to the family
            fam_structs = list(map(lambda x: x[0], filter(lambda x: x[1].getAttribute('family') == fam, structs.iteritems())))
            fam_structs.sort()
            x = curdoc.createElement("Structures")
            for t in fam_structs:
                x.appendChild(structs[t])
            # Write structure count
            x.setAttribute("count", "%d"%len(fam_structs))
            top_elem.appendChild(x)
    else:
        #Creating a single XML
        all_funcs = decls.keys()
        all_funcs.sort()
        top_elem = doc.documentElement
        x = doc.createElement("Functions")
        for f in all_funcs:
            x.appendChild(decls[f])
        x.setAttribute("count", "%d"%len(all_funcs))
        top_elem.appendChild(x)
        struct_names = structs.keys()
        if len(struct_names) > 0:
            struct_names.sort()
            x = doc.createElement("Structures")
            for t in struct_names:
                x.appendChild(structs[t])
            x.setAttribute("count", "%d"%len(struct_names))
            top_elem.appendChild(x)
        x = doc.createElement("Enums")
        for t in enums.keys():
            x.appendChild(enums[t])
        top_elem.appendChild(x)

    #Remove family attributes
    for k,v in decls.iteritems():
        if v.getAttribute('family') != '':
            v.removeAttribute('family')
    for k,v in structs.iteritems():
        if v.getAttribute('family') != '':
            v.removeAttribute('family')

    # Add timestamp and command line
    doc.appendChild(doc.createComment(" "+" ".join(sys.argv)+" "))
    doc.appendChild(doc.createComment(" Generated "+time.ctime()+" "))

    # Write XML and close file
    if split_xml == True:
        for key, entry in xml_files.iteritems():
            entry['doc'].appendChild(entry['doc'].createComment(" "+" ".join(sys.argv)+" "))
            entry['doc'].appendChild(entry['doc'].createComment(" Generated "+time.ctime()+" "))
            entry['doc'].writexml(entry['handler'],"", " ", "\n", "utf-8")
            entry['handler'].close()
    else:
        doc.writexml(of,"", " ", "\n", "utf-8")
        if is_file:
            of.close()

    if len(bad_structs) > 0 or len(bad_functions) > 0:
        if bad_struct_fname != "<STDOUT>":
            bad_struct_f = open(bad_struct_fname, "w")
        else:
            bad_struct_f = sys.stdout

        bdoc = xml_impl.createDocument(None,'CpssFunctionPrototypes',None)
        top_elem = bdoc.documentElement

        if len(bad_functions) > 0:
            x = doc.createElement("Functions")
            for t,v in bad_functions.items():
                x.appendChild(v)
            x.setAttribute("count", "%d"%len(bad_functions))
            top_elem.appendChild(x)
        if len(bad_structs) > 0:
            x = doc.createElement("Structures")
            for t,v in bad_structs.items():
                x.appendChild(v)
            x.setAttribute("count", "%d"%len(bad_structs))
            top_elem.appendChild(x)

        bdoc.writexml(bad_struct_f,"", " ", "\n", "utf-8")

        if bad_struct_fname != "<STDOUT>":
            bad_struct_f.close()


