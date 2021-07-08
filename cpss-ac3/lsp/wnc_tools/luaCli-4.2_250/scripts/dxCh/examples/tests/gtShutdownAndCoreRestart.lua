--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* gtShutdownAndCoreRestart.lua
--*
--* DESCRIPTION:
--*       sanity check for the 'gtShutdownAndCoreRestart' processing of the PSS:
--*       1. do 'gtShutdownAndCoreRestart'
--*       2. check TO/FROM CPU traffic
--*       3. check FDB learning
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- this test is relevant to ALL tested devices
--##################################
--##################################
SUPPORTED_FEATURE_DECLARE(devNum, "gtShutdownAndCoreRestart")


local fromLua_gtShutdownAndCoreRestart = [[
    do shell-execute fromLua_gtShutdownAndCoreRestart 
]]

-- function to run LUA test from another file
local function runTest(testName)
    local res = pcall(dofile,"dxCh/examples/tests/"..testName)
    if not res then 
        setFailState() 
    end
end

-- operate 'gtShutdownAndCoreRestart' using mainUT function
executeStringCliCommands(fromLua_gtShutdownAndCoreRestart)
-- run the tests:
-- 1. 'sanity_send_and_capture.lua'
-- 2. 'sanity_learn_mac.lua'
runTest("sanity_send_and_capture.lua")
runTest("sanity_learn_mac.lua")


