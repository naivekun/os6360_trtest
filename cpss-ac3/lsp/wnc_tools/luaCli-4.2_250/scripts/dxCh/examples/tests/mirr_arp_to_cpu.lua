--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* mirr_arp_to_cpu.lua
--*
--* DESCRIPTION:
--*       The test checks mirroring traffic to CPU using LUA mechanism 
--*       and due to spesial tests configuration.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--******************************************************************************** 
local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

NOT_SUPPORTED_DEV_DECLARE(devNum, "CPSS_PP_FAMILY_CHEETAH_E") 

--reset ports counters
resetPortCounters(devNum, port1)
resetPortCounters(devNum, port2)
resetPortCounters(devNum, port3)
resetPortCounters(devNum, port4)

local arpBcPacket = 
    "ffffffffffff00070daff454"..
    "810000020806000108000604"..
    "000100070daff45418a6ac01"..
    "00000000000018a6ad9f0601"..
    "040000000002010003020000"..
    "050003010000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000"..
    "000000000000000000000000"..
    "0000000000000000"

-- flooding from port 3 to all other ports
transmitInfo = {
    devNum  = devNum,
    portNum = port1,
    pktInfo = {fullPacket = arpBcPacket}
}
egressMirrorInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = "CPU", pktInfo = {fullPacket = arpBcPacket, cpuCodeNumber = 5}},
     {portNum = port2, pktInfo = {fullPacket = arpBcPacket}, packetCount = 1},
     {portNum = port3, pktInfo = {fullPacket = arpBcPacket}, packetCount = 1}, 
     {portNum = port4, pktInfo = {fullPacket = arpBcPacket}, packetCount = 1}
}

egressTrapInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = "CPU", pktInfo = {fullPacket = arpBcPacket, cpuCodeNumber = 5}},
     {portNum = port2, packetCount = 0},
     {portNum = port3, packetCount = 0}, 
     {portNum = port4, packetCount = 0}
}

egressMirror1InfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = "CPU", pktInfo = {fullPacket = arpBcPacket, cpuCodeNumber = 5}},
     {portNum = port3, pktInfo = {fullPacket = arpBcPacket}, packetCount = 1}, 
}

-- function to implement the test
local function doTest()
    executeLocalConfig("dxCh/examples/configurations/mirr_arp_to_cpu_trap.txt")
    -- transmit packet and check that egress as expected
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressTrapInfoTable)
    if rc ~= 0 then
        printLog ("Trap packet test failed \n")
        setFailState()
    end

    executeLocalConfig("dxCh/examples/configurations/mirr_arp_to_cpu_mirror.txt")
    -- transmit packet and check that egress as expected
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressMirrorInfoTable)
    if rc ~= 0 then
        printLog ("Mirror packet test failed \n")
        setFailState()
    end

    executeLocalConfig("dxCh/examples/configurations/mirr_arp_to_cpu_mirror.txt")
    -- transmit packet and check that egress as expected
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressMirror1InfoTable)
    if rc ~= 0 then
        printLog ("Mirror packet test failed \n")
        setFailState()
    end

    if rc == 0 then
        printLog ("Test passed \n")
    end
   
end

local configFileName    = "mirr_arp_to_cpu"
local deconfigFileName  = "mirr_arp_to_cpu_deconfig"

-- run pre-test config
local function testConfig()
    --set config
    executeLocalConfig(luaTgfBuildConfigFileName(configFileName)) 
end

-- run pre-test config
local function testDeconfig()
    --set config
    executeLocalConfig(luaTgfBuildConfigFileName(deconfigFileName)) 
end

-- run pre-test config
testConfig()
-- run the test
doTest()
-- run the test deconfig
testDeconfig()

