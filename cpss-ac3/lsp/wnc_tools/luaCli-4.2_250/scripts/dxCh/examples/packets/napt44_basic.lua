--
-- Basic NAPT: Modifying IP addresses and L4 ports
-- 

    --            PRIVATE                                          PUBLIC
    --
    --                         VLAN 5  |-----------------|
    --                       2---------|                 |
    --                                 |                 |
    --                       4---------|       NAT       |
    --                                 |      ROUTER     |   VLAN 6
    --                                 |                 |X-----------3
    --                         VLAN 7  |            (PUBLIC IP)   (SERVER IP)
    --                       1---------|                 |
    --                    (PRIVATE IP) |-----------------|
    --

-- Global Router configuration
--          00:01:02:03:04:05

-- PORT 2: PRIVATE
--         00:04:05:06:07:11
--         vlan 5
--         192.168.1.1
--         udpPort 45994

-- PORT 4: PRIVATE
--         00:04:05:06:07:12
--         vlan 5
--         192.168.1.2

-- PORT 1: PRIVATE
--         00:04:05:06:07:13
--         vlan 7
--         192.168.2.3
--         udpPort 45995

-- PORT 3: SERVER
--         00:04:05:06:07:14
--         vlan 6
--         198.51.100.1
--         udpPort 51111

-- PORT X: PUBLIC
--          17.18.19.20
--         udpPort1 50000
--         udpPort2 50001

---------------- UDP ---------------------
--[[
0000  00 01 02 03 04 05 00 04 05 06 07 11 81 00 00 05
0010  08 00 45 00 00 5C 00 00 00 00 40 11 8E B3 C0 A8
0020  01 01 C6 33 64 01 B3 AA C7 A7 00 48 B4 29 00 01
0030  02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11
0040  12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21
0050  22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31
0060  32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F
 ]]--
--sending private port 2--> to public port 3
local private_to_public_packet_udp_2_to_3 = ""..
        "000102030405"..             --mac da 00:01:02:03:04:05
        "000405060711"..             --mac sa 00:04:05:06:07:11
        "81000005"..                 --vlan 5
        "0800"..                     --ethertype
        "4500005C0000000040118EB3".. --ipv4 Header: timetoleave=0x40=64, protocol=0x11
        "C0A80101"..                 --source ip 192.168.1.1
        "C6336401"..                 --destination ip 198.51.100.1
        "B3AAC7A70048B429"..         --UDP Header: srcPort=0xB3AA=45994 destPort=0xC7A7=51111
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F"

--[[ the 'egress packet'
0X0000 :  00 04 05 06 07 14 00 01 02 03 04 05 81 00 00 06
0X0010 :  08 00 45 00 00 5C 00 00 00 00 40 11 2C 37 11 12
0X0020 :  13 14 C6 33 64 01 C3 50 C7 A7 00 48 42 07 00 01
0X0030 :  02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11
0X0040 :  12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21
0X0050 :  22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31
0X0060 :  32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F
--]]
local egress_private_to_public_packet_udp_2_to_3 = ""..
      "000405060714"..             --mac da 00:04:05:06:07:14
      "000102030405"..             --mac sa 00:01:02:03:04:05
      "81000006"..                 --vlan 6
      "0800"..                     --ethertype
      "4500005C0000000040112C37".. --ipv4 Header: timetoleave=0x40=64, protocol=0x11
      "11121314"..                 --source ip 17.18.19.20
      "C6336401"..                 --destination ip 198.51.100.1
      "C350C7A700484207"..         --UDP Header: srcPort=0xC350=50000 destPort=0xC7A7=51111
      "000102030405060708090A0B".. --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

--[[
0000  00 01 02 03 04 05 00 04 05 06 07 14 81 00 00 06
0010  08 00 45 00 00 5C 00 00 00 00 40 11 2C 37 C6 33
0020  64 01 11 12 13 14 C7 A7 C3 50 00 48 42 07 00 01
0030  02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11
0040  12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21
0050  22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31
0060  32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F
--]]
--sending public port 3--> to private port 2
local public_to_private_packet_udp_3_to_2 = ""..
        "000102030405"..             --mac da 00:01:02:03:04:05
        "000405060714"..             --mac sa 00:04:05:06:07:14
        "81000006"..                 --vlan 6
        "0800"..                     --ethertype
        "4500005C0000000040112C37".. --ipv4 Header: timetoleave=0x40=64, protocol=0x11
        "C6336401"..                 --source ip 198.51.100.1
        "11121314"..                 --destination ip 17.18.19.20
        "C7A7C35000484207"..         --UDP Header: srcPort=0xC7A7=51111 dstPort=0xC350=50000
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F"

