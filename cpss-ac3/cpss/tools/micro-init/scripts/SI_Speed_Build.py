#!\usr\bin\python

import getopt, re, fnmatch, string, glob
import sys
import os
import subprocess
import csv
import time
import shutil
from pprint import *

import re
#----------------------------------------------------------------------

def csv_reader_b2(\
    CPSS_PATH,\
    WORK_DIR, \
    MICRO_INIT_CONF, \
    RESULT_DIR, \
    group_name, \
    MI_SI_Speed_ParmCsv, \
    dict_si_speed, \
    key1, \
    cpu, \
    folder, \
    dict_txt_file, \
    dict_version):


    print 'group_name=',group_name
    print 'key1=',key1
    #pprint (dict_version)


    #n:\shared\mi\Scripts_SI\aldrin\file_lists\

    if  not dict_version.has_key(key1):
        print '! not exists' +key1
        return

    if  dict_version.has_key(key1):
        version_list  =    dict_version[key1]  

        #pprint (version_list)

        nn=0
        for version in version_list:
            nn = nn+1

            #print 'version=',version
            txt_file =  dict_txt_file[key1+version]

            key = cpu+','+version+','+folder

            if not  dict_si_speed.has_key(key):
                continue

            print "key= ", key

            OUT_FILE = os.path.join(\
                RESULT_DIR, \
                "Micro_Init", \
                folder, \
                "file_lists", \
                "file_list_" + \
                folder +'_'+ \
                txt_file+'.txt')

            print "OUT_FILE   => ", OUT_FILE   
            folder_path = os.path.dirname(OUT_FILE)
            if not os.path.exists(folder_path):
                os.makedirs(folder_path)
            of = open(OUT_FILE, "wb")

            of.write('#path name type  cpy_offset exec_offset spi_offset hex_bmp \n')
           
            line_list = dict_si_speed[key]
            nnn=0
            for line in line_list:
                nnn = nnn+1
            

                if line["enable"] != 'Y':
                    continue

                #pprint (line)
                #print nnn, \
                #    line["cpu"],\
                #    line["version"],\
                #    line["enable"],\
                #    line["nn"],\
                #    line["folder"],\
                #    line["path"],\
                #    line["file"],\
                #    line["name"],\
                #    line["type"],\
                #    line["cpy_offset"],\
                #    line["exec_offset"],\
                #    line["spi_offset"],\
                #    line["hex_bmp"]

                if line["path"] == '.':
                    path = os.path.join(\
                        MICRO_INIT_CONF, \
                        line["folder"], \
                        line["file"])
                else:
                    path = os.path.join(\
                        MICRO_INIT_CONF, \
                        line["folder"], \
                        line["path"], \
                        line["file"])
                    
                of.write(\
                    path       +' '+\
                    line["name"]       +' '+\
                    line["type"]       +' '+\
                    line["cpy_offset"] +' '+\
                    line["exec_offset"]+' '+\
                    line["spi_offset"] +' '+\
                    line["hex_bmp"]    +\
                    '\n')

            of.close()

            script_mi = os.path.join(\
                CPSS_PATH, \
                "tools", \
                "micro-init", \
                "scripts", \
                'build_image.sh')

            print "script_mi= ", script_mi

            #${CPSS_PATH}/tools/micro-init/scripts/build_image.sh -m CM3 -v Snake_Test_Aldrin_1000Base_X_1  /local/store/shared/mi/aldrin/file_lists/file_list_1000Base_X_1.txt
            subprocess.call([script_mi, \
                "-m", \
                group_name, \
                "-v", \
                "Snake_Test_" + cpu + "_" + line["version"], \
                OUT_FILE])


            IN_BIN = os.path.join(\
                WORK_DIR, \
                'super_image.bin')

            OUT_BIN = os.path.join(\
                RESULT_DIR, \
                "Micro_Init", \
                folder, \
                "super_images", \
                "Snake_Test_" +\
                cpu + "_" +\
                txt_file+'.bin')

            #print IN_BIN, "= ",IN_BIN
            #print OUT_BIN,"= ",OUT_BIN
            print "=> ",OUT_BIN

            if os.path.exists(IN_BIN):

                folder_path = os.path.dirname(OUT_BIN)
                if not os.path.exists(folder_path):
                    os.makedirs(folder_path)
                shutil.copy(IN_BIN, OUT_BIN)
                #shutil.move(OUT_BIN, IN_BIN)

                os.remove(IN_BIN)
            else:    ## Show an error ##
                print "Error: file not found " + IN_BIN

#----------------------------------------------------------------------

