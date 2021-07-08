--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* wirespeed_with_shaper_example.lua
--*
--* DESCRIPTION:
--*       test basic shaper on egress ports , by generating ingress wirespeed 
--*       traffic by command of 'traffic-generator emulation'
--*       
--*       purpose of example:
--*       1. show basic 'per port' (not per 'tc') shaper on egress port(s)
--*       2. show how to generate 'wirespeed' traffic without an external packet 
--*       generator ,like : smartbits/IXIA.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--ch1,2,3 not support it
SUPPORTED_FEATURE_DECLARE(devNum, "WIRESPEED_EMULATION")  

local devNum  = devEnv.dev

if (isTmEnabled(devNum)) then
    -- test do not work for TM enabled systems. Need to use TM shapers.
    setTestStateSkipped()
    return
end

local savePort3   = devEnv.port[3]
local savePort4   = devEnv.port[4]

local devFamily = wrlCpssDeviceFamilyGet(devNum)
if devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN2_E" then
    -- swap ports 3 and 4 to make port4 be 100G
    devEnv.port[3] = savePort4
    devEnv.port[4] = savePort3
end

local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]
local port5   = devEnv.port[5]
local port6   = devEnv.port[6]

--reset ports counters
resetPortCounters(devNum, port1)
resetPortCounters(devNum, port2)
resetPortCounters(devNum, port3)
resetPortCounters(devNum, port4)

-- this test is relevant to ALL tested devices
--##################################
--##################################

local delimiter = "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"

local packet_size = 64

local rates = {
    committedRateGet(1 , 25  , packet_size, true),
    committedRateGet(2 , 50  , packet_size, true),
    committedRateGet(3 , 80  , packet_size, true),
    committedRateGet(4 , 100 , packet_size, true) -- the wirespeed generating port
}

--##################################
--##################################
local transmitInfo = nil -- packet already doing WS inside the device from the 'cofig' file !!!
local egressInfoTable = {
    -- expected the mac-da to be 'unknown' --> flood the vlan
    {portNum = port1  , rate = rates[1]},
    {portNum = port2  , rate = rates[2]},
    {portNum = port3  , rate = rates[3]},
    {portNum = port4  , rate = rates[4]}
}

-- function to implement the test
local function doTest()
    --set config
    local sectionName = "Port shaper + Wirespeed from the emulated packet generator"
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

local configFileName = "wirespeed_with_shaper"

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

local rc

printLog (" Disable Force Link UP for ports: "..port5.." "..port6)
--power down ports 5 and 6 to avoid influence on WS
rc = prvLuaTgfPortForceLinkUpEnableSet(devNum,port5,false)
if rc ~= 0 then
    printLog (
        "ERROR: prvLuaTgfPortForceLinkUpEnableSet failed with code " .. rc
        .. " for port " .. devNum .. "/" .. port5)
end

prvLuaTgfPortForceLinkUpEnableSet(devNum,port6,false)
if rc ~= 0 then
    printLog (
        "ERROR: prvLuaTgfPortForceLinkUpEnableSet failed with code " .. rc
        .. " for port " .. devNum .. "/" .. port6)
end

-- run pre-test config
preTestConfig()
-- run the test
doTest()
-- run post-test config
postTestConfig()

if devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN2_E" then
    -- restore ports 3 and 4
    devEnv.port[3] = savePort3
    devEnv.port[4] = savePort4
end

