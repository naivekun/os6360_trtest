--
-- broadcast packet with vlan=2
-- 
-- = "FFFFFFFFFFFF 000000000001 8100 0002 0800   4500002E0000000040047ACD  00000000 00000000 00000000000000000000000000000000000000000000"
--
ipPacket = genPktL2({
     dstMAC                =  "FFFFFFFFFFFF",
     srcMAC                =  "000000000001",
     ieee802_1q_Tag        =  "8100 0002",  -- optional, vlan=2 
     etherType             =  "0800",       -- =(ipv4)

     payload = genPktIpV4({
         srcIp = "0.0.0.0",
         dstIp = "0.0.0.0",
         data  = "00000000000000000000000000000000000000000000"
     })
 })

return ipPacket