def csv_reader_b1(\
    CPSS_PATH,\
    WORK_DIR, \
    MICRO_INIT_CONF, \
    RESULT_DIR, \
    MI_SI_Speed_ParmCsv, \
    f_obj, \
    f_obj2, \
    f_obj3):

    #----------------------------------------------------------
    dict_si_speed  = dict()
    reader = csv.DictReader(f_obj3, delimiter=',')
    nn=0
    for line in reader:
        nn = nn+1

        if line["enable"] != 'Y':
            continue

        #print nn, \
        #    line["cpu"],\
        #    line["version"],\
        #    line["enable"],\
        #    line["nn"],\
        #    line["folder"],\
        #    line["path"],\
        #    line["file"],\
        #    line["name"],\
        #    line["type"],\
        #    line["cpy_offset"],\
        #    line["exec_offset"],\
        #    line["spi_offset"],\
        #    line["hex_bmp"]


        key = line["cpu"]+','+line["version"]+','+line["folder"]

        if not  dict_si_speed.has_key(key):
            dict_si_speed[key]  =  list()

        if line["version"] not in dict_si_speed[key]:
            dict_si_speed[key].append(line)

        #if nn >= 21:
        #    break

    #----------------------------------------------------------
    dict_version  = dict()
    dict_txt_file  = dict()
    reader = csv.DictReader(f_obj2, delimiter=',')
    nn=0
    for line in reader:
        nn = nn+1

        if line["enable"] != 'Y':
            continue

        #print nn, line["group_name"], line["cpu"], line["enable"], line["folder"], line["version"], line["txt_file"]

        key = line["group_name"]+','+line["cpu"]+','+line["folder"]

        if not  dict_version.has_key(key):
            dict_version[key]  =  list()

        if line["version"] not in dict_version[key]:
            dict_version[key].append(line["version"])

        dict_txt_file[key+line["version"]] = line["txt_file"]

    #----------------------------------------------------------
    reader = csv.DictReader(f_obj, delimiter=',')
    nn=0
    for line in reader:
        nn = nn+1

        if line["enable"] != 'Y':
            continue

        print nn, line["firmware_group"], line["enable"], line["group_name"],line["cpu"], line["folder"], line["extended_image"], line["super_image"],line["firmware"] 

        key = line["group_name"]+','+line["cpu"]+','+line["folder"]
        #n:\shared\mi\Scripts_SI\aldrin\file_lists\

        csv_reader_b2(\
            CPSS_PATH,\
            WORK_DIR, \
            MICRO_INIT_CONF, \
            RESULT_DIR, \
            line["group_name"], \
            MI_SI_Speed_ParmCsv, \
            dict_si_speed, \
            key, \
            line["cpu"], \
            line["folder"], \
            dict_txt_file, \
            dict_version)

        if nn >= 1:
            break

#-------------------------------------------------------------------------------
# main module with import feature
#
def akaMain(argv):

    argc = len(argv)

    i=1
    CPSS_PATH                = sys.argv[i]; i=i+1
    WORK_DIR                 = sys.argv[i]; i=i+1
    MICRO_INIT_CONF          = sys.argv[i]; i=i+1
    RESULT_DIR               = sys.argv[i]; i=i+1
    Micro_Init_FWCsv         = sys.argv[i]; i=i+1
    MI_SI_Speed_ParmCsv      = sys.argv[i]; i=i+1
    SI_CPU_SpeedCsv          = sys.argv[i]; i=i+1

    print "CPSS_PATH          = ", CPSS_PATH
    print "WORK_DIR           = ", WORK_DIR
    print "MICRO_INIT_CONF    = ", MICRO_INIT_CONF
    print "Micro_Init_FWCsv   = ", Micro_Init_FWCsv  
    print "MI_SI_Speed_ParmCsv= ", MI_SI_Speed_ParmCsv
    print "SI_CPU_SpeedCsv    = ", SI_CPU_SpeedCsv   
    
    try:
        f_obj = open(Micro_Init_FWCsv, 'r')
    except IOError:
        print "Error: cannot open file: " + Micro_Init_FWCsv + "\n"
        sys.exit(2)

    try:
        f_obj2 = open(SI_CPU_SpeedCsv, 'r')
    except IOError:
        print "Error: cannot open file: " + SI_CPU_SpeedCsv + "\n"
        sys.exit(2)

    try:
        f_obj3 = open(MI_SI_Speed_ParmCsv, 'r')
    except IOError:
        print "Error: cannot open file: " + MI_SI_Speed_ParmCsv + "\n"
        sys.exit(2)

    csv_reader_b1(\
        CPSS_PATH,\
        WORK_DIR, \
        MICRO_INIT_CONF, \
        RESULT_DIR, \
        MI_SI_Speed_ParmCsv, \
        f_obj, \
        f_obj2, \
        f_obj3)

    f_obj.close()
    f_obj2.close()
    f_obj3.close()

    return

#-------------------------------------------------------------------------------
# main module stub to prevent auto execute
#

if __name__ == '__main__':
    akaMain(sys.argv)
    sys.exit(0)




