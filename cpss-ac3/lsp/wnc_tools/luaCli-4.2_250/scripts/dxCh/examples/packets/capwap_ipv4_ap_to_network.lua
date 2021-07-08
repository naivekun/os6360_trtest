--
-- Capwap 2 packet
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

local capwap_2_ipv4_direction1_packet = ""..
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
"3435363738393a3b3c3d3e36" --payload

--[[
0x0000 :  00 01 02 03 34 02 00 04 05 06 07 11 81 00 00 0a
0x0010 :  00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
0x0020 :  10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f
0x0030 :  20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f
0x0040 :  30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 36
-- ]]
local capwap_2_ipv4_direction1_packet_expected = ""..
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
0000  88 77 11 11 55 66 98 32 21 19 18 06 81 00 00 64
0010  08 00 45 00 00 74 00 00 00 00 21 11 89 66 05 06
0020  07 08 01 02 03 04 FA D4 14 7F 00 60 40 93 00 10
0030  02 00 00 00 00 00 00 01 02 03 34 02 00 04 05 06
0040  07 11 81 00 00 0A 00 01 02 03 04 05 06 07 08 09
0050  0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19
0060  1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29
0070  2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39
0080  3A 3B 3C 3D 3E 3F
]]--

local capwap_2_ipv4_direction1_defaultPacket = ""..
        "887711115566"..             --mac da 88:77:11:11:55:66
        "983221191806"..             --mac sa 98:32:21:19:18:06
        "81000064"..                 --vlan 100 -- expected 200 for hit --> will go to default TTI --> DROP
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
        "3435363738393a3b3c3d3e36" --payload

--[[
        --vlan 100 -- expected 200 for hit --> will go to default TTI --> DROP
-- ]]
local capwap_2_ipv4_direction1_packet_defaultExpected = ""


local capwap_2_ipv4_direction1_controlPacket = ""..
        "887711115566"..             --mac da 88:77:11:11:55:66
        "983221191806"..             --mac sa 98:32:21:19:18:06
        "810000c8"..                 --vlan 200
        "0800"..                     --ethernet type 0x800
        "450000740000000021118966".. --ipv4Header: version=4, headerLength=5, timetoleave=0x21=33, protocol=0x11
        "05060708"..                 --source ip 5.6.7.8
        "01020304"..                 --destination ip 1.2.3.4
        "fad4147e00604093"..         -- srcPort=0xfad4=64212 destPort=0x147e=5246 totalLength=0x60, checksum=0x4093
        "0010020000000000"..         -- capwap_preamle=0 HLEN=2 WBID=1
        "000102033402"..             -- mac da 00:01:02:03:34:02
        "000405060711"..             -- mac sa 00:04:06:06:07:11
        "8100000a"..                 -- vlan 10
        "000102030405060708090a0b0c"..--payload
        "0d0e0f10111213141516171819"..--payload
        "1a1b1c1d1e1f20212223242526"..--payload
        "2728292a2b2c2d2e2f30313233"..--payload
        "3435363738393a3b3c3d3e36" --payload

--[[
        -destPort=0x147e=5246 --> will go to control TTI rule --> trap to CPU
-- ]]
local capwap_2_ipv4_direction1_packet_controlExpected = ""

--[[
0000  00 04 05 06 07 11 00 00 00 00 00 09 81 00 00 0A
0010  08 00 45 00 00 2A 00 00 00 00 40 04 72 C7 03 03
0020  03 03 01 01 01 03 00 01 02 03 04 05 06 07 08 09
0030  0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19
 --]]
local capwap_2_ipv4_direction2_packet = ""..
    "000405060711"..                --mac da
    "000000000099"..                --mac sa
    "8100000a"..                    --vlan 10
    "0800"..                        --ethernet type 0x800
    "4500002e00000000400472c7"..    --ipv4 header: protocol=4, ttl=0x40
    "09101112"..                    --ipv4 src=9.10.11.12
    "01020304"..                    --ipv4 dst=1.2.3.4
    "000102030405060708090A0B0C0D0E0F10111213141516171819" --payload

--[[
0x0000 :  98 32 21 19 18 06 88 77 11 11 55 66 81 00 00 c8
0x0010 :  08 00 45 00 00 5c 00 00 00 00 40 11 6a 7e 01 02
0x0020 :  03 04 05 06 07 08 00 08 14 7f 00 48 00 00 00 04
0x0030 :  05 06 07 11 00 00 00 00 00 99 81 00 00 0a 08 00
0x0040 :  45 00 00 2e 00 00 00 00 40 04 72 c7 09 10 11 12
0x0050 :  01 02 03 04 00 01 02 03 04 05 06 07 08 09 0a 0b
0x0060 :  0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19
 ]]
local capwap_2_ipv4_direction2_packet_expected = ""..
        "983221191806"..             --mac da 98:32:21:19:18:06
        "887711115566"..             --mac sa 88:77:11:11:55:66
        "810000c8"..                 --vlan 200
        "0800"..                     --ethernet type 0x800
        "4500005c0000000022116a7e".. --ipv4Header: version=4, headerLength=5, timetoleave=0x22, protocol=0x11
        "01020304"..                 --source ip 1.2.3.4
        "05060708"..                 --destination ip 5.6.7.8
        "0008147f00480000"..         -- srcPort=0x0008 destPort=0x147f=5247 totalLength=0x48, checksum=0x0000
        "000405060711"..             -- mac da 00:04:06:06:07:11
        "000000000099"..             -- mac sa 00:00:00:00:00:99
        "8100000a"..                 -- vlan 10
        "0800"..                     --ethernet type 0x800
        "4500002e00000000400472c7"..    --ipv4 header: protocol=4, ttl=0x40
        "09101112"..                    --ipv4 src=9.10.11.12
        "01020304"..                    --ipv4 dst=1.2.3.4
        "000102030405060708090a0b0c0d0e0f10111213141516171819" --payload

return  { direction1=capwap_2_ipv4_direction1_packet,
          direction1_default=capwap_2_ipv4_direction1_defaultPacket,
          direction1_control=capwap_2_ipv4_direction1_controlPacket,
          direction2=capwap_2_ipv4_direction2_packet,
          direction1_expected=capwap_2_ipv4_direction1_packet_expected,
          direction1_defaultExpected=capwap_2_ipv4_direction1_packet_defaultExpected,
          direction1_controlExpected=capwap_2_ipv4_direction1_packet_controlExpected,
          direction2_expected=capwap_2_ipv4_direction2_packet_expected}


