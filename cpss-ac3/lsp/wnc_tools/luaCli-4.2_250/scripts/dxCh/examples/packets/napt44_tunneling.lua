--
-- NAPT+Tunneling:
-- For traffic from private to public the packet is tunneled after the NAT modification => ePort-out is associated with MPLS TS
-- For traffic from public to private the packet is TT before the NAT modification => ePort-in is assigned by the TTI (MPLS)
--

---------------- UDP ---------------------
--[[
0000  00 01 02 03 34 02 00 04 05 06 07 11 81 00 00 05
0010  08 00 45 00 00 5C 00 00 00 00 40 11 94 C2 C0 A8
0020  01 01 11 12 13 14 B3 AA C7 A7 00 48 BA 38 00 01
0030  02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11
0040  12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21
0050  22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31
0060  32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F
 ]]--
local private_to_public_packet_udp = ""..
      "000102033402"..             --mac da 00:01:02:03:34:02
      "000405060711"..             --mac sa 00:04:05:06:07:11
      "81000005"..                 --vlan 5
      "0800"..                     --ethertype
      "4500005C00000000401194C2".. --ipv4 Header: timetoleave=0x40=64, protocol=0x11
      "C0A80101"..                 --source ip 192.168.1.1
      "11121314"..                 --destination ip 17.18.19.20
      "B3AAC7A70048BA38"..         --UDP Header: srcPort=0xB3AA=45994 destPort=0xC7A7=51111
      "000102030405060708090A0B".. --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

--[[ the 'egress packet'
0X0000 :  11 22 33 44 55 66 00 01 02 03 34 02 81 00 00 07
0X0010 :  88 47 00 33 31 40 00 04 05 06 07 12 00 01 02 03
0X0020 :  34 02 81 00 00 06 08 00 45 00 00 5C 00 00 00 00
0X0030 :  40 11 2C 37 C6 33 64 01 11 12 13 14 C3 50 C7 A7
0X0040 :  00 48 42 07 00 01 02 03 04 05 06 07 08 09 0A 0B
0X0050 :  0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B
0X0060 :  1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29 2A 2B
0X0070 :  2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39 3A 3B
0X0080 :  3C 3D 3E 3F
--]]
local egress_private_to_public_packet_udp = ""..
      "112233445566"..             --mac da 11:22:33:44:55:66
      "000102033402"..             --mac sa 00:01:02:03:34:02
      "81000007"..                 --vlan 7
      "8847"..                     --ethertype
      "00333140"..                 --MPLS Header: label1=0x333 timetoleave=0x40=64
      "000405060712"..             --mac da 00:04:05:06:07:12
      "000102033402"..             --mac sa 00:01:02:03:34:02
      "81000006"..                 --vlan 6
      "0800"..                     --ethertype
      "4500005C0000000040112C37".. --ipv4 Header: timetoleave=0x40=64, protocol=0x11
      "C6336401"..                 --source ip 198.51.100.1
      "11121314"..                 --destination ip 17.18.19.20
      "C350C7A700484207"..         --UDP Header: srcPort=0xC350=50000 destPort=0xC7A7=51111
      "000102030405060708090A0B".. --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"
--[[
0000  00 01 02 03 34 02 11 22 33 44 55 66 81 00 00 07
0010  88 47 00 33 31 40 00 01 02 03 34 02 00 04 05 06
0020  07 12 81 00 00 06 08 00 45 00 00 5C 00 00 00 00
0030  40 11 2C 37 11 12 13 14 C6 33 64 01 C7 A7 C3 50
0040  00 48 42 07 00 01 02 03 04 05 06 07 08 09 0A 0B
0050  0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B
0060  1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29 2A 2B
0070  2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39 3A 3B
0080  3C 3D 3E 3F
-- ]]
local public_to_private_packet_udp = ""..
        "002122232425"..             --mac da 00:21:22:23:24:25
        "112233445566"..             --mac sa 11:22:33:44:55:66
        "81000007"..                 --vlan 7
        "8847"..                     --ethertype
        "00333140"..                 --MPLS Header: label1=0x333 timetoleave=0x40=64
        "000102033402"..             --mac da 00:01:02:03:34:02
        "000405060712"..             --mac sa 00:04:05:06:07:12
        "81000006"..                 --vlan 6
        "0800"..                     --ethertype
        "4500005C0000000040112C37".. --ipv4 Header: timetoleave=0x40=64, protocol=0x11
        "11121314"..                 --source ip 17.18.19.20
        "C6336401"..                 --destination ip 198.51.100.1
        "C7A7C35000484207"..         --UDP Header: srcPort=0xC7A7=51111 destPort=0xC350=50000
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3FB2B334EC"

