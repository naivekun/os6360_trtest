--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vss_lc1_vss4.lua
--*
--* DESCRIPTION:
--*       The test for testing vss device cases.
--*       tests for 'lc1' in vss4.  (system described in examples/configurations/vss_lc1_vss4.txt)
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

local port1   = getTestPort(1)-- dsa tag port
local port2   = getTestPort(2)-- dsa tag port
local port3   = getTestPort(3)
local port4   = getTestPort(4)-- dsa tag port

local port5   = getTestPort(5)
local portsInTest = {port1,port2,port3,port4,port5}

local port_peer_4   = port4 - 1 -- 22
local port_peer_2   = port2 + 1 -- 9
local port_peer_1   = port1 + 1 -- 1
-- define the ports needed for send/capture dsa tag packets on xcat devices 
local peersInfo = {{port4,port_peer_4},{port1,port_peer_1},{port2,port_peer_2}}

-- init tested ports : force link up + reset mac counters
luaTgfConfigTestedPorts(devNum,portsInTest,true) 

-- call the engine about the 'peer ports'
luaTgfStatePhysicalConnectedPairs(devNum,peersInfo)

--set config
executeLocalConfig(luaTgfBuildConfigFileName("vss_lc1_vss4")) 
--run the tests for this device
util_vss_run_tests(devicesName_vss4_lc1)
--unset config
executeLocalConfig(luaTgfBuildConfigFileName("vss_lc1_vss4",true))
-- do flush after the 'restore' -- bug in lua flush set mask to '0x10' for hwDevNum
executeLocalConfig(luaTgfBuildConfigFileName("flush_fdb"))

-- end tested ports : unforce link up + reset mac counters
luaTgfConfigTestedPorts(devNum,portsInTest,false) 

-- call the engine to remove existing 'peer ports'
luaTgfStatePhysicalConnectedPairs()

-- print failed cases
luaTgfPrintTestResults()
-- clear failed tests
luaTgfClearTestResults()


