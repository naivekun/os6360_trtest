--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* port_isolation_lookup_bits.lua
--*
--* DESCRIPTION:
--*       Testing port isolation lookup bits
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local sendData
local counters1, counters2
local ret, status

-- xcat and above
SUPPORTED_FEATURE_DECLARE(devNum, "PORT_ISOLATION_LOOKUP_BITS")

--reset ports and CPU counters
resetPortCounters(devEnv.dev, devEnv.port[1])
resetPortCounters(devEnv.dev, devEnv.port[2])
resetPortCounters(devEnv.dev, devEnv.port[3])
resetPortCounters(devEnv.dev, devEnv.port[4])

--generate broadcast packet
do
  ret, sendData = pcall(dofile, "dxCh/examples/packets/broadcast.lua")

  if not ret then
    printLog ('Error in packet')
    setFailState()
    return
  end
end

executeLocalConfig("dxCh/examples/configurations/port_isolation_lookup_bits.txt")
executeLocalConfig("dxCh/examples/configurations/loopback.txt")



printLog("Sending traffic ..")
ret,status = sendPacket({[devEnv.dev] = {devEnv.port[1]}}, sendData)

--check expected counters:
printLog("Check counters ..")
checkExpectedCounters(debug.getinfo(1).currentline,
                         {[devEnv.dev] = { 
                                           [devEnv.port[1]] = {Rx=1, Tx=1},
                                           [devEnv.port[2]] = {Rx=0, Tx=0},
                                           [devEnv.port[3]] = {Rx=0, Tx=1},
                                           [devEnv.port[4]] = {Rx=0, Tx=1}  
                                           }})



executeStringCliCommands("end");
executeStringCliCommands("configure");
executeStringCliCommands("isolate lookup bits device all portBits 8 devBits 5 trunkBits 7")
executeStringCliCommands("show isolate lookup bits device all")

printLog("Sending traffic ..")
ret,status = sendPacket({[devEnv.dev] = {devEnv.port[1]}}, sendData)

--check expected counters:
printLog("Check counters ..")
checkExpectedCounters(debug.getinfo(1).currentline,
                         {[devEnv.dev] = { 
                                           [devEnv.port[1]] = {Rx=1, Tx=1},
                                           [devEnv.port[2]] = {Rx=0, Tx=1},
                                           [devEnv.port[3]] = {Rx=0, Tx=1},
                                           [devEnv.port[4]] = {Rx=0, Tx=1}  
                                          }})
executeStringCliCommands("end");
executeStringCliCommands("configure");
executeStringCliCommands("no isolate lookup bits device all")
executeStringCliCommands("show isolate lookup bits device all")




executeStringCliCommands("interface range ethernet ${dev}/${port[1]}")
executeStringCliCommands ("no switchport isolate")
executeStringCliCommands("switchport isolate ${dev}/${port[2]} cmd L2L3")
printLog("Sending traffic ..")
ret,status = sendPacket({[devEnv.dev] = {devEnv.port[1]}}, sendData)

--check expected counters:
printLog("Check counters ..")
checkExpectedCounters(debug.getinfo(1).currentline,
                         {[devEnv.dev] = { 
                                           [devEnv.port[1]] = {Rx=1, Tx=1},
                                           [devEnv.port[2]] = {Rx=0, Tx=0},
                                           [devEnv.port[3]] = {Rx=0, Tx=1},
                                           [devEnv.port[4]] = {Rx=0, Tx=1}  
                                           }})

executeStringCliCommands("end");

printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/port_isolation_lookup_bits_deconfig.txt")
executeLocalConfig("dxCh/examples/configurations/loopback_deconfig.txt")
