
--[[ ingress first packet:
     000102033402 is the my-mac-addr configured in the mpls_transmit command: configuration added to macToMe table
     00333040  - is the ingress-labels-values-list configured in the mpls_transmit command: configuration added to key for TTI match
     00007040  -ELI label: will be used for TTI match
     04545100  -EL label: will NOT be used for TTI match location of the EL label is defined in entropy-labels-locations-list
--]]
local ingress_1_entropy_label_packet_with_1_tunnel_label_and_entropy_label = ""..
      "000102033402"..                      --mac da 00:01:02:03:34:02
      "000405060711"..                      --mac sa 00:04:05:06:07:11
      "8847"..                              --ethertype MPLS
      "00333040"..                          --MPLS Header: label0=0x333 timetoleave=0x40=64
      "00007040"..                          --ELI label: Sbit=0x0 EXP=0x0 ttl=0x40
      "0454D100"..                          --EL label=0x4545 Sbit=0x1 EXP=0x0 ttl=0x0
      "000102033401"..                      --mac da 00:01:02:03:34:01
      "000405060710"..                      --mac sa 00:04:05:06:07:10
      "8600"..                              --ethertype
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

--[[ ingress second packet:
     000102033402 is the my-mac-addr configured in the mpls_transmit command: configuration added to macToMe table
     00333040  - is the ingress-labels-values-list configured in the mpls_transmit command: configuration added to key for TTI match
     00007040  -ELI label: will be used for TTI match
     03232100  -EL label: will NOT be used for TTI match location of the EL label is defined in entropy-labels-locations-list
--]]
local ingress_2_entropy_label_packet_with_1_tunnel_label_and_entropy_label = ""..
      "000102033402"..                      --mac da 00:01:02:03:34:02
      "000405060711"..                      --mac sa 00:04:05:06:07:11
      "8847"..                              --ethertype MPLS
      "00333040"..                          --MPLS Header: label0=0x333 timetoleave=0x40=64
      "00007040"..                          --ELI label: Sbit=0x0 EXP=0x0 ttl=0x40
      "03232100"..                          --EL label=0x3232 Sbit=0x1 EXP=0x0 ttl=0x0
      "000102033401"..                      --mac da 00:01:02:03:34:01
      "000405060710"..                      --mac sa 00:04:05:06:07:10
      "8600"..                              --ethertype
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

--[[ egress first packet:
     000102033403 is the nexthop-mac-da-addr configured in the mpls_transmit command: configuration added to TS table or ARP table according to mpls_command
     000405060712 is the egress-mac-sa-addr configured in the mpls_transmit command
     00123040  - is the mpls-new-label configured in the mpls_transmit command, mpls_command configured to SWAP
     00007040  -ELI label: not changed in egress packet
     04545100  -EL label: not changed in egress packet
--]]
local egress_1_entropy_label_packet_with_1_tunnel_label_and_entropy_label = ""..
      "000102033403"..                      --mac da 00:01:02:03:34:03
      "000405060712"..                      --mac sa 00:04:05:06:07:12
      "81000001"..                          --vlan 1
      "8847"..                              --ethertype MPLS
      "00123040"..                          --MPLS Label swaped from TS: label0=0x123 timetoleave=0x40=64
      "00007040"..                          --ELI label: Sbit=0x0 EXP=0x0 ttl=0x40
      "0454D100"..                          --EL label=0x4545 Sbit=0x1 EXP=0x0 ttl=0x0
      "000102033401"..                      --mac da 00:01:02:03:34:01
      "000405060710"..                      --mac sa 00:04:05:06:07:10
      "8600"..                              --ethertype
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"


--[[ egress second packet:
     000102033403 is the nexthop-mac-da-addr configured in the mpls_transmit command: configuration added to TS table or ARP table according to mpls_command
     000405060712 is the egress-mac-sa-addr configured in the mpls_transmit command
     00123040  - is the mpls-new-label configured in the mpls_transmit command, mpls_command configured to SWAP
     00007040  -ELI label: not changed in egress packet
     04545100  -EL label: not changed in egress packet
--]]
local egress_2_entropy_label_packet_with_1_tunnel_label_and_entropy_label = ""..
      "000102033403"..                      --mac da 00:01:02:03:34:03
      "000405060712"..                      --mac sa 00:04:05:06:07:12
      "81000001"..                          --vlan 1
      "8847"..                              --ethertype MPLS
      "00123040"..                          --MPLS Label swaped from TS: label0=0x123 timetoleave=0x40=64
      "00007040"..                          --ELI label: Sbit=0x0 EXP=0x0 ttl=0x40
      "03232100"..                          --EL label=0x3232 Sbit=0x1 EXP=0x0 ttl=0x0
      "000102033401"..                      --mac da 00:01:02:03:34:01
      "000405060710"..                      --mac sa 00:04:05:06:07:10
      "8600"..                              --ethertype
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

return  { ingress_1 = ingress_1_entropy_label_packet_with_1_tunnel_label_and_entropy_label,
          ingress_2 = ingress_2_entropy_label_packet_with_1_tunnel_label_and_entropy_label,
          egress_1 = egress_1_entropy_label_packet_with_1_tunnel_label_and_entropy_label,
          egress_2 = egress_2_entropy_label_packet_with_1_tunnel_label_and_entropy_label }