--[[ the 'egress packet'
0X0000 :  00 04 05 06 07 11 00 01 02 03 04 05 81 00 00 05
0X0010 :  08 00 45 00 00 5C 00 00 00 00 40 11 8E B3 C6 33
0X0020 :  64 01 C0 A8 01 01 C7 A7 B3 AA 00 48 B4 29 00 01
0X0030 :  02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11
0X0040 :  12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21
0X0050 :  22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31
0X0060 :  32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F
--]]
local egress_public_to_private_packet_udp_3_to_2 = ""..
        "000405060711"..             --mac da 00:04:05:06:07:11
        "000102030405"..             --mac sa 00:01:02:03:04:05
        "81000005"..                 --vlan 5
        "0800"..                     --ethertype
        "4500005C0000000040118EB3".. --ipv4 Header: timetoleave=0x40=64, protocol=0x11
        "C6336401"..                 --source ip 198.51.100.1
        "C0A80101"..                 --destination ip 192.168.1.1
        "C7A7B3AA0048B429"..         --UDP Header: srcPort=0xC7A7=51111 destPort=0xB3AA=45994
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F"

--[[
0000  00 01 02 03 04 05 00 04 05 06 07 13 81 00 00 07
0010  08 00 45 00 00 5C 00 00 00 00 40 11 8D B1 C0 A8
0020  02 03 C6 33 64 01 B3 AB C7 A7 00 48 B3 26 00 01
0030  02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11
0040  12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21
0050  22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31
0060  32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F
 ]]--
--sending private port 1--> to public port 3
local private_to_public_packet_udp_1_to_3 = ""..
        "000102030405"..             --mac da 00:01:02:03:04:05
        "000405060713"..             --mac sa 00:04:05:06:07:13
        "81000007"..                 --vlan 7
        "0800"..                     --ethertype
        "4500005C0000000040118DB1".. --ipv4 Header: timetoleave=0x40=64, protocol=0x11
        "C0A80203"..                 --source ip 192.168.2.3
        "C6336401"..                 --destination ip 198.51.100.1
        "B3ABC7A70048B326"..         --UDP Header: srcPort=0xB3AB=45995 destPort=0xC7A7=51111
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F"

--[[ the 'egress packet'
0X0000 :  00 04 05 06 07 14 00 01 02 03 04 05 81 00 00 06
0X0010 :  08 00 45 00 00 5C 00 00 00 00 40 11 2C 37 11 12
0X0020 :  13 14 C6 33 64 01 C3 51 C7 A7 00 48 42 06 00 01
0X0030 :  02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11
0X0040 :  12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21
0X0050 :  22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31
0X0060 :  32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F
--]]
local egress_private_to_public_packet_udp_1_to_3 = ""..
        "000405060714"..             --mac da 00:04:05:06:07:14
        "000102030405"..             --mac sa 00:01:02:03:04:05
        "81000006"..                 --vlan 6
        "0800"..                     --ethertype
        "4500005C0000000040112C37".. --ipv4 Header: timetoleave=0x40=64, protocol=0x11
        "11121314"..                 --source ip 17.18.19.20
        "C6336401"..                 --destination ip 198.51.100.1
        "C351C7A700484206"..         --UDP Header: srcPort=0xC351=50001 destPort=0xC7A7=51111
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F"
--[[
0000  00 01 02 03 04 05 00 04 05 06 07 14 81 00 00 06
0010  08 00 45 00 00 5C 00 00 00 00 40 11 2C 37 C6 33
0020  64 01 11 12 13 14 C7 A7 C3 51 00 48 42 06 00 01
0030  02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11
0040  12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21
0050  22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31
0060  32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F
--]]
--sending public port 3--> to private port 1
local public_to_private_packet_udp_3_to_1 = ""..
        "000102030405"..             --mac da 00:01:02:03:04:05
        "000405060714"..             --mac sa 00:04:05:06:07:14
        "81000006"..                 --vlan 6
        "0800"..                     --ethertype
        "4500005C0000000040112C37".. --ipv4 Header: timetoleave=0x40=64, protocol=0x11
        "C6336401"..                 --source ip 198.51.100.1
        "11121314"..                 --destination ip 17.18.19.20
        "C7A7C35100484206"..         --UDP Header: srcPort=0xC7A7=51111 destPort=0xC351=50001
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F"

