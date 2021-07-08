--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vss_cc5_vss3.lua
--*
--* DESCRIPTION:
--*       The test for testing vss device cases.
--*       tests for 'cc5' in vss3.  (system described in examples/configurations/vss_cc5_vss3.txt)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev 
SUPPORTED_FEATURE_DECLARE(devNum, "VSS-CC")
--- AC3X need proper ports for this test
-- devEnv.port[3,4] must support the "DSA_CASCADE"
-- the call to SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM may change the value of devEnv.port[3,4] !!!
SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM(devNum, "DSA_CASCADE" , "ports" , {devEnv.port[3],devEnv.port[4]}) 

local origPorts = deepcopy(devEnv.port)
-- note the 2 ports must be in ascending order !!! (for trunk LBH on cascade ports that hold no order)
-- meaning must have : devEnv.port[3] < devEnv.port[4]
if(devEnv.port[3] > devEnv.port[4]) then
    printLog("note the 2 ports must be in ascending order !!! (for trunk LBH on cascade ports that hold no order)")
    printLog("meaning must have : devEnv.port[3] < devEnv.port[4]")
    printLog("so swapping the 2 ports: index 3:" .. devEnv.port[3] .. " ,and index 4 : "..devEnv.port[4])
    devEnv.port[3] = origPorts[4]
    devEnv.port[4] = origPorts[3]
end

-----------------------*******************
-- get extra packet's util functions
-- use dofile instead of require .. to allow quick load of test
dofile("dxCh/examples/common/vss_utils.lua") 

--set config
executeLocalConfig(luaTgfBuildConfigFileName("vss_cc5_vss3")) 
--run the tests for this device
util_vss_run_tests(devicesName_vss3_cc5)
--unset config
executeLocalConfig(luaTgfBuildConfigFileName("vss_cc5_vss3",true))
-- do flush after the 'restore' -- bug in lua flush set mask to '0x10' for hwDevNum
executeLocalConfig(luaTgfBuildConfigFileName("flush_fdb"))

-- print failed cases
luaTgfPrintTestResults()
-- clear failed tests
luaTgfClearTestResults()

-- restore ports
devEnv.port = origPorts




