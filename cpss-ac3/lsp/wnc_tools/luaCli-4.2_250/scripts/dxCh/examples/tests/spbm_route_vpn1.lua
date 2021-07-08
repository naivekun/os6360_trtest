--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* spbm_route_vpn1.lua
--*
--* DESCRIPTION:
--*       An SPBM routing test: UC Routing in VPN-1.
--*       Send 3 IP UC packets to a device A, port 1. The packets will be routed:
--*       172.16.2.1 ->MAC SA = devA, MAC DA = 0x121, vlan=2000, port = devA/2.
--*       172.16.3.1 ->MAC SA = devB, MAC DA = 0x131, untagged, port = devB/3.
--*       172.16.4.1 ->MAC SA = devC, MAC DA = 0x141, untagged, port = devC/1.
--*
--*       The core network involves three devices A, B, C. A, C are provider
--*       egde bridges(PE devices), B plays two roles: PE-device and P-device.
--*       The devices inter-connections are:
--*       A, port 4 <-> B, port 4
--*       B, port 5 <-> C, port 4
--*       A VPN1 is on ports A/1,2; B/3;   C/1
--*       A VPN2 is on ports A/3;   B/1,2; C/2
--*       This test checks a packet processing inside all devices A, B, C.
--*
--*      A more detailed description of the Backbone network you can find
--*      in config/deconfig-files of the test.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5") 

-- local declaration
local strPacketFileName   = "dxCh/examples/packets/spbm_route_vpn1.lua"

local configFileA   = "dxCh/examples/configurations/spbm_route_vpn_dev_a.txt"
local deconfigFileA = "dxCh/examples/configurations/spbm_route_vpn_dev_a_deconfig.txt"

local configFileB   = "dxCh/examples/configurations/spbm_route_vpn_dev_b.txt"
local deconfigFileB = "dxCh/examples/configurations/spbm_route_vpn_dev_b_deconfig.txt"

local configFileC   = "dxCh/examples/configurations/spbm_route_vpn_dev_c.txt"
local deconfigFileC = "dxCh/examples/configurations/spbm_route_vpn_dev_c_deconfig.txt"

local devNum = devEnv.dev
local port1 = devEnv.port[1]
local port2 = devEnv.port[2]
local port3 = devEnv.port[3]
local port4 = devEnv.port[4]
local port5 = devEnv.port[5]

-- check the packets transmission on the device B. A function can generate exceptions
local function testDevA(packets)
   local rxPackets = packets.rxA
   local txPackets = packets.txA

   -- values for second agrument of luaTgfTransmitPacketsWithExpectedEgressInfo
   local expectedPortPackets = {
      {
         {portNum = port2, packetCount = 1, pktInfo = {fullPacket = txPackets[1]}},
         {portNum = port3, packetCount = 0},
         {portNum = port4, packetCount = 0}
      },
      {
         {portNum = port2, packetCount = 0},
         {portNum = port3, packetCount = 0},
         {portNum = port4, packetCount = 1, pktInfo = {fullPacket = txPackets[2]}}
      },
      {
         {portNum = port2, packetCount = 0},
         {portNum = port3, packetCount = 0},
         {portNum = port4, packetCount = 1, pktInfo = {fullPacket = txPackets[3]}}
      }
   }

   -- send the packet and check captured packets
   for i=1, #rxPackets do
      print("========================================")
      print("Send a packet #"..i)

      local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
         {  -- transmitInfo
            devNum = devNum,
            portNum = port1, pktInfo = {fullPacket = rxPackets[i]}
         }, expectedPortPackets[i])

      assert(rc==0, string.format("packet transmitting error: %s",
                                  returnCodes[rc]))
   end
end

-- check the packets transmission on the device B. A function can generate exceptions
local function testDevB(packets)
   local rxPackets = {packets.txA[2], packets.txA[3]}
   local txPackets = {packets.txB[1], packets.txA[3]}

   local expectedPortPackets = {
      {
         {portNum = port1, packetCount = 0},
         {portNum = port2, packetCount = 0},
         {portNum = port3, packetCount = 1, pktInfo = {fullPacket = txPackets[1]}},
         {portNum = port5, packetCount = 0}
      },
      {
         {portNum = port1, packetCount = 0},
         {portNum = port2, packetCount = 0},
         {portNum = port3, packetCount = 0},
         {portNum = port5, packetCount = 1, pktInfo = {fullPacket = txPackets[2]}}
      }
   }

   -- send the packet and check captured packets
   for i=1, #rxPackets do
      print("========================================")
      print("Send a packet #"..i)
      local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
         {  -- transmitInfo
            devNum = devNum,
            portNum = port4, pktInfo = {fullPacket = rxPackets[i]}
         }, expectedPortPackets[i])

      assert(rc==0, string.format("packet transmitting error: %s",
                                  returnCodes[rc]))
   end
end


-- check the packets transmission on the device C. A function can generate exceptions
local function testDevC(packets)
   local rxPackets = {packets.txA[3]}
   local txPacket1 = packets.txC[1]

   -- values for second agrument of luaTgfTransmitPacketsWithExpectedEgressInfo
   local expectedPortPackets = {
      {
         {portNum = port1, packetCount = 1, pktInfo = {fullPacket = txPacket1}},
         {portNum = port2, packetCount = 0},
         {portNum = port3, packetCount = 0}
      }
   }
   -- send the packet and check captured packets
   for i=1, #rxPackets do
      print("========================================")
      print("Send a packet #"..i)
      local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
         {  -- transmitInfo
            devNum = devNum,
            portNum = port4, pktInfo = {fullPacket = rxPackets[i]}
         }, expectedPortPackets[i])

      assert(rc==0, string.format("packet transmitting error: %s",
                                  returnCodes[rc]))
   end
end


------------------------------ MAIN PART ---------------------------------------
-- read the packets from the file
local ret, packets = pcall(dofile, strPacketFileName)
if not ret then
   error("\nError while reading the the packets:"..packets)
end

xcat3x_start(true)

local isOk = true;
if isOk then
   print("-------------------------------------------------")
   print("SPBM route test VPN 1 (vrf-id 1), device A")
   isOk = luaTgfSimpleTest(configFileA, deconfigFileA, testDevA, packets)
end
if isOk then
   print("-------------------------------------------------")
   print("SPBM route test VPN 1 (vrf-id 1), device B")
   isOk = luaTgfSimpleTest(configFileB, deconfigFileB, testDevB, packets)
end
if isOk then
   print("-------------------------------------------------")
   print("SPBM route test VPN 1 (vrf-id 1), device C")
   isOk = luaTgfSimpleTest(configFileC, deconfigFileC, testDevC, packets)
end

xcat3x_start(false)
