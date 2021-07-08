--
-- Packets for IP filtering
-- 
packets = {}

packets["ipv4_match_packet"] = genPktL2({
     dstMAC                =  "010101010101",
     srcMAC                =  "000000000011",
     etherType             =  "0800",     -- =(ipv4)

     payload = genPktIpV4({
         srcIp = "192.168.0.5",
         dstIp = "192.168.0.23",
         data  = "000102030405060708090A0B0C0D0E0F"..
                 "101112131415161718191A1B1C1D1E1F"..
                 "202122232425262728292A2B2C2D2E2F"..
                 "303132333435363738393A3B3C3D3E3F"
     })
 })

 packets["ipv4_nomatch_packet"] = genPktL2({
     dstMAC                =  "010101010101",
     srcMAC                =  "000000000011",
     etherType             =  "0800",     -- =(ipv4)

     payload = genPktIpV4({
         srcIp = "192.168.0.5",
         dstIp = "192.169.0.23",
         data  = "000102030405060708090A0B0C0D0E0F"..
                 "101112131415161718191A1B1C1D1E1F"..
                 "202122232425262728292A2B2C2D2E2F"..
                 "303132333435363738393A3B3C3D3E3F"
     })
 })

packets["ipv6_match_packet"] = genPktL2({
     dstMAC                =  "010101010101",
     srcMAC                =  "000000000011",
     etherType             =  "86DD",     -- =(ipv6)

     payload = genPktIpV6({
         srcIp = "0001:0002:0003:0004:0005:0006:0007:0008",
         dstIp = "FF02::1111",
         data  = "000102030405060708090A0B0C0D0E0F"..
                 "101112131415161718191A1B1C1D1E1F"..
                 "202122232425262728292A2B2C2D2E2F"..
                 "303132333435363738393A3B3C3D3E3F"
     })
 })

packets["ipv6_nomatch_packet"] = genPktL2({
     dstMAC                =  "010101010101",
     srcMAC                =  "000000000011",
     etherType             =  "86DD",     -- =(ipv6)

     payload = genPktIpV6({
         srcIp = "0001:0002:0003:0004:0005:0006:0007:0008",
         dstIp = "FF03::1111",
         data  = "000102030405060708090A0B0C0D0E0F"..
                 "101112131415161718191A1B1C1D1E1F"..
                 "202122232425262728292A2B2C2D2E2F"..
                 "303132333435363738393A3B3C3D3E3F"
     })
 })

return packets
