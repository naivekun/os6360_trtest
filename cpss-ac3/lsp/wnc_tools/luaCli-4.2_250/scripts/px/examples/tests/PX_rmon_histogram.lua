--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* rmon_histogram.lua
--*
--* DESCRIPTION:
--*       The test for testing rmon histogram [rx-tx|rx|tx|disable] feature
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- the GM not supports the full mac config and counters

local payload
local counters1, counters2
local ret, status

local stage1 = false
local stage2 = false

--generate broadcast packet
do
  ret, payload = pcall(dofile, "px/examples/packets/PX_broadcast.lua")

  if not ret then
    printLog ('Error in packet')
    setFailState()
    return
  end
end

local devNum1 = devEnv.dev
local portNum1 = devEnv.port[1]
local devNum2 = devEnv.dev
local portNum2 = devEnv.port[2]
local port1_str = devNum1 .."/".. portNum1
local port2_str = devNum2 .."/".. portNum2

local trgDevPortList = {[devNum1]={portNum1,portNum2}}


-- tested 4 cases for modes disable, rx, tx, rx-tx
-- rx-tx mode is default and should be last to avoid deconfiguration
-- same mode used on both ports
-- the tested ports numbers calculated by the system,
-- so the ports can be in the same group and the different modes for ports not tested

local function check_stage(mode, port1Expected, port2Expected)
    printLog(
        "MODE: " .. mode ..
        " pots: " .. port1_str .. " and " .. port2_str)

    resetPortCounters(devNum1, portNum1)
    resetPortCounters(devNum2, portNum2)
    local config_cmd = [[
    end
    configure
    interface ethernet ${dev}/${port[2]}
    ]] ..
    "rmon histogram " .. mode .. "\n" .. [[
    exit
    interface ethernet ${dev}/${port[1]}
    ]] ..
    "rmon histogram " .. mode .. "\n" .. [[
    end
    ]]
    ret = pcall(executeStringCliCommands, config_cmd)
    if (ret == false) then
        printLog("ERROR : Failed to run commands (1)")
        setFailState()
    end

    --sending packet
    printLog("Sending traffic ..")
    -- [dev] = {ports list}


    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo({portNum = portNum1, pktInfo = {fullPacket = payload}}, nil)
    if rc ~= 0 then
        setFailState()
    end

    counters1 = getTrafficCounters(devNum1, portNum1)
    counters2 = getTrafficCounters(devNum2, portNum2)

    if port1Expected.goodOctetsRcv ~= counters1.goodOctetsRcv["l"][0] or
       port1Expected.goodOctetsSent ~= counters1.goodOctetsSent["l"][0] or
       port1Expected.pkts64Octets ~= counters1.pkts64Octets["l"][0] or
       port2Expected.goodOctetsRcv ~= counters2.goodOctetsRcv["l"][0] or
       port2Expected.goodOctetsSent ~= counters2.goodOctetsSent["l"][0] or
       port2Expected.pkts64Octets ~= counters2.pkts64Octets["l"][0] then
       printLog("Counter check failed..")
       setFailState()
    end
    printLog("PORT " .. port1_str .. " goodOctetsRcv =      ",counters1.goodOctetsRcv["l"][0])
    printLog("PORT " .. port1_str .. " goodOctetsSent =     ",counters1.goodOctetsSent["l"][0])
    printLog("PORT " .. port1_str .. " pkts64Octets =       ",counters1.pkts64Octets["l"][0])
    printLog("PORT " .. port2_str .. " goodOctetsRcv =      ",counters2.goodOctetsRcv["l"][0])
    printLog("PORT " .. port2_str .. " goodOctetsSent =     ",counters2.goodOctetsSent["l"][0])
    printLog("PORT " .. port2_str .. " pkts64Octets =       ",counters2.pkts64Octets["l"][0])
end

local loopback_port1 = [[
end
configure
interface ethernet ${dev}/${port[1]}
loopback mode internal
end
]]

-- tested 4 cases for modes disable, rx, tx, rx-tx
-- rx-tx mode is default and should be last to avoid deconfiguration
local stages = {
    {mode = "disable", port1Expected = {goodOctetsRcv = 82, goodOctetsSent = 82, pkts64Octets = 0},
                       port2Expected = {goodOctetsRcv = 0, goodOctetsSent = 82, pkts64Octets = 0}},
    {mode = "rx",      port1Expected = {goodOctetsRcv = 82, goodOctetsSent = 82, pkts64Octets = 1},
                       port2Expected = {goodOctetsRcv = 0, goodOctetsSent = 82, pkts64Octets = 0}},
    {mode = "tx",      port1Expected = {goodOctetsRcv = 82, goodOctetsSent = 82, pkts64Octets = 1},
                       port2Expected = {goodOctetsRcv = 0, goodOctetsSent = 82, pkts64Octets = 1}},
    {mode = "rx-tx",   port1Expected = {goodOctetsRcv = 82, goodOctetsSent = 82, pkts64Octets = 2},
                       port2Expected = {goodOctetsRcv = 0, goodOctetsSent = 82, pkts64Octets = 1}}
}
local i;

local no_loopback_port1 = [[
end
configure
interface ethernet ${dev}/${port[1]}
no loopback
end
]]

ret = pcall(executeStringCliCommands, loopback_port1)
if (ret == false) then
    printLog("ERROR : Failed to run commands (2)")
    setFailState()
end

for i = 1,#stages do
    check_stage(stages[i].mode, stages[i].port1Expected, stages[i].port2Expected);
end

ret = pcall(executeStringCliCommands, no_loopback_port1)
if (ret == false) then
    printLog("ERROR : Failed to run commands (3)")
    setFailState()
end
