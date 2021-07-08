-- define packets to test a double tag FDB lookup key mode
-- feature for a multicast IPv4/6 bridging
--
-- NOTE:
-- MAC DA for IPv4 MC should be in range 01:00:5E:00:00:00 - 01:00:5E:7F:FF:FF
-- and 23 low-order bits of MAC should correspond to low bits of IP address
-- MAC DA for IPv6 MC should be 33:33:xx:xx:xx:xx. And 32 low bits should
-- correcport to low bits of IPv6 address.

local macDa4        = "01005e010101"
local macDa4s       = "01005e020202"
local macDa6        = "333300000001"
local macDa6s       = "333300000002"
local macSa         = "000000000011"
local macDaBad      = "0123450f0e0d" -- not applicable for IPv4/6 MC bridging

local vid0          = "81000006" -- vlan configured for (*, G) MC Bridging
local vid0s         = "81000007" -- vlan configured for (S, G) MC Bridging
local vid1_1        = "8100000a" -- vid1=10
local vid1_2        = "8100000b" -- vid1=11
local etherTypeIpv4 = "0800"     -- IPv4
local etherTypeIpv6 = "86DD"     -- IPv6

local payload       = ""
    .. "000102030405060708090a0b0c0d0e0f"
    .. "101112131415161718191a1b1c1d1e1f"
    .. "202122232425262728292a2b2c2d2e2f"
    .. "303132333435363738393a3b3c3d3e3f"

-- IPv4 part (with payload) of packet sent in vlan vid0 (with payload)
local ipPart4 =  genPktIpV4({srcIp = "1.2.3.4",
                             dstIp = "224.1.1.1",
                             data  = payload})

-- IPv4 part  (with payload) packet sent in vlan vid0s
local ipPart4s = genPktIpV4({srcIp = "9.8.7.6",
                             dstIp = "224.2.2.2",
                             data  = payload})

-- IPv6 part (with payload) of packet sent in vlan vid0
ipPart6 = genPktIpV6({srcIp = "::1",
                      dstIp = "FFF1::1",
                      data  = payload})

-- IPv6 part (with payload) of packet sent in vlan vid0s
ipPart6s = genPktIpV6({srcIp = "::2",
                       dstIp = "FFF1::2",
                       data  = payload})

-- (*, G) MC Bridging, two IPv4 packets different by VID1 only
local pkt4_1   = macDa4..macSa..vid0..vid1_1..etherTypeIpv4..ipPart4
local pkt4_2   = macDa4..macSa..vid0..vid1_2..etherTypeIpv4..ipPart4
-- (S, G) MC Bridging, two IPv4 packets different by VID1 only
local pkt4s_1   = macDa4s..macSa..vid0s..vid1_1..etherTypeIpv4..ipPart4s
local pkt4s_2   = macDa4s..macSa..vid0s..vid1_2..etherTypeIpv4..ipPart4s
-- (*, G) MC Bridging, two IPv6 packets different by VID1 only
local pkt6_1   = macDa6..macSa..vid0..vid1_1..etherTypeIpv6..ipPart6
local pkt6_2   = macDa6..macSa..vid0..vid1_2..etherTypeIpv6..ipPart6
-- (S, G) MC Bridging, two IPv6 packets different by VID1 only
local pkt6s_1 = macDa6s..macSa..vid0s..vid1_1..etherTypeIpv6..ipPart6s
local pkt6s_2 = macDa6s..macSa..vid0s..vid1_2..etherTypeIpv6..ipPart6s
-- MAC Bridging (IPv4/6 MC packets with 'bad' MAC DA)
local pkt4_bad = macDaBad..macSa..vid0..vid1_1..etherTypeIpv4..ipPart4
local pkt6_bad = macDaBad..macSa..vid0..vid1_1..etherTypeIpv6..ipPart6

return {pkt4_1  = pkt4_1,
        pkt4_2  = pkt4_2,
        pkt4s_1 = pkt4s_1,
        pkt4s_2 = pkt4s_2,
        pkt6_1  = pkt6_1,
        pkt6_2  = pkt6_2,
        pkt6s_1 = pkt6s_1,
        pkt6s_2 = pkt6s_2,
        pkt4_bad = pkt4_bad,
        pkt6_bad = pkt6_bad
}
