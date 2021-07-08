--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* spbm_route_test1_deva.lua
--*
--* DESCRIPTION:
--*       An SPBM routing test: local bridging.
--*       A tagged Ethernet frame came to a device A, port 1.
--*       The packet should be flooded only to port devA/2.
--*       This test checks a packet processing inside the device A.
--*
--*       The core network involves three devices A, B, C. A, C are provider
--*       egde bridges(PE devices), B plays two roles: PE-device and P-device.
--*       device). The devices inter-connections are:
--*       A, port 54 <-> B, port 54
--*       B, port 5 <-> C, port 54
--*       A VPN1 is on ports A/0,18; B/36;   C/0
--*       A VPN2 is on ports A/36;   B/0,18; C/18
--*       This test checks a packet processing inside all devices A, B, C.
--*
--*      A more detailed description of the Backbone network you can find
--*      in config/deconfig-files of the test.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- local declaration
local strPacketFileName = "dxCh/examples/packets/spbm_route_local_bridging.lua"
local strConfigFileName = "dxCh/examples/configurations/spbm_route_vpn_dev_a.txt"
local strDeconfigFileName =  "dxCh/examples/configurations/spbm_route_vpn_dev_a_deconfig.txt"

local devNum = devEnv.dev
local port1 = devEnv.port[1]
local port2 = devEnv.port[2]
local port3 = devEnv.port[3]
local port4 = devEnv.port[4]

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5") 

local function testBody()
   -- read the original packet
   local ret, origPacket = pcall(dofile, strPacketFileName)
   assert(ret, 'Error while reading the packets: '.. origPacket)

   -- a packet that should be captured on the port 2
   local txPacket2 = origPacket

   -- send the packet and check captured packets
   local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
      {  -- transmitInfo
         devNum = devNum,
         portNum = port1, pktInfo = {fullPacket = origPacket}
      },
      {  -- expected packets
         {portNum = port2, pktInfo = {fullPacket = txPacket2}, packetCount = 1},
         {portNum = port3, packetCount = 0},
         {portNum = port4, packetCount = 0},
      }
   )
   assert(rc==0, string.format("packet transmitting error: %s",
                               returnCodes[rc]))
end


------------------------------ MAIN PART ---------------------------------------
print("-------------------------------------------------")
print("SPBM route test #1 (local bridging), device A")

-- run a configuration
if not executeLocalConfig(strConfigFileName) then
   setFailState()
   print("error while running config " .. strConfigFileName)
else
   local isOk, errMsg = pcall(testBody)  -- catch exceptions in testBody()
   if not isOk then
      print(errMsg)
      setFailState()
   end
end

-- clear configuration
if not executeLocalConfig(strDeconfigFileName) then
   setFailState()
   print("executeLocalConfig " .. strDeconfigFileName)
end
