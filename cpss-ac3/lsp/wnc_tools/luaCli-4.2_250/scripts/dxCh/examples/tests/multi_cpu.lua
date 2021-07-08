--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* multi_cpu.lua
--*
--* DESCRIPTION:
--*       The test checks packets to/from CPU port 80,81,82 (BC3 only)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--
-- ACL matched and not matched packets
--
cmdLuaCLI_registerCfunction("prvLuaTgfTrafficGeneratorDefaultTxSdmaQueueSet")
local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

local aclNotMatchContent =
    "0000BBCCDDEE000000003402"..
    "333300000000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000"

local aclMatchContent =
    "00AABBCCDDEE000000003402"..
    "333300000000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000"

local test_scenario =
{
    no_matc =
    {
        transmitInfo =
        {
          devNum = devNum,
          portNum = port3, pktInfo = {fullPacket = aclNotMatchContent}
        },
        egressInfoTable =
        {
            {portNum = port1, pktInfo = {fullPacket = aclNotMatchContent}, packetCount = 1},
            {portNum = port2, packetCount = 1},
            {portNum = port4, packetCount = 1},
        }
    },
    matc =
    {
        transmitInfo =
        {
          devNum = devNum,
          portNum = port3, pktInfo = {fullPacket = aclMatchContent}
        },
        egressInfoTable =
        {
            {portNum = "CPU", pktInfo = {fullPacket = aclMatchContent, cpuCodeNumber = 195}},
            {portNum = port1, packetCount = 0},
            {portNum = port2, packetCount = 0},
            {portNum = port4, packetCount = 0},
        }
    }
}

-- local declaration
local configFile   = "dxCh/examples/configurations/second_cpu_port.txt"
local deconfigFile = "dxCh/examples/configurations/cpu_port_deconfig.txt"

local function trafficCheck(trafficScenario)
    local key, scenario;
    for key, scenario in pairs(trafficScenario) do
        print("========================================")
            print("Send a packet #".. tostring(key))
        local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
           scenario.transmitInfo, scenario.egressInfoTable);
        assert(
            rc==0, string.format("packet transmitting error: %s",
            returnCodes[rc]))
    end
end

------------------------------ MAIN PART ---------------------------------------
-- skip test for unsupported devices
SUPPORTED_DEV_DECLARE(devNum,"CPSS_PP_FAMILY_DXCH_ALDRIN2_E","CPSS_PP_FAMILY_DXCH_BOBCAT3_E")
SUPPORTED_FEATURE_DECLARE(devNum, "HARDWARE")

-- test the second cpu port
local isOk;
local rc = prvLuaTgfTrafficGeneratorDefaultTxSdmaQueueSet(9);
assert(
            rc==0, string.format("tx queue set error: %s",
            returnCodes[rc]))
isOk = luaTgfSimpleTest(configFile, deconfigFile, trafficCheck, test_scenario)
-- test the third cpu port
rc = prvLuaTgfTrafficGeneratorDefaultTxSdmaQueueSet(17);
assert(
            rc==0, string.format("tx queue set error: %s",
            returnCodes[rc]))
configFile   = "dxCh/examples/configurations/third_cpu_port.txt"
isOk = luaTgfSimpleTest(configFile, deconfigFile, trafficCheck, test_scenario)

-- test the forth cpu port
rc = prvLuaTgfTrafficGeneratorDefaultTxSdmaQueueSet(25);
assert(
            rc==0, string.format("tx queue set error: %s",
            returnCodes[rc]))
configFile   = "dxCh/examples/configurations/forth_cpu_port.txt"
isOk = luaTgfSimpleTest(configFile, deconfigFile, trafficCheck, test_scenario)

-- restore TX SDMA Queue to be 7
rc = prvLuaTgfTrafficGeneratorDefaultTxSdmaQueueSet(7);
assert(
            rc==0, string.format("tx queue set error: %s",
            returnCodes[rc]))

