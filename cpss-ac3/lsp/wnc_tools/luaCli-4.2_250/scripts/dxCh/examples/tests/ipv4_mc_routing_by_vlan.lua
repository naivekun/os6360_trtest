--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ipv4_mc_routing_by_vlan.lua
--*
--* DESCRIPTION:
--*       The test for ipv4 mc routing by vlan feature. first case is when feature
--*       is turned off and routing is expected according to vrf-id associated with
--*       incoming packet's VID entry. second case is when the feature is turned on and
--*       routing is expected according to vrf-id which is equal to incoming packet's
--*       VID.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev
local devFamily, subFamily = wrlCpssDeviceFamilyGet(devNum) 
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

local pkt
local pktInfo
local payload
local status

local transmitInfo;
local egressInfoTable;
local rc = 0;

local countersPtr = {inMcNonRoutedExcpPkts = 0}


-- skip test for unsupported devices
if(not is_sip_5(devNum) and "CPSS_PP_FAMILY_DXCH_XCAT3_E" ~= devFamily) then
    setTestStateSkipped() 
    return
end

-- skip test if xCat3 revision <= 3
if ("CPSS_PP_FAMILY_DXCH_XCAT3_E" == devFamily and wrlCpssDeviceRevisionGet(devNum) <= 3) then
    setTestStateSkipped() 
    return
end

-- skip test if device is not Bobk
if ("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily and "CPSS_PP_SUB_FAMILY_NONE_E" == subFamily) then
    setTestStateSkipped() 
    return
end


--generate test packet
do
    status, payload = pcall(dofile, "dxCh/examples/packets/ipv4_mc_routing_by_vlan.lua")
    if not status then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end

printLog("========================== Set configuration ===========================")

executeLocalConfig("dxCh/examples/configurations/ipv4_mc_routing_by_vlan.txt")

printLog("Test case 1: routing key to LPM is VRF-ID of ingress packet VLAN entry.")

executeLocalConfig("dxCh/examples/configurations/ipv4_mc_routing_by_vlan_case_by_vrf-id.txt")

-- IPv4 packet to port 1, check on all ports.
-- we expect packet to egress out port 2 according to VRF-ID 500

transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payload} }

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

-- clear IPvX counters

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

printLog("Test case 2: routing key to LPM is VID of ingress packet.")

executeLocalConfig("dxCh/examples/configurations/ipv4_mc_routing_by_vlan_case_by_vlan.txt")

-- IPv4 packet to port 1, check on all ports.
-- we expect packet to egress out port 3 according to VRF-ID 100

egressInfoTable = {
    {portNum = port2  , packetCount = 0 },
    {portNum = port3  , packetCount = 1 },
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

-- clear IPvX counters

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

printLog("======================== Restore configuration =========================")
executeLocalConfig("dxCh/examples/configurations/ipv4_mc_routing_by_vlan_deconfig.txt")
