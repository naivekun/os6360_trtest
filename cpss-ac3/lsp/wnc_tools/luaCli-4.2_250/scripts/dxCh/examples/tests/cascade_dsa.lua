--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cascade_dsa.lua
--*
--* DESCRIPTION:
--*       The test for cascade DSA feature
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local payloads

local devNum  = devEnv.dev
--- AC3X need proper ports for this test
-- only devEnv.port[2] must support the "DSA_CASCADE"
-- the call to SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM may change the value of devEnv.port[3] !!!
SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM(devNum, "DSA_CASCADE" , "ports" , {devEnv.port[3]}) 


local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

local ret, status

--generate cascade test packets

ret, payloads = pcall(dofile, "dxCh/examples/packets/cascade_dsa.lua")

--reset ports counters
resetPortCounters(devNum, port1)
resetPortCounters(devNum, port2)
resetPortCounters(devNum, port3)
resetPortCounters(devNum, port4)

--------------------------------------------------------------------------------
--load configuration for test
executeLocalConfig("dxCh/examples/configurations/cascade_dsa.txt")

printLog("Sending traffic from port " ..  tostring(port2))

-- sending from port 2, expect to get output on port 3
transmitInfo = {
    devNum = devNum,
    portNum = port2 ,
    pktInfo = {fullPacket = payloads["cascadeDsa"]}
}
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port1, packetCount = 0 },
--     {portNum = port2, packetCount = 0 },
     {portNum = port3, packetCount = 1 },
     {portNum = port4, packetCount = 0 }
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
executeLocalConfig("dxCh/examples/configurations/cascade_dsa_deconfig.txt")

