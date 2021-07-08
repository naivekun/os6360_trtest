--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cnc_egress_queue.lua
--*
--* DESCRIPTION:
--*       The test for testing CNC egress queue
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5") 
-- BC2 GM doesn't support CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E,
-- CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E clients.
SUPPORTED_FEATURE_DECLARE(devEnv.dev, "NOT_BC2_GM")

local devNum1 = devEnv.dev
local portNum1 = devEnv.port[1]
local devNum2 = devEnv.dev
local portNum2 = devEnv.port[2]
local trgDevPortList = {[devNum1]={portNum1,portNum2}}

if (isTmEnabled(devNum1)) then
    -- test do not work for TM enabled systems because all port use same TxQ.
    setTestStateSkipped()
    return
end

local sendData
local counters1, counters2
local ret, status, val
local stdout = ""
local numberOfPackets, numberOfBytes

--generate broadcast packet
do
  ret, sendData = pcall(dofile, "dxCh/examples/packets/cnc_egress_queue_and_acl.lua")

  if not ret then
    printLog ('Error in packet')
    setFailState()
    return
  end
end

executeLocalConfig("dxCh/examples/configurations/cnc_egress_queue.txt")
executeLocalConfig("dxCh/examples/configurations/loopback.txt")

--sending packet
ret,status = sendPacket(trgDevPortList, sendData)

counters1 = getTrafficCounters(devNum1, portNum1)

counters2 = getTrafficCounters(devNum2, portNum2)


local params = {}
params["dev_port"] = {}
params["dev_port"]["devId"] = devNum2
params["dev_port"]["portNum"] = portNum2
params["tc"] = 0
params["dp"] = 0

local ret, showVals = showCountersEgressQueue(params)

executeLocalConfig("dxCh/examples/configurations/cnc_egress_queue_deconfig.txt")
executeLocalConfig("dxCh/examples/configurations/loopback_deconfig.txt")

if (ret == false) then
    printLog ("ERROR :  in showCountersEgressQueue")
    setFailState()
else

    printLog ("show info:",to_string(showVals))

    if showVals["pass"] == nil or showVals["pass"][1] == nil then
        showVals = {["pass"] = {[1] = {["Packet-Counter"] = 0 , ["Byte-count counter"] = 0}}}

        printLog ("UPDATED : show info:",to_string(showVals))
    end

    numberOfPackets = showVals["pass"][1]["Packet-Counter"]
    numberOfBytes = showVals["pass"][1]["Byte-count counter"]

    if not ( counters1.goodOctetsRcv["l"][0] ~= 0 and counters2.goodOctetsSent["l"][0] ~= 0 and
             numberOfPackets == 1                 and numberOfBytes >= 64                       ) then
        printLog ("ERROR : failed on goodOctetsRcv or goodOctetsSent or numberOfBytes or numberOfPackets")
        setFailState()
    end
end
