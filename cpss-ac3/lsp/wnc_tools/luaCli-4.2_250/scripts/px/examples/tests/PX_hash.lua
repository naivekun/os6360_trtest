--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* PX_hash.lua
--*
--* DESCRIPTION:
--*       Hash calculation example for PIPE
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

-- Check that egress port of LAG depends on CRC32 seed in hash indexing mode,
-- where hash is calculated from CRC32. CRC32 hash calculation mode is set up by
-- default
local function ingressHashCRC32Test()
    local i
    local isErr

    executeLocalConfig(luaTgfBuildConfigFileName("PX_hash"))
    seeds = { 0xFEEDBABE, 0xDEADBEEF }

    local egressInfoTableHash = {
        {
            {portNum = port1, packetCount = 0},
            {portNum = port2, packetCount = 0},
            {portNum = port4, packetCount = 1},
            {portNum = port5, packetCount = 0}
        },
        {
            {portNum = port1, packetCount = 0},
            {portNum = port2, packetCount = 1},
            {portNum = port4, packetCount = 0},
            {portNum = port5, packetCount = 0}
        },
    }

    status, testPacket = pcall(dofile, "px/examples/packets/PX_hash.lua")
    if not status then
        printLog ('Error in packet generator')
        setFailState()
        return
    end

    local transmitInfo = {portNum = port0 , pktInfo = {fullPacket = testPacket}}

    for i=1,#seeds do
        executeStringCliCommands("cpss-api call cpssPxIngressHashSeedSet devNum  ${dev} crc32Seed "..seeds[i])

        local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo, egressInfoTableHash[i])
        if rc ~= 0 then
            local error_string = "Test FAILED."
            printLog ("ENDED : " .. error_string .. "\n")
            setFailState()
            executeLocalConfig(luaTgfBuildConfigFileName("PX_hash_deconfig"))
            return
        end
    end
    executeLocalConfig(luaTgfBuildConfigFileName("PX_hash_deconfig"))
end

ingressHashCRC32Test()