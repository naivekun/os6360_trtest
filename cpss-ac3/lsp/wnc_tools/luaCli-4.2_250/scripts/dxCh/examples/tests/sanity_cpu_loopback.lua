--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* sanity_cpu_loopback.lua
--*
--* DESCRIPTION:
--*       check 'from cpu' with target port is 'CPU port' (63)
--*       the Test bypass the need for MAC ports ! (in Falcon this is without Raven)
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
local portCPU = devEnv.portCPU -- cpu port

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
local transmitInfo = {portNum = "CPU" , pktInfo = {fullPacket = ingressPacket}}
local egressInfoTable = {
    -- expected to get to CPU without changes
    {portNum = "CPU", pktInfo = {fullPacket = ingressPacket}}
}

local function nonSip5_allowCpuToCpuTraffic(devNum,allow)
    local apiName = "cpssDxChNstPortIngressFrwFilterSet"
    local ret = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",          "devNum",     devNum },
        { "IN",     "GT_PORT_NUM",    "portNum",    portCPU},
        { "IN",     "CPSS_NST_INGRESS_FRW_FILTER_ENT",    "filterType",    "CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E"},
        { "IN",     "GT_BOOL",    "enable",    (not allow)}
    })
end

-- function to implement the test
local function doTest()
    --set config
    local sectionName = "check 'from cpu' with target port is 'CPU port' (63)"
    
    if(not is_sip_5()) then
        --nonSip5_allowCpuToCpuTraffic(devNum,true--[[allow traffic]])
    end
    
    -- check that packet egress the needed port(s)
    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)

    if(not is_sip_5()) then
        -- restore 
        --nonSip5_allowCpuToCpuTraffic(devNum,false--[[not allow traffic]])
    end

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
