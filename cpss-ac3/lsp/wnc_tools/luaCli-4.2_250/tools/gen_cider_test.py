#!/usr/bin/python
# -*- coding: utf-8 -*-

from xml.dom.minidom import parse
import os,sys,getopt

def Usage():
    print "Usage:"
    print sys.argv[0]+" [-c] [cider.xml]"
    print "   -c          - check only"
    sys.exit(0)


cider_regs = dict()
cider_units = dict()

def xml_nodetext(node):
    t = list()
    for i in node.childNodes:
        if i.nodeType == i.TEXT_NODE:
            t.append(i.nodeValue.encode("utf-8"))
    return " ".join(t)

def get_int_attr(node, name, forcehex=False):
    s = node.getAttribute(name).encode("utf-8")
    if s == None or s == "":
        return 0
    if forcehex and s[:2].lower != "0x":
        s = "0x" + s
    return int(s, 0)

def cider_unitname(node):
    s = None
    for i in node.childNodes:
        if i.nodeType != i.ELEMENT_NODE:
            continue
        if i.tagName == "swName":
            s = xml_nodetext(i)
    if s == None:
        s = node.getAttribute("name").encode("utf-8")
    if s == None:
        return s
    return s.strip().replace("  ", " ").replace(" ", "_")

def lookup_child(node, path):
    for i in node.childNodes:
        if i.nodeType != i.ELEMENT_NODE:
            continue
        if i.tagName != path[0]:
            continue
        if len(path) == 1:
            return i
        return lookup_child(i, path[1:])
    return None

def lookup_alternatives(node, paths):
    for path in paths:
        i = lookup_child(node, path)
        if i != None:
            return i
    return None

def walk_nodes(node, path):
    global cider_regs
    global cider_units
    if node.nodeType != node.ELEMENT_NODE:
        return
    if node.tagName not in ["UNIT","MEM","REGISTER"]:
        return

    unitname = cider_unitname(node)
    npath = path[:]
    npath.append(unitname)

    # check duplicated
    unitpath = " ".join(npath[1:])
    if unitpath in cider_units:
        print >>sys.stderr, "duplicate unit: "+unitpath
    else:
        cider_units[unitpath] = 1

    if node.tagName == "MEM" or node.tagName == "REGISTER":
        # process register
        regname = unitname
        if unitname in cider_regs:
            regname = unitname + "_("+"_".join(path[1:])+")"
            if "count" in cider_regs[unitname]:
                cider_regs[unitname]["count"] += 1
            else:
                nname = unitname + "_("+"_".join(cider_regs[unitname]["path"])+")"
                cider_regs[nname] = cider_regs[unitname]
                cider_regs[unitname] = dict()
                cider_regs[unitname]["count"] = 2
        n = dict()
        n["path"] = path[1:]
        n["name"] = unitname
        n["node"] = node
        n["type"] = node.tagName.encode("utf-8")
        if node.tagName == "REGISTER":
            n["baseaddr"] = get_int_attr(node, "addr", True)
            n["addr"] = lookup_child(node,["ADDR"])
        if node.tagName == "MEM":
            n["baseaddr"] = get_int_attr(node, "mem_base", True)
            n["addr"] = lookup_alternatives(node, [
                        ["TABLE", "DS", "ADDR"],
                        ["TABLE", "ADDR"],
                        ["ADDR"] ])
        #if regname in cider_regs:
        #    print >>sys.stderr, "duplicate name: "+regname
        cider_regs[regname] = n
        return

    # node.tagName == "UNIT"
    for i in node.childNodes:
        if i.nodeType != i.ELEMENT_NODE:
            continue
        walk_nodes(i, npath)


