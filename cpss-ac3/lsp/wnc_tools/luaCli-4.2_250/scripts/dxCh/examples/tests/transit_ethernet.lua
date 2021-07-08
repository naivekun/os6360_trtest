--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* transit_ethernet.lua
--*
--* DESCRIPTION:
--*       The test for transit Ethernet packet with TTI VLAN manipulations
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5") 

local error = false

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

local pktInfo
local payload
local status
local rc


--generate test packets
do
    status, payload = pcall(dofile, "dxCh/examples/packets/transit_ethernet.lua")
    if not status then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end

executeLocalConfig("dxCh/examples/configurations/transit_ethernet.txt")

-- test part
--------------

-- from port 2 to port 3
local transmitInfo = {devNum = devNum, portNum = port2 ,
    pktInfo = {fullPacket = payload["ingress"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , packetCount = 0 }
    ,{portNum = port3 , pktInfo = {fullPacket = payload["egress"],}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nThe test passed \n")
else
    printLog ("\nThe test failed \n")
    error = true
end

printLog("Restore configuration ...")
executeLocalConfig("dxCh/examples/configurations/transit_ethernet_deconfig.txt")

if error == true then setFailState() end




