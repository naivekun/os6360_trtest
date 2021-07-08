
--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* drop_vid1_mismatch.lua
--*
--* DESCRIPTION:
--*       The test of egress filtering feature "drop on VID1 mismatch"
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--************************************************************************

--  tryExecuteFile
--        @description  load and execute configuration file line by line.
--                      Generate and exception if an eror occured.
--
--        @param filename  - name of configuration file
--
local function tryExecuteFile(filename)
   if not executeLocalConfig(filename) then
      error("executeLocalConfig " .. filename)
   end
end


--==============================================================================
--===================== DECLARE LOCAL VARIABLES ================================

local devNum  = devEnv.dev
local port0   = devEnv.port[1]
local port1   = devEnv.port[2]
local ret
local status
local packet

-- this test is relevant for SIP_5_10 devices (BOBCAT 2 B0 and above)
SUPPORTED_SIP_DECLARE(devNum,"SIP_5_10")

local cfgNameEnableVid1Filtering =
   "dxCh/examples/configurations/drop_vid1_mismatch.txt"

local cfgNameDisableVid1Filtering =
   "dxCh/examples/configurations/drop_vid1_mismatch_deconfig.txt"

-- CLI commands to create vlan with tagged ports 0, 18
local cmdCreateVlan = [[
end
configure
interface vlan device ${dev} vid 215
exit
interface range ethernet ${dev}/${port[1]},${port[2]}
switchport allowed vlan add 215 tagged
end
]]

local cmdAddFdbEntry = [[
end
configure
mac address-table static 00:00:00:00:34:02 device ${dev} vlan 215 ethernet ${dev}/${port[2]}
end
]]

local cmdClearVlanEntryAndFdb = [[
end
configure
no interface vlan device ${dev} vid 215
do clear bridge interface ${dev} all
exit
end
]]



--==============================================================================
--=============================== MAIN PART ====================================
-- read unicast double vlan tagged packet (tag0 = tag1 = 215)
ret, packet = pcall(dofile, "dxCh/examples/packets/drop_vid1_mismatch.lua")
if not ret then
   printLog ('Error in the network packet')
   setFailState()
   return
end

--reset ports counters
resetPortCounters(devNum, port0)
resetPortCounters(devNum, port1)

-- set loopback on port 0
tryExecuteFile("dxCh/examples/configurations/loopback.txt")

-- create vlan with tagged ports 0, 18
executeStringCliCommands(cmdCreateVlan)

--================ Check multi-target packet(unknown unicast) ==================
printLog("Check multi-targer (unknown unicast) packet")

printLog("Enable egress filtering 'drop VID1 mismatch'")
tryExecuteFile(cfgNameEnableVid1Filtering)

-- [dev] = {ports list}
local trgDevPortList = {[devNum]={port0}}

printLog("Sending traffic ..")
ret,status = sendPacket(trgDevPortList, packet)

printLog("Check counters ..") -- the packet should be filtered on port 18.
checkExpectedCounters(debug.getinfo(1).currentline,
                      {[devNum] = { [port0]  = {Rx=1, Tx=1},
                               [port1]  = {Rx=0, Tx=0}  }})
printLog("Disable egress filtering 'drop VID1 mismatch'")
tryExecuteFile(cfgNameDisableVid1Filtering)

printLog("Sending traffic ..")
ret,status = sendPacket(trgDevPortList, packet)


printLog("Check counters ..") -- the packet should be egressed from port 18
checkExpectedCounters(debug.getinfo(1).currentline,
                      {[devNum] = { [port0]  = {Rx=1, Tx=1},
                               [port1]  = {Rx=0, Tx=1}  }})

--================ Check single-target packet(unknown unicast) =================
printLog("Check unicast (multi-target) packet")

-- add FDB entry with key equal to the packet's vid0/MAC DA
executeStringCliCommands(cmdAddFdbEntry)

printLog("Sending traffic ..")
ret,status = sendPacket(trgDevPortList, packet)

printLog("Check counters ..") -- the packet should be egressed from port 18
checkExpectedCounters(debug.getinfo(1).currentline,
                      {[devNum] = { [port0]  = {Rx=1, Tx=1},
                               [port1]  = {Rx=0, Tx=1}  }})

printLog("Enable egress filtering 'drop VID1 mismatch'")
tryExecuteFile(cfgNameEnableVid1Filtering)

printLog("Sending traffic ..")
ret,status = sendPacket(trgDevPortList, packet)

printLog("Check counters ..") -- the packet should be filtered
checkExpectedCounters(debug.getinfo(1).currentline,
                      {[devNum] = { [port0]  = {Rx=1, Tx=1},
                               [port1]  = {Rx=0, Tx=0}  }})


printLog("Restore configuration ..")
tryExecuteFile(cfgNameDisableVid1Filtering)

-- delete vlan, clear FDB
executeStringCliCommands(cmdClearVlanEntryAndFdb)

tryExecuteFile("dxCh/examples/configurations/loopback_deconfig.txt")
