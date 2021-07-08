--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vpls_uni_to_uni.lua
--*
--* DESCRIPTION:
--*       The test for testing vpls uni to uni interface
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

SUPPORTED_FEATURE_DECLARE(devNum, "VPLS")

local pkt
local pktInfo
local payloads
local status

--generate vpls test packets
do
    status, payloads = pcall(dofile, "dxCh/examples/packets/vpls_uni_to_uni.lua")
    if not status then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end

printLog("============================== Set configuration ===============================")
executeLocalConfig("dxCh/examples/configurations/vpls_uni_to_uni.txt")

local configuration_2 = [[
end
configure
interface ethernet ${dev}/${port[2]}
no uni vid 30 src-mac 00:00:00:24:00:03
uni vsi 1000 assign-eport 663 eport-pvid 230 vid 30 src-mac 00:00:00:24:00:03 tag-state untagged
end
]] 

local configuration_3 = [[
end
configure
interface ethernet ${dev}/${port[2]}
no uni vid 30 src-mac 00:00:00:24:00:03
uni vsi 1000 assign-eport 663 eport-pvid 230 vid 30 src-mac 00:00:00:24:00:03 tag-state tagged 
end
]]

local transmitInfo;
local egressInfoTable;
local rc = 0;

transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payloads["tagged_packet"]} }

--configuration 1: UNI tag-state=tagged
printLog("====================== Test case 1: UNI tag-state=tagged =======================")

printLog("====================== Test case 1.1: Send tagged packet =======================")

-- tagged packet to port 1, check on port 2
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2  , pktInfo = {fullPacket = payloads["expected_tagged"] }}
}
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

--configuration 2: UNI tag-state=untagged
printLog("===================== Test case 2: UNI tag-state=untagged ======================")

printLog("====================== Test case 2.1: Send tagged packet =======================")
executeStringCliCommands(configuration_2)

egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2  , pktInfo = {fullPacket = payloads["expected_untagged"] }}
}
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

printLog("===================== Test case 2.2: Send untagged packet ======================")
-- tagged packet to port 1, check on port 2
transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payloads["untagged_packet"]} }

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

--configuration 3: UNI tag-state=tagged
executeStringCliCommands(configuration_3)
printLog("====================== Test case 3: UNI tag-state=tagged =======================")

printLog("===================== Test case 3.1: Send untagged packet ======================")
transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payloads["untagged_packet"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2  , pktInfo = {fullPacket = payloads["expected_dif_tagged"] }}
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

--configuration 4: send packet with different VID
printLog("Test case 4: send packet with different VID")
printLog("================ Test case 4: send packet with different VID =================")

printLog("===================== Test case 4.1: Send tagged packet ======================")

-- tagged packet to port 1, check on port 2
transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payloads["tagged_packet_different_vid"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2  , packetCount = 0 },
    {portNum = port3  , packetCount = 0 },
    {portNum = port4  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
printLog("Send tagged packet")
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end


printLog("============================ Restore configuration =============================")
executeLocalConfig("dxCh/examples/configurations/vpls_uni_to_uni_deconfig.txt")
