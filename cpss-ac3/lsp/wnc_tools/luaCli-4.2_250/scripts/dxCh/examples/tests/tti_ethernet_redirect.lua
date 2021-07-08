--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* tti.lua
--*
--* DESCRIPTION:
--*       The test for tti feature
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local sendData

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

NOT_SUPPORTED_DEV_DECLARE(devNum, "CPSS_PP_FAMILY_CHEETAH_E")

local ret, status

--generate mac pdu test packets
-- function to allow test to get valid tti index
function validTtiIndex(origIndex)
    local devNum  = devEnv.dev
    local maxIndex = getNumberOfTTIindices(devNum)

    if (not maxIndex) or (maxIndex == 0) then
        return 0
    end
    -- return normalized index
    return origIndex % maxIndex
end

ret, sendData = pcall(dofile, "dxCh/examples/packets/tti_ethernet_redirect.lua")

--reset ports and CPU counters
resetPortCounters(devNum, port1)
resetPortCounters(devNum, port2)
resetPortCounters(devNum, port3)
resetPortCounters(devNum, port4)

--------------------------------------------------------------------------------
--load configuration for 1st part of test
executeLocalConfig("dxCh/examples/configurations/tti_ethernet_redirect.txt")
executeLocalConfig("dxCh/examples/configurations/loopback.txt")

printLog("Testing ports")
printLog("Sending traffic ..")
ret, status = sendPacket({[devNum]={port1}}, sendData["vlan10"])
delay(100)
printLog("Check counters ..")
checkExpectedCounters(debug.getinfo(1).currentline,
                        {[devNum] = { [port1] = {Rx=1, Tx = 1},
                                 [port2] = {Rx=0, Tx = 0},
                                 [port3] = {Rx=0, Tx = 1},
                                 [port4] = {Rx=0, Tx = 0} }})


printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/tti_ethernet_redirect_deconfig.txt")
executeLocalConfig("dxCh/examples/configurations/loopback_deconfig.txt")


