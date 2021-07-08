#!/usr/bin/env python
#------------------------------------------------------------------------------
#  File name: make_cpss_api_xml.py
#------------------------------------------------------------------------------
# Usage:
#   python output_file.xml input_file1.xml input_file2.xml input_file3.xml
#
#------------------------------------------------------------------------------
#  File version: 1 $
#------------------------------------------------------------------------------

from xml.dom import minidom
import getopt, sys, time

# The function adds merges new XML file with resulting XML object
def appendXml(funcs, structs, xmld):
    top_elem = xmld.documentElement

    # Merge functions
    functionElements = funcs.getElementsByTagName("Function")
    funcList = list(map(lambda x: x.getAttribute("name"), functionElements))
    fcount = 0
    for e in top_elem.getElementsByTagName("Functions"):
        for s in e.getElementsByTagName("Function"):
            if s.getAttribute("name") not in funcList:
                funcs.appendChild(s)
                fcount += 1

    # Merge structures
    structureElements = structs.getElementsByTagName("Structure")
    structList = list(map(lambda x: x.getAttribute("name"), structureElements))
    scount = 0
    for e in top_elem.getElementsByTagName("Structures"):
        for s in e.getElementsByTagName("Structure"):
            if s.getAttribute("name") not in structList:
                structs.appendChild(s)
                scount += 1

    return fcount, scount

if __name__ == '__main__':
    # Parse arguments
    of = sys.stdout
    xmls = list()
    of = open(sys.argv[1], "w")
    xmls.extend(sys.argv[2:])
    if len(sys.argv) < 2:
        print "Usage: make_cpss_api_xml.py -o cpssAPI.xml -X infile1.xml [-X infile2.xml ..]"
        print "    -o outfile        Output file"
        print "    -X infile         XML file to merge in output file"
        sys.exit(0)

    # Create resulting XML object
    xml_impl = minidom.getDOMImplementation()
    doc = xml_impl.createDocument(None,'CpssFunctionPrototypes',None)
    top_elem = doc.documentElement
    functions = doc.createElement("Functions")
    structures = doc.createElement("Structures")

    # Merge resulting XML with XMLs passed
    function_count = 0
    structure_count = 0
    for xname in xmls:
        print "Including ", xname
        fcount, scount = appendXml(functions, structures, minidom.parse(xname))
        function_count += fcount
        structure_count += scount

    # Set count of structures and functions
    functions.setAttribute("count", "%d"%function_count)
    structures.setAttribute("count", "%d"%structure_count)

    top_elem.appendChild(functions)
    top_elem.appendChild(structures)

    # Add timestamp and commandline
    doc.appendChild(doc.createComment(" "+" ".join(sys.argv)+" "))
    doc.appendChild(doc.createComment(" Generated "+time.ctime()+" "))

    #Write changes and save file
    doc.writexml(of, "", " ", "\n", "utf-8")
    of.close()
