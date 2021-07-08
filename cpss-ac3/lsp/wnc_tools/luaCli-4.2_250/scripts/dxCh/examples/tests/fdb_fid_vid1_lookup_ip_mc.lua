--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* fdb_fid_vid1_lookup_ip_mc.lua
--*
--* DESCRIPTION:
--*      test an FDB lookup key mode for multicast IPv4/6 packets
--*
--* Configure VLAN 6 multicast IPv4/6 mode to be an ip-group:
--* - vlan 6: ip-group
--* - vlan 7: ip-src-group
--* IPv4 224.1.1.1 vid 6 vid1 10 ===>> 0/18,36
--* IPv4 224.1.1.1 vid 6 vid1 11 ===>> 0/18,54
--*
--* IPv6 fff1::1   vid 6 vid1 10 ===>> 0/18,36
--* IPv6 fff1::1   vid 6 vid1 11 ===>> 0/18,54
--*
--* Configure VLAN 7 multicast IPv4/6 mode to be an ip-src-group:
--* IPv4 Group/Source 224.2.2.2/9.8.7.6  vid 7 vid1 10 ===>> 0/18,36
--* IPv4 Group/Source 224.2.2.2/9.8.7.6  vid 7 vid1 11 ===>> 0/18,54
--*
--* IPv6 Group/Source FFF1::2/::2        vid 7 vid1 10 ===>> 0/18,36
--* IPv6 Group/Source FFF1::2/::2        vid 7 vid1 11 ===>> 0/18,54
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-------------------- local declarations --------------------
local devNum = devEnv.dev
local port1  = devEnv.port[1]
local port2  = devEnv.port[2]
local port3  = devEnv.port[3]
local port4  = devEnv.port[4]
-- this test is relevant for SIP_5_10 devices (BOBCAT 2 B0 and above)
-- the test not relevant to sip6 because VID1 is not part of the FDB IPMC entry format
-- from Falcon IAS doc : "VID1 would muxed with SIP and would not be used for this entry type. 
--  It means that a lookup with double tags <eVLAN, VID1, S, G> would not be supported"
SUPPORTED_SIP_DECLARE(devNum,"SIP_5_10",true, "SIP_6")

local configDir    = "dxCh/examples/configurations/"
local configFile   = configDir .. "fdb_fid_vid1_lu_ip_mc.txt"
local deconfigFile = configDir .. "fdb_fid_vid1_lu_ip_mc_deconfig.txt"
local packetFileName = "dxCh/examples/packets/fdb_fid_vid1_lookup_ip_mc.lua"

local function sendAndCheckTraffic(packets)
    local rc = 0

    -- all packets will be send in order of declaration in the table
    local scenario = {
        -- IPv4 packets

        { -- vid 6: send IPv4, vid1=10
            transmitInfo = {devNum = devNum, portNum = port1,
                            pktInfo = {fullPacket = packets.pkt4_1}},
            egressInfo = {
                {portNum = port2, packetCount = 1},
                {portNum = port3, packetCount = 1},
                {portNum = port4, packetCount = 0}
            }
        },
        { -- vid 6: send IPv4, vid1=11
            transmitInfo = {devNum = devNum, portNum = port1,
                            pktInfo = {fullPacket = packets.pkt4_2}},
            egressInfo = {
                {portNum = port2, packetCount = 1},
                {portNum = port3, packetCount = 0},
                {portNum = port4, packetCount = 1}
            }
        },

        { -- vid 7: send IPv4, vid1=10
            transmitInfo = {devNum = devNum, portNum = port1,
                            pktInfo = {fullPacket = packets.pkt4s_1}},
            egressInfo = {
                {portNum = port2, packetCount = 1},
                {portNum = port3, packetCount = 1},
                {portNum = port4, packetCount = 0}
            }
        },
        { -- vid 7: send IPv4, vid1=11
            transmitInfo = {devNum = devNum, portNum = port1,
                            pktInfo = {fullPacket = packets.pkt4s_2}},
            egressInfo = {
                {portNum = port2, packetCount = 1},
                {portNum = port3, packetCount = 0},
                {portNum = port4, packetCount = 1}
            }
        },

        -- IPv6 packets

        { -- vid 6: send IPv6, vid1=10
            transmitInfo = {devNum = devNum, portNum = port1,
                            pktInfo = {fullPacket = packets.pkt6_1}},
            egressInfo = {
                {portNum = port2, packetCount = 1},
                {portNum = port3, packetCount = 1},
                {portNum = port4, packetCount = 0}
            }
        },
        { -- vid 6: send IPv6, vid1=11
            transmitInfo = {devNum = devNum, portNum = port1,
                            pktInfo = {fullPacket = packets.pkt6_2}},
            egressInfo = {
                {portNum = port2, packetCount = 1},
                {portNum = port3, packetCount = 0},
                {portNum = port4, packetCount = 1}
            }
        },
        { -- vid 7: send IPv6, vid1=10
            transmitInfo = {devNum = devNum, portNum = port1,
                            pktInfo = {fullPacket = packets.pkt6s_1}},
            egressInfo = {
                {portNum = port2, packetCount = 1},
                {portNum = port3, packetCount = 1},
                {portNum = port4, packetCount = 0}
            }
        },
        { -- vid 7: send IPv6, vid1=11
            transmitInfo = {devNum = devNum, portNum = port1,
                            pktInfo = {fullPacket = packets.pkt6s_2}},
            egressInfo = {
                {portNum = port2, packetCount = 1},
                {portNum = port3, packetCount = 0},
                {portNum = port4, packetCount = 1}
            }
        },

        -- IPv4/6 packets with MAC DA out of IANA range intended for IPv4/6 MC.
        { -- IPv4: vid6, vid1 10
            transmitInfo = {devNum = devNum, portNum = port1,
                            pktInfo = {fullPacket = packets.pkt4_bad}},
            egressInfo = {
                {portNum = port2, packetCount = 0},
                {portNum = port3, packetCount = 0},
                {portNum = port4, packetCount = 1}
            }
        },
        { -- IPv6: vid6, vid1 10
            transmitInfo = {devNum = devNum, portNum = port1,
                            pktInfo = {fullPacket = packets.pkt6_bad}},
            egressInfo = {
                {portNum = port2, packetCount = 0},
                {portNum = port3, packetCount = 0},
                {portNum = port4, packetCount = 1}
            }
        }
    }

    for _, traffic in ipairs(scenario) do
        -- send the packet and check captured packets
        rc = luaTgfTransmitPacketsWithExpectedEgressInfo(traffic.transmitInfo,
                                                         traffic.egressInfo)
        if rc~=0 then
            setFailState()
            print("packet transmitting error: ".. returnCodes[rc])
            return false
        end
    end
    return (rc == 0)
end

------------------------------ MAIN PART ---------------------------------------

-- read the packets from the file
local ret, packets = pcall(dofile, packetFileName)
if not ret then
   error("\nError while reading the the packets:"..packets)
end


print("-------------------------------------------------")
print("FDB double tag lookup key test (IPv4/6 MC)")

luaTgfSimpleTest(configFile, deconfigFile,
                 sendAndCheckTraffic, packets);
