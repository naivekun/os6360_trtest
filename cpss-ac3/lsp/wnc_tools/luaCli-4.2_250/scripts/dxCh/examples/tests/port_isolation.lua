--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* port_isolation.lua
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
resetPortCounters(devEnv.dev, devEnv.port[2])
resetPortCounters(devEnv.dev, devEnv.port[3])

--generate broadcast packet
do
  ret, sendData = pcall(dofile, "dxCh/examples/packets/broadcast.lua")

  if not ret then
    printLog ('Error in packet')
    setFailState()
    return
  end
end

executeLocalConfig("dxCh/examples/configurations/port_isolation.txt")
executeLocalConfig("dxCh/examples/configurations/loopback.txt")



printLog("Sending traffic ..")
ret,status = sendPacket({[devEnv.dev] = {devEnv.port[1]}}, sendData)

--check expected counters:
printLog("Check counters ..")
checkExpectedCounters(debug.getinfo(1).currentline,
                         {[devEnv.dev] = { [devEnv.port[2]] = {Rx=0, Tx=0},
                                  [devEnv.port[3]] = {Rx=0, Tx=1}  }})

printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/port_isolation_deconfig.txt")
executeLocalConfig("dxCh/examples/configurations/loopback_deconfig.txt")
