--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* mpls_entropy_label_demo.lua
--*
--* DESCRIPTION:
--*       The test for entropy label demo. 
--*       MPLS header and Entropy label will be added to egress packet 
--*       Egress packet ingress the same port that egress from (loopback)
--*       Then packet egress trunk according to load balancing 
--*       done due to different entropy labels
--*       
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

cmdLuaCLI_registerCfunction("prvLuaTgfPortLoopbackModeEnableSet")

local error = false

dofile ("dxCh/examples/common/mpls_utils.lua") -- show_mpls_test and show_mpls_transit_test functions declaration

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

-- set/unset port as MAC loopback
local function setLoopbackPort(devNum,portNum,enable)
    local rc = prvLuaTgfPortLoopbackModeEnableSet(devNum,portNum,enable)

    if rc ~= 0 then
        testAddErrorString("Unable to set {dev/port} [" ..devNum .."/"..portNum.."] with MAC loopback set to: " .. to_string(enable))
    end

end

--generate test packets
do
    status, payload = pcall(dofile, "dxCh/examples/packets/mpls_entropy_label_demo.lua")
    if not status then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end

executeLocalConfig("dxCh/examples/configurations/mpls_entropy_label_demo.txt")

show_mpls_test()
show_mpls_transit_test()

-- set the port into MAC loopback.
setLoopbackPort(devNum,port2,true)

-- test part1: egress 3 tunnel label and entropy label
------------------------------------------------------
-- from port 1 to port 3
local transmitInfo = {portNum = port1 , pktInfo = {fullPacket = payload["ethernet"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port3, pktInfo = {fullPacket = payload["egress_entropy_label_1"]}},
    {portNum = port4, packetCount = 0}
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nThe test part1 passed \n")
else
    printLog ("\nThe test part1 failed \n")
    error = true
end

-- test part2: egress 4 tunnel label and entropy label
-----------------------------------------------------------------
-- from port 1 to port 4
local transmitInfo = {portNum = port1 , pktInfo = {fullPacket = payload["ethernet_2"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
   -- {portNum = port2, packetCount = 1},
    {portNum = port3, packetCount = 0},
    {portNum = port4 , pktInfo = {fullPacket = payload["egress_entropy_label_2"]}}
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nThe test part2 passed \n")
else
    printLog ("\nThe test part2 failed \n")
    error = true
end

-- unset the port into MAC loopback.
setLoopbackPort(devNum,port2,false)

printLog("Restore configuration ...")
executeLocalConfig("dxCh/examples/configurations/mpls_entropy_label_demo_deconfig.txt")

if error == true then setFailState() end
