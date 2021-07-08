--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* acl_set_vpt.lua
--*
--* DESCRIPTION:
--*       The test for ACL set vpt feature
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

NOT_SUPPORTED_DEV_DECLARE(devNum, "CPSS_PP_FAMILY_CHEETAH_E")

local payloads

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]


local ret, status

ret, payloads = pcall(dofile, "dxCh/examples/packets/acl_set_vpt.lua")

--reset ports counters
resetPortCounters(devNum, port1)
resetPortCounters(devNum, port2)
resetPortCounters(devNum, port3)

--load configuration for test
executeLocalConfig("dxCh/examples/configurations/acl_set_vpt.txt")

local transmitInfo = {
    devNum = devNum,
    portNum = port3 ,
    pktInfo = {fullPacket = payloads["acl_set_vpt"]}
}

local egressInfoTable = {
    -- expected egress port         
    {portNum = port1, packetCount = 1, pktInfo = {fullPacket = payloads["new_vpt"]}},
    {portNum = port2, packetCount = 0 }
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
executeLocalConfig("dxCh/examples/configurations/acl_set_vpt_deconfig.txt")

