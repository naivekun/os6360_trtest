--
-- Basic NAT: Modifying only IP addresses
--

---------------- UDP ---------------------

local private_to_public_2_to_3_packet_udp = ""..
      "000102033402"..                      --mac da 00:01:02:03:34:02
      "000405060711"..                      --mac sa 00:04:05:06:07:11
      "81000005"..                          --vlan 5
      "86DD"..                              --ethertype
      "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
      "FD010000000000000000000000001234"..  --source ip FD01::1234
      "65450000000000000000000000005678"..  --destination ip 6545::5678
      "B3AAC3500048BE8F"..                  --UDP Header: srcPort=0xB3AA=45994 destPort=0xC350=50000
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

local private_to_public_2_to_3_packet_udp_secondNat66_1_config = ""..
      "000102033402"..                      --mac da 00:01:02:03:34:02
      "000405060722"..                      --mac sa 00:04:05:06:07:22
      "81000007"..                          --vlan 7
      "86DD"..                              --ethertype
      "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
      "FD012222333344445555000000006666"..  --source ip FD01:2222:3333:4444:5555::6666
      "65450000000000000000000000005678"..  --destination ip 6545::5678
      "B3AAC3500048BE8F"..                  --UDP Header: srcPort=0xB3AA=45994 destPort=0xC350=50000
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

local egress_private_to_public_2_to_3_packet_udp = ""..
      "000405060713"..                      --mac da 00:04:05:06:07:13
      "000102033402"..                      --mac sa 00:01:02:03:34:02
      "81000006"..                          --vlan 6
      "86DD"..                              --ethertype
      "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
      "20010000000000000000000000005678"..  --source ip 2001::5678
      "65450000000000000000000000005678"..  --destination ip 6545::5678
      "B3AAC3500048BE8F"..                  --UDP Header: srcPort=0xB3AA=45994 destPort=0xC350=50000
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

local public_to_private_3_to_2_packet_udp = ""..
      "000102033402"..                      --mac da 00:01:02:03:34:02
      "000405060713"..                      --mac sa 00:04:05:06:07:13
      "81000006"..                          --vlan 6
      "86DD"..                              --ethertype
      "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
      "65450000000000000000000000005678"..  --source ip 6545::5678
      "20010000000000000000000000005678"..  --destination ip 2001::5678
      "C7A7C35000484207"..                  --UDP Header: srcPort=0xC7A7=51111 destPort=0xC350=50000
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

local egress_public_to_private_3_to_2_packet_udp = ""..
      "000405060711"..                      --mac da 00:04:05:06:07:11
      "000102033402"..                      --mac sa 00:01:02:03:34:02
      "81000005"..                          --vlan 5
      "86DD"..                              --ethertype
      "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
      "65450000000000000000000000005678"..  --source ip 6545::5678
      "FD010000000000000000000000001234"..  --destination ip FD01::1234
      "C7A7C35000484207"..                  --UDP Header: srcPort=0xC7A7=51111 destPort=0xC350=50000
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

---------------- TCP ---------------------

local private_to_public_1_to_3_packet_tcp = ""..
      "000102033402"..                      --mac da 00:01:02:03:34:02
      "000405060712"..                      --mac sa 00:04:05:06:07:12
      "81000007"..                          --vlan 7
      "86DD"..                              --ethertype
      "6000000000500640"..                  --ipv6 Header: version=6, payloadLength=0x50, nextHeader=0x6, hopLimit=0x40
      "FD010000000000000000000000003456"..  --source ip FD01::3456
      "65450000000000000000000000005679"..  --destination ip 6545::5679
      "B3AAC3500000000000000000"..          --TCP Header: srcPort=0xB3AA=54994 destPort=0xC350=50000
      "50000000E9560000"..
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"

local egress_private_to_public_1_to_3_packet_tcp = ""..
      "000405060714"..                      --mac da 00:04:05:06:07:14
      "000102033402"..                      --mac sa 00:01:02:03:34:02
      "81000006"..                          --vlan 6
      "86DD"..                              --ethertype
      "6000000000500640"..                  --ipv6 Header: version=6, payloadLength=0x50, nextHeader=0x6, hopLimit=0x40
      "20010000000000000000000000005679"..  --source ip 2001::5679
      "65450000000000000000000000005679"..  --destination ip 6545::5679
      "B3AAC3500000000000000000"..          --TCP Header: srcPort=0xB3AA=54994 destPort=0xC350=50000
      "50000000E9560000"..
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"

