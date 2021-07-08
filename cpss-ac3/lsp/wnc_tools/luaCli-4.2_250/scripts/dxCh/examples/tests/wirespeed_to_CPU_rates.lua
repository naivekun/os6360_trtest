--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* wirespeed_to_CPU_rates.lua
--*
--* DESCRIPTION:
--*       generating ingress wirespeed to measure the "to CPU" rates
--*       generate traffic to CPU by using the command of 'traffic-generator emulation', 
--*       doing capture from port 0/58 and setting mac loopback on port 58,
--*       show ports and CPU rates and indications of AUQ/FUQ messages
--*       
--*       purpose of example:
--*       1. Run test to load simulation with wire speed traffic on port,
--*          traffic TO_CPU and AU messages.
--*       2. Show all ports and CPU rates.
--*       3. Stop traffic on all ports.
--*		  4. one second delay to let simulation drain the queues.
--*		  5. show there is no traffic (rates are zero).
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--ch1,2,3 not support it
SUPPORTED_FEATURE_DECLARE(devNum, "WIRESPEED_EMULATION")  

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4] 

-- this test is relevant to ALL tested devices
--##################################
--##################################

-- Store Loopback state on capture port
local restoreLoopback = false
local result, values = cpssPerPortParamGet("cpssDxChPortInternalLoopbackEnableGet", devNum, port4, "enablePtr", "GT_BOOL")
if 0 == result then
   restoreLoopback = values["enablePtr"]
end

local delayTimeInSec = 5
local packet_size = 64

--##################################
--##################################
local transmitInfo = nil -- packet already doing WS inside the device from the 'cofig' file !!!

-- function to implement the test
local function doTest()
    delay(1000 * delayTimeInSec)
end

local configFileName = "wirespeed_to_CPU_rates"

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

if true == restoreLoopback then
    myGenWrapper(
        "cpssDxChPortInternalLoopbackEnableSet",{  --enable internal
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_U32","portNum",port4},
            {"IN","GT_BOOL","enable", restoreLoopback}
    })
end

