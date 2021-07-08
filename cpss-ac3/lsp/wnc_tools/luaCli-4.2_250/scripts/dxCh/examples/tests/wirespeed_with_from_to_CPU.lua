--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* wirespeed_with_from_to_CPU.lua
--*
--* DESCRIPTION:
--*       stress test by generating ingress wirespeed 
--*       traffic on port1 by command of 'traffic-generator emulation', 
--        and ingress burst traffic on port2 with capture
--*       
--*       purpose of example:
--*       1. Run test to load simulation with wire speed traffic on port,
--*          and FROM_CPU, traffic TO_CPU and AU messages
--*       2. Delay 5 seconds and see AU messages are sent to CPU
--*       3. Stop traffic on all ports  
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

local delayTimeInSec = 5
local packet_size = 64

--##################################
--##################################
local transmitInfo = nil -- packet already doing WS inside the device from the 'cofig' file !!!

-- function to implement the test
local function doTest()
    delay(1000 * delayTimeInSec)
end

local configFileName = "wirespeed_with_from_to_CPU"

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

