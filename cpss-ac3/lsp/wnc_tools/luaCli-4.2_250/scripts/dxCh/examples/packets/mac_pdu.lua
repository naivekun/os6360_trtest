--
-- Bpdu packet for mac pdu test
-- 
 mac_Bpdu_Packet = genPktL2({
     dstMAC                =  "0180C2000000",
     srcMAC                =  "000000000001",
     etherType             =  "0800",     -- =(ipv4)

     payload = genPktIpV4({
         srcIp = "224.1.1.1",
         dstIp = "1.1.10.10",
         data  = "000102030405060708"
     })
 })


--
-- gmrp packet for mac pdu test
-- 
 mac_gmrp_Packet = genPktL2({
     dstMAC                =  "0180C2000020",
     srcMAC                =  "000000000001",
     etherType             =  "0800",     -- =(ipv4)

     payload = genPktIpV4({
         srcIp = "224.1.1.1",
         dstIp = "1.1.10.10",
         data  = "000102030405060708"
     })
 })


--
-- gvrp packet for mac pdu test
-- 
 mac_gvrp_Packet = genPktL2({
     dstMAC                =  "0180C2000021",
     srcMAC                =  "000000000001",
     etherType             =  "0800",     -- =(ipv4)

     payload = genPktIpV4({
         srcIp = "224.1.1.1",
         dstIp = "1.1.10.10",
         data  = "000102030405060708"
     })
 })

--
-- lacp packet for mac pdu test
-- 
 mac_lacp_Packet = genPktL2({
     dstMAC                =  "0180C2000002",
     srcMAC                =  "000000000001",
     etherType             =  "0800",     -- =(ipv4)

     payload = genPktIpV4({
         srcIp = "224.1.1.1",
         dstIp = "1.1.10.10",
         data  = "000102030405060708"
     })
 })

--
-- ldp packet for mac pdu test
-- 
 mac_ldp_Packet = genPktL2({
     dstMAC                =  "0180C200000E",
     srcMAC                =  "000000000001",
     etherType             =  "0800",     -- =(ipv4)

     payload = genPktIpV4({
         srcIp = "224.1.1.1",
         dstIp = "1.1.10.10",
         data  = "000102030405060708"
     })
 })

--
-- pae packet for mac pdu test
-- 
 mac_pae_Packet = genPktL2({
     dstMAC                =  "0180C2000003",
     srcMAC                =  "000000000001",
     etherType             =  "0800",     -- =(ipv4)

     payload = genPktIpV4({
         srcIp = "224.1.1.1",
         dstIp = "1.1.10.10",
         data  = "000102030405060708"
     })
 })

--
-- providerBPDU packet for mac pdu test
-- 
 mac_providerBPDU_Packet = genPktL2({
     dstMAC                =  "0180C2000008",
     srcMAC                =  "000000000001",
     etherType             =  "0800",     -- =(ipv4)

     payload = genPktIpV4({
         srcIp = "224.1.1.1",
         dstIp = "1.1.10.10",
         data  = "000102030405060708"
     })
 })

--
-- providerGVRP packet for mac pdu test
-- 
 mac_providerGVRP_Packet = genPktL2({
     dstMAC                =  "0180C200000D",
     srcMAC                =  "000000000001",
     etherType             =  "0800",     -- =(ipv4)

     payload = genPktIpV4({
         srcIp = "224.1.1.1",
         dstIp = "1.1.10.10",
         data  = "000102030405060708"
     })
 })

--[[
return  mac_Bpdu_Packet, mac_gmrp_Packet, mac_gvrp_Packet, 
        mac_lacp_Packet, mac_ldp_Packet, mac_pae_Packet, 
        mac_providerBPDU_Packet, mac_providerGVRP_Packet
]]--

return  { bpdu=mac_Bpdu_Packet, gmrp=mac_gmrp_Packet, gvrp=mac_gvrp_Packet, 
        lacp=mac_lacp_Packet, ldp=mac_ldp_Packet, pae=mac_pae_Packet, 
        providerBPDU=mac_providerBPDU_Packet, providerGVRP=mac_providerGVRP_Packet}
