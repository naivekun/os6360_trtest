--
-- unicast vlan tagged packet (vlan tag = 5)
--

local packetInfo = {
     dstMAC                =  "000000003402",
     srcMAC                =  "000000000022",
     ieee802_1q_Tag        =  "81000005",  -- tag0 vlan id = 5
     etherType             =  "0030",       -- payload's length 0x30(48)
     payload               =  "fffefdfcfbfa00000000000000000000"..
                              "00000000000000000000000000000000"..
                              "00000000000000000000000000000000"
}

local packet = genPktL2(packetInfo)

return packetInfo, packet
