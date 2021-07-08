--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* mac_pdu.lua
--*
--* DESCRIPTION:
--*       The test for testing mac pdu feature
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local pName
local payload
local payloads

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local portCPU = devEnv.portCPU

NOT_SUPPORTED_DEV_DECLARE(devNum, "CPSS_PP_FAMILY_CHEETAH_E")

local counters1, counters2, countersCPU
local ret, status

local function sendWithCapturing(devPortList, payload)
    local ret, status = execWithCapturing(
        sendPacket, devPortList, payload)
    if ret ~= 0 then
        printLog(status)
        setFailState()
    end
end

local function checkCaptured(num, packetInfoList)
    local ret, status = checkPacketsWereCaptured(num, "TGF_PACKET_TYPE_ANY_E", packetInfoList)
    if not ret then
        print("AAAAAAAAAAA", ret, status)
        setFailState()
        printLog(status)
    end
end

local function resetAllPortCounters()
    resetPortCounters(devNum, port1)
    resetPortCounters(devNum, port2)
    resetPortCounters(devNum, portCPU)
end


--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

--generate mac pdu test packets
do
    ret, payloads = pcall(dofile, "dxCh/examples/packets/mac_pdu.lua")

    if not ret then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end

--reset ports and CPU counters
resetAllPortCounters()
--------------------------------------------------------------------------------
printLog("STAGE1: all forward")
--load configuration for 1st part of test
executeLocalConfig("dxCh/examples/configurations/mac_pdu.txt")
executeLocalConfig("dxCh/examples/configurations/loopback.txt")

for pName, payload in pairs(payloads) do

    printLog("payload is "..pName)

    --send packet
    printLog("Sending traffic ..")
    sendWithCapturing({[devNum]={port1}}, payload)
    -- delay by 100 mSec
    delay(100)

    --check expected counters:
    printLog("Check counters ..")
    checkExpectedCounters(debug.getinfo(1).currentline,
                          {[devNum] = {
                                  [port1]   = {Rx=1, Tx=1},
                                  [port2]   = {Rx=0, Tx=1}}})

    checkCaptured(0)

    --reset ports and CPU counters
    resetAllPortCounters()
end

--------------------------------------------------------------------------------
printLog("STAGE2: all trap")
--load configuration for 2nd part of test
executeLocalConfig("dxCh/examples/configurations/mac_pdu_2.txt")

for pName, payload in pairs(payloads) do

    printLog("payload is "..pName)

    --send packet
    printLog("Sending traffic ..")
    sendWithCapturing({[devNum]={port1}}, payload)

    local packetInfoList = {
        { rxParam = {
              dsaCmdIsToCpu = true,
              cpuCode = "CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_E"}}
    }
    checkCaptured(1, packetInfoList)

    --reset ports and CPU counters
    resetAllPortCounters()
end

--------------------------------------------------------------------------------
printLog("STAGE3: all mirror")
--load configuration for 3rd part of test
executeLocalConfig("dxCh/examples/configurations/mac_pdu_3.txt")

for pName, payload in pairs(payloads) do

    printLog("payload is "..pName)

    --send packet
    printLog("Sending traffic ..")
    sendWithCapturing({[devNum]={port1}}, payload)

    -- check packets in CPU.
    local packetInfoList = {
        { rxParam = {
              dsaCmdIsToCpu = true,
              cpuCode = "CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_E"}}
    }
    checkCaptured(1, packetInfoList)

    -- delay(100)
    checkExpectedCounters(debug.getinfo(1).currentline,
                          {[devNum] = {
                                  [port1]   = {Rx=1, Tx=1},
                                  [port2]   = {Rx=0, Tx=1}}})
    --reset ports and CPU counters
    resetAllPortCounters()
end

--------------------------------------------------------------------------------
printLog("STAGE4: all drop")
--load configuration for 4th part of test
executeLocalConfig("dxCh/examples/configurations/mac_pdu_4.txt")

for pName, payload in pairs(payloads) do

    printLog("payload is "..pName)

    --send packet
    printLog("Sending traffic ..")
    sendWithCapturing({[devNum]={port1}}, payload)

    checkCaptured(0)

    -- delay(100)

    --check expected counters:
    printLog("Check counters ..")
    checkExpectedCounters(debug.getinfo(1).currentline,
                          {[devNum] = {
                                  [port1]   = {Rx=1, Tx=1},
                                  [port2]   = {Rx=0, Tx=0}}})

    --reset ports and CPU counters
    resetAllPortCounters()
end

printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/mac_pdu_deconfig.txt")
executeLocalConfig("dxCh/examples/configurations/loopback_deconfig.txt")
