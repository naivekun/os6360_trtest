--
-- fastBoot: check traffic after Eeprom configuration
--
--[[  0. cpssInitSystem PEX only
      1. load Eeprom configuration from file - this is special customer configuration.vlan 4094 with ports (4,58,67,69)
         force link up + reset mac counters is done only for port 4 (port 58 is in linkup)
      2. raise fast boot flag fast_boot INIT
      3. check traffic (drop counters and mac counters on ports 4,58)
      4. cpssInitSystem
      5. raise fast boot flag fast_boot CATCHUP
      6. check traffic
      7. raise fast boot flag fast_boot COMPLETE
      8. check traffic

      system description:
      1.    ports 4,58 belong to vlan 4094.
      2.    port 4 is in 'loopback' mode.
      3.    sending packet with 'dsa' tag 'from_cpu' to port port 63.
      4.    the packet egress from port 4 without 'dsa' tag on vlan 4094.
      5.    the loopback ingress the packet again to port 4.
      6.    the packet egress from port 58 and the replication to port 4 is filtered.
]]--

local broadcast_1_to_all_vlan_4094_packet = ""..
      "FFFFFFFFFFFF"..                      --mac da FF:FF:FF:FF:FF:FF
      "000000000001"..                      --mac sa 00:00:00:00:00:01
--      "81000FFE"..                          --vlan 4094
      "5555"..                              --ethertype
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

return  { broadcast_1_to_all_vlan_X = broadcast_1_to_all_vlan_4094_packet}
