--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cpu_rx_rate_limit.lua
--*
--* DESCRIPTION:
--*       The test for TO CPU Rate Limit
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- this test is not relevant for CH+ and Simulations
SUPPORTED_FEATURE_DECLARE(devNum,"CPU_CODE_RATE_LIMIT")

local devNum  = devEnv.dev
local rxQueue = 0
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]
local rateLimitCfg = 10000
local rateLimitDiffAccept = 3 -- in percent
local delayTimeInSec = 4.095
local defaultWindowSleep = 4095 -- in miliseconds

-- function to implement the test
local function doTest()
    -- device should finish default window before use configuration of the test
    delay(defaultWindowSleep)

    local sectionName = "Get counter to reset them"
    local rc, rxInPkts, rxInOctets = rx_sdma_counters_get(devNum, rxQueue)

    if rc == 0 then
        local pass_string = "Section " .. sectionName .. " PASSED."
        printLog ("ENDED : " .. pass_string .. "\n")
        testAddPassString(pass_string)
    else
        local error_string = "Section " .. sectionName .. " FAILED."
        printLog ("ENDED : " .. error_string .. "\n")
        setFailState() 
        testAddErrorString(error_string)
    end  

    delay(1000 * delayTimeInSec)

    sectionName = "Get counter after sleep"
    rc, rxInPkts, rxInOctets = rx_sdma_counters_get(devNum, rxQueue)
    printLog ("rxInPkts: " .. tostring(rxInPkts).." rxInOctets: " .. tostring(rxInOctets) .. "\n")

    local diff = math.abs ((rxInPkts / delayTimeInSec) - rateLimitCfg)
    local diffInPercent = diff * 100 / rateLimitCfg
    printLog ("diff: " .. tostring(diff).." diff In Percent: " .. tostring(diffInPercent) .. "\n")
    
    if rc == 0 and diffInPercent <= rateLimitDiffAccept then
        local pass_string = "Section " .. sectionName .. " PASSED."
        printLog ("ENDED : " .. pass_string .. "\n")
        testAddPassString(pass_string)
    else
        local error_string = "Section " .. sectionName .. " FAILED."
        printLog ("ENDED : " .. error_string .. "\n")
        setFailState() 
        testAddErrorString(error_string)
    end  

    -- print results summary
    testPrintResultSummary(sectionName) 
end

local configFileName = "cpu_rx_rate_limit"

-- run pre-test config
local function preTestConfig()
    -- disable interrupts
    local CPSS_PP_SCT_RATE_LIMITER_E = 242
    local CPSS_EVENT_MASK_E = 0
    --[[
    GT_STATUS cpssEventDeviceMaskSet
    (
        IN GT_U8                    devNum,
        IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
        IN CPSS_EVENT_MASK_SET_ENT  operation
    );
    ]]--
    -- call CPSS with direct call as 'shell command' because no 'cpss-api' support for this
    local stcEventSetString = [[
        end

        shell-execute cpssEventDeviceMaskSet ]]..devNum..","..CPSS_PP_SCT_RATE_LIMITER_E..","..CPSS_EVENT_MASK_E..[[

    ]]

    executeStringCliCommands(stcEventSetString)

    --set config
    executeLocalConfig(luaTgfBuildConfigFileName(configFileName)) 
end
-- run post-test config
local function postTestConfig()
    --unset config
    executeLocalConfig(luaTgfBuildConfigFileName(configFileName,true)) 
end

-- run pre-test config
preTestConfig()
-- run the test
doTest()
-- run post-test config
postTestConfig()