--[[ the 'egress packet'
0X0000 :  00 04 05 06 07 13 00 01 02 03 04 05 81 00 00 07
0X0010 :  08 00 45 00 00 5C 00 00 00 00 40 11 8D B1 C6 33
0X0020 :  64 01 C0 A8 02 03 C7 A7 B3 AB 00 48 B3 26 00 01
0X0030 :  02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11
0X0040 :  12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21
0X0050 :  22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31
0X0060 :  32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F
--]]
local egress_public_to_private_packet_udp_3_to_1 = ""..
        "000405060713"..             --mac da 00:04:05:06:07:13
        "000102030405"..             --mac sa 00:01:02:03:04:05
        "81000007"..                 --vlan 7
        "0800"..                     --ethertype
        "4500005C0000000040118DB1".. --ipv4 Header: timetoleave=0x40=64, protocol=0x11
        "C6336401"..                 --source ip 198.51.100.1
        "C0A80203"..                 --destination ip 192.168.2.3
        "C7A7B3AB0048B326"..         --UDP Header: srcPort=0xC7A7=51111 destPort=0xB3AB=45995
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F"

---------------- TCP ---------------------
--[[
0000  00 01 02 03 04 05 00 04 05 06 07 11 81 00 00 05
0010  08 00 45 00 00 68 00 00 00 00 40 06 8E B2 C0 A8
0020  01 01 C6 33 64 01 B3 AA C7 A7 00 00 00 00 00 00
0030  00 00 50 00 00 00 64 70 00 00 00 01 02 03 04 05
0040  06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15
0050  16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25
0060  26 27 28 29 2A 2B 2C 2D 2E 2F 30 31 32 33 34 35
0070  36 37 38 39 3A 3B 3C 3D 3E 3F
--]]
--sending private port 2--> to public port 3
local private_to_public_packet_tcp_2_to_3 = ""..
        "000102030405"..             --mac da 00:01:02:03:04:05
        "000405060711"..             --mac sa 00:04:05:06:07:11
        "81000005"..                 --vlan 5
        "0800"..                     --ethertype
        "450000680000000040068EB2".. --ipv4 Header: timetoleave=0x40=64, protocol=0x6
        "C0A80101"..                 --source ip 192.168.1.1
        "C6336401"..                 --destination ip 198.51.100.1
        "B3AAC7A70000000000000000".. --TCP Header: srcPort=0xB3AA=45994 destPort=0xC7A7=51111
        "5000000064700000"..
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F"

--[[ the 'egress packet'
0X0000 :  00 04 05 06 07 14 00 01 02 03 04 05 81 00 00 06
0X0010 :  08 00 45 00 00 68 00 00 00 00 40 06 2C 36 11 12
0X0020 :  13 14 C6 33 64 01 C3 50 C7 A7 00 00 00 00 00 00
0X0030 :  00 00 50 00 00 00 F2 4D 00 00 00 01 02 03 04 05
0X0040 :  06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15
0X0050 :  16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25
0X0060 :  26 27 28 29 2A 2B 2C 2D 2E 2F 30 31 32 33 34 35
0X0070 :  36 37 38 39 3A 3B 3C 3D 3E 3F
--]]

local egress_private_to_public_packet_tcp_2_to_3 = ""..
        "000405060714"..             --mac da 00:04:05:06:07:14
        "000102030405"..             --mac sa 00:01:02:03:04:05
        "81000006"..                 --vlan 6
        "0800"..                     --ethertype
        "450000680000000040062C36".. --ipv4 Header: timetoleave=0x40=64, protocol=0x6
        "11121314"..                 --source ip 17.18.19.20
        "C6336401"..                 --destination ip 198.51.100.1
        "C350C7A70000000000000000".. --TCP Header: srcPort=0xC350=50000 destPort=0xC7A7=51111
        "50000000F24D0000"..
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F"

