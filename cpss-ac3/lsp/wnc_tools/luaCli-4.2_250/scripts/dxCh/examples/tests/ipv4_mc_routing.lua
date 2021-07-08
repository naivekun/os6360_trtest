--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ipv4_mc_routing.lua
--*
--* DESCRIPTION:
--*       The test for mc routing
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

local pkt
local pktInfo
local payloads
local status

SUPPORTED_FEATURE_DECLARE(devNum, "IP_MC_ROUTING")

-- skip test for LION and LION2 devices 
NOT_SUPPORTED_DEV_DECLARE(devNum,"CPSS_PP_FAMILY_DXCH_LION_E","CPSS_PP_FAMILY_DXCH_LION2_E")

--generate test packets
do
    status, payloads = pcall(dofile, "dxCh/examples/packets/ipv4_mc_routing.lua")
    if not status then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end 


printLog("============================== Set configuration ===============================")
executeLocalConfig("dxCh/examples/configurations/ipv4_mc_routing.txt")

local transmitInfo;
local egressInfoTable;
local rc = 0;

printLog("================ Test case 1: Send packet with correct header ==================")

-- IPv4 packet to port 1, check on all ports
transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payloads["sent_correct_header_packet"]} }
egressInfoTable = {
    {portNum = port2  , packetCount = 1 },
    {portNum = port3  , packetCount = 0 },
    {portNum = port4  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

-- read IPvX counters
print("Number of ingress multicast packets that were not routed due to one or more exceptions: ")
result, value = myGenWrapper(
    "cpssDxChIpCntGet", {
    { "IN",     "GT_U8",                        "devNum",   devNum },
    { "IN",     "CPSS_IP_CNT_SET_ENT",          "cntSet",   "CPSS_IP_CNT_SET0_E" },
    { "OUT",    "CPSS_DXCH_IP_COUNTER_SET_STC", "countersPtr"}
})
if result ~= 0 then
    print("ERROR in cpssDxChIpCntGet")
    error = true
end
print("    inMcNonRoutedExcpPkts: "..to_string(value.countersPtr.inMcNonRoutedExcpPkts))
if tonumber(value.countersPtr.inMcNonRoutedExcpPkts) ~= 0 then
    print("ERROR in inMcNonRoutedExcpPkts")
    error = true
end

local countersPtr = {
    inMcNonRoutedExcpPkts = 0,
}
-- clear IPvX counters
print("Number of ingress multicast packets that were not routed due to one or more exceptions: ")
result, value = myGenWrapper(
    "cpssDxChIpCntSet", {
    { "IN",     "GT_U8",                        "devNum",   devNum },
    { "IN",     "CPSS_IP_CNT_SET_ENT",          "cntSet",   "CPSS_IP_CNT_SET0_E" },
    { "IN",     "CPSS_DXCH_IP_COUNTER_SET_STC", "countersPtr", countersPtr}
})
if result ~= 0 then
    print("ERROR in cpssDxChIpCntSet")
    error = true
end

printLog("================= Test case 2: Send packet with wrong header ===================")

-- IPv4 packet to port 1, check on all ports
transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payloads["sent_wrong_header_packet"]} }
egressInfoTable = {
    {portNum = port2  , packetCount = 0 },
    {portNum = port3  , packetCount = 0 },
    {portNum = port4  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

-- read IPvX counters
print("Number of ingress multicast packets that were not routed due to one or more exceptions: ")
result, value = myGenWrapper(
    "cpssDxChIpCntGet", {
    { "IN",     "GT_U8",                        "devNum",   devNum },
    { "IN",     "CPSS_IP_CNT_SET_ENT",          "cntSet",   "CPSS_IP_CNT_SET0_E" },
    { "OUT",    "CPSS_DXCH_IP_COUNTER_SET_STC", "countersPtr"}
})
if result ~= 0 then
    print("ERROR in cpssDxChIpCntGet")
    error = true
end
print("    inMcNonRoutedExcpPkts: "..to_string(value.countersPtr.inMcNonRoutedExcpPkts))
if tonumber(value.countersPtr.inMcNonRoutedExcpPkts) ~= 1 then
    print("ERROR in inMcNonRoutedExcpPkts")
    error = true
end

local countersPtr = {
    inMcNonRoutedExcpPkts = 0,
}
-- clear IPvX counters
print("Number of ingress multicast packets that were not routed due to one or more exceptions: ")
result, value = myGenWrapper(
    "cpssDxChIpCntSet", {
    { "IN",     "GT_U8",                        "devNum",   devNum },
    { "IN",     "CPSS_IP_CNT_SET_ENT",          "cntSet",   "CPSS_IP_CNT_SET0_E" },
    { "IN",     "CPSS_DXCH_IP_COUNTER_SET_STC", "countersPtr", countersPtr}
})
if result ~= 0 then
    print("ERROR in cpssDxChIpCntSet")
    error = true
end

printLog("============================ Restore configuration =============================")
executeLocalConfig("dxCh/examples/configurations/ipv4_mc_routing_deconfig.txt")
