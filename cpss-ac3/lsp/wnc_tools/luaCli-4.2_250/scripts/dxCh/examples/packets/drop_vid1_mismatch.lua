--
-- unicast double vlan tagged packet (tag0 = tag1 - 215)
-- 
   
local packet = genPktL2({
     dstMAC                =  "000000003402",
     srcMAC                =  "000000000022",
     ieee802_1q_OuterTag   =  "810000D7",  -- tag0 vlan id = 215 (0xD7)
     ieee802_1q_InnerTag   =  "810000D7",  -- tag1 vlan id = 215
     etherType             =  "0030",       -- payload's length 0x30(48)
     payload               =  "fffefdfcfbfa00000000000000000000"..
                              "00000000000000000000000000000000"..
                              "00000000000000000000000000000000"

 })

return packet
