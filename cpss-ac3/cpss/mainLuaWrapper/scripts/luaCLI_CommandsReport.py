#!/usr/bin/python
#------------------------------------------------------------------------------
#  File name: luaCLI_CommandsReport.py
#------------------------------------------------------------------------------
# Usage:
#   luaCLI_CommandsReport.py [-o <outfile>] -d <scripts_dir>
#       -o outfile        Output file
#       -d scripts_dir    CLI scripts dir path
# Example
#   python luaCLI_CommandsReport.py -o /tmp/cmds_stat.html -d /lua/lua_cli/scripts
#
#--------------------------------------------------------------------------
#  File version: 1 $
#--------------------------------------------------------------------------

import os, re, sys, getopt
import argparse

from xml.dom import minidom
from numpy.core.defchararray import startswith


reAddCommand =  re.compile(r'CLI_addCommand\s*\(\s*"([^"][^"]*)"\s*,\s*"([^"][^"]*)"',re.M)
reAddCommand2 = re.compile(r'CLI_addCommand\s*\(\s*\{([^\}]*)\}\s*,\s*"([^"][^"]*)"',re.M)
reModeListSplit = re.compile(r'"\s*,\s*"',re.M)


commands = dict()
cmdlist = list()

sourceName = ""
luaDir = ""

# parse command line arguments.
#______________________________________________________________________________
def parseCmdArgs():
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--DIR', type=str, metavar='luaScriptsDir', required=True,
                         help = "the lua scripts path")
    parser.add_argument('-o', '--OUT', type=str, metavar='outHtmlReportName', required=False, default='luaCommandsStat.html',
                         help = "an output html file with statistics of existent CLI commands ")
    args = parser.parse_args()
    return args.DIR, args.OUT

#______________________________________________________________________________
def updateCommandsTree(mode, cmd):
    global commands
    global sourceName

    if not commands.has_key(cmd):
        cmdlist.append(cmd)
                    
        commands[cmd] = dict()
        commands[cmd]['common'] = dict()
        commands[cmd]['px']     = dict()
        commands[cmd]['dxCh']   = dict()
        commands[cmd]['root']   = dict()
        
        commands[cmd]['common']['mode'] = list() 
        commands[cmd]['common']['fname'] = ""
        commands[cmd]['px']['mode'] = list()
        commands[cmd]['px']['fname'] = ""
        commands[cmd]['dxCh']['mode'] = list()
        commands[cmd]['dxCh']['fname'] = ""
        commands[cmd]['root']['mode'] = list() 
        commands[cmd]['root']['fname'] = ""

    if sourceName.startswith('common'):
        fam = 'common'
    elif sourceName.startswith('px'):
        fam = 'px'
    elif sourceName.startswith('dxCh'):
        fam = 'dxCh'
    else:
        fam = 'root'

    commands[cmd][fam]['mode'].append(mode)
    if commands[cmd][fam]['fname'] == "":
        commands[cmd][fam]['fname'] = sourceName
    else:
        commands[cmd][fam]['fname'] += ("<br>" + sourceName)

        
    
#______________________________________________________________________________
def do_addCommand(m):
    updateCommandsTree(m.group(1), m.group(2))

#______________________________________________________________________________
def do_addCommand2(m):
    cmd = m.group(2)
    mds = m.group(1).strip()
    if not mds.startswith('"') or not mds.endswith('"'):
        return
    mds = mds[1:-1]
    for m in  reModeListSplit.split(mds):
        updateCommandsTree(m, cmd)

#______________________________________________________________________________
def scan_file(fname):
    global luaDir
    global sourceName
    f=open(fname)
    content=f.read()
    f.close()
    sourceName = fname[len(luaDir)+1:] 
    reAddCommand.sub(do_addCommand, content)
    reAddCommand2.sub(do_addCommand2, content)


