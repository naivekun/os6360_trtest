--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cnc_acl.lua
--*
--* DESCRIPTION:
--*       The test for testing CNC acl
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local sendData
local counters1, counters2
local ret, status, val
local stdout = ""
local numberOfPackets, numberOfBytes
local ruleID = 6

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5") 

--generate broadcast packet
do
  ret, sendData = pcall(dofile, "dxCh/examples/packets/cnc_egress_queue_and_acl.lua")

  if not ret then
    printLog ('Error in packet')
    setFailState()
    return
  end
end

executeLocalConfig("dxCh/examples/configurations/cnc_acl.txt")
executeLocalConfig("dxCh/examples/configurations/loopback.txt")

local devNum1 = devEnv.dev
local portNum1 = devEnv.port[1]
local devNum2 = devEnv.dev
local portNum2 = devEnv.port[2]
local trgDevPortList = {[devNum1]={portNum1,portNum2}}

--sending packet
ret,status = sendPacket(trgDevPortList, sendData)

counters1 = getTrafficCounters(devNum1, portNum1)

counters2 = getTrafficCounters(devNum2, portNum2)

local params = {}
params["devID"] = devNum1
params["ruleIdx"] = ruleID

local ret, showVals = showCountersACL(params)
--printLog("ret" , to_string(ret))
--printLog("showVals" , to_string(showVals))


executeLocalConfig("dxCh/examples/configurations/cnc_acl_deconfig.txt")
executeLocalConfig("dxCh/examples/configurations/loopback_deconfig.txt")

if (ret == false) or showVals["pass"][1] == nil then
    setFailState()
else

     numberOfPackets = showVals["pass"][1]["Packet-Counter"]
     numberOfBytes = showVals["pass"][1]["Byte-count counter"]
     if not (counters1.goodOctetsRcv["l"][0] ~= 0 and counters2.goodOctetsSent["l"][0] ~= 0 and
             numberOfPackets == 1                 and numberOfBytes >= 64                       ) then
       setFailState()
    end
end