--[[ the 'egress packet'
0X0000 :  00 04 05 06 07 11 00 01 02 03 34 02 81 00 00 05
0X0010 :  08 00 45 00 00 5C 00 00 00 00 40 11 94 C2 11 12
0X0020 :  13 14 C0 A8 01 01 C7 A7 B3 AA 00 48 BA 38 00 01
0X0030 :  02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11
0X0040 :  12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21
0X0050 :  22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31
0X0060 :  32 33 34 35 36 37 38 39 3A 3B
--]]

local egress_public_to_private_packet_udp = ""..
        "000405060711"..             --mac da 00:04:05:06:07:11
        "000102033402"..             --mac sa 00:01:02:03:34:02
        "81000005"..                 --vlan 5
        "0800"..                     --ethertype
        "4500005C00000000401194C2".. --ipv4 Header: timetoleave=0x40=64, protocol=0x11
        "11121314"..                 --source ip 17.18.19.20
        "C0A80101"..                 --destination ip 192.168.1.1
        "C7A7B3AA0048BA38"..         --UDP Header: srcPort=0xC7A7=51111 destPort=0xB3AA=45994
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F"

---------------- TCP ---------------------

--[[
0000  00 01 02 03 34 02 00 04 05 06 07 11 81 00 00 05
0010  08 00 45 00 00 64 00 00 00 00 40 06 94 C5 C0 A8
0020  01 01 11 12 13 14 B3 AA C7 A7 00 00 00 00 00 00
0030  00 00 50 00 00 00 E4 FF 00 00 00 01 02 03 04 05
0040  06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15
0050  16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25
0060  26 27 28 29 2A 2B 2C 2D 2E 2F 30 31 32 33 34 35
0070  36 37 38 39 3A 3B
--]]

local private_to_public_packet_tcp = ""..
        "000102033402"..             --mac da 00:01:02:03:34:02
        "000405060711"..             --mac sa 00:04:05:06:07:11
        "81000005"..                 --vlan 5
        "0800"..                     --ethertype
        "4500006400000000400694C5".. --ipv4 Header: timetoleave=0x40=64, protocol=0x6
        "C0A80101"..                 --source ip 192.168.1.1
        "11121314"..                 --destination ip 17.18.19.20
        "B3AAC7A70000000000000000".. --TCP Header: srcPort=0xB3AA=54994 destPort=0xC350=50000
        "50000000E4FF0000"..
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"


--[[ the 'egress packet'
0X0000 :  11 22 33 44 55 66 00 01 02 03 34 02 81 00 00 07
0X0010 :  88 47 00 33 31 40 00 04 05 06 07 12 00 01 02 03
0X0020 :  34 02 81 00 00 06 08 00 45 00 00 64 00 00 00 00
0X0030 :  40 06 2C 3A C6 33 64 01 11 12 13 14 C3 50 C7 A7
0X0040 :  00 00 00 00 00 00 00 00 50 00 00 00 6C CE 00 00
0X0050 :  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
0X0060 :  10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F
0X0070 :  20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F
0X0080 :  30 31 32 33 34 35 36 37 38 39 3A 3B
--]]

