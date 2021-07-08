--
-- Capwap packet
-- 

--[[
[TGF]: tgfTrafficTracePacket: packet trace, length [134](CRC not included)
0000  88 77 11 11 55 66 98 32 21 19 18 06 81 00 00 C8
0010  08 00 45 00 00 74 00 00 00 00 21 11 89 66 05 06
0020  07 08 01 02 03 04 FA D4 14 7F 00 60 40 93 00 10
0030  02 00 00 00 00 00 00 01 02 03 34 02 00 04 05 06
0040  07 11 81 00 00 0A 00 01 02 03 04 05 06 07 08 09
0050  0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19
0060  1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29
0070  2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39
0080  3A 3B 3C 3D 3E 3F
]]--

local capwap_ipv4_direction1_packet = ""..
        "887711115566"..             --mac da 88:77:11:11:55:66
        "983221191806"..             --mac sa 98:32:21:19:18:06
        "810000c8"..                 --vlan 200
        "0800"..                     --ethernet type 0x800
        "450000740000000021118966".. --ipv4Header: version=4, headerLength=5, timetoleave=0x21=33, protocol=0x11
        "05060708"..                 --source ip 5.6.7.8
        "01020304"..                 --destination ip 1.2.3.4
        "fad4147f00604093"..         -- srcPort=0xfad4=64212 destPort=0x147f=5247 totalLength=0x60, checksum=0x4093
        "0010020000000000"..         -- capwap_preamle=0 HLEN=2 WBID=1
        "000102033402"..             -- mac da 00:01:02:03:34:02
        "000405060711"..             -- mac sa 00:04:06:06:07:11
        "8100000a"..                 -- vlan 10
        "000102030405060708090a0b0c"..--payload
        "0d0e0f10111213141516171819"..--payload
        "1a1b1c1d1e1f20212223242526"..--payload
        "2728292a2b2c2d2e2f30313233"..--payload
        "3435363738393a3b3c3d3e36"    --payload


--[[ the 'egress packet' that result from sending 'capwap_ipv4_direction1_packet'
0x0000 :  98 32 21 19 18 07 88 77 11 11 55 67 81 00 00 c8
0x0010 :  08 00 45 00 00 6c 00 00 00 00 21 11 81 66 01 02
0x0020 :  03 04 09 0a 0b 0c 00 13 14 7f 00 58 00 00 00 01
0x0030 :  02 03 34 02 00 04 05 06 07 11 81 00 00 0a 00 01
0x0040 :  02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11
0x0050 :  12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f 20 21
0x0060 :  22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f 30 31
0x0070 :  32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 36
--]]
local egress_capwap_ipv4_direction1_packet = ""..
        "983221191807"..             --mac da 98:32:21:19:18:07 --> received from TS configuration of eport 636, ap-mac-addr 98:32:21:19:18:07
        "887711115567"..             --mac sa 88:77:11:11:55:67 --> received from TS configuration of eport 636, ac-mac-addr 88:77:11:11:55:67
        "810000c8"..                 --vlan 200
        "0800"..                     --ethernet type 0x800
        "4500006c0000000022118166".. --ipv4Header: version=4, headerLength=0x6c, timetoleave=0x22=34, protocol=0x11
        "01020304"..                 --source ip 1.2.3.4
        "090a0b0c"..                 --destination ip 9.10.11.12
        "0013147f00580000"..         -- srcPort=0x0013 destPort=0x147f=5247 totalLength=0x58, checksum=0x0000
        "000102033402"..             -- mac da 00:01:02:03:34:02
        "000405060711"..             -- mac sa 00:04:06:06:07:11
        "8100000a"..                 -- vlan 10
        "000102030405060708090a0b0c"..--payload
        "0d0e0f10111213141516171819"..--payload
        "1a1b1c1d1e1f20212223242526"..--payload
        "2728292a2b2c2d2e2f30313233"..--payload
        "3435363738393a3b3c3d3e36"    --payload


--[[
[TGF]: tgfTrafficTracePacket: packet trace, length [134](CRC not included)
0000  88 77 11 11 55 66 98 32 21 19 18 06 81 00 00 c8
0010  08 00 45 00 00 74 00 00 00 00 21 11 89 66 05 06
0020  07 08 01 02 03 04 FA D4 14 7E 00 60 40 93 00 10
0030  02 00 00 00 00 00 00 01 02 03 34 02 00 04 05 06
0040  07 11 81 00 00 0A 00 01 02 03 04 05 06 07 08 09
0050  0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19
0060  1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29
0070  2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39
0080  3A 3B 3C 3D 3E 3F
]]--

local capwap_ipv4_direction1_controlPacket = ""..
        "887711115566"..             --mac da 88:77:11:11:55:66
        "983221191806"..             --mac sa 98:32:21:19:18:06
        "810000c8"..                 --vlan 200
        "0800"..                     --ethernet type 0x800
        "450000740000000021118966".. --ipv4Header: version=4, headerLength=5, timetoleave=0x21=33, protocol=0x11
        "05060708"..                 --source ip 5.6.7.8
        "01020304"..                 --destination ip 1.2.3.4
        "fad4147e00604093"..         -- srcPort=0xfad4=64212 totalLength=0x60, checksum=0x4093
                                     -- destPort=0x147e=5246 (this will cause a catch on the control tunnel rule)
        "0010020000000000"..         -- capwap_preamle=0 HLEN=2 WBID=1
        "000102033402"..             -- mac da 00:01:02:03:34:02
        "000405060711"..             -- mac sa 00:04:06:06:07:11
        "8100000a"..                 -- vlan 10
        "000102030405060708090a0b0c"..--payload
        "0d0e0f10111213141516171819"..--payload
        "1a1b1c1d1e1f20212223242526"..--payload
        "2728292a2b2c2d2e2f30313233"..--payload
        "3435363738393a3b3c3d3e36"    --payload

