-- Incoming and egressed packets used in SPBM routing test (VPN1).
-- Used tunnel headers are MAC-in-MAC headers (IEEE802.1ah)

local devNum = devEnv.dev
local port1 = devEnv.port[1]
local port2 = devEnv.port[2]
local port3 = devEnv.port[3]
local port4 = devEnv.port[4]
local port5 = devEnv.port[5]

local dev_own_mac = {
        a = "00000000deaa", b = "00000000debb", c = "00000000decc"}
local mc_mac = "01005e010101";
local vpn1_src_mac1 = "000000000123";
local vpn1_src_vid1 = 1000;
local vpn1_local_routed_vid1 = 2000;
local mc_dip = "e0010101";
local vpn1_src_sip1 = "ac100101"; -- 172.16.1.1
local pbb_mc_mac_prefix = "a1000a";
local pbb_vid = 511;
local send_pkt_sid = 0x010000;
local customerMaskedBytes = {
    {startByte = 14, endByte = 14, startByteCompareMask = 0x0F} -- exclude Vlan UP and CFI
};
local pbbMaskedBytes = {
    {startByte = 14, endByte = 14, startByteCompareMask = 0x0F}, -- exclude B-Vlan UP and CFI
    {startByte = 18, endByte = 18} -- exclusde SID flags
};

-- da and sa - 12-octet strings
local function l2Hdr(da, sa, vid)
    if not vid then
        return (da .. sa);
    end
    return (da .. sa .. "8100" .. "0" .. string.format("%3.3x", vid));
end

-- sip and dip - 8-octet strings, len - payload length
local function ipv4Hdr(sip, dip, len)
    local part1 = "4500"
        .. string.format("%4.4x", (len + 20))
        .. "00000000" .. "4000";
    local part2 = sip .. dip .. "00000000";
    local cs = luaTgfCalculate16bitCheckSum({part1, part2});
    return (part1 .. string.format("%4.4x", cs) .. part2);
end

local function backboneMcGenHdr (da, sa, vid, sid)
    return (
        string.sub(da, 1, 6) .. string.format("%6.6x", sid) .. sa
        .. "88A8" .. "0" .. string.format("%3.3x", vid) --vlan tag
        .. "88E7" .. "00" .. string.format("%6.6x", sid));
end

local function backboneMcHdr (dev)
    return backboneMcGenHdr(pbb_mc_mac_prefix, dev_own_mac[dev], pbb_vid, send_pkt_sid);
end

local payload = ""
   .. "000102030405060708090a0b0c0d0e0f"
   .. "101112131415161718191a1b1c1d1e1f"
   .. "202122232425262728292a2b2c2d2e2f"
   .. "303132333435363738393a3b3c3d3e3f";

local payload_len = math.floor(string.len(payload) / 2);

-- source packet sent to A/port1
local sentPacket_A_1 =
    l2Hdr(mc_mac, vpn1_src_mac1, vpn1_src_vid1) .. "0800" ..
    ipv4Hdr(vpn1_src_sip1, mc_dip, payload_len) .. payload;

-- routed packet received at A/port2
local receivedPacket_A_2_routed =
    l2Hdr(mc_mac, dev_own_mac["a"], vpn1_local_routed_vid1) .. "0800" ..
    ipv4Hdr(vpn1_src_sip1, mc_dip, payload_len) .. payload;
local receivedPacket_A_2_brigdged = sentPacket_A_1;

local receivedPacket_A_4 =
    backboneMcHdr("a") ..
    l2Hdr(mc_mac, vpn1_src_mac1, nil --[[vid--]]) .. "0800" ..
    ipv4Hdr(vpn1_src_sip1, mc_dip, payload_len) .. payload;

local sentPacket_B_4 = receivedPacket_A_4;
local receivedPacket_B_5 = sentPacket_B_4;
-- routed packet received at B/port3
local receivedPacket_B_3 =
    l2Hdr(mc_mac, dev_own_mac["b"], nil --[[vid--]]) .. "0800" ..
    ipv4Hdr(vpn1_src_sip1, mc_dip, payload_len) .. payload;
local sentPacket_C_4 = receivedPacket_B_5;
-- routed packet received at C/port1
local receivedPacket_C_1 =
    l2Hdr(mc_mac, dev_own_mac["c"], nil --[[vid--]]) .. "0800" ..
    ipv4Hdr(vpn1_src_sip1, mc_dip, payload_len) .. payload;

return {
    a = {
            {
                transmitInfo = {
                    devNum = devNum,
                    portNum = port1,
                    pktInfo = {fullPacket = sentPacket_A_1}
                },
                egressInfoTable = {
                    {
                        devNum = devNum,
                        portNum = port2,
                        pktInfoSet = {
                            {
                                fullPacket = receivedPacket_A_2_routed,
                                maskedBytesTable = customerMaskedBytes
                            },
                            {
                                fullPacket = receivedPacket_A_2_brigdged,
                                maskedBytesTable = customerMaskedBytes
                            }
                        }
                    },
                    {portNum = port3, packetCount = 0},
                    {
                        devNum = devNum,
                        portNum = port4,
                        pktInfo = {
                            fullPacket = receivedPacket_A_4,
                            maskedBytesTable = pbbMaskedBytes
                        }
                    }
                }
            },
    },
    b = {
            {
                transmitInfo = {
                    devNum = devNum,
                    portNum = port4,
                    pktInfo = {fullPacket = sentPacket_B_4}
                },
                egressInfoTable = {
                    {portNum = port1, packetCount = 0},
                    {portNum = port2, packetCount = 0},
                    {
                        devNum = devNum,
                        portNum = port3,
                        pktInfo = {
                            fullPacket = receivedPacket_B_3
                        }
                    },
                    {
                        devNum = devNum,
                        portNum = port5,
                        pktInfo = {
                            fullPacket = receivedPacket_B_5,
                            maskedBytesTable = pbbMaskedBytes
                        }
                    }
                }
            },
    },
    c = {
            {
                transmitInfo = {
                    devNum = devNum,
                    portNum = port4,
                    pktInfo = {fullPacket = sentPacket_C_4}
                },
                egressInfoTable = {
                    {
                        devNum = devNum,
                        portNum = port1,
                        pktInfo = {
                            fullPacket = receivedPacket_C_1
                        }
                    },
                    {portNum = port2, packetCount = 0},
                    {portNum = port3, packetCount = 0},
                }
            },
    }

};


