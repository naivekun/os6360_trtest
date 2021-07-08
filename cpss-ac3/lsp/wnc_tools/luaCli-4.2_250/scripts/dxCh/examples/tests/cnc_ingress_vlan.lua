--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cnc_ingress_vlan.lua
--*
--* DESCRIPTION:
--*       The test for testing CNC ingress vlan
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5") 

local sendData
local counters1, counters2
local ret, status, val
local vlanID = 0x14
local stdout = ""
local numberOfPackets, numberOfBytes



--generate broadcast packet
do
  ret, sendData = pcall(dofile, "dxCh/examples/packets/cnc_ingress_vlan.lua")

  if not ret then
    printLog ('Error in packet')
    setFailState()
    return
  end
end

executeLocalConfig("dxCh/examples/configurations/cnc_ingress_vlan.txt")
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
params["vlanIdOrAll"] = vlanID

local ret, showVals = showCountersIngressVlan(params)

executeLocalConfig("dxCh/examples/configurations/cnc_ingress_vlan_deconfig.txt")
executeLocalConfig("dxCh/examples/configurations/loopback_deconfig.txt")

if (ret == false) then
    setFailState()
else
    numberOfPackets = showVals["pass"][1]["Packet-Counter"]
    numberOfBytes = showVals["pass"][1]["Byte-count counter"]

    if not (counters1.goodOctetsRcv["l"][0] ~= 0 and counters2.goodOctetsSent["l"][0] ~= 0 and
            numberOfPackets == 1                 and numberOfBytes >= 64                       ) then
        setFailState()
    end
end

