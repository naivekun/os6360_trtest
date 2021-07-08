--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* PX_counters.lua
--*
--* DESCRIPTION:
--*       test for PIPE device : egress-processing configuration.
--*                    done in 'DX system' but only as sanity since actual commands
--*                    currently only manipulate 'DB' (in the LUA)
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local port0 = devEnv.port[1]
local port1 = devEnv.port[5]
local port2 = devEnv.port[6]
local port4 = devEnv.port[2]
local port5 = devEnv.port[3]

local counterValues = {
    {portNum = port1, packetCount = 1},
    {portNum = port2, packetCount = 1},
    {portNum = port4, packetCount = 1},
    {portNum = port5, packetCount = 1}
}

local cncConfiguration =
{
    {["client"] = "egress-queue", ["eq_mode"] = "reduced"},
    {["client"] = "egress-queue", ["eq_mode"] = "cn"},
    {["client"] = "egress-queue", ["eq_mode"] = "tail-drop"},
    {["client"] = "egress-processing"},
    {["client"] = "ingress-processing"}
}

local expectedCounters = -- only for test ports { 0, 4, 5, 12, 1, 2 }
{
    {  [0] = 1,  [16] = 1,  [32] = 1,  [64] = 1,  [80] = 1, [192] = 1 },
    {  [0] = 1,  [32] = 1,  [64] = 1, [128] = 1, [160] = 1, [384] = 1 },
    {  [0] = 1,  [32] = 1,  [64] = 1, [128] = 1, [160] = 1, [384] = 1 },
    { },
    {  [0] = 1,  [16] = 1 }
}

local function cncCountersTest()
    local ii, jj
    local ret
    local clearEnabled
    local packets, bytes

    testFailed = false

    for ii=1,#cncConfiguration do
        currentCncClient = cncConfiguration[ii]["client"]
        if cncConfiguration[ii]["eq_mode"] then
            cncEqModePrefix = "mode"
            cncEqMode = cncConfiguration[ii]["eq_mode"]
        else
            cncEqModePrefix = ""
            cncEqMode = ""
        end
        executeLocalConfig(luaTgfBuildConfigFileName("PX_counters"))

        status, testPacket = pcall(dofile, "px/examples/packets/PX_hash.lua")
        if not status then
            printLog ('Error in packet generator')
            setFailState()
            return
        end

        local transmitInfo = {portNum = port0 , pktInfo = {fullPacket = testPacket}}

        local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo, counterValues)
        if rc ~= 0 then
            local error_string = "Test FAILED."
            printLog ("ENDED : " .. error_string .. "\n")
            setFailState()
            return
        end

        ret, clearEnabled = cncCounterClearByReadEnableGet(devEnv.dev, 0)
        if ret == false then
            setFailState()
        end
        ret = cncCounterClearByReadEnableSet(devEnv.dev, 0, true)
        if ret == false then
            setFailState()
        end

        for jj=0,1023 do
            packets, bytes = counterGet(devEnv.dev, jj)
            if expectedCounters[ii][jj] ~= nil then
                if packets ~= expectedCounters[ii][jj] then
                    setFailState()
                    print(string.format("Bad counter value for counter index %d:  %d instead of %d",
                    jj, packets, expectedCounters[ii][jj]))
                end
            end
        end
        ret = cncCounterClearByReadEnableSet(devEnv.dev, 0, clearEnabled)
        if ret == false then
            setFailState()
        end

        executeLocalConfig(luaTgfBuildConfigFileName("PX_counters_deconfig"))
    end
end

cncCountersTest()