#______________________________________________________________________________
if __name__ == '__main__':
    scripts_dir, outFile = parseCmdArgs()

    luaDir = scripts_dir
    
    try:
         of = open(outFile, "w")
    except IOError:
        print "Error: cannot create output file: " + outFile + "\n"
        sys.exit(2)


    for path, subdirs, files in os.walk(scripts_dir):
        for name in files:
            if not name.endswith(".lua"):
                continue
            # .lua file
            scan_file(os.path.join(path, name))



    of.write('<head><title>CLI Commands report</title></head>\n')
    of.write('<body><p style="font-family: Helvetica, Tahoma, Verdana, Helv, serif;">\n')
#    of.write('<br><b>CLI Commands report:<br> %s </b><br><br>\n' % (" "+" ".join(sys.argv)+" "))
    of.write('<br><b>CLI Commands report<br>\n')
    
    of.write('<br><table border><tr><th>&nbsp;</th><th colspan=2>common</th><th colspan=2>DX</th><th colspan=2>PX</th><th colspan=2>root</th></tr>\n')

    of.write('<tr><td>command name</td><td align="left">fname</td><td align="left">mode</td>'\
             '<td align="left">fname</td><td align="left">mode</td>'\
             '<td align="left">fname</td><td align="left">mode</td>'\
             '<td align="left">fname</td><td align="left">mode</td></tr>\n')


    commandsNum = 0
    redLabel = 0
    greenLabel = 0
    yellowLabel = 0

    for i in range(len(cmdlist)):
        k = cmdlist[i]
        # print k
        commandsNum +=1
        
        bgColor = "#FFFFFF"
        if (commands[k]['root']['fname'] != ""):
            if (commands[k]['common']['fname'] != "") or\
               (commands[k]['px']['fname'] != "") or \
               (commands[k]['dxCh']['fname'] != ""):
                bgColor = "#FFAAAA"
                redLabel += 1
            
            else:
                bgColor = "#AAFFAA"
                greenLabel += 1
                
        elif (commands[k]['common']['fname'] != ""):
            if (commands[k]['px']['fname'] != "") or \
               (commands[k]['dxCh']['fname'] != ""):
                bgColor = "#FFAAAA"
                redLabel += 1

            else:
                bgColor = "#AAFFAA"
                greenLabel += 1

        else:
            if (commands[k]['px']['fname'] != "") and \
               (commands[k]['dxCh']['fname'] != ""):
                bgColor = "#FFFFAA"
                yellowLabel += 1

            else:
                bgColor = "#AAFFAA"
                greenLabel += 1
 

        of.write('<tr bgcolor=%s><td align="left"><b>%s</b></td>' \
                  '<td align="left"><small>%s</small></td>'\
                  '<td align="left"><small>%s</small></td>'\
                  '<td align="left"><small>%s</small></td>'\
                  '<td align="left"><small>%s</small></td>'\
                  '<td align="left"><small>%s</small></td>'\
                  '<td align="left"><small>%s</small></td>'\
                  '<td align="left"><small>%s</small></td>'\
                  '<td align="left">%s</td></tr>\n' % (bgColor, k, 
                                                       commands[k]['common']['fname'], commands[k]['common']['mode'],
                                                       commands[k]['px']['fname'], commands[k]['px']['mode'],
                                                       commands[k]['dxCh']['fname'], commands[k]['dxCh']['mode'],
                                                       commands[k]['root']['fname'], commands[k]['root']['mode']))

    of.write("</table><br>\n")
    of.write("<b> ========== </b>\n")
    of.write("<b>Total different commands: %d</b><br>\n" % commandsNum)
    of.write("<b> red(needs to check): %d</b><br>\n" % redLabel)
    of.write("<b> green(ok):           %d</b><br>\n" % greenLabel)
    of.write("<b> yellow(to do):       %d</b><br>\n" % yellowLabel)
    of.write("<b> ========== </b>\n")
    of.write("</p></body>\n")
    of.close()
    