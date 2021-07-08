--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* spbm_route_inter_i_sid.lua
--*
--* DESCRIPTION:
--*       A inter I-SID SPBM routing test. Check a transmitting of routed
--*       packet through the SPBM network with different I-SID
--*       A packet came to devA/0 wit DIP = 172.16.2.1.
--*       Device C terminates a tunnel header and routes the packet through the
--*       Backbone network back to devB/5 with I-SID = 0x020000.
--*       dev B terminates a tunnel header and the packet is
--*       finally egressed to a client port devB/1. I.e. a packet's path is:
--*           A/0 -> A/54 -> B/54 -> B/5 -> C/5 -> C/5 -> B/5 -> B/0
--*
--*       The core network involves three devices A, B, C. Devices has client
--*       ports too (.e. they are PE-devices)
--*       P-device. The Backbone devices inter-connections are:
--*       A, port 4 <-> B, port 4
--*       B, port 5 <-> C, port 4
--*       A client ports on the devices are connected to subnetworks:
--*           device A port 0 - 172.16.1/24
--*           device B port 0 - 172.16.2/24
--*           device C port 0 - 172.16.3/24
--*
--*      A more detailed description of the Backbone network and route entries
--*      you can find in config/deconfig-files of the test.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- local declaration
local packetsFile    = "dxCh/examples/packets/spbm_route_inter_i_sid.lua"

local configFileA   = "dxCh/examples/configurations/spbm_route_inter_i_sid_dev_a.txt"
local deconfigFileA = "dxCh/examples/configurations/spbm_route_inter_i_sid_dev_a_deconfig.txt"

local configFileB   = "dxCh/examples/configurations/spbm_route_inter_i_sid_dev_b.txt"
local deconfigFileB = "dxCh/examples/configurations/spbm_route_inter_i_sid_dev_b_deconfig.txt"

local configFileC   = "dxCh/examples/configurations/spbm_route_inter_i_sid_dev_c.txt"
local deconfigFileC = "dxCh/examples/configurations/spbm_route_inter_i_sid_dev_c_deconfig.txt"

local devNum = devEnv.dev
local hwDevNum

local port1 = devEnv.port[1]
local port2 = devEnv.port[2]
local port3 = devEnv.port[3]
local port4 = devEnv.port[4]
local port5 = devEnv.port[5]

local ePortToDevB = 1002  -- an ePort leading to the device B through the tunnel.

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5") 

local function callWithErrorHandling(cpssApi, params)
   local rc, values = myGenWrapper(cpssApi, params,1)
   if rc ~= 0 then
      setFailState()
      print("error of calling "..cpssApi..": "..returnCodes[rc])
   end
   return rc, values
end

-- enables(isEnable = false) or disables(isEnable = false) mirroring on ePort.
local function configMirroring(isEnable)
   isEnable = isEnable or false
   local txAnalyzerIndex = 0
   local txAnalyzerPort = 0 -- default
   if isEnable then
      txAnalyzerPort = port2
   end

   -- just in case if the forwarding mode is  HOP-BY-HOP
   callWithErrorHandling(
      "cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet",
      {
         {"IN", "GT_U8",   "devNum", devNum},
         {"IN", "GT_BOOL", "enable", isEnable},
         {"IN", "GT_U32",  "index",  txAnalyzerIndex}
      }
   )

   -- assing an interface to a tx analyzer index
   local iface =  {type    = "CPSS_INTERFACE_PORT_E",
                   devPort = { devNum  = hwDevNum,
                               portNum = txAnalyzerPort}}
   callWithErrorHandling(
      "cpssDxChMirrorAnalyzerInterfaceSet",
      {
         {"IN", "GT_U8",  "devNum", devNum},
         {"IN", "GT_U32", "index", txAnalyzerIndex},
         {"IN", "CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC", "interfacePtr", {interface = iface}}
      }
   )

   -- setup tx ePort to be mirrored
   callWithErrorHandling(
      "cpssDxChMirrorTxPortSet",
      {
         {"IN", "GT_U8",       "devNum",         devNum},
         {"IN", "GT_PORT_NUM", "mirrPort",       ePortToDevB},
         {"IN", "GT_BOOL",     "isPhysicalPort", false},
         {"IN", "GT_BOOL",     "enable",         isEnable} ,
         {"IN", "GT_U32",      "index",          txAnalyzerIndex} -- ignored in the HOP-BY-HOP mode
      }
   )
end

-- return false if an error occurred and true otherwise.
local function testBody(...)
   local isOk = true
   local rc = 1 -- not GT_OK

   for _, traffic in ipairs({...}) do
      -- send the packet and check captured packets
      rc = luaTgfTransmitPacketsWithExpectedEgressInfo(traffic.transmitInfo,
                                                             traffic.egressInfo)
      if rc~=0 then
         setFailState()
         print("packet transmitting error: ".. returnCodes[rc])
         return false
      end
   end
   return (rc==0)
