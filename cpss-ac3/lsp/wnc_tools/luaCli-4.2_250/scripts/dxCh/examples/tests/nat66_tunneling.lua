--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* nat66_tunneling.lua
--*
--* DESCRIPTION:
--*       The test for NAT+Tunneling:
--*       For traffic from private to public the packet is tunneled after the NAT modification => ePort-out is associated with MPLS TS
--*       For traffic from public to private the packet is TT before the NAT modification => ePort-in is assigned by the TTI (MPLS)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

dofile ("dxCh/examples/common/nat66_utils.lua") -- show_nat66_test function declaration

local error = false

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]
-- this test is relevant for SIP_5_15 devices (BobK)
SUPPORTED_SIP_DECLARE(devNum,"SIP_5_15")

local pktInfo
local payload
local status
local rc

--generate test packets
do
    status, payload = pcall(dofile, "dxCh/examples/packets/nat66_tunneling.lua")
    if not status then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end

local devFamily, devSubFamily = wrlCpssDeviceFamilyGet(devNum)
if "CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily then
    global_test_data_env.localEPort = 0x218
    global_test_data_env.outerEPort = 0x236
else
    global_test_data_env.localEPort = 0x1018
    global_test_data_env.outerEPort = 0x1036
end

executeLocalConfig("dxCh/examples/configurations/nat66_tunneling.txt")

show_nat66_test()

-- test part: private to public udp packet
------------------------------------------
-- from port 2 to port 3
local transmitInfo = {portNum = port2 , pktInfo = {fullPacket = payload["private_to_public_packet"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , packetCount = 0 }
    ,{portNum = port3 , pktInfo = {fullPacket = payload["egress_private_to_public_packet"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nUDP Packet: The test part1 passed \n")
else
    printLog ("\nUDP Packet: The test part1 failed \n")
    error = true
end

-- test part: public to private udp packet
------------------------------------------
-- from port 3 to port 2
local transmitInfo = {portNum = port3 , pktInfo = {fullPacket = payload["public_to_private_packet"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , packetCount = 0 }
    ,{portNum = port2 , pktInfo = {fullPacket = payload["egress_public_to_private_packet"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nUDP Packet: The test part2 passed \n")
else
    printLog ("\nUDP Packet: The test part2 failed \n")
    error = true
end

printLog("Restore configuration ...")
executeLocalConfig("dxCh/examples/configurations/nat66_tunneling_deconfig.txt")

if error == true then setFailState() end
