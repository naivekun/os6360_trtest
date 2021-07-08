--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vpls_uni_to_nni.lua
--*
--* DESCRIPTION:
--*       The test for testing vpls uni to nni interface
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

local configuration_2 = [[
end
configure
interface ethernet ${dev}/${port[2]}
no nni vpls vsi 1000 assign-eport 653
nni vpls vsi 1000 assign-eport 653 pw-label 0x55552 in-tunnel-label 0x66662 out-tunnel-label 0x77772 pw-tag-mode raw-mode  p-tag 101 c-tag-state tagged nexthop-mac-addr 00:00:00:B0:1A:11
end
]]

local configuration_3 = [[
end
configure
interface ethernet ${dev}/${port[2]}
no nni vpls vsi 1000 assign-eport 653
nni vpls vsi 1000 assign-eport 653 pw-label 0x55552 in-tunnel-label 0x66662 out-tunnel-label 0x77772 pw-tag-mode raw-mode  p-tag 101 c-tag-state untagged nexthop-mac-addr 00:00:00:B0:1A:11
end
]]

local configuration_4 = [[
end
configure
interface ethernet ${dev}/${port[2]}
no nni vpls vsi 1000 assign-eport 653
nni vpls vsi 1000 assign-eport 653 pw-label 0x55552 in-tunnel-label 0x66662 out-tunnel-label 0x77772 pw-tag-mode tag-mode  p-tag 101 c-tag-state untagged nexthop-mac-addr 00:00:00:B0:1A:11
end
]]


executeLocalConfig("dxCh/examples/configurations/vpls_uni_to_nni.txt")

xcat3x_set_ingress_port_vpt(port1,0)
xcat3x_set_ingress_port_vpt(port2,0)
xcat3x_set_ingress_port_vpt(port3,0)
xcat3x_set_ingress_port_vpt(port4,0)

--generate vpls test packets
do
    status, payloads = pcall(dofile, "dxCh/examples/packets/vpls_uni_to_nni.lua")
    if not status then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end

local transmitInfo;
local egressInfoTable;
local rc = 0;

--configuration 1: NNI c-tag-state=tagged
printLog("===================== Test case 1: NNI c-tag-state=tagged ======================")

-- tagged packet to port 1, check on port 2
transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payloads["tagged_packet"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2  , pktInfo = {fullPacket = payloads["expected_double_tagged"] }}
}

-- transmit packet and check that egress as expected
printLog("====================== Test case 1.1: Send tagged packet =======================")
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end


-- untagged packet to port 1, check on port 2
transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payloads["untagged_packet"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2  , pktInfo = {fullPacket = payloads["expected_double_tagged_2"] }}
}

-- transmit packet and check that egress as expected
printLog("===================== Test case 1.2: Send untagged packet ======================")
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

--configuration 2: NNI pw-tag-mode=raw-mode
executeStringCliCommands(configuration_2)
printLog("==================== Test case 2: NNI pw-tag-mode=raw-mode =====================")

-- tagged packet to port 1, check on port 2
transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payloads["tagged_packet"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2  , pktInfo = {fullPacket = payloads["expected_single_tagged"] }}
}
-- transmit packet and check that egress as expected
printLog("====================== Test case 2.1: Send tagged packet =======================")
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

-- untagged packet to port 1, check on port 2
transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payloads["untagged_packet"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2  , pktInfo = {fullPacket = payloads["expected_single_tagged_2"] }}
}

-- transmit packet and check that egress as expected
printLog("===================== Test case 2.2: Send untagged packet ======================")
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

--configuration 3: NNI c-tag-state=untagged
executeStringCliCommands(configuration_3)
printLog("==================== Test case 3: NNI c-tag-state=untagged =====================")

-- tagged packet to port 1, check on port 2
transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payloads["tagged_packet"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2  , pktInfo = {fullPacket = payloads["expected_untagged"] }}
}
-- transmit packet and check that egress as expected
printLog("====================== Test case 3.1: Send tagged packet =======================")
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end


--configuration 4: NNI c-tag-state=untagged and pw-tag-mode=tag-mode
executeStringCliCommands(configuration_4)
printLog("======== Test case 4: NNI c-tag-state=untagged and pw-tag-mode=tag-mode ========")


-- untagged packet to port 1, check on port 2
transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payloads["untagged_packet"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2  , pktInfo = {fullPacket = payloads["expected_single_pw_tagged"] }}
}

-- transmit packet and check that egress as expected
printLog("===================== Test case 4.1: Send untagged packet ======================")
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

--configuration 5: send packet with different VID
printLog("================= Test case 5: send packet with different VID ==================")

-- tagged packet to port 1, check on port 2
transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payloads["tagged_packet_dif_vid"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2  , packetCount = 0 },
    {portNum = port3  , packetCount = 0 },
    {portNum = port4  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
printLog("====================== Test case 5.1: Send tagged packet =======================")
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

-- restore the defaults
xcat3x_set_ingress_port_vpt(port1,1)
xcat3x_set_ingress_port_vpt(port2,1)
xcat3x_set_ingress_port_vpt(port3,1)
xcat3x_set_ingress_port_vpt(port4,1)

printLog("Restore configuration")
executeLocalConfig("dxCh/examples/configurations/vpls_uni_to_nni_deconfig.txt")
