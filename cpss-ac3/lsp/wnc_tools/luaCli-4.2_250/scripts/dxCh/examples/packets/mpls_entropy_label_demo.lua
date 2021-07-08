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
      "000102033401"..                      --mac da 00:01:02:03:34:01
      "08090A0B0C19"..                      --mac sa 08:09:0A:0B:0C:19
      "8600"..                              --ethertype
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

--[[ first egress packet on port [2] = ingress packer on port [2] (after loopback)
     000102033402 - mac da 00:01:02:03:34:02
     000405060711 - mac sa 00:04:05:06:07:11
     00333040     - MPLS Header: label0=0x333 timetoleave=0x40
     00007040     - ELI label: Sbit=0x0 EXP=0x0 ttl=0x40
     00011100     - EL label: hashvalue=0x11 Sbit=0x1 EXP=0x0 ttl=0x0
     
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
--]]

--[[ second egress packet on port [2] = ingress packer on port [2] (after loopback)
     000102033402 - mac da 00:01:02:03:34:02
     000405060711 - mac sa 00:04:05:06:07:11
     00333040     - MPLS Header: label0=0x333 timetoleave=0x40
     00007040     - ELI label: Sbit=0x0 EXP=0x0 ttl=0x40
     00018100     - EL label: hashvalue=0x18 Sbit=0x1 EXP=0x0 ttl=0x0
     
    "000102033402"..                      --mac da 00:01:02:03:34:02
    "000405060711"..                      --mac sa 00:04:05:06:07:11
    "81000001"..                          --VLAN
    "8847"..                              --ethertype MPLS
    "00333040"..                          --MPLS Header: label0=0x333 timetoleave=0x40
    "00007040"..                          --ELI label: Sbit=0x0 EXP=0x0 ttl=0x40
    "00018100"..                          --EL label: hashvalue=0x18 Sbit=0x1 EXP=0x0 ttl=0x0
    "000102033401"..                      --mac da 00:01:02:03:34:01
    "08090A0B0C19"..                      --mac sa 08:09:0A:0B:0C:19
    "8600"..                              --ethertype
    "000102030405060708090A0B"..          --payload
    "0C0D0E0F1011121314151617"..
    "18191A1B1C1D1E1F20212223"..
    "2425262728292A2B2C2D2E2F"..
    "303132333435363738393A3B"..
    "3C3D3E3F"     
--]]

--[[ egress first packet on trunk port:
     000102033403 - is the nexthop-mac-da-addr configured in the mpls_transmit command: configuration added to TS table or ARP table according to mpls_command
     000405060712 - is the egress-mac-sa-addr configured in the mpls_transmit command
     00123040     - is the mpls-new-label configured in the mpls_transmit command, mpls_command configured to SWAP
     00007040     - ELI label: not changed in egress packet
     00011100     - EL label: not changed in egress packet
--]]
local egress_1_entropy_label_packet_with_1_tunnel_label_and_entropy_label = ""..
      "000102033403"..                      --mac da 00:01:02:03:34:03
      "000405060712"..                      --mac sa 00:04:05:06:07:12
      "81000001"..                          --vlan 1
      "8847"..                              --ethertype MPLS
      "00123040"..                          --MPLS Label swaped from TS: label0=0x123 timetoleave=0x40=64
      "00007040"..                          --ELI label: Sbit=0x0 EXP=0x0 ttl=0x40
      "00011100"..                          --EL label=0x11 Sbit=0x1 EXP=0x0 ttl=0x0
      "000102033401"..                      --mac da 00:01:02:03:34:01
      "000405060710"..                      --mac sa 00:04:05:06:07:10
      "8600"..                              --ethertype
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

--[[ egress second packet on trunk port:
     000102033403 - is the nexthop-mac-da-addr configured in the mpls_transmit command: configuration added to TS table or ARP table according to mpls_command
     000405060712 - is the egress-mac-sa-addr configured in the mpls_transmit command
     00123040     - is the mpls-new-label configured in the mpls_transmit command, mpls_command configured to SWAP
     00007040     - ELI label: not changed in egress packet
     00018100     - EL label: not changed in egress packet
--]]
local egress_2_entropy_label_packet_with_1_tunnel_label_and_entropy_label = ""..
      "000102033403"..                      --mac da 00:01:02:03:34:03
      "000405060712"..                      --mac sa 00:04:05:06:07:12
      "81000001"..                          --vlan 1
      "8847"..                              --ethertype MPLS
      "00123040"..                          --MPLS Label swaped from TS: label0=0x123 timetoleave=0x40=64
      "00007040"..                          --ELI label: Sbit=0x0 EXP=0x0 ttl=0x40
      "00018100"..                          --EL label=0x18 Sbit=0x1 EXP=0x0 ttl=0x0
      "000102033401"..                      --mac da 00:01:02:03:34:01
      "08090A0B0C19"..                      --mac sa 08:09:0A:0B:0C:19
      "8600"..                              --ethertype
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

return  { ethernet = ethernet_packet, ethernet_2 = ethernet_packet_2, 
          egress_entropy_label_1 = egress_1_entropy_label_packet_with_1_tunnel_label_and_entropy_label,
          egress_entropy_label_2 = egress_2_entropy_label_packet_with_1_tunnel_label_and_entropy_label}

