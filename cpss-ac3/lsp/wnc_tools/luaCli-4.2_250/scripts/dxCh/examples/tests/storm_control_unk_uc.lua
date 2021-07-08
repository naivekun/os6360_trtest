--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* storm_control_unk_uc.lua
--*
--* DESCRIPTION:
--*       test storm control command on ingress ports , by generating ingress wirespeed 
--*       traffic by command of 'traffic-generator emulation'
--*       
--*       purpose of example:
--*       1. show basic 'per port' (not per 'tc') limit on egress port(s)
--*       2. show how to generate 'wirespeed' traffic without an external packet 
--*       generator ,like : smartbits/IXIA.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--ch1,2,3 not support it
SUPPORTED_FEATURE_DECLARE(devNum, "BRIDGE_RATE_LIMIT") 

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4] 

-- this test is relevant to ALL tested devices
--##################################
--##################################

local delimiter = "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"

local packet_size = 64
local rate_limit_in_kbps = 15000
local rate_limit_diff_in_percent = 15

-- default diff is 2%
local src_port_rate_diff_in_percent = 2

if is_sip_5_20(devNum) then
    -- test use 100G ports. Ingress Storm Control reduces rate on source port.
    src_port_rate_diff_in_percent = 50
end

local rates = {
    committedRateGet(1 , 100 , packet_size, true), -- the wirespeed generating port
    rate_limit_in_kbps,
    rate_limit_in_kbps,
    rate_limit_in_kbps 
}

--##################################
--##################################

local transmitInfo = nil -- packet already doing WS inside the device from the 'cofig' file !!!
local egressInfoTable = {
    -- expected the mac-da to be 'unknown' --> flood the vlan
    {portNum = port1 , rate = rates[1], rateDiffInPercent = src_port_rate_diff_in_percent},
    {portNum = port2 , rate = rates[2], rateDiffInPercent = rate_limit_diff_in_percent},
    {portNum = port3 , rate = rates[3], rateDiffInPercent = rate_limit_diff_in_percent},
    {portNum = port4 , rate = rates[4], rateDiffInPercent = rate_limit_diff_in_percent}
}


-- function to implement the test
local function doTest()
    --set config
    local sectionName = "storm control + Wirespeed from the emulated packet generator"
    -- check that packet egress the needed port(s) , when the 'threshold' allow it
    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)

    printLog (delimiter)
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

    -- print results summary
    testPrintResultSummary(sectionName) 
end

local configFileName = "storm_control_unk_uc"

-- run pre-test config
local function preTestConfig()
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