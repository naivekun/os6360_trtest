#!/usr/bin/python

######################################################################
# iDebugPrepareXml.py
#
# DESCRIPTION:
#       This script prepare .xml file for iDebug.lua
#       It does the following:
#           1. Add internalName attribute to <Interface>, <Value>
#           2. Add index/value suffix to internalName if name is duplicated
#           3. Remove empty <Description>, <internalDescription>,
#              <InternalDescription>, <Enums>
#           4. Set <Field ... type="enum"> for these ones which has
#              non-empty <Enums>
#
#
#       Usage:
#       iDebugPrepareXml.py [-o fname] [-N] generated_xml_name
#
#
# INPUTS:
#
# OUTPUTS:
#
# RETURNS:
#       none
#
# COMMENTS:
#
# FILE REVISION NUMBER:
#       $Revision: $
#####################################################################

import re, os, getopt, sys
from xml.dom import minidom
xml_impl = minidom.getDOMImplementation()

rSpecials = re.compile(r"\W")
def do_elements(e):
    #print "p1. e.localName="+e.localName
    # create dict of interfaces to detect duplicate internalName s
    ifdict = dict()
    if e.localName in [ "Interfaces", "Enums" ]:
        c = e.firstChild
        while c != None:
            if c.nodeType == c.ELEMENT_NODE:
                if c.localName == "Value":
                    if not c.hasAttribute("internalName"):
                        a = c.getAttribute("name").strip().replace("  ", " ")
                        a = a.replace("&", "n")
                        a = rSpecials.sub("_", a)
                        c.setAttribute("internalName", a)
                        c.removeAttribute("name")
                if c.localName in [ "Interface", "Value" ]:
                    a = c.getAttribute("internalName")
                    if a not in ifdict:
                        ifdict[a] = 0
                    ifdict[a] += 1
            c = c.nextSibling

    c = e.firstChild
    while c != None:
        n = c.nextSibling
        if c.nodeType == c.CDATA_SECTION_NODE:
            t = e.ownerDocument.createTextNode(c.wholeText)
            e.replaceChild(t, c)
        elif c.nodeType == c.TEXT_NODE:
            pass
        elif c.nodeType == c.ELEMENT_NODE:
            if c.localName in [ "Description", "internalDescription", "InternalDescription" ] and c.firstChild == None:
                e.removeChild(c)
            elif c.localName == "Enums" and e.localName == "Field":
                if c.firstChild == None:
                    e.removeChild(c)
                    # add attr type="raw" when length>32bit
                    bs = e.getAttribute("bitStart")
                    be = e.getAttribute("bitStop")
                    if bs != None and be != None:
                        if int(be) - int(bs) + 1 > 32:
                            e.setAttribute("type", "raw")
                else:
                    e.setAttribute("type", "enum")
                    do_elements(c)
            elif c.localName == "Master" and e.localName == "Instance":
                a = c.getAttribute("internalName")
                if a != None:
                    e.setAttribute("masterName", a)
                    e.removeChild(c)
            elif c.localName == "Interface":
                a = c.getAttribute("internalName")
                if ifdict[a] > 1:
                    c.setAttribute("internalName", a + "_" + c.getAttribute("index"))
                do_elements(c)
            elif c.localName == "Value":
                a = c.getAttribute("internalName")
                if ifdict[a] > 1:
                    c.setAttribute("internalName", a + "_" + c.getAttribute("value"))
                do_elements(c)
            else:
                do_elements(c)
                if c.localName == "Instance" and c.getAttribute("masterName") == None:
                    print "No masterName for Instance "+c.getAttribute("name")
        #
        c = n

        
def Usage():
    print "Usage: iDebugPrepareXml.py [-o output_file] [-N] xml_file"
    print "    -o outfile        Output file"
    print "    -N                Formatted output"
    sys.exit(0)

if __name__ == '__main__':
    input_file = "instances.xml"
    output_file = "iDebug.xml"
    output_nice = False
    opts,rest = getopt.getopt(sys.argv[1:], "o:N?")
    for (opt,val) in opts:
        if opt == '-o':
            output_file = val
        elif opt == '-N':
            output_nice = True

        elif opt == '-?':
            Usage()
    if len(rest) < 1:
        Usage()

    print rest[0] 
    mdoc = minidom.parse(rest[0])
    top_elem = mdoc.documentElement
    do_elements(top_elem)

    if output_nice:
        mdoc.writexml(open(output_file,"w"),"", " ", "\n", "utf-8")
    else:
        mdoc.writexml(open(output_file,"w"),"", "", "", "utf-8")
