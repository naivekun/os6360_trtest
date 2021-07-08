--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cnc1.lua
--*
--* DESCRIPTION:
--*       The test for cnc feature
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local payloads

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]

local ret, status

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5") 

--generate mac pdu test packets

ret, payloads = pcall(dofile, "dxCh/examples/packets/vlan10_20.lua")

--reset ports and CPU counters
resetPortCounters(devNum, port1)
resetPortCounters(devNum, port2)
resetPortCounters(devNum, port3)

--------------------------------------------------------------------------------
--load configuration for 1st part of test
executeLocalConfig("dxCh/examples/configurations/cnc1.txt")
executeLocalConfig("dxCh/examples/configurations/loopback.txt")

printLog("Testing port " .. port2 .. " not getting traffic in vlan 10")
printLog("Sending traffic ..")
ret, status = sendPacket({[devNum]={port1}}, payloads["vlan10"])
delay(100)
printLog("Check counters ..")
checkExpectedCounters(debug.getinfo(1).currentline,
                        {[devNum] = { [port1]  = {Rx=1},
                                 [port2] = {Rx=0, Tx = 0},
                                 [port3] = {Rx=0, Tx = 1}        }})


printLog("Testing port " .. port3 .. " not getting traffic in vlan 20")
printLog("Sending traffic ..")
ret, status = sendPacket({[devNum]={port1}}, payloads["vlan20"])
delay(100)
printLog("Check counters ..")
checkExpectedCounters(debug.getinfo(1).currentline,
                        {[devNum] = { [port1]  = {Rx=1},
                                 [port2] = {Rx=0, Tx = 1},
                                 [port3] = {Rx=0, Tx = 0}        }})

printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/cnc1_deconfig.txt")
executeLocalConfig("dxCh/examples/configurations/loopback_deconfig.txt")


