--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* dsa_on_trunk.lua
--*
--* DESCRIPTION:
--*       The test for DSA feature on Trunk port
--*       To test srcPort info is received at CPU from a Trunk port.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- this test is relevant for SIP5 devices (BOBCAT 2 and above)
SUPPORTED_SIP_DECLARE(devNum,"SIP_5")

-- this test is not supported by GM
SUPPORTED_FEATURE_DECLARE(devNum, "NOT_BC2_GM")

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]

local function resetCounter()
    resetPortCounters(devNum, port1)
    resetPortCounters(devNum, port2)
end

local function sendPacket_with_delay(devPortTable,packetData)
    local ret = sendPacket(devPortTable,packetData)

    delay(50)
    return ret
end


local function sendWithCapturing(devPortList, payload)
    local ret, status = execWithCapturing(
        sendPacket_with_delay, devPortList, payload)
    if ret ~= 0 then
        printLog(status)
        setFailState()
    end
end

local tx_pkt = "0180c2000002000000000001"

--------------------------------------------------------------------------------
--load configuration for test
resetCounter()
executeLocalConfig("dxCh/examples/configurations/dsa_on_trunk.txt")

printLog("Sending traffic ..")
sendWithCapturing({[devNum]={port2}}, tx_pkt)

printLog("Check counters ..")
checkExpectedCounters(debug.getinfo(1).currentline,
                      {[devNum] = {[port2] = {Rx=stormingExpectedIndication, Tx=stormingExpectedIndication, isSrcOfStorming = true}}})

--printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/dsa_on_trunk_deconfig.txt")

--Reset port counters
delay(1000)
resetCounter()