local public_to_private_3_to_1_packet_tcp = ""..
      "000102033402"..                      --mac da 00:01:02:03:34:02
      "000405060714"..                      --mac sa 00:04:05:06:07:14
      "81000006"..                          --vlan 6
      "86DD"..                              --ethertype
      "6000000000500640"..                  --ipv6 Header: version=6, payloadLength=0x50, nextHeader=0x6, hopLimit=0x40
      "65450000000000000000000000005679"..  --source ip 6545::5679
      "20010000000000000000000000005679"..  --destination ip 2001::5679
      "C7A7C3500000000000000000"..          --TCP Header: srcPort=0xC7A7=51111 destPort=0xC350=50000
      "500000006CCE0000"..
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"

local egress_public_to_private_3_to_1_packet_tcp = ""..
      "000405060712"..                      --mac da 00:04:05:06:07:12
      "000102033402"..                      --mac sa 00:01:02:03:34:02
      "81000007"..                          --vlan 7
      "86DD"..                              --ethertype
      "6000000000500640"..                  --ipv6 Header: version=6, payloadLength=0x50, nextHeader=0x6, hopLimit=0x40
      "65450000000000000000000000005679"..  --source ip 6545::5678
      "FD010000000000000000000000003456"..  --destination ip FD01::3456
      "C7A7C3500000000000000000"..          --TCP Header: srcPort=0xC7A7=51111 destPort=0xC350=50000
      "500000006CCE0000"..
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"

---------------- IPv6 ---------------------

local public_to_public_2_to_1_packet_ipv6 = ""..
      "000102033402"..                      --mac da 00:01:02:03:34:02
      "000405060711"..                      --mac sa 00:04:05:06:07:11
      "81000005"..                          --vlan 5
      "86DD"..                              --ethertype
      "6000000000500040"..                  --ipv6 Header: version=6, payloadLength=0x50, nextHeader=0x0, hopLimit=0x40
      "20010000000000000000000000005678"..  --source ip 2001::5678
      "20010000000000000000000000005679"..  --destination ip 2001::5679
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F4041424344454647"..
      "48494A4B4C4D4E4F"

local egress_public_to_public_2_to_1_packet_ipv6 = ""..
      "000405060712"..                      --mac da 00:04:05:06:07:12
      "000102033402"..                      --mac sa 00:01:02:03:34:02
      "81000007"..                          --vlan 7
      "86DD"..                              --ethertype
      "6000000000500040"..                  --ipv6 Header: version=6, payloadLength=0x50, nextHeader=0x0, hopLimit=0x40
      "20010000000000000000000000005678"..  --source ip 2001::5678
      "fd010000000000000000000000003456"..  --destination ip FD01::3456
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F4041424344454647"..
      "48494A4B4C4D4E4F"

local public_to_public_1_to_2_packet_ipv6 = ""..
      "000102033402"..                      --mac da 00:01:02:03:34:02
      "000405060712"..                      --mac sa 00:04:05:06:07:12
      "81000006"..                          --vlan 6
      "86DD"..                              --ethertype
      "6000000000500040"..                  --ipv6 Header: version=6, payloadLength=0x50, nextHeader=0x0, hopLimit=0x40
      "20010000000000000000000000005679"..  --source ip 2001::5679
      "20010000000000000000000000005678"..  --destination ip 2001::5678
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F4041424344454647"..
      "48494A4B4C4D4E4F"

local egress_public_to_public_1_to_2_packet_ipv6 = ""..
      "000405060711"..                      --mac da 00:04:05:06:07:11
      "000102033402"..                      --mac sa 00:01:02:03:34:02
      "81000005"..                          --vlan 5
      "86DD"..                              --ethertype
      "6000000000500040"..                  --ipv6 Header: version=6, payloadLength=0x50, nextHeader=0x0, hopLimit=0x40
      "20010000000000000000000000005679"..  --source ip 2001::5679
      "fd010000000000000000000000001234"..  --destination ip FD01::3456
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F4041424344454647"..
      "48494A4B4C4D4E4F"


