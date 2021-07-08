--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* policer_dscp_remark.lua
--*
--* DESCRIPTION:
--*       Remark DSCP of packets by Policer
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5")

local payloads

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

local ret, status

--generate DSCP Remarking test packets

ret, payloads = pcall(dofile, "dxCh/examples/packets/policer_dscp_remark.lua")

--reset ports counters 
resetPortCounters(devNum, port1) 
resetPortCounters(devNum, port2) 
resetPortCounters(devNum, port3) 
resetPortCounters(devNum, port4) 

--------------------------------------------------------------------------------
--load configuration for test
executeLocalConfig("dxCh/examples/configurations/policer_dscp_remark.txt")

printLog("Sending DSCP traffic from port " ..  tostring(port3))

-- Sending from port 2 to port 0 
local transmitInfo = {
  devNum = devNum,
  portNum = port3, 
  pktInfo = {fullPacket = payloads["policerDscpRemark"]}
}
local egressInfoTable = {
    {portNum = port1, pktInfo = {fullPacket = payloads["policerDscpRemarkExpected"]}, packetCount = 1},
    {portNum = port2, packetCount = 0},
    {portNum = port4, packetCount = 0}, 
}

-- transmit packet and check that egress as expected 
local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable) 
if rc == 0 then 
    printLog ("Test pass \n") 
else 
    printLog ("Test failed \n")
    setFailState()
end 

-------------------------------------------------------------------------------- 
printLog("Restore configuration ...") 
executeLocalConfig("dxCh/examples/configurations/policer_dscp_remark_deconfig.txt")