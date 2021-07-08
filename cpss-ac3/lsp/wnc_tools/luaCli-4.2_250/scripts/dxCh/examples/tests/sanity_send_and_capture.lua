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

local macDa   = "000000000058"
local macSa   = "000000001111"

local function buildPacketUc(partAfterMac)
    if not partAfterMac then
        partAfterMac = ""
    end
    
    return macDa .. macSa ..  partAfterMac .. packetPayload
end

--##################################
--##################################
local ingressPacket = buildPacketUc(nil)
local transmitInfo = {portNum = port2 , pktInfo = {fullPacket = ingressPacket} }
local egressInfoTable = {
    -- expected to egress unmodified
    {portNum = port1  , pktInfo = {fullPacket = ingressPacket}},
    {portNum = port3  , pktInfo = {fullPacket = ingressPacket}},
    {portNum = port4  , pktInfo = {fullPacket = ingressPacket}}
}

-- function to implement the test
local function doTest()
    --set config
    local sectionName = "sanity check for the LUA tests environment"
    -- check that packet egress the needed port(s) , when the 'threshold' allow it
    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)

    printLog (delimiter)
    if rc == 0 then
        local pass_string = "Section " .. sectionName .. " PASSED."
        printLog ("ENDED : " .. pass_string .. "\n")
--        testAddPassString(pass_string)
    else
        local error_string = "Section " .. sectionName .. " FAILED."
        printLog ("ENDED : " .. error_string .. "\n")
        setFailState() 
--        testAddErrorString(error_string)
    end  

    -- print results summary
--    testPrintResultSummary(sectionName) 
end
-- run the test
doTest()
