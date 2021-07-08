--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* remote_switch_port_analyzer.lua
--*
--* DESCRIPTION:
--*       Check a transmitting of Tx/Rx mirrored packet to analyzer port 
--*       with remote analyzer VLAN tag.
--*       - enable Rx/Tx mirror and Rx/Tx analyzer ports
--*       - enable analyzer port adding additional VLAN Tag to mirrored packets. 
--*       - set global Rx/Tx mirroring VLAN tag details: EtherType, VID, VPT, CFI
--*       - send traffic to destination port
--*       - capture traffic on analyzer ports - chack analyzer VLAN tags added to packets
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- local declaration
local packetsFile  = "dxCh/examples/packets/remote_switch_port_analyzer.lua"

local configFile   = "dxCh/examples/configurations/remote_switch_port_analyzer.txt"
local deconfigFile = "dxCh/examples/configurations/remote_switch_port_analyzer_deconfig.txt"


local devNum = devEnv.dev

local port1 = devEnv.port[1]
local port2 = devEnv.port[2]
local port3 = devEnv.port[3]
local port4 = devEnv.port[4]


SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5")

local function callWithErrorHandling(cpssApi, params)
   local rc, values = myGenWrapper(cpssApi, params,1)
   if rc ~= 0 then
      setFailState()
      print("error of calling "..cpssApi..": "..returnCodes[rc])
   end
   return rc, values
end

-- enables(isEnable = true) or disables(isEnable = false) adding of VLAN tag on analyzer port.
local function mirrorAnalyzerVlanTag(isEnable)
   local rxAnalyzerPort = port3 
   local txAnalyzerPort = port4 
   local vlan1 = 0;
   local vlan2 = 0;
   local etherType1 = 0x8100
   local etherType2 = 0x8100
   if isEnable then
      etherType1 = 0x8110
      etherType2 = 0x8120
      vlan1 = 500
      vlan2 = 1000
   end

   -- setup analyzer port adding additional VLAN Tag to mirrored packets.
   callWithErrorHandling(
      "cpssDxChMirrorAnalyzerVlanTagEnable",
      {
         {"IN", "GT_U8",       "devNum",            devNum},
         {"IN", "GT_PORT_NUM", "portNum",           rxAnalyzerPort},
         {"IN", "GT_BOOL",     "enable",            isEnable}
      }
   )

   callWithErrorHandling(
      "cpssDxChMirrorAnalyzerVlanTagEnable",
      {
         {"IN", "GT_U8",       "devNum",            devNum},
         {"IN", "GT_PORT_NUM", "portNum",           txAnalyzerPort},
         {"IN", "GT_BOOL",     "enable",            isEnable}
      }
   )

   -- assing an interface to a rx analyzer index
   local vlanCfg = {
       etherType   = etherType1,
       vpt         = 0,
       cfi         = 0,
       vid         = vlan1
   } 

   -- set global Tx mirroring VLAN tag details: EtherType, VID, VPT, CFI
   callWithErrorHandling(
      "cpssDxChMirrorRxAnalyzerVlanTagConfig",
      {
         {"IN", "GT_U8",       "devNum",            devNum},
         {"IN", "CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC",     "analyzerVlanTagConfigPtr",   vlanCfg}
      }
   )

   -- assing an interface to a tx analyzer index
   local vlanCfg = {
       etherType   = etherType2,
       vpt         = 0,
       cfi         = 0,
       vid         = vlan2
   } 

   -- set global Tx mirroring VLAN tag details: EtherType, VID, VPT, CFI
   callWithErrorHandling(
      "cpssDxChMirrorTxAnalyzerVlanTagConfig",
      {
         {"IN", "GT_U8",       "devNum",            devNum},
         {"IN", "CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC",     "analyzerVlanTagConfigPtr",   vlanCfg}
      }
   )

end

-- return false if an error occcured and true overwise.
local function testBody(...)
   local isOk = true
   local rc = 1 -- not GT_OK

    mirrorAnalyzerVlanTag(true)

    for _, traffic in ipairs({...}) do
      -- send the packet and check captured packets
      rc = luaTgfTransmitPacketsWithExpectedEgressInfo(traffic.transmitInfo, traffic.egressInfo)
      if rc~=0 then
         setFailState()
         print("packet transmitting error: ".. rc)
         break
      end
    end
    
    mirrorAnalyzerVlanTag(false)

   return (rc==0)
end

------------------------------ MAIN PART ---------------------------------------

-- read the packets from the file
local ret, values = pcall(dofile, packetsFile)
if not ret then
   error("\nError while reading the the packets:"..values)
end

local traffic = {
   transmitInfo = {
      devNum = devNum,
      portNum = port1, pktInfo = {fullPacket = values.rx1}},
   egressInfo = {
      {portNum = port2, packetCount = 1, pktInfo = {fullPacket = values.tx2} , captureMode = "pcl"},
      {portNum = port3, packetCount = 1, pktInfo = {fullPacket = values.tx3} , captureMode = "pcl"},
      {portNum = port4, packetCount = 1, pktInfo = {fullPacket = values.tx4} , captureMode = "pcl"}
}}

local isOk = true;

print("")
isOk = luaTgfSimpleTest(configFile, deconfigFile, testBody, traffic)

return isOk
