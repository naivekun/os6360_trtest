local packetInfo = {
     dstMAC                =  "000000000456",
     srcMAC                =  "000000000123",
     ieee802_1q_Tag        =  "810003E8",  -- tag0 vlan id = 1000 (0x3E8)
     etherType             =  "0030",       -- payload's length 0x30(48)
     payload               =  "fffefdfcfbfa00000000000000000000"..
                              "00000000000000000000000000000000"..
                              "00000000000000000000000000000000"
}

local packet = genPktL2(packetInfo)

return packet
