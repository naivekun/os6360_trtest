--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* fdb_fid_vid1_lookup.lua
--*
--* DESCRIPTION:
--*       .
--* TEST ON DOUBLE VTAG FDB LOOKUP
--* Configure FDB static entries and sending packets
--* unicast
--* mac 00:11:22:33:44:55 vid 5 vid1 7 ===>> 0/18
--* mac 00:11:22:33:44:55 vid 5 vid1 8 ===>> 0/36
--* multicast
--* mac A1:11:22:33:44:55 vid 5 vid1 7 ===>> 0/18,54
--* mac A1:11:22:33:44:55 vid 5 vid1 8 ===>> 0/36,54
--*
--* Send packet DA = 00:11:22:33:44:55 tag0_vid = 5 tag1_vid = 7 to port 0
--* Check egress packet counters port 18 - count = 1, port 36 - count = 0, port 54 - count = 0
--* Send packet DA = 00:11:22:33:44:55 tag0_vid = 5 tag1_vid = 8 to port 0
--* Check egress packet counters port 18 - count = 0, port 36 - count = 1, port 54 - count = 0
--* Send packet DA = A1:11:22:33:44:55 tag0_vid = 5 tag1_vid = 7 to port 0
--* Check egress packet counters port 18 - count = 1, port 36 - count = 0, port 54 - count = 1
--* Send packet DA = A1:11:22:33:44:55 tag0_vid = 5 tag1_vid = 8 to port 0
--* Check egress packet counters port 18 - count = 0, port 36 - count = 1, port 54 - count = 1
--*
--* Restore configurations
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
local devNum  = devEnv.dev
-- this test is relevant for SIP_5_10 devices (BOBCAT 2 B0 and above)
SUPPORTED_SIP_DECLARE(devNum,"SIP_5_10")

-- local declaration
local port1 = devEnv.port[1]
local port2 = devEnv.port[2]
local port3 = devEnv.port[3]
local port4 = devEnv.port[4]
local configFile   = "dxCh/examples/configurations/fdb_fid_vid1_lu.txt"
local deconfigFile = "dxCh/examples/configurations/fdb_fid_vid1_lu_deconfig.txt"

local src_mac = "000000000011";
local uc_mac  = "001122334455";
local mc_mac  = "a11122334455";
local vtag0   = "81000005"
local vtag1_0 = "81000007"
local vtag1_1 = "81000008"
local payload = ""
   .. "000102030405060708090a0b0c0d0e0f"
   .. "101112131415161718191a1b1c1d1e1f"
   .. "202122232425262728292a2b2c2d2e2f"
   .. "303132333435363738393a3b3c3d3e3f";
local uc_packet1 = uc_mac .. src_mac .. vtag0 .. vtag1_0 .. payload;
local uc_packet2 = uc_mac .. src_mac .. vtag0 .. vtag1_1 .. payload;
local mc_packet1 = mc_mac .. src_mac .. vtag0 .. vtag1_0 .. payload;
local mc_packet2 = mc_mac .. src_mac .. vtag0 .. vtag1_1 .. payload;


local fdbTestScenario = {
    ["UC Packet VID=5 VID1=7 expected port " ..port2] = {
        transmitInfo = {
            devNum = devNum,
            portNum = port1,
            pktInfo = {fullPacket = uc_packet1}
        },
        egressInfoTable = {
            {portNum = port2, packetCount = 1},
            {portNum = port3, packetCount = 0},
            {portNum = port4, packetCount = 0},
        },
    },
    ["UC Packet VID=5 VID1=8 expected ports "..port3] = {
        transmitInfo = {
            devNum = devNum,
            portNum = port1,
            pktInfo = {fullPacket = uc_packet2}
        },
        egressInfoTable = {
            {portNum = port2, packetCount = 0},
            {portNum = port3, packetCount = 1},
            {portNum = port4, packetCount = 0},
        },
    },
    ["MC Packet VID=5 VID1=7 expected ports "..port2..","..port4] = {
        transmitInfo = {
            devNum = devNum,
            portNum = port1,
            pktInfo = {fullPacket = mc_packet1}
        },
        egressInfoTable = {
            {portNum = port2, packetCount = 1},
            {portNum = port3, packetCount = 0},
            {portNum = port4, packetCount = 1},
        },
    },
    ["MC Packet VID=5 VID1=8 expected ports "..port3..","..port4] = {
        transmitInfo = {
            devNum = devNum,
            portNum = port1,
            pktInfo = {fullPacket = mc_packet2}
        },
        egressInfoTable = {
            {portNum = port2, packetCount = 0},
            {portNum = port3, packetCount = 1},
            {portNum = port4, packetCount = 1},
        },
    },
}

local function trafficCheck(trafficScenario)
    local key, scenario
    for key, scenario in pairs(trafficScenario) do
        print("========================================")
            print("Send a packet #".. tostring(key))
        local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
           scenario.transmitInfo, scenario.egressInfoTable);
        -- assert(
        --    rc==0, string.format("packet transmitting error: %s",
        --    returnCodes[rc]))
    end
end

------------------------------ MAIN PART ---------------------------------------
print("-------------------------------------------------")
print("FDB FID-VID1 Lookup")
isOk = luaTgfSimpleTest(configFile, deconfigFile, trafficCheck, fdbTestScenario);
if not isOk then
    error("\FDB FID-VID1 Lookup failed");
end

-- flush the FDB (use LUA CLI command !)
local function flushFdbDynamic()
    local fileName = "flush_fdb_only_dynamic"
    executeLocalConfig(luaTgfBuildConfigFileName(fileName))
end
flushFdbDynamic();
print("-------------------------------------------------")