--[[
        -destPort=0x147e=5246 --> will go to control TTI rule --> trap to CPU
-- ]]
local egress_capwap_ipv4_direction1_controlPacket = ""


local capwap_ipv4_direction1_defaultPacket = ""..
        "887711115566"..             --mac da 88:77:11:11:55:66
        "983221191806"..             --mac sa 98:32:21:19:18:06
        "81000064"..                 --vlan 100 -- expected 200 for hit --> will go to default TTI --> DROP
        "0800"..                     --ethernet type 0x800
        "450000740000000021118966".. --ipv4Header: version=4, headerLength=5, timetoleave=0x21=33, protocol=0x11
        "05060708"..                 --source ip 5.6.7.8
        "01020304"..                 --destination ip 1.2.3.4
        "fad4147f00604093"..         -- srcPort=0xfad4=64212 totalLength=0x60, checksum=0x4093 destPort=0x147f=5247
        "0010020000000000"..         -- capwap_preamle=0 HLEN=2 WBID=1
        "000102033402"..             -- mac da 00:01:02:03:34:02
        "000405060711"..             -- mac sa 00:04:06:06:07:11
        "8100000a"..                 -- vlan 10
        "000102030405060708090a0b0c"..--payload
        "0d0e0f10111213141516171819"..--payload
        "1a1b1c1d1e1f20212223242526"..--payload
        "2728292a2b2c2d2e2f30313233"..--payload
        "3435363738393a3b3c3d3e36"    --payload

--[[
        --vlan 100 -- expected 200 for hit --> will go to default TTI --> DROP
-- ]]
local egress_capwap_ipv4_direction1_defaultPacket = ""

local capwap_ipv4_direction2_packet = ""..
        "887711115567"..             --mac da 88:77:11:11:55:67 --> received from TS configuration of eport 636, ac-mac-addr 88:77:11:11:55:67
        "983221191807"..             --mac sa 98:32:21:19:18:07 --> received from TS configuration of eport 636, ap-mac-addr 98:32:21:19:18:07
        "810000c8"..                 --vlan 200
        "0800"..                     --ethernet type 0x800
        "4500006c0000000021118166".. --ipv4Header: version=4, headerLength=0x6c, timetoleave=0x21=33, protocol=0x11
        "090A0B0C"..                 --source ip 9.10.11.12
        "01020304"..                 --destination ip 1.2.3.4
        "0013147f00580000"..         -- srcPort=0x0013 destPort=0x147f=5247 totalLength=0x58, checksum=0x0000
        "0010020000000000"..         -- capwap_preamle=0 HLEN=2 WBID=1
        "000405060711"..             -- mac da 00:04:06:06:07:11
        "000102033402"..             -- mac sa 00:01:02:03:34:02
        "8100000a"..                 -- vlan 10
        "000102030405060708090a0b0c"..--payload
        "0d0e0f10111213141516171819"..--payload
        "1a1b1c1d1e1f20212223242526"..--payload
        "2728292a2b2c2d2e2f30313233"..--payload
        "3435363738393a3b3c3d3e36"    --payload



--[[ the 'egress packet' that result from sending 'capwap_ipv4_direction2_packet'
0x0000 :  98 32 21 19 18 06 88 77 11 11 55 66 81 00 00 c8
0x0010 :  08 00 45 00 00 6c 00 01 00 00 21 11 89 6d 01 02
0x0020 :  03 04 05 06 07 08 00 13 14 7f 00 58 00 00 00 04
0x0030 :  05 06 07 11 00 01 02 03 34 02 81 00 00 0a 00 01
0x0040 :  02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11
0x0050 :  12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f 20 21
0x0060 :  22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f 30 31
0x0070 :  32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 36
--]]
local egress_capwap_ipv4_direction2_packet = ""..
        "983221191806"..             --mac da 98:32:21:19:18:06 --> received from TS configuration of eport 618, ap-mac-addr 98:32:21:19:18:06
        "887711115566"..             --mac sa 88:77:11:11:55:66 --> received from TS configuration of eport 618, ac-mac-addr 88:77:11:11:55:66
        "810000c8"..                 --vlan 200
        "0800"..                     --ethernet type 0x800
        "4500006c000000002211896d".. --ipv4Header: version=4, headerLength=0x6c, timetoleave=0x22=34, protocol=0x11
        "01020304"..                 --source ip 1.2.3.4
        "05060708"..                 --destination ip 5.6.7.8
        "0013147f00580000"..         -- srcPort=0x0013 destPort=0x147f=5247 totalLength=0x58, checksum=0x0000
        "000405060711"..             -- mac da 00:04:06:06:07:11
        "000102033402"..             -- mac sa 00:01:02:03:34:02
        "8100000a"..                 -- vlan 10
        "000102030405060708090a0b0c"..--payload
        "0d0e0f10111213141516171819"..--payload
        "1a1b1c1d1e1f20212223242526"..--payload
        "2728292a2b2c2d2e2f30313233"..--payload
        "3435363738393a3b3c3d3e36"    --payload


return  { direction1=capwap_ipv4_direction1_packet, direction1_expected = egress_capwap_ipv4_direction1_packet ,
          direction1_default=capwap_ipv4_direction1_defaultPacket, direction1_default_expected = egress_capwap_ipv4_direction1_defaultPacket,
          direction1_control=capwap_ipv4_direction1_controlPacket, direction1_control_expected = egress_capwap_ipv4_direction1_controlPacket,
          direction2=capwap_ipv4_direction2_packet, direction2_expected = egress_capwap_ipv4_direction2_packet}
