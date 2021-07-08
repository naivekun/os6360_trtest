--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vss_lc5_vss3.lua
--*
--* DESCRIPTION:
--*       The test for testing vss device cases.
--*       tests for 'lc5' in vss3.  (system described in examples/configurations/vss_lc5_vss3.txt)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev 
SUPPORTED_FEATURE_DECLARE(devNum, "VSS-LC")
-- only on simulation because uses 'peer ports' by using SLANs
SUPPORTED_FEATURE_DECLARE(devNum, "DYNAMIC_SLAN")

-----------------------*******************
-- get extra packet's util functions
-- use dofile instead of require .. to allow quick load of test
dofile("dxCh/examples/common/vss_utils.lua") 


local port1   = getTestPort(1)
local port2   = getTestPort(2)
local port3   = getTestPort(3)
local port4   = getTestPort(4)

local port_peer_4   = port4 - 1 -- 22
local port_peer_1   = port1 + 1 -- 1
-- define the ports needed for send/capture dsa tag packets on xcat devices 
local peersInfo = {{port4,port_peer_4},{port1,port_peer_1}}
-- call the engine about the 'peer ports'
luaTgfStatePhysicalConnectedPairs(devNum,peersInfo)

--set config
executeLocalConfig(luaTgfBuildConfigFileName("vss_lc5_vss3")) 
--run the tests for this device
util_vss_run_tests(devicesName_vss3_lc5)
--unset config
executeLocalConfig(luaTgfBuildConfigFileName("vss_lc5_vss3",true))
-- do flush after the 'restore' -- bug in lua flush set mask to '0x10' for hwDevNum
executeLocalConfig(luaTgfBuildConfigFileName("flush_fdb"))

-- call the engine to remove existing 'peer ports'
luaTgfStatePhysicalConnectedPairs()

-- print failed cases
luaTgfPrintTestResults()
-- clear failed tests
luaTgfClearTestResults()


