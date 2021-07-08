--
-- NAT+Tunneling:
-- For traffic from private to public the packet is tunneled after the NAT modification => ePort-out is associated with MPLS TS
-- For traffic from public to private the packet is TT before the NAT modification => ePort-in is assigned by the TTI (MPLS)
--

---------------- IPV6 ---------------------
local private_to_public_packet = ""..
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

local egress_private_to_public_packet = ""..
      "112233445566"..                      --mac da 11:22:33:44:55:66
      "000102033402"..                      --mac sa 00:01:02:03:34:02
      "81000007"..                          --vlan 7
      "8847"..                              --ethertype
      "00333140"..                          --MPLS Header: label1=0x333 timetoleave=0x40=64
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

local public_to_private_packet = ""..
      "002122232425"..                      --mac da 00:21:22:23:24:25
      "112233445566"..                      --mac sa 11:22:33:44:55:66
      "81000007"..                          --vlan 7
      "8847"..                              --ethertype
      "00333140"..                          --MPLS Header: label1=0x333 timetoleave=0x40=64
      "000102033402"..                      --mac da 00:01:02:03:34:02
      "000405060713"..                      --mac sa 00:04:05:06:07:13
      "81000006"..                          --vlan 6
      "86DD"..                              --ethertype
      "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
      "65450000000000000000000000005678"..  --source ip 6545::5678
      "20010000000000000000000000005678"..  --destination ip 2001::5678
      "B3AAC3500048BE8F"..                  --UDP Header: srcPort=0xB3AA=45994 destPort=0xC350=50000
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F00000000"                    -- 4 last bytes "00000000" are CRC for the passenger, since Tunnel Termination type
                                            -- is "Passenger type is Ethernet with crc" (arrived from MPLS command)

local egress_public_to_private_packet = ""..
      "000405060711"..                      --mac da 00:04:05:06:07:11
      "000102033402"..                      --mac sa 00:01:02:03:34:02
      "81000005"..                          --vlan 5
      "86DD"..                              --ethertype
      "6000000000481140"..                  --ipv6 Header: version=6, payloadLength=0x48, nextHeader=0x11, hopLimit=0x40
      "65450000000000000000000000005678"..  --source ip 6545::5678
      "FD010000000000000000000000001234"..  --destination ip FD01::1234
      "B3AAC3500048BE8F"..                  --UDP Header: srcPort=0xB3AA=45994 destPort=0xC350=50000
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

return  { private_to_public_packet = private_to_public_packet, egress_private_to_public_packet = egress_private_to_public_packet,
          public_to_private_packet = public_to_private_packet, egress_public_to_private_packet = egress_public_to_private_packet}

