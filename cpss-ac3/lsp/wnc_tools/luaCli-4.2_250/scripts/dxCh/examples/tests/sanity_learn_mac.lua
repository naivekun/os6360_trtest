--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* sanity_send_and_capture.lua
--*
--* DESCRIPTION:
--*       sanity check for the LUA tests environment :
--*       loopback , send flood in default vlan 1 to check 'from cpu' .
--*       and capture on egress ports to check 'to cpu'
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

-- this test is relevant to ALL tested devices
--##################################
--##################################

local delimiter = "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"

--generate test packet
local packetPayload = "6666"..-- dummy ethertype
        "000102030405060708090a0b0c0d0e0f"..
        "101112131415161718191a1b1c1d1e1f"..
        "202122232425262728292a2b2c2d2e2f"..
        "303132333435363738393a3b3c3d3e3f"

local macOfPort2   = "000000002222"
local macOfPort4   = "000000004444"
local packetFromPort2 = macOfPort4 .. macOfPort2 .. packetPayload
local packetFromPort4 = macOfPort2 .. macOfPort4 .. packetPayload

--##################################
--##################################
local fromPort2Traffic = {
    transmitInfo = {portNum = port2 , pktInfo = {fullPacket = packetFromPort2} },
    egressInfoTable = {
        -- flooded as unknownd
        {portNum = port1, packetCount = 1},
        {portNum = port3, packetCount = 1},
        {portNum = port4, packetCount = 1}
    }
}

local fromPort4Traffic = {
    transmitInfo = {portNum = port4 , pktInfo = {fullPacket = packetFromPort4} },
    egressInfoTable = {
        -- mac already learned to port 2
        {portNum = port1, packetCount = 0},
        {portNum = port2, packetCount = 1},
        {portNum = port3, packetCount = 0}
    }
}

local function doTest()
    local sectionName = "sanity check for MAC Learning: unknown UC"
    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
        fromPort2Traffic.transmitInfo, fromPort2Traffic.egressInfoTable)

    printLog (delimiter)
    if rc == 0 then
        local pass_string = "Section " .. sectionName .. " PASSED."
        printLog ("ENDED : " .. pass_string .. "\n")
    else
        local error_string = "Section " .. sectionName .. " FAILED."
        printLog ("ENDED : " .. error_string .. "\n")
        setFailState()
    end

    local sectionName = "sanity check for MAC Learning: known UC"
    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
        fromPort4Traffic.transmitInfo, fromPort4Traffic.egressInfoTable)

    printLog (delimiter)
    if rc == 0 then
        local pass_string = "Section " .. sectionName .. " PASSED."
        printLog ("ENDED : " .. pass_string .. "\n")
    else
        local error_string = "Section " .. sectionName .. " FAILED."
        printLog ("ENDED : " .. error_string .. "\n")
        setFailState()
    end
end
-- run the test
doTest()
ret = pcall(
    executeStringCliCommands,
    "end\n" .. "clear bridge type " .. to_string(devNum) .. " all")
if (ret == false) then
    setFailState()
end