---------------- Prefix replacement ---------------------

-- local private_to_public = ""..
--       "000102033402"..                      --mac da 00:01:02:03:34:02
--       "000405060711"..                      --mac sa 00:04:05:06:07:11
--       "81000005"..                          --vlan 5
--       "86DD"..                              --ethertype
--       "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
--       "FD010000000000000000000000001234"..  --source ip FD01::1234
--       "65450000000000000000000000005678"..  --destination ip 6545::5678
--       "B3AAC3500048BE8F"..                  --UDP Header: srcPort=0xB3AA=45994 destPort=0xC350=50000
--       "000102030405060708090A0B"..          --payload
--       "0C0D0E0F1011121314151617"..
--       "18191A1B1C1D1E1F20212223"..
--       "2425262728292A2B2C2D2E2F"..
--       "303132333435363738393A3B"..
--       "3C3D3E3F"

local private_to_public_2_to_3_packet_56_dropped = ""..
      "000102033402"..                      --mac da 00:01:02:03:34:02
      "000405060711"..                      --mac sa 00:04:05:06:07:11
      "81000005"..                          --vlan 5
      "86DD"..                              --ethertype
      "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
      "FD01FFFFFFFFFFFFFFFFFFFFFFFFFFFF"..  --source ip FD01:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF
      "65450000000000000000000000005678"..  --destination ip 6545::5678
      "B3AAC3500048EB9F"..                  --UDP Header: srcPort=0xB3AA=45994 destPort=0xC350=50000
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"
local egress_private_to_public_packet_32 = ""..
      "000405060713"..                      --mac da 00:04:05:06:07:13
      "000102033402"..                      --mac sa 00:01:02:03:34:02
      "81000006"..                          --vlan 6
      "86DD"..                              --ethertype
      "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
      "200102030000DAFD0000000000001234"..  --source ip 20010203::1234, takes 32 bits from NAT ip address 2001:0203:0405:0607:0809::5678
      "65450000000000000000000000005678"..  --destination ip 6545::5678
      "B3AAC3500048BE8F"..                  --UDP Header: srcPort=0xB3AA=45994 destPort=0xC350=50000
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

local egress_private_to_public_packet_56 = ""..
      "000405060713"..                      --mac da 00:04:05:06:07:13
      "000102033402"..                      --mac sa 00:01:02:03:34:02
      "81000006"..                          --vlan 6
      "86DD"..                              --ethertype
      "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
      "2001020304050600D0F8000000001234"..  --source ip 20010203040506::1234, takes 56 bits from NAT ip address 2001:0203:0405:0607:0809::5678
      "65450000000000000000000000005678"..  --destination ip 6545::5678
      "B3AAC3500048BE8F"..                  --UDP Header: srcPort=0xB3AA=45994 destPort=0xC350=50000
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

local egress_private_to_public_packet_56_secondNat66_1_config = ""..
      "000405060713"..                      --mac da 00:04:05:06:07:13
      "000102033402"..                      --mac sa 00:01:02:03:34:02
      "81000006"..                          --vlan 6
      "86DD"..                              --ethertype
      "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
      "2001020304050644bfa3000000006666"..  --source ip 2001:0203:0405:0644::6666, takes 56 bits from NAT ip address 2001:0203:0405:0607:0809::5678
      "65450000000000000000000000005678"..  --destination ip 6545::5678
      "B3AAC3500048BE8F"..                  --UDP Header: srcPort=0xB3AA=45994 destPort=0xC350=50000
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

local egress_private_to_public_packet_64 = ""..
      "000405060713"..                      --mac da 00:04:05:06:07:13
      "000102033402"..                      --mac sa 00:01:02:03:34:02
      "81000006"..                          --vlan 6
      "86DD"..                              --ethertype
      "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
      "2001020304050607D0F1000000001234"..  --source ip 2001020304050607::1234, takes 64 bits from NAT ip address 2001:0203:0405:0607:0809::5678
      "65450000000000000000000000005678"..  --destination ip 6545::5678
      "B3AAC3500048BE8F"..                  --UDP Header: srcPort=0xB3AA=45994 destPort=0xC350=50000
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

