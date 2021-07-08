--[[ ingress first packet:
     000102033401 - is the mac da - mac address of configured device
     000405060710 - is the mac sa - different values in packets used for calculation different hash values
--]]
local ethernet_packet = ""..
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
     000102033401 - is the mac da - mac address of configured device
     08090A0B0C19 - is the mac sa - different values in packets used for calculation different hash values
--]]      
local ethernet_packet_2 = ""..
      "000102033405"..                      --mac da 00:01:02:03:34:01
      "08090A0B0C19"..                      --mac sa 08:09:0A:0B:0C:15
      "8600"..                              --ethertype
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"
--[[ ingress third packet:
     000102033401 - is the mac da - mac address of configured device
     002536475869 - is the mac sa - different values in packets used for calculation different hash values
--]]      
local ethernet_packet_3 = ""..
      "000102033407"..                      --mac da 00:01:02:03:34:01
      "002536475869"..                      --mac sa 00:25:36:47:58:69
      "8600"..                              --ethertype
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"
--[[ egress first packet:
     000102033402 - is the nexthop-mac-addr configured in the mpls command
     000405060711 - is the my-mac-addr configured in the mpls command: configuration added to macToMe entry
     00333040     - is the mpls label: configuration added to TS table according to mpls command
     00007040     - ELI label
     00011100     - EL label
--]]
local egress_entropy_label_packet_with_1_tunnel_label_and_entropy_label = ""..
      "000102033402"..                      --mac da 00:01:02:03:34:02
      "000405060711"..                      --mac sa 00:04:05:06:07:11
      "81000001"..                          --VLAN
      "8847"..                              --ethertype MPLS
      "00333040"..                          --MPLS Header: label0=0x333 timetoleave=0x40
      "00007040"..                          --ELI label: Sbit=0x0 EXP=0x0 ttl=0x40
      "00011100"..                          --EL label: hashvalue=0x11 Sbit=0x1 EXP=0x0 ttl=0x0
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
     000102033402 - is the nexthop-mac-addr configured in the mpls command
     000405060711 - is the my-mac-addr configured in the mpls command: configuration added to macToMe entry
     00333040     - is the mpls label0: configuration added to TS table according to mpls command
     00007040     - ELI label0
     0001C000     - EL label0
     00444040     - is the mpls label1: configuration added to TS table according to mpls command
     00555040     - is the mpls label2: configuration added to TS table according to mpls command
     00007040     - ELI label2
     0001C100     - EL label2
--]]
local egress_entropy_label_packet_with_3_tunnel_labels_and_EL_after_labels_0_and_2 = ""..
      "000102033402"..                      --mac da 00:01:02:03:34:02
      "000405060711"..                      --mac sa 00:04:05:06:07:11
      "81000001"..                          --VLAN
      "8847"..                              --ethertype MPLS
      "00333040"..                          --MPLS Header: label0=0x333 timetoleave=0x40
      "00007040"..                          --ELI label: Sbit=0x0 EXP=0x0 ttl=0x40
      "0001C000"..                          --EL label: hashvalue=0x1C Sbit=0x0 EXP=0x0 ttl=0x0
      "00444040"..                          --MPLS Header: label1=0x444 timetoleave=0x40
      "00555040"..                          --MPLS Header: label2=0x555 timetoleave=0x40
      "00007040"..                          --ELI label: Sbit=0x0 EXP=0x0 ttl=0x40
      "0001C100"..                          --EL label: hashvalue=0x1C Sbit=0x1 EXP=0x0 ttl=0x0
      "000102033405"..                      --mac da 00:01:02:03:34:05
      "08090A0B0C19"..                      --mac sa 08:09:0A:0B:0C:19
      "8600"..                              --ethertype
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"
--[[ egress third packet:
     000102033402 - is the nexthop-mac-addr configured in the mpls command
     000405060711 - is the my-mac-addr configured in the mpls command: configuration added to macToMe entry
     00333040     - is the mpls label0: configuration added to TS table according to mpls command
     00007040     - ELI label0
     0002E000     - EL label0
     00444040     - is the mpls label1: configuration added to TS table according to mpls command
     00007040     - ELI label1
     0002E100     - EL label1
--]]
local egress_entropy_label_packet_with_2_tunnel_labels_and_EL_after_labels_0_and_1 = ""..
      "000102033402"..                      --mac da 00:01:02:03:34:02
      "000405060711"..                      --mac sa 00:04:05:06:07:11
      "81000001"..                          --VLAN
      "8847"..                              --ethertype MPLS
      "00333040"..                          --MPLS Header: label0=0x333 timetoleave=0x40
      "00007040"..                          --ELI label: Sbit=0x0 EXP=0x0 ttl=0x40
      "0002E000"..                          --EL label: hashvalue=0x2E Sbit=0x0 EXP=0x0 ttl=0x0
      "00444040"..                          --MPLS Header: label1=0x444 timetoleave=0x40
      "00007040"..                          --ELI label: Sbit=0x0 EXP=0x0 ttl=0x40
      "0002E100"..                          --EL label: hashvalue=0x2E Sbit=0x1 EXP=0x0 ttl=0x0
      "000102033407"..                      --mac da 00:01:02:03:34:07
      "002536475869"..                      --mac sa 00:25:36:47:58:69
      "8600"..                              --ethertype
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

return  { ethernet = ethernet_packet, ethernet_2 = ethernet_packet_2, ethernet_3 = ethernet_packet_3, 
          egress_entropy_label_1 = egress_entropy_label_packet_with_1_tunnel_label_and_entropy_label,
          egress_entropy_label_2 = egress_entropy_label_packet_with_3_tunnel_labels_and_EL_after_labels_0_and_2,
          egress_entropy_label_3 = egress_entropy_label_packet_with_2_tunnel_labels_and_EL_after_labels_0_and_1 }

