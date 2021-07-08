--
-- fastBoot: check traffic after Eeprom configuration
--
--[[  0. cpssInitSystem PEX only
      1. load Eeprom configuration from file - this is special customer configuration.vlan 0x801 with ports (0,2,8)
         force link up + reset mac counters is done only for ports 0,8
      2. raise fast boot flag fast_boot INIT
      3. check traffic (drop counters and mac counters on ports 0,8)
      4. cpssInitSystem
      5. raise fast boot flag fast_boot CATCHUP
      6. check traffic
      7. raise fast boot flag fast_boot COMPLETE
      8. check traffic

      system description:
      1.    ports 0,8 belong to vlan 0x801.
      2.    port 0 is in 'loopback' mode.
      3.    sending packet with 'dsa' tag 'from_cpu' to port port 63.
      4.    the packet egress from port 0 without 'dsa' tag on vlan 0x801.
      5.    the loopback ingress the packet again to port 0.
      6.    the packet egress from port 8 and the replication to port 0 is filtered.
]]--

local broadcast_1_to_all_vlan_801_packet = ""..
      "FFFFFFFFFFFF"..                      --mac da FF:FF:FF:FF:FF:FF
      "000000000001"..                      --mac sa 00:00:00:00:00:01
      "81000801"..                          --vlan 801
      "5555"..                              --ethertype
      "000102030405060708090A0B"..          --payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

return  { broadcast_1_to_all_vlan_X = broadcast_1_to_all_vlan_801_packet}