-- local public_to_private = ""..
--       "000102033402"..                      --mac da 00:01:02:03:34:02
--       "000405060713"..                      --mac sa 00:04:05:06:07:13
--       "81000006"..                          --vlan 6
--       "86DD"..                              --ethertype
--       "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
--       "65450000000000000000000000005678"..  --source ip 6545::5678
--       "20010000000000000000000000005678"..  --destination ip 2001::5678
--       "C7A7C35000484207"..                  --UDP Header: srcPort=0xC7A7=51111 destPort=0xC350=50000
--       "000102030405060708090A0B"..          --payload
--       "0C0D0E0F1011121314151617"..
--       "18191A1B1C1D1E1F20212223"..
--       "2425262728292A2B2C2D2E2F"..
--       "303132333435363738393A3B"..
--       "3C3D3E3F"

local public_to_private_packet_32 = ""..
      "000102033402"..                      --mac da 00:01:02:03:34:02
      "000405060713"..                      --mac sa 00:04:05:06:07:13
      "81000006"..                          --vlan 6
      "86DD"..                              --ethertype
      "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
      "65450000000000000000000000005678"..  --source ip 6545::5678
      "20010203000000000000000000005678"..  --destination ip 2001:0203::5678
      "C7A7C35000484207"..                  --UDP Header: srcPort=0xC7A7=51111 destPort=0xC350=50000
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

local egress_public_to_private_packet_32 = ""..
      "000405060711"..                      --mac da 00:04:05:06:07:11
      "000102033402"..                      --mac sa 00:01:02:03:34:02
      "81000005"..                          --vlan 5
      "86DD"..                              --ethertype
      "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
      "65450000000000000000000000005678"..  --source ip 6545::5678
      "FD010203000022ff0000000000005678"..  --destination ip FD01:0203:0000:22FF::5678 , takes 32 bits from NAT ip address FD01:0203:0405:0607:0809::1234
      "C7A7C35000484207"..                  --UDP Header: srcPort=0xC7A7=51111 destPort=0xC350=50000
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

local public_to_private_packet_56 = ""..
      "000102033402"..                      --mac da 00:01:02:03:34:02
      "000405060713"..                      --mac sa 00:04:05:06:07:13
      "81000006"..                          --vlan 6
      "86DD"..                              --ethertype
      "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
      "65450000000000000000000000005678"..  --source ip 6545::5678
      "20010203040506000000000000005678"..  --destination ip 2001:0203:0405:0600::5678
      "C7A7C35000484207"..                  --UDP Header: srcPort=0xC7A7=51111 destPort=0xC350=50000
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

local egress_public_to_private_packet_56 = ""..
      "000405060711"..                      --mac da 00:04:05:06:07:11
      "000102033402"..                      --mac sa 00:01:02:03:34:02
      "81000005"..                          --vlan 5
      "86DD"..                              --ethertype
      "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
      "65450000000000000000000000005678"..  --source ip 6545::5678
      "fd0102030405060022ff000000005678"..  --destination ip fd01:0203:0405:0600:22ff::5678 , takes 56 bits from NAT ip address FD01:0203:0405:0607:0809::1234
      "C7A7C35000484207"..                  --UDP Header: srcPort=0xC7A7=51111 destPort=0xC350=50000
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

local public_to_private_packet_56_secondNat66_1_config = ""..
      "000102033402"..                      --mac da 00:01:02:03:34:02
      "000405060713"..                      --mac sa 00:04:05:06:07:13
      "81000006"..                          --vlan 6
      "86DD"..                              --ethertype
      "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
      "65450000000000000000000000005678"..  --source ip 6545::5678
      "20010203040506000000000000005679"..  --destination ip 2001:0203:0405:0600::5679
      "C7A7C35000484207"..                  --UDP Header: srcPort=0xC7A7=51111 destPort=0xC350=50000
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

