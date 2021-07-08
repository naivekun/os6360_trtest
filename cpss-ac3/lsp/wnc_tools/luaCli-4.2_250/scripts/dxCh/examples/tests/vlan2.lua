--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vlan2.lua
--*
--* DESCRIPTION:
--*       The test for testing vlan2
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local sendData
local counters1, counters2
local ret, status

--generate broadcast packet
do
  ret, sendData = pcall(dofile, "dxCh/examples/packets/vlan2.lua")

  if not ret then
    printLog ('Error in packet')
    setFailState()
    return
  end
end

--reset ports counters
resetPortCounters(devEnv.dev, devEnv.port[1])
resetPortCounters(devEnv.dev, devEnv.port[2])

executeLocalConfig("dxCh/examples/configurations/vlan2.txt")
executeLocalConfig("dxCh/examples/configurations/loopback.txt")

local trgDevPortList = {[devEnv.dev]={devEnv.port[1]}}

--sending packet
printLog("Sending traffic ..")
ret,status = sendPacket(trgDevPortList, sendData)

--check expected counters:
printLog("Check counters ..")
checkExpectedCounters(debug.getinfo(1).currentline,
                         {[devEnv.dev] = { [devEnv.port[1]]  = {Rx=1},
                                  [devEnv.port[2]]  = {Rx=0}  }})

printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/vlan2_deconfig.txt")
executeLocalConfig("dxCh/examples/configurations/loopback_deconfig.txt")
