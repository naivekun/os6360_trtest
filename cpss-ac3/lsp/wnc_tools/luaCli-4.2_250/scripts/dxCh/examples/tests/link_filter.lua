--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* link_filter.lua
--*
--* DESCRIPTION:
--*       The test for Link state filter
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local payloads

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

local ret, status

--reset ports counters before force link configuration
--resetPortCounters forces link to be UP
resetPortCounters(devNum, port1)
resetPortCounters(devNum, port2)
resetPortCounters(devNum, port3)
resetPortCounters(devNum, port4)

local rc = prvLuaTgfPortForceLinkUpEnableSet(devEnv.dev, port1, false)
if rc ~= 0 then
    printLog ("prvLuaTgfPortForceLinkUpEnableSet failed \n")
    setFailState()
end

-- disable force automatic link up in LUA packet engine
global_do_not_force_link_UP = true

--use packet from ACL redirect test packets
ret, payloads = pcall(dofile, "dxCh/examples/packets/acl_redirect.lua")

--------------------------------------------------------------------------------
--load configuration for test
executeLocalConfig("dxCh/examples/configurations/link_filter.txt")

printLog("Sending not matched flooded traffic from port " ..  tostring(port3))

-- flooding from port 2 to all other ports. Port 1 must not send traffic.
transmitInfo = {
    devNum = devNum,
    portNum = port3 ,
    pktInfo = {fullPacket = payloads["aclNotMatch"]}
}
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port1, packetCount = 0 },
     {portNum = port2, packetCount = 1 },
     {portNum = port4, packetCount = 1 }
}
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/link_filter_deconfig.txt")

rc = prvLuaTgfPortForceLinkUpEnableSet(devEnv.dev, port1, true)
if rc ~= 0 then
    printLog ("prvLuaTgfPortForceLinkUpEnableSet failed \n")
    setFailState()
end

-- restore force automatic link up in LUA packet engine
global_do_not_force_link_UP = false

