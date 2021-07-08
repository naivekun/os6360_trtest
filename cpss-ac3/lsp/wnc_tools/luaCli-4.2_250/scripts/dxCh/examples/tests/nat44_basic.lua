--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* nat44_basic.lua
--*
--* DESCRIPTION:
--*       The test for Basic NAT44: Modifying only IP addresses
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

local error = false

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]
-- this test is relevant for SIP_5_10 devices (BOBCAT 2 B0 and above)
SUPPORTED_SIP_DECLARE(devNum,"SIP_5_10")

local pktInfo
local payload
local status
local rc

--generate test packets
do
    status, payload = pcall(dofile, "dxCh/examples/packets/nat44_basic.lua")
    if not status then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end

executeLocalConfig("dxCh/examples/configurations/nat44_basic.txt")

-- test part: private to public udp packet
------------------------------------------
-- from port 2 to port 3
local transmitInfo = {portNum = port2 , pktInfo = {fullPacket = payload["private_to_public_udp"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , packetCount = 0 }
    ,{portNum = port3 , pktInfo = {fullPacket = payload["egress_private_to_public_udp"]}}
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
local transmitInfo = {portNum = port3 , pktInfo = {fullPacket = payload["public_to_private_udp"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , packetCount = 0 }
    ,{portNum = port2 , pktInfo = {fullPacket = payload["egress_public_to_private_udp"]}}
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

-- test part: private to public tcp packet
------------------------------------------
-- from port 2 to port 3
local transmitInfo = {portNum = port2 , pktInfo = {fullPacket = payload["private_to_public_tcp"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , packetCount = 0 }
    ,{portNum = port3 , pktInfo = {fullPacket = payload["egress_private_to_public_tcp"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nTCP Packet: The test part1 passed \n")
else
    printLog ("\nTCP Packet: The test part1 failed \n")
    error = true
end

-- test part: public to private tcp packet
------------------------------------------
-- from port 3 to port 2
local transmitInfo = {portNum = port3 , pktInfo = {fullPacket = payload["public_to_private_tcp"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , packetCount = 0 }
    ,{portNum = port2 , pktInfo = {fullPacket = payload["egress_public_to_private_tcp"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nTCP Packet: The test part2 passed \n")
else
    printLog ("\nTCP Packet: The test part2 failed \n")
    error = true
end

-- test part: private to public ipv4 packet
------------------------------------------
-- from port 2 to port 3
local transmitInfo = {portNum = port2 , pktInfo = {fullPacket = payload["private_to_public_ipv4"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , packetCount = 0 }
    ,{portNum = port3 , pktInfo = {fullPacket = payload["egress_private_to_public_ipv4"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nIPv4 Packet: The test part1 passed \n")
else
    printLog ("\nIPv4 Packet: The test part1 failed \n")
    error = true
end

-- test part: public to private ipv4 packet
------------------------------------------
-- from port 3 to port 2
local transmitInfo = {portNum = port3 , pktInfo = {fullPacket = payload["public_to_private_ipv4"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , packetCount = 0 }
    ,{portNum = port2 , pktInfo = {fullPacket = payload["egress_public_to_private_ipv4"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nIPv4 Packet: The test part2 passed \n")
else
    printLog ("\nIPv4 Packet: The test part2 failed \n")
    error = true
end

printLog("Restore configuration ...")
executeLocalConfig("dxCh/examples/configurations/nat44_basic_deconfig.txt")

if error == true then setFailState() end


