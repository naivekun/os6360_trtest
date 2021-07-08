--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* port_isolation2.lua
--*
--* DESCRIPTION:
--*       The test for testing port isolation
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local sendData
local counters1, counters2
local ret, status

-- xcat and above
SUPPORTED_FEATURE_DECLARE(devNum, "PORT_ISOLATION")
 
--reset ports and CPU counters
resetPortCounters(devEnv.dev, devEnv.port[1])
resetPortCounters(devEnv.dev, devEnv.port[2])
resetPortCounters(devEnv.dev, devEnv.port[3])
resetPortCounters(devEnv.dev, devEnv.port[4])

--generate regular packet
do
  ret, sendData = pcall(dofile, "dxCh/examples/packets/vlan10_20.lua")

  if not ret then
    printLog ('Error in packet')
    setFailState()
    return
  end
end

--we expect flooding on the MAC DA ... so make sure FDB is empty
pcall(executeLocalConfig,"dxCh/examples/configurations/flush_fdb.txt") 

executeLocalConfig("dxCh/examples/configurations/port_isolation2.txt")
executeLocalConfig("dxCh/examples/configurations/loopback.txt")

printLog("Sending traffic ..")
ret,status = sendPacket({[devEnv.dev] = {devEnv.port[1]}}, sendData["vlanNo"])

--check expected counters:
printLog("Check counters port "..devEnv.port[4])
checkExpectedCounters(debug.getinfo(1).currentline,
                         {[devEnv.dev] = { [devEnv.port[2]] = {Rx=0, Tx=1},
                                  [devEnv.port[3]] = {Rx=0, Tx=0},
                                  [devEnv.port[4]] = {Rx=0, Tx=1}}})

printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/port_isolation2_deconfig.txt")
executeLocalConfig("dxCh/examples/configurations/loopback_deconfig.txt")
