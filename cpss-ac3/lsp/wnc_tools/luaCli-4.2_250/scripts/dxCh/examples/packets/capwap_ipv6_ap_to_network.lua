--
-- Capwap 2 packet
--

--[[
[TGF]: tgfTrafficTracePacket: packet trace, length [134](CRC not included)
0000  88 77 11 11 55 66 98 32 21 19 18 06 81 00 00 C8
0010  86 DD 60 00 00 00 00 4C 11 40 65 45 00 00 00 00
0020  00 00 00 00 00 00 00 00 56 78 11 22 00 00 00 00
0030  00 00 00 00 00 00 CC DD 12 34 FA D4 14 7F 00 60
0040  40 93 00 10 02 00 00 00 00 00 00 01 02 03 34 02
0050  00 04 05 06 07 11 81 00 00 0A 00 01 02 03 04 05
0060  06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15
0070  16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25
0080  26 27 28 29 2A 2B
]]--

local capwap_2_ipv6_direction1_packet = ""..
        "887711115566"..             --mac da 88:77:11:11:55:66
        "983221191806"..             --mac sa 98:32:21:19:18:06
        "810000c8"..                 --vlan 200
        "86dd"..                     --ethernet type 0x86dd
        "60000000004C1140"..         --ipv6Header: version=6, payloadLength=0x4C, nextHeader=0x11, hopLimit=0x40
        "65450000000000000000000000005678".. --source ip 6545::5678
        "112200000000000000000000CCDD1234".. --destination ip 1122::CCDD:1234
        "fad4147f00604093"..         -- srcPort=0xfad4=64212 destPort=0x147f=5247 totalLength=0x60, checksum=0x4093
        "0010020000000000"..         -- capwap_preamle=0 HLEN=2 WBID=1
        "000102033402"..             -- mac da 00:01:02:03:34:02
        "000405060711"..             -- mac sa 00:04:06:06:07:11
        "8100000a"..                 -- vlan 10
        "000102030405060708090a0b0c"..--payload
        "0d0e0f10111213141516171819"..--payload
        "1a1b1c1d1e1f20212223242526"..--payload
        "2728292a2b"                 --payload


--[[
0x0000 :  00 01 02 03 34 02 00 04 05 06 07 11 81 00 00 0a
0x0010 :  00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
0x0020 :  10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f
0x0030 :  20 21 22 23 24 25 26 27 28 29 2a 2b
-- ]]
local capwap_2_ipv6_direction1_packet_expected = ""..
        "000102033402"..             -- mac da 00:01:02:03:34:02
        "000405060711"..             -- mac sa 00:04:06:06:07:11
        "8100000a"..                 -- vlan 10
        "000102030405060708090a0b0c"..--payload
        "0d0e0f10111213141516171819"..--payload
        "1a1b1c1d1e1f20212223242526"..--payload
        "2728292a2b"                  --payload

--[[
[TGF]: tgfTrafficTracePacket: packet trace, length [134](CRC not included)
0000  88 77 11 11 55 66 98 32 21 19 18 06 81 00 00 64
0010  08 00 60 00 00 00 00 4C 11 40 65 45 00 00 00 00
0020  00 00 00 00 00 00 00 00 56 78 11 22 00 00 00 00
0030  00 00 00 00 00 00 CC DD 12 34 FA D4 14 7F 00 60
0040  40 93 00 10 02 00 00 00 00 00 00 01 02 03 34 02
0050  00 04 05 06 07 11 81 00 00 0A 00 01 02 03 04 05
0060  06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15
0070  16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25
0080  26 27 28 29 2A 2B
]]--

local capwap_2_ipv6_direction1_defaultPacket = ""..
        "887711115566"..             --mac da 88:77:11:11:55:66
        "983221191806"..             --mac sa 98:32:21:19:18:06
        "81000064"..                 --vlan 100 -- expected 200 for hit --> will go to default TTI --> DROP
        "86dd"..                     --ethernet type 0x86dd
        "60000000004C1140"..         --ipv6Header: version=6, payloadLength=0x4C, nextHeader=0x11, hopLimit=0x40
        "65450000000000000000000000005678".. --source ip 6545::5678
        "112200000000000000000000CCDD1234".. --destination ip 1122::CCDD:1234
        "fad4147f00604093"..         -- srcPort=0xfad4=64212 destPort=0x147f=5247 totalLength=0x60, checksum=0x4093
        "0010020000000000"..         -- capwap_preamle=0 HLEN=2 WBID=1
        "000102033402"..             -- mac da 00:01:02:03:34:02
        "000405060711"..             -- mac sa 00:04:06:06:07:11
        "8100000a"..                 -- vlan 10
        "000102030405060708090a0b0c"..--payload
        "0d0e0f10111213141516171819"..--payload
        "1a1b1c1d1e1f20212223242526"..--payload
        "2728292a2b"                  --payload


--[[
        --vlan 100 -- expected 200 for hit --> will go to default TTI --> DROP
-- ]]
local capwap_2_ipv6_direction1_packet_defaultExpected = ""