def cider_iterator_init(unit):
    it = dict()
    it["baseaddr"] = unit["baseaddr"]
    it["valid"] = True
    addr = unit["addr"]
    if addr == None:
        return it
    it["baseaddr"] = get_int_attr(addr,"baseAddr")
    it["indices"] = dict()
    for param in unit["addr"].getElementsByTagName("PARAM"):
        idx = dict()
        idx["name"] = param.getAttribute("name").encode("utf-8")
        idx["startVal"] = get_int_attr(param,"startVal")
        idx["endVal"] = get_int_attr(param,"endVal")
        idx["incr"] = get_int_attr(param,"incr")
        ordinal = get_int_attr(param,"ordinal")
        idx["value"] = idx["startVal"]
        it["indices"][ordinal] = idx
    it["formula"] = addr.getAttribute("formula").encode("utf-8")
    return it

# return (address,index_string)
def cider_iterator_next(it):
    if not it["valid"]:
        return None, ""
    if "indices" not in it:
        it["valid"] = False
        return it["baseaddr"], ""
    index_string = ""
    d = dict()
    for i in range(3):
        if i not in it["indices"]:
            continue
        v = it["indices"][i]
        d[v["name"]] = v["value"]
        if i == 0:
            index_string += " index "
        else:
            index_string += " index%d "%(i+1)
        index_string += "%d"%v["value"]
    i = 2
    while i >= 0:
        if i not in it["indices"]:
            i -= 1
            continue
        cidx = it["indices"][i]
        cidx["value"] += cidx["incr"]
        if cidx["value"] <= cidx["endVal"]:
            break
        cidx["value"] = cidx["startVal"]
        i -= 1
    if i < 0:
        it["valid"] = False
    return it["baseaddr"]+eval(it["formula"], d), index_string


if __name__ == '__main__':
    cider_file = "cider.xml"
    check_only = False
    opts, files = getopt.getopt(sys.argv[1:],"?c")
    for opt,val in opts:
        if opt == "-c":
            check_only = True
        else:
            Usage()
    if len(files) > 0:
        cider_file = files[0]

    # parse cider.xml
    dom1 = parse(cider_file)
    cider_root = dom1.documentElement.getElementsByTagName("UNITS")[0].getElementsByTagName("UNIT")[0]
    walk_nodes(cider_root, list())

    if check_only:
        sys.exit(0)

    print "+ pass 1:"
    print "+     show unit %cider_unitpath [index %number [index2 %number]]"
    print "+     check register/mem address"
    print "+     (%d total)"%len(cider_regs)
    regno = 0
    tenperc = len(cider_regs) / 10
    for k, v in cider_regs.items():
        regno += 1
        if regno % tenperc == 0:
            print "+     %d done..."%regno
        if regno > 20:
            break
        if "count" in v:
            continue
        it = cider_iterator_init(v)
        c = 0
        print "*"+k
        # Check up to 5 table entries
        while True and c < 5:
            addr, index_string = cider_iterator_next(it)
            if addr == None:
                break
            c += 1
            print ">show unit "+" ".join(v["path"])+" "+v["name"]+index_string
            print "<0x%x"%addr


    print >>sys.stderr, len(cider_regs)
    print "+ pass 2:"
    print "+     read unit %cider_register_name [index 0]"
    print "+     check register/mem address"
    print "+     (%d total)"%len(cider_regs)
    regno = 0
    tenperc = len(cider_regs) / 10
    for k, v in cider_regs.items():
        regno += 1
        if regno % tenperc == 0:
            print "+     %d done..."%regno
        if regno > 20:
            break
        if "count" in v:
            continue
        it = cider_iterator_init(v)
        print "*"+k
        addr, index_string = cider_iterator_next(it)
        if addr != None:
            print ">read "+k+index_string
            print "<0x%x"%addr

    print "+ pass 3:"
    print "+     find all 0x12345678"
    print "+     check register name, index"
    print "+     (%d total)"%len(cider_regs)
    regno = 0
    tenperc = len(cider_regs) / 10
    for k, v in cider_regs.items():
        regno += 1
        if regno % tenperc == 0:
            print "+     %d done..."%regno
        if regno > 20:
            break
        if "count" in v:
            continue
        it = cider_iterator_init(v)
        print "*"+k
        addr, index_string = cider_iterator_next(it)
        if addr != None:
            print ">find all 0x%x"%addr
            print "<"+k+index_string