local egress_public_to_private_packet_56_secondNat66_1_config = ""..
      "000405060722"..                      --mac da 00:04:05:06:07:11
      "000102033402"..                      --mac sa 00:01:02:03:34:02
      "81000007"..                          --vlan 7
      "86DD"..                              --ethertype
      "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
      "65450000000000000000000000005678"..  --source ip 6545::5678
      "fd0122223333440095b1000000005679"..  --destination ip fd01:2222:3333:4400:95b1::5678 , takes 56 bits from NAT ip address FD01:0203:0405:0607:0809::1234
      "C7A7C35000484207"..                  --UDP Header: srcPort=0xC7A7=51111 destPort=0xC350=50000
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

local public_to_private_packet_64 = ""..
      "000102033402"..                      --mac da 00:01:02:03:34:02
      "000405060713"..                      --mac sa 00:04:05:06:07:13
      "81000006"..                          --vlan 6
      "86DD"..                              --ethertype
      "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
      "65450000000000000000000000005678"..  --source ip 6545::5678
      "20010203040506070000000000005678"..  --destination ip 2001:0203:0405:0607::5678
      "C7A7C35000484207"..                  --UDP Header: srcPort=0xC7A7=51111 destPort=0xC350=50000
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

local egress_public_to_private_packet_64 = ""..
      "000405060711"..                      --mac da 00:04:05:06:07:11
      "000102033402"..                      --mac sa 00:01:02:03:34:02
      "81000005"..                          --vlan 5
      "86DD"..                              --ethertype
      "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
      "65450000000000000000000000005678"..  --source ip 6545::5678
      "fd0102030405060722ff000000005678"..  --destination ip fd01:0203:0405:0607:22ff::5678 , takes 64 bits from NAT ip address FD01:0203:0405:0607:0809::1234
      "C7A7C35000484207"..                  --UDP Header: srcPort=0xC7A7=51111 destPort=0xC350=50000
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

return  { private_to_public_2_to_3_udp = private_to_public_2_to_3_packet_udp, egress_private_to_public_2_to_3_udp = egress_private_to_public_2_to_3_packet_udp,
          private_to_public_2_to_3_udp_secondNat66_1_config = private_to_public_2_to_3_packet_udp_secondNat66_1_config,
          public_to_private_3_to_2_udp = public_to_private_3_to_2_packet_udp, egress_public_to_private_3_to_2_udp = egress_public_to_private_3_to_2_packet_udp,
          private_to_public_1_to_3_tcp = private_to_public_1_to_3_packet_tcp, egress_private_to_public_1_to_3_tcp = egress_private_to_public_1_to_3_packet_tcp,
          public_to_private_3_to_1_tcp = public_to_private_3_to_1_packet_tcp, egress_public_to_private_3_to_1_tcp = egress_public_to_private_3_to_1_packet_tcp,
          public_to_public_2_to_1_ipv6 = public_to_public_2_to_1_packet_ipv6, egress_public_to_public_2_to_1_ipv6 = egress_public_to_public_2_to_1_packet_ipv6,
          public_to_public_1_to_2_ipv6 = public_to_public_1_to_2_packet_ipv6, egress_public_to_public_1_to_2_ipv6 = egress_public_to_public_1_to_2_packet_ipv6,
          egress_private_to_public_32  = egress_private_to_public_packet_32,
          public_to_private_32 = public_to_private_packet_32,egress_public_to_private_32 = egress_public_to_private_packet_32,
          egress_private_to_public_56  = egress_private_to_public_packet_56,
          egress_private_to_public_56_secondNat66_1_config  = egress_private_to_public_packet_56_secondNat66_1_config,
          public_to_private_56  = public_to_private_packet_56,egress_public_to_private_56  = egress_public_to_private_packet_56,
          public_to_private_56_secondNat66_1_config  = public_to_private_packet_56_secondNat66_1_config,egress_public_to_private_56_secondNat66_1_config  = egress_public_to_private_packet_56_secondNat66_1_config,
          egress_private_to_public_64  = egress_private_to_public_packet_64,
          public_to_private_64  = public_to_private_packet_64,egress_public_to_private_64  = egress_public_to_private_packet_64,
          private_to_public_2_to_3_56_dropped = private_to_public_2_to_3_packet_56_dropped}