end

local function testBodyC(traffic1, traffic2)
   local isOk = true;
   local ipRouteCmd = {
      "end",
      "configure",
      "ip route 172.16.2.1 255.255.255.255 nexthop 11 vrf-id 1",
      "end"
   };
   local noIpRouteCmd = {
      "end",
      "configure",
      "no ip route 172.16.2.1 255.255.255.255 vrf-id 1",
      "end"
   };

   configMirroring(true)

   if isOk then
      for i = 1,#(ipRouteCmd) do
         print("Performing: " .. ipRouteCmd[i])
         isOk = cmdLuaCLI_execute(ipRouteCmd[i]) and isOk
      end
      if not isOk then print("ipRouteCmd  error"); end
   end

   if isOk then
      isOk = testBody(traffic1)
      if not isOk then print("testBody1  error"); end
   end

   if isOk then
      for i = 1,#(noIpRouteCmd) do
         print("Performing: " .. noIpRouteCmd[i])
         isOk = cmdLuaCLI_execute(noIpRouteCmd[i]) and isOk
      end
      if not isOk then print("noIpRouteCmd  error"); end
   end

   if isOk then
      isOk = testBody(traffic2)
      if not isOk then print("testBody2  error"); end
   end

   configMirroring(false);

   if not isOk then  setFailState() end

   return isOk
end

------------------------------ MAIN PART ---------------------------------------

-- hwDevNum/port is an Analyzer interface (a mirroring is usedon the device C)
local rc, value = device_to_hardware_format_convert(devNum)
if rc ~= 0 then
   error("\n"..result)
end
hwDevNum = value                -- a local variable

-- read the packets from the file
local ret, values = pcall(dofile, packetsFile)
if not ret then
   error("\nError while reading the the packets:"..values)
end

local trafficA = {
   transmitInfo = {
      devNum = devNum,
      portNum = port1, pktInfo = {fullPacket = values.rxA}},
   egressInfo = {
      {portNum = port2, packetCount = 0},
      {portNum = port3, packetCount = 0},
      {portNum = port4, packetCount = 1, pktInfo = {fullPacket = values.txA}}
}}

-- a transit packet (with tunnel header) from device A to device C
local trafficB1 = {
   transmitInfo = {
      devNum = devNum,
      portNum = port4, pktInfo = {fullPacket = values.txA}},
   egressInfo = {
      {portNum = port1, packetCount = 0},
      {portNum = port2, packetCount = 0},
      {portNum = port3, packetCount = 0},
      {portNum = port5, packetCount = 1, pktInfo = {fullPacket = values.txA}}
}}

-- a packet that came from device C and should be egressed locally
local trafficB2 = {
   transmitInfo = {
      devNum = devNum,
      portNum = port5, pktInfo = {fullPacket = values.txC}},
   egressInfo = {
      {portNum = port1, packetCount = 1, pktInfo = {fullPacket = values.txB}},
      {portNum = port2, packetCount = 0},
      {portNum = port3, packetCount = 0},
      {portNum = port4, packetCount = 0}
}}

local trafficC1 = {
   transmitInfo = {
      devNum = devNum,
      portNum = port4, pktInfo = {fullPacket = values.txA},
                   loopbackStorming = true},
   egressInfo = {
      {portNum = port1, packetCount = 0},
      {portNum = port2, packetCount = 1},
      -- port2 gets storm of egress mirrored packets -
      -- {portNum = port2, pktInfo = {fullPacket = values.txC}}, -- mirrored packet
      {portNum = port3, packetCount = 0},
}}

local trafficC2 = {
   transmitInfo = {
      devNum = devNum,
      portNum = port4, pktInfo = {fullPacket = values.txA},},
   egressInfo = {
      {portNum = port1, packetCount = 0},
      {portNum = port2, packetCount = 0},
      {portNum = port3, packetCount = 0},
      -- {portNum = port4, packetCount = 1}
}}

xcat3x_start(true)

local isOk = true;
if isOk then
   print("-------------------------------------------------")
   print("SPBM route inter I-DIS test, device A")
   isOk = luaTgfSimpleTest(configFileA, deconfigFileA, testBody, trafficA)
end
if isOk then
   print("-------------------------------------------------")
   print("SPBM route inter I-DIS test, device B")
   isOk = luaTgfSimpleTest(configFileB, deconfigFileB, testBody, trafficB1, trafficB2)
end
if isOk then
   print("-------------------------------------------------")
   print("SPBM route inter I-DIS test, device C")
   isOk = luaTgfSimpleTest(configFileC, deconfigFileC, testBodyC, trafficC1, trafficC2)
end

xcat3x_start(false)