--[[
0000  00 01 02 03 04 05 00 04 05 06 07 14 81 00 00 06
0010  08 00 45 00 00 68 00 00 00 00 40 06 2C 36 C6 33
0020  64 01 11 12 13 14 C7 A7 C3 50 00 00 00 00 00 00
0030  00 00 50 00 00 00 F2 4D 00 00 00 01 02 03 04 05
0040  06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15
0050  16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25
0060  26 27 28 29 2A 2B 2C 2D 2E 2F 30 31 32 33 34 35
0070  36 37 38 39 3A 3B 3C 3D 3E 3F
--]]
--sending public port 3--> to private port 2
local public_to_private_packet_tcp_3_to_2 = ""..
        "000102030405"..             --mac da 00:01:02:03:04:05
        "000405060714"..             --mac sa 00:04:05:06:07:14
        "81000006"..                 --vlan 6
        "0800"..                     --ethertype
        "450000680000000040062C36".. --ipv4 Header: timetoleave=0x40=64, protocol=0x6
        "C6336401"..                 --source ip 198.51.100.1
        "11121314"..                 --destination ip 17.18.19.20
        "C7A7C3500000000000000000".. --TCP Header: srcPort=0xC7A7=51111 dstPort=0xC350=50000
        "50000000F24D0000"..
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F"

--[[ the 'egress packet'
0X0000 :  00 04 05 06 07 11 00 01 02 03 04 05 81 00 00 05
0X0010 :  08 00 45 00 00 68 00 00 00 00 40 06 8E B2 C6 33
0X0020 :  64 01 C0 A8 01 01 C7 A7 B3 AA 00 00 00 00 00 00
0X0030 :  00 00 50 00 00 00 64 70 00 00 00 01 02 03 04 05
0X0040 :  06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15
0X0050 :  16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25
0X0060 :  26 27 28 29 2A 2B 2C 2D 2E 2F 30 31 32 33 34 35
0X0070 :  36 37 38 39 3A 3B 3C 3D 3E 3F
--]]

local egress_public_to_private_packet_tcp_3_to_2 = ""..
        "000405060711"..             --mac da 00:04:05:06:07:11
        "000102030405"..             --mac sa 00:01:02:03:04:05
        "81000005"..                 --vlan 5
        "0800"..                     --ethertype
        "450000680000000040068EB2".. --ipv4 Header: timetoleave=0x40=64, protocol=0x6
        "C6336401"..                 --source ip 198.51.100.1
        "C0A80101"..                 --destination ip 192.168.1.1
        "C7A7B3AA0000000000000000".. --TCP Header: srcPort=0xC7A7=51111 destPort=0xB3AA=45994
        "5000000064700000"..
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F"

---------------- IPv4 ---------------------
--[[
0000  00 04 05 06 07 12 00 04 05 06 07 11 81 00 00 05
0010  08 00 45 00 00 54 00 00 00 00 40 00 F7 56 C0 A8
0020  01 01 C0 A8 01 02 00 01 02 03 04 05 06 07 08 09
0030  0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19
0040  1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29
0050  2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39
0060  3A 3B 3C 3D 3E 3F
 ]]--
--sending private port 2--> to private port 4
local private_to_private_packet_2_to_4 = ""..
        "000405060712"..             --mac da 00:04:05:06:07:12
        "000405060711"..             --mac sa 00:04:05:06:07:11
        "81000005"..                 --vlan 5
        "0800"..                     --ethertype
        "45000054000000004000F756".. --ipv4 Header: timetoleave=0x40=64, protocol=0
        "C0A80101"..                 --source ip 192.168.1.1
        "C0A80102"..                 --destination ip 192.168.1.2
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F"

--[[ the 'egress packet'
0000  00 04 05 06 07 12 00 04 05 06 07 11 81 00 00 05
0010  08 00 45 00 00 54 00 00 00 00 40 00 F7 56 C0 A8
0020  01 01 C0 A8 01 02 00 01 02 03 04 05 06 07 08 09
0030  0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19
0040  1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29
0050  2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39
0060  3A 3B 3C 3D 3E 3F
--]]
local egress_private_to_private_packet_2_to_4 = ""..
        "000405060712"..             --mac da 00:04:05:06:07:12
        "000405060711"..             --mac sa 00:04:05:06:07:11
        "81000005"..                 --vlan 5
        "0800"..                     --ethertype
        "45000054000000004000F756".. --ipv4 Header: timetoleave=0x40=64, protocol=0
        "C0A80101"..                 --source ip 192.168.1.1
        "C0A80102"..                 --destination ip 192.168.1.2
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F"

--[[
0000  00 04 05 06 07 11 00 04 05 06 07 12 81 00 00 05
0010  08 00 45 00 00 54 00 00 00 00 40 00 F7 56 C0 A8
0020  01 01 C0 A8 01 02 00 01 02 03 04 05 06 07 08 09
0030  0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19
0040  1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29
0050  2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39
0060  3A 3B 3C 3D 3E 3F
--]]
--sending private port 4--> to private port 2
local private_to_private_packet_4_to_2 = ""..
        "000405060711"..             --mac da 00:04:05:06:07:11
        "000405060712"..             --mac sa 00:04:05:06:07:12
        "81000005"..                 --vlan 5
        "0800"..                     --ethertype
        "45000054000000004000F756".. --ipv4 Header: timetoleave=0x40=64, protocol=0
        "C0A80101"..                 --source ip 192.168.1.1
        "C0A80102"..                 --destination ip 192.168.1.2
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F"

--[[ the 'egress packet'
0000  00 04 05 06 07 11 00 04 05 06 07 12 81 00 00 05
0010  08 00 45 00 00 54 00 00 00 00 40 00 F7 56 C0 A8
0020  01 01 C0 A8 01 02 00 01 02 03 04 05 06 07 08 09
0030  0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19
0040  1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29
0050  2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39
0060  3A 3B 3C 3D 3E 3F
--]]

local egress_private_to_private_packet_4_to_2 = ""..
        "000405060711"..             --mac da 00:04:05:06:07:11
        "000405060712"..             --mac sa 00:04:05:06:07:12
        "81000005"..                 --vlan 5
        "0800"..                     --ethertype
        "45000054000000004000F756".. --ipv4 Header: timetoleave=0x40=64, protocol=0
        "C0A80101"..                 --source ip 192.168.1.1
        "C0A80102"..                 --destination ip 192.168.1.2
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F"

--[[
0000  00 01 02 03 04 05 00 04 05 06 07 11 81 00 00 05
0010  08 00 45 00 00 54 00 00 00 00 40 00 F6 55 C0 A8
0020  01 01 C0 A8 02 03 00 01 02 03 04 05 06 07 08 09
0030  0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19
0040  1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29
0050  2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39
0060  3A 3B 3C 3D 3E 3F
 ]]--
--sending private port 2--> to private port 1
local private_to_private_packet_2_to_1 = ""..
        "000102030405"..             --mac da 00:01:02:03:04:05
        "000405060711"..             --mac sa 00:04:05:06:07:11
        "81000005"..                 --vlan 5
        "0800"..                     --ethertype
        "45000054000000004000F655".. --ipv4 Header: timetoleave=0x40=64, protocol=0
        "C0A80101"..                 --source ip 192.168.1.1
        "C0A80203"..                 --destination ip 192.168.2.3
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F"

--[[ the 'egress packet'
0X0000 :  00 04 05 06 07 13 00 01 02 03 04 05 81 00 00 07
0X0010 :  08 00 45 00 00 54 00 00 00 00 40 00 F6 55 C0 A8
0X0020 :  01 01 C0 A8 02 03 00 01 02 03 04 05 06 07 08 09
0X0030 :  0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19
0X0040 :  1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29
0X0050 :  2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39
0X0060 :  3A 3B 3C 3D 3E 3F
--]]
local egress_private_to_private_packet_2_to_1 = ""..
        "000405060713"..             --mac da 00:04:05:06:07:13
        "000102030405"..             --mac sa 00:01:02:03:04:05
        "81000007"..                 --vlan 7
        "0800"..                     --ethertype
        "45000054000000004000F655".. --ipv4 Header: timetoleave=0x40=64, protocol=0x0
        "C0A80101"..                 --source ip 192.168.1.1
        "C0A80203"..                 --destination ip 192.168.2.3
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F"

--[[
0000  00 01 02 03 04 05 00 04 05 06 07 13 81 00 00 07
0010  08 00 45 00 00 54 00 00 00 00 40 00 F6 55 C0 A8
0020  02 03 C0 A8 01 01 00 01 02 03 04 05 06 07 08 09
0030  0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19
0040  1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29
0050  2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39
0060  3A 3B 3C 3D 3E 3F
--]]
--sending private port 1--> to private port 2
local private_to_private_packet_1_to_2 = ""..
        "000102030405"..             --mac da 00:01:02:03:04:05
        "000405060713"..             --mac sa 00:04:05:06:07:13
        "81000007"..                 --vlan 7
        "0800"..                     --ethertype
        "45000054000000004000F655".. --ipv4 Header: timetoleave=0x40=64, protocol=0
        "C0A80203"..                 --source ip 192.168.2.3
        "C0A80101"..                 --destination ip 192.168.1.1
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F"

--[[ the 'egress packet'
0X0000 :  00 00 00 00 00 12 00 04 05 06 07 12 81 00 00 05
0X0010 :  08 00 45 00 00 54 00 00 00 00 40 00 F6 55 C0 A8
0X0020 :  02 03 C0 A8 01 01 00 01 02 03 04 05 06 07 08 09
0X0030 :  0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19
0X0040 :  1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29
0X0050 :  2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39
0X0060 :  3A 3B 3C 3D 3E 3F
--]]

local egress_private_to_private_packet_1_to_2 = ""..
        "000405060711"..             --mac da 00:00:00:00:00:12
        "000102030405"..             --mac sa 00:01:02:03:04:05
        "81000005"..                 --vlan 5
        "0800"..                     --ethertype
        "45000054000000004000F655".. --ipv4 Header: timetoleave=0x40=64, protocol=0x0
        "C0A80203"..                 --source ip 192.168.2.3
        "C0A80101"..                 --destination ip 192.168.1.1
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F"

return  { --UDP
          private_to_public_udp_2_to_3 = private_to_public_packet_udp_2_to_3, egress_private_to_public_udp_2_to_3 = egress_private_to_public_packet_udp_2_to_3,
          public_to_private_udp_3_to_2 = public_to_private_packet_udp_3_to_2, egress_public_to_private_udp_3_to_2 = egress_public_to_private_packet_udp_3_to_2,
          private_to_public_udp_1_to_3 = private_to_public_packet_udp_1_to_3, egress_private_to_public_udp_1_to_3 = egress_private_to_public_packet_udp_1_to_3,
          public_to_private_udp_3_to_1 = public_to_private_packet_udp_3_to_1, egress_public_to_private_udp_3_to_1 = egress_public_to_private_packet_udp_3_to_1,
          --TCP
          private_to_public_tcp_2_to_3 = private_to_public_packet_tcp_2_to_3, egress_private_to_public_tcp_2_to_3 = egress_private_to_public_packet_tcp_2_to_3,
          public_to_private_tcp_3_to_2 = public_to_private_packet_tcp_3_to_2, egress_public_to_private_tcp_3_to_2 = egress_public_to_private_packet_tcp_3_to_2,
          --IPV4
          private_to_private_2_to_4 = private_to_private_packet_2_to_4, egress_private_to_private_2_to_4 = egress_private_to_private_packet_2_to_4,
          private_to_private_4_to_2 = private_to_private_packet_4_to_2, egress_private_to_private_4_to_2 = egress_private_to_private_packet_4_to_2,
          private_to_private_2_to_1 = private_to_private_packet_2_to_1, egress_private_to_private_2_to_1 = egress_private_to_private_packet_2_to_1,
          private_to_private_1_to_2 = private_to_private_packet_1_to_2, egress_private_to_private_1_to_2 = egress_private_to_private_packet_1_to_2}

