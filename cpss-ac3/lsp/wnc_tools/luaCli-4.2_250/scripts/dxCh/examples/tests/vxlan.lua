--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vxlan.lua
--*
--* DESCRIPTION:
--*       The test for testing vxlan uni and nni
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5") 


local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

local pkt
local pktInfo
local payloads
local status

--generate vxlan test packets
do
    status, payloads = pcall(dofile, "dxCh/examples/packets/vxlan.lua")
    if not status then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end

executeLocalConfig("dxCh/examples/configurations/vxlan.txt")

-- for vlan untagged packet
local my_maskedBytesTable = {
    {startByte = 18, endByte = 19 }-- 2 bytes 'identification field'
    ,{startByte = 24, endByte = 25 }-- 2 bytes 'checksum'
    ,{startByte = 34, endByte = 35 }-- 2 bytes UDP Source port
}

local transmitInfo;
local egressInfoTable;
local rc = 0;

-- from port 1 to port 3
transmitInfo = {devNum = devNum, portNum = port1 ,
    pktInfo = {fullPacket = payloads["direction1_from_port1"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2  ,
        pktInfo = {fullPacket = payloads["direction1_expected_port_2"] }}
    ,{portNum = port3 ,
        pktInfo = {fullPacket = payloads["direction1_expected_port_3"],
            maskedBytesTable = my_maskedBytesTable}}
    ,{portNum = port4 ,
        pktInfo = {fullPacket = payloads["direction1_expected_port_4"],
            maskedBytesTable = my_maskedBytesTable} }
}
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

-- from port 3 to port 1
transmitInfo = {devNum = devNum, portNum = port3 ,
    pktInfo = {fullPacket = payloads["direction2_from_port3"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , pktInfo = {fullPacket = payloads["direction2_expected_port_1"] }}
    ,{portNum = port2 , pktInfo = {fullPacket = payloads["direction2_expected_port_2"], maskedBytesTable = my_maskedBytesTable}}
    ,{portNum = port4 , packetCount = 0} -- filtered as the same mesh member
}
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

executeLocalConfig("dxCh/examples/configurations/vxlan_deconfig.txt")



