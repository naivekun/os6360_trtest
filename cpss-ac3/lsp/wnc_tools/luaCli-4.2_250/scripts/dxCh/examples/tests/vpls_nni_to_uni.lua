--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vpls_nni_to_uni.lua
--*
--* DESCRIPTION:
--*       The test for testing vpls nni to uni interface
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
    status, payloads = pcall(dofile, "dxCh/examples/packets/vpls_nni_to_uni.lua")
    if not status then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end

printLog("============================== Set configuration ===============================")
executeLocalConfig("dxCh/examples/configurations/vpls_nni_to_uni.txt")

local configuration_2 = [[
end
configure
interface ethernet ${dev}/${port[1]}
no uni vid 20 src-mac 00:00:00:24:00:02
uni vsi 1000 assign-eport 662 vid 20 src-mac 00:00:00:24:00:02 tag-state untagged
end
]] 

local configuration_3 = [[
end
configure
interface ethernet ${dev}/${port[1]}
no uni vid 20 src-mac 00:00:00:24:00:02
uni vsi 1000 assign-eport 662 vid 20 src-mac 00:00:00:24:00:02 tag-state tagged
end
]]

local configuration_4 = [[
end
configure
interface ethernet ${dev}/${port[2]}
no nni vpls vsi 1000 assign-eport 653
nni vpls vsi 1000 assign-eport 653 eport-pvid 300 pw-label 0x55552 in-tunnel-label 0x66662 out-tunnel-label 0x77772 pw-tag-mode raw-mode p-tag 101 c-tag-state tagged nexthop-mac-addr 00:00:00:B0:1A:11
end
]]

local transmitInfo;
local egressInfoTable;
local rc = 0;

printLog("================ Test case 1: UNI: tagged. Packet: c-tag, p-tag ================")
--configuration 1: UNI tag-state=tagged

-- tagged packet from port 2 to port 1
transmitInfo = {devNum = devNum, portNum = port2 , pktInfo = {fullPacket = payloads["ctagged_ptagged_packet"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , pktInfo = {fullPacket = payloads["expected_tagged"] }}
}
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

printLog("=============== Test case 2: UNI: untagged. Packet: c-tag, p-tag ===============")
--configuration 2: UNI tag-state=untagged
executeStringCliCommands(configuration_2)

egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , pktInfo = {fullPacket = payloads["expected_untagged"] }}
}
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

printLog("================== Test case 3: UNI untagged. Packet: p-tag ====================")
-- tagged packet from port 2 to port 1
transmitInfo = {devNum = devNum, portNum = port2 , pktInfo = {fullPacket = payloads["ptagged_packet"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , pktInfo = {fullPacket = payloads["expected_untagged"] }}
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end
--[[
printLog("=================== Test case 4: UNI tagged. Packet: p-tag =====================")
--configuration 3: UNI tag-state=tagged
executeStringCliCommands(configuration_3)
transmitInfo = {devNum = devNum, portNum = port2 , pktInfo = {fullPacket = payloads["ptagged_packet"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , pktInfo = {fullPacket = payloads["expected_dif_tagged"] }}
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end
]]
printLog("================== Test case 5: UNI tagged. Packet: untagged ===================")
-- tagged packet from port 2 to port 1
transmitInfo = {devNum = devNum, portNum = port2 , pktInfo = {fullPacket = payloads["untagged_packet"]} }
egressInfoTable = {
    -- check packet dropped
    {portNum = port1, packetCount = 0},
    {portNum = port3, packetCount = 0},
    {portNum = port4, packetCount = 0}
}
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end
--[[
printLog("================= Test case 6: NNI: raw-mode. Packet: untagged =================")
--configuration 4: NNI pw-tag-mode=raw-mode
executeStringCliCommands(configuration_4)

transmitInfo = {devNum = devNum, portNum = port2 , pktInfo = {fullPacket = payloads["untagged_packet"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , pktInfo = {fullPacket = payloads["expected_dif_tagged"] }}
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end
]]
printLog("============================ Restore configuration =============================")
executeLocalConfig("dxCh/examples/configurations/vpls_nni_to_uni_deconfig.txt")
