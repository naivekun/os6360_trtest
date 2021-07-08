--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* capwap_ipv6_ap_to_network.lua
--*
--* DESCRIPTION:
--*       The test for testing capwap ipv6 ap_to_network feature
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
-- this test is relevant for SIP5 devices (BOBCAT 2 and above)
SUPPORTED_SIP_DECLARE(devNum,"SIP_5")

local pktInfo
local payloads

local isError = false

--generate capwap test packets
do
    status, payloads = pcall(dofile, "dxCh/examples/packets/capwap_ipv6_ap_to_network.lua")
    if not status then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end
--printLog("payload is "..payloads["direction1"])

--------------------------------------------------------------------------------
executeLocalConfig("dxCh/examples/configurations/capwap_ipv6_ap_to_network.txt")

--------------------------------------------------------------------------------
local sendPort   = port2
local egressPort = port4
local ingressPacket = payloads["direction1"]
local egressPacket = payloads["direction1_expected"]

local transmitInfo = {portNum = sendPort , pktInfo = {fullPacket = ingressPacket} }
local egressInfoTable = {
    -- this port need to get traffic
    {portNum = egressPort , pktInfo = {fullPacket = egressPacket} },
    -- other port should not
    {portNum = port1  , packetCount = 0},
    {portNum = port3 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if  rc == 0 then
    printLog ("PART1 pass\n")
else
    printLog ("PART1 fail\n")
    isError = true
end

--------------------------------------------------------------------------------
-- Check Control Rule --
ingressPacket = payloads["direction1_control"]
egressPacket = payloads["direction1_controlExpected"]

transmitInfo = {portNum = sendPort , pktInfo = {fullPacket = ingressPacket} }
egressInfoTable = {
    {portNum = port1  , packetCount = 0},
    {portNum = port3 , packetCount = 0 },
    {portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if  rc == 0 then
    printLog ("PART1_control pass\n")
else
    printLog ("PART1_control fail\n")
    isError = true
end

--------------------------------------------------------------------------------
-- Check Default Rule --
ingressPacket = payloads["direction1_default"]
egressPacket = payloads["direction1_defaultExpected"]

transmitInfo = {portNum = sendPort , pktInfo = {fullPacket = ingressPacket} }
egressInfoTable = {
    {portNum = port1  , packetCount = 0},
    {portNum = port3 , packetCount = 0 },
    {portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if  rc == 0 then
    printLog ("PART1_default pass\n")
else
    printLog ("PART1_default fail\n")
    isError = true
end


--------------------------------------------------------------------------------
sendPort   = port4
egressPort = port2
ingressPacket = payloads["direction2"]
egressPacket = payloads["direction2_expected"]

-- mask the 'identification field' and the 'checkseum' in the ipv4 header ... as the TS will generate different
-- 'identification field' every time
local my_maskedBytesTable = {
    {startByte = 22, endByte = 23 },-- 2 bytes 'identification field'
    {startByte = 58, endByte = 59 },-- 2 bytes 'srcUdpPort'
    {startByte = 64, endByte = 65 } -- 2 bytes 'checksum'
}

transmitInfo = {portNum = sendPort , pktInfo = {fullPacket = ingressPacket} }
egressInfoTable = {
    -- this port need to get traffic
    {portNum = egressPort , pktInfo = {fullPacket = egressPacket,maskedBytesTable = my_maskedBytesTable}},
    -- other port should not
    {portNum = port1  , packetCount = 0},
    {portNum = port3 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if  rc == 0 then
    printLog ("PART2 pass\n")
else
    printLog ("PART2 fail\n")
    isError = true
end

printLog("Restore configuration ...")
executeLocalConfig("dxCh/examples/configurations/capwap_ipv6_ap_to_network_deconfig.txt")
executeLocalConfig("dxCh/examples/configurations/flush_fdb.txt")

if isError == true then setFailState() end