local egress_private_to_public_packet_tcp = ""..
        "112233445566"..             --mac da 11:22:33:44:55:66
        "000102033402"..             --mac sa 00:01:02:03:34:02
        "81000007"..                 --vlan 7
        "8847"..                     --ethertype
        "00333140"..                 --MPLS Header: label1=0x333 timetoleave=0x40=64
        "000405060712"..             --mac da 00:04:05:06:07:12
        "000102033402"..             --mac sa 00:01:02:03:34:02
        "81000006"..                 --vlan 6
        "0800"..                     --ethertype
        "450000640000000040062C3A".. --ipv4 Header: timetoleave=0x40=64, protocol=0x6
        "C6336401"..                 --source ip 198.51.100.1
        "11121314"..                 --destination ip 17.18.19.20
        "C350C7A70000000000000000".. --TCP Header: srcPort=0xC350=50000 destPort=0xC7A7=51111
        "500000006CCE0000"..
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"

--[[
0000  00 01 02 03 34 02 11 22 33 44 55 66 81 00 00 07
0010  88 47 00 33 31 40 00 01 02 03 34 02 00 04 05 06
0020  07 12 81 00 00 06 08 00 45 00 00 64 00 00 00 00
0030  40 06 2C 3A 11 12 13 14 C6 33 64 01 C7 A7 C3 50
0040  00 00 00 00 00 00 00 00 50 00 00 00 6C CE 00 00
0050  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
0060  10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F
0070  20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F
0080  30 31 32 33 34 35 36 37 38 39 3A 3B 4B 14 D8 CB
--]]

local public_to_private_packet_tcp = ""..
        "002122232425"..             --mac da 00:21:22:23:24:25
        "112233445566"..             --mac sa 11:22:33:44:55:66
        "81000007"..                 --vlan 7
        "8847"..                     --ethertype
        "00333140"..                 --MPLS Header: label1=0x333 timetoleave=0x40=64
        "000102033402"..             --mac da 00:01:02:03:34:02
        "000405060712"..             --mac sa 00:04:05:06:07:12
        "81000006"..                 --vlan 6
        "0800"..                     --ethertype
        "450000640000000040062C3A".. --ipv4 Header: timetoleave=0x40=64, protocol=0x6
        "11121314"..                 --source ip 17.18.19.20
        "C6336401"..                 --destination ip 198.51.100.1
        "C7A7C3500000000000000000".. --TCP Header: srcPort=0xC7A7=51111 destPort=0xC350=50000
        "500000006CCE0000"..
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "4B14D8CB"


--[[ the 'egress packet'
0X0000 :  00 04 05 06 07 11 00 01 02 03 34 02 81 00 00 05
0X0010 :  08 00 45 00 00 64 00 00 00 00 40 06 94 C5 11 12
0X0020 :  13 14 C0 A8 01 01 C7 A7 B3 AA 00 00 00 00 00 00
0X0030 :  00 00 50 00 00 00 E4 FF 00 00 00 01 02 03 04 05
0X0040 :  06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15
0X0050 :  16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25
0X0060 :  26 27 28 29 2A 2B 2C 2D 2E 2F 30 31 32 33 34 35
0X0070 :  36 37 38 39 3A 3B
--]]

local egress_public_to_private_packet_tcp = ""..
        "000405060711"..             --mac da 00:04:05:06:07:11
        "000102033402"..             --mac sa 00:01:02:03:34:02
        "81000005"..                 --vlan 5
        "0800"..                     --ethertype
        "4500006400000000400694C5".. --ipv4 Header: timetoleave=0x40=64, protocol=0x6
        "11121314"..                 --source ip 17.18.19.20
        "C0A80101"..                 --destination ip 192.168.1.1
        "C7A7B3AA0000000000000000".. --TCP Header: srcPort=0xC7A7=51111 destPort=0xB3AA=45994
        "50000000E4FF0000"..
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"

return  { private_to_public_udp = private_to_public_packet_udp, egress_private_to_public_udp = egress_private_to_public_packet_udp,
          public_to_private_udp = public_to_private_packet_udp, egress_public_to_private_udp = egress_public_to_private_packet_udp,
          private_to_public_tcp = private_to_public_packet_tcp, egress_private_to_public_tcp = egress_private_to_public_packet_tcp,
          public_to_private_tcp = public_to_private_packet_tcp, egress_public_to_private_tcp = egress_public_to_private_packet_tcp}

