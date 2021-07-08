--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vss_cc7_vss4.lua
--*
--* DESCRIPTION:
--*       The test for testing vss device cases.
--*       tests for 'cc7' in vss4.  (system described in examples/configurations/vss_cc7_vss4.txt)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev 
SUPPORTED_FEATURE_DECLARE(devNum, "VSS-CC")
--- AC3X need proper ports for this test
-- devEnv.port[3] must support the "DSA_CASCADE"
-- the call to SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM may change the value of devEnv.port[3] !!!
SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM(devNum, "DSA_CASCADE" , "ports" , {devEnv.port[3]}) 

-----------------------*******************
-- get extra packet's util functions
-- use dofile instead of require .. to allow quick load of test
dofile("dxCh/examples/common/vss_utils.lua") 


--set config
executeLocalConfig(luaTgfBuildConfigFileName("vss_cc7_vss4")) 
--run the tests for this device
util_vss_run_tests(devicesName_vss4_cc7)
--unset config
executeLocalConfig(luaTgfBuildConfigFileName("vss_cc7_vss4",true))
-- do flush after the 'restore' -- bug in lua flush set mask to '0x10' for hwDevNum
executeLocalConfig(luaTgfBuildConfigFileName("flush_fdb"))

-- print failed cases
luaTgfPrintTestResults()
-- clear failed tests
luaTgfClearTestResults()






