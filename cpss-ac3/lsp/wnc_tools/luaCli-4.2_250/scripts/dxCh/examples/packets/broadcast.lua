--
-- broadcast packet generator
-- 
 ipPacket = genPktL2({
     dstMAC                =  "FFFFFFFFFFFF",
     srcMAC                =  "000000000001",
     etherType             =  "0800",     -- =(ipv4)

     payload = genPktIpV4({
         srcIp = "224.1.1.1",
         dstIp = "1.1.10.10",
         data  = "000102030405060708"
     })
 })

return ipPacket