local capwap_2_ipv6_direction1_controlPacket = ""..
        "887711115566"..             --mac da 88:77:11:11:55:66
        "983221191806"..             --mac sa 98:32:21:19:18:06
        "810000c8"..                  --vlan 200
        "86dd"..                     --ethernet type 0x86dd
        "60000000004C1140"..         --ipv6Header: version=6, payloadLength=0x4C, nextHeader=0x11, hopLimit=0x40
        "65450000000000000000000000005678".. --source ip 6545::5678
        "112200000000000000000000CCDD1234".. --destination ip 1122::CCDD:1234
        "fad4147e00604093"..         -- srcPort=0xfad4=64212 destPort=0x147e=5246 totalLength=0x60, checksum=0x4093
        "0010020000000000"..         -- capwap_preamle=0 HLEN=2 WBID=1
        "000102033402"..             -- mac da 00:01:02:03:34:02
        "000405060711"..             -- mac sa 00:04:06:06:07:11
        "8100000a"..                 -- vlan 10
        "000102030405060708090a0b0c"..--payload
        "0d0e0f10111213141516171819"..--payload
        "1a1b1c1d1e1f20212223242526"..--payload
        "2728292a2b"                  --payload


--[[
        -destPort=0x147e=5246 --> will go to control TTI rule --> trap to CPU
-- ]]
local capwap_2_ipv6_direction1_packet_controlExpected = ""


--[[
0000  00 04 05 06 07 11 00 00 00 00 00 99 81 00 00 0A
0010  86 DD 60 00 00 00 00 06 3B 40 65 45 00 00 00 00
0020  00 00 00 00 00 00 00 00 67 89 11 22 00 00 00 00
0030  00 00 00 00 00 00 CC DD 12 34 00 01 02 03 04 05
 --]]
local capwap_2_ipv6_direction2_packet = ""..
        "000405060711"..                --mac da
        "000000000099"..                --mac sa
        "8100000a"..                    --vlan 10
        "86dd"..                        --ethernet type 0x86dd
        "6000000000063B40"..            --ipv6Header: version=6, payloadLength=0x6, nextHeader=0x3b, hopLimit=0x40
        "65450000000000000000000000006789"..--source ip 6545::6789
        "112200000000000000000000CCDD1234"..--destination ip 1122::CCDD:1234
        "000102030405" --payload

--[[
0x0000 :  98 32 21 19 18 06 88 77 11 11 55 66 81 00 00 c8
0x0010 :  86 dd 60 00 00 00 00 48 11 40 11 22 00 00 00 00
0x0020 :  00 00 00 00 00 00 cc dd 12 34 65 45 00 00 00 00
0x0030 :  00 00 00 00 00 00 00 00 56 78 00 08 14 7f 00 48
0x0040 :  00 00 00 04 05 06 07 11 00 00 00 00 00 99 81 00
0x0050 :  00 0a 86 dd 60 00 00 00 00 06 3b 40 65 45 00 00
0x0060 :  00 00 00 00 00 00 00 00 00 00 67 89 11 22 00 00
0x0070 :  00 00 00 00 00 00 00 00 cc dd 12 34 00 01 02 03
0x0080 :  04 05
 ]]

local capwap_2_ipv6_direction2_packet_expected = ""..
        "983221191806"..             --mac da 98:32:21:19:18:06
        "887711115566"..             --mac sa 88:77:11:11:55:66
        "810000c8"..                 --vlan 200
        "86dd"..                     --ethernet type 0x86dd
        "6000000000481122"..            --ipv6Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x22
        "112200000000000000000000ccdd1234"..--source ip  1122::CCDD:1234
        "65450000000000000000000000005678"..--destination 6545::5678
        "0008147f00480000"..         -- srcPort=0x0008 destPort=0x147f=5247 totalLength=0x48, checksum=0x0000
        "000405060711"..             -- mac da 00:04:06:06:07:11
        "000000000099"..             -- mac sa 00:00:00:00:00:99
        "8100000a"..                 -- vlan 10
        "86dd"..                     --ethernet type 0x86dd
        "6000000000063b40"..         --ipv6Header: version=6, payloadLength=0x6, nextHeader=0x3b, hopLimit=0x40
        "65450000000000000000000000006789"..--source ip 6545::6789
        "112200000000000000000000ccdd1234"..--destination ip 1122::CCDD:1234
        "000102030405"                      --payload

return  { direction1=capwap_2_ipv6_direction1_packet,
        direction1_default=capwap_2_ipv6_direction1_defaultPacket,
        direction1_control=capwap_2_ipv6_direction1_controlPacket,
        direction2=capwap_2_ipv6_direction2_packet,
        direction1_expected=capwap_2_ipv6_direction1_packet_expected,
        direction1_defaultExpected=capwap_2_ipv6_direction1_packet_defaultExpected,
        direction1_controlExpected=capwap_2_ipv6_direction1_packet_controlExpected,
        direction2_expected=capwap_2_ipv6_direction2_packet_expected}


