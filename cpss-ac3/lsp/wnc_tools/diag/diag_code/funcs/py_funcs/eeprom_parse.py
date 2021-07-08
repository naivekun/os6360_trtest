"""
Usage: python eeprom_parse.py <eeprom_cfg> <eeprom_bin> [debug]

Input:
      <eeprom_cfg>: it's a file name. 
                    The content format is as following:
                    [Target Field]
                    offset = <hex>
                    length = <int> (length must be larger than 0)

                    Example :
                    [Manufactures Name]
                    offset = 0x1b
                    length = 9

                    [Product Name]
                    offset = 0x25
                    length = 12

      <eeprom_bin>: it's a file name.
                    The content is eeprom data.

      <debug>     : it is optional and show all information for debug
"""

import sys
#import configparser   #for python 3
import ConfigParser

debug_mode = False

eeprom_cfg = sys.argv[1]
eeprom_bin = sys.argv[2]
if len(sys.argv) == 4 and sys.argv[3] == "debug" :
    debug_mode = True

config = ConfigParser.ConfigParser()
config.read(eeprom_cfg)

# dump entire config file
if debug_mode == True:
    for section in config.sections():
        print section
        for option in config.options(section):
            print " ", option, "=", config.get(section, option)

# read eeprom bin file
fp = open(eeprom_bin, "rb")
hex_list = ["{:02x}".format(ord(c)) for c in fp.read()]

# dump entire eeprom content
if debug_mode == True:
    print(hex_list)

# parser
def read_eeprom_content (offset, length):
    temp=(hex_list[(offset):(offset+length)])
    return temp

for section in config.sections():
    offset = int(config.get(section, "offset"), 16)
    length = int(config.get(section, "length"), 0)
    if length > 0:
        result = read_eeprom_content(offset, length)
        str1 = ''.join(str(e) for e in result)
        #print (section + ":" + str1.decode("hex"))
        print "{:<25} {:<1} {:<30}".format(section, ":", str1.decode("hex"))
    else:
        print "{:<25} {:<1} {:<30}".format(section, ":","!! Error Length !!")