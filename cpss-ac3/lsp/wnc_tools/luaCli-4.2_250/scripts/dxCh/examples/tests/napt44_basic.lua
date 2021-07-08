--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* napt44_basic.lua
--*
--* DESCRIPTION:
--*       The test for Basic NAPT44: Modifying IP addresses and L4 ports
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
    status, payload = pcall(dofile, "dxCh/examples/packets/napt44_basic.lua")
    if not status then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end

executeLocalConfig("dxCh/examples/configurations/napt44_basic.txt")

-- test part: private to public udp packet
------------------------------------------
-- from port 2 to port 3
local transmitInfo = {
    devNum = devNum,
    portNum = port2,
    pktInfo = {fullPacket = payload["private_to_public_udp_2_to_3"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , packetCount = 0 }
    ,{portNum = port3 , pktInfo = {fullPacket = payload["egress_private_to_public_udp_2_to_3"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nUDP Packet: The test private port 2 to public port 3 passed \n")
else
    printLog ("\nUDP Packet: The test private port 2 to public port 3 failed \n")
    error = true
end

-- test part: public to private udp packet
------------------------------------------
-- from port 3 to port 2
local transmitInfo = {
    devNum = devNum,
    portNum = port3,
    pktInfo = {fullPacket = payload["public_to_private_udp_3_to_2"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , packetCount = 0 }
    ,{portNum = port2 , pktInfo = {fullPacket = payload["egress_public_to_private_udp_3_to_2"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nUDP Packet: The test public port 3 to private port 2 passed \n")
else
    printLog ("\nUDP Packet: The test public port 3 to private port 2 failed \n")
    error = true
end


-- test part: private to public udp packet
------------------------------------------
-- from port 1 to port 3
local transmitInfo = {
    devNum = devNum,
    portNum = port1,
    pktInfo = {fullPacket = payload["private_to_public_udp_1_to_3"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2  , packetCount = 0 }
    ,{portNum = port3 , pktInfo = {fullPacket = payload["egress_private_to_public_udp_1_to_3"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nUDP Packet: The test private port 1 to public port 3 passed \n")
else
    printLog ("\nUDP Packet: The test private port 1 to public port 3 failed \n")
    error = true
end

-- test part: public to private udp packet
------------------------------------------
-- from port 3 to port 1
local transmitInfo = {
    devNum = devNum,
    portNum = port3,
    pktInfo = {fullPacket = payload["public_to_private_udp_3_to_1"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1 , pktInfo = {fullPacket = payload["egress_public_to_private_udp_3_to_1"]}}
    ,{portNum = port2  , packetCount = 0 }
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nUDP Packet: The test public port 3 to private port 1 passed \n")
else
    printLog ("\nUDP Packet: The test public port 3 to private port 1 failed \n")
    error = true
end


-- test part: private to public tcp packet
------------------------------------------
-- from port 2 to port 3
local transmitInfo = {
    devNum = devNum,
    portNum = port2,
    pktInfo = {fullPacket = payload["private_to_public_tcp_2_to_3"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , packetCount = 0 }
    ,{portNum = port3 , pktInfo = {fullPacket = payload["egress_private_to_public_tcp_2_to_3"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nTCP Packet: The test private port 2 to public port 3 passed \n")
else
    printLog ("\nTCP Packet: The test private port 2 to public port 3 failed \n")
    error = true
end

-- test part: public to private tcp packet
------------------------------------------
-- from port 3 to port 2
local transmitInfo = {
    devNum = devNum,
    portNum = port3,
    pktInfo = {fullPacket = payload["public_to_private_tcp_3_to_2"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , packetCount = 0 }
    ,{portNum = port2 , pktInfo = {fullPacket = payload["egress_public_to_private_tcp_3_to_2"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nTCP Packet: The test public port 3 to private port 2 passed \n")
else
    printLog ("\nTCP Packet: The test public port 3 to private port 2 failed \n")
    error = true
end

-- test part: private to private Ipv4 packet
--------------------------------------------
-- from port 2 to port 4
local transmitInfo = {
    devNum = devNum,
    portNum = port2,
    pktInfo = {fullPacket = payload["private_to_private_2_to_4"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , packetCount = 0 }
    ,{portNum = port3  , packetCount = 0 }
    ,{portNum = port4 , pktInfo = {fullPacket = payload["egress_private_to_private_2_to_4"]}}
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nIpv4 Packet: The test private port 2 to private port 4 passed \n")
else
    printLog ("\nIpv4 Packet: The test private port 2 to private port 4 failed \n")
    error = true
end

-- test part: private to private Ipv4 packet
--------------------------------------------
-- from port 4 to port 2
local transmitInfo = {
    devNum = devNum,
    portNum = port4,
    pktInfo = {fullPacket = payload["private_to_private_4_to_2"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , packetCount = 0 }
    ,{portNum = port2 , pktInfo = {fullPacket = payload["egress_private_to_private_4_to_2"]}}
    ,{portNum = port3  , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nIpv4 Packet: The test private port 4 to private port 2 passed \n")
else
    printLog ("\nIpv4 Packet: The test private port 4 to private port 2 failed \n")
    error = true
end

-- test part: private to private Ipv4 packet
--------------------------------------------
-- from port 2 to port 1
local transmitInfo = {
    devNum = devNum,
    portNum = port2,
    pktInfo = {fullPacket = payload["private_to_private_2_to_1"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1 , pktInfo = {fullPacket = payload["egress_private_to_private_2_to_1"]}}
    ,{portNum = port3  , packetCount = 0 }
    ,{portNum = port4  , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nIpv4 Packet: The test private port 2 to private port 1 passed \n")
else
    printLog ("\nIpv4 Packet: The test private port 2 to private port 1 failed \n")
    error = true
end

-- test part: private to private Ipv4 packet
--------------------------------------------
-- from port 1 to port 2
local transmitInfo = {
    devNum = devNum,
    portNum = port1,
    pktInfo = {fullPacket = payload["private_to_private_1_to_2"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2 , pktInfo = {fullPacket = payload["egress_private_to_private_1_to_2"]}}
    ,{portNum = port3  , packetCount = 0 }
    ,{portNum = port4  , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nIpv4 Packet: The test private port 1 to private port 2 passed \n")
else
    printLog ("\nIpv4 Packet: The test private port 1 to private port 2 failed \n")
    error = true
end

printLog("Restore configuration ...")
executeLocalConfig("dxCh/examples/configurations/napt44_basic_deconfig.txt")

if error == true then setFailState() end


