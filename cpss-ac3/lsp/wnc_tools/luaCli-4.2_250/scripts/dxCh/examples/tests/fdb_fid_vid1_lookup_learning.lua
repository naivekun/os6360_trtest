--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* fdb_fid_vid1_lookup_learning.lua
--*
--* DESCRIPTION:
--*      test an auto/control learning in double tag FDB lookup key mode
--*
--*      case A)
--*      1) Configure double tag FDB lookup key mode and enable auto learning on
--*      the ports.
--*      2) Send two broadcast Ethernet packets to learn MAC SA, VID0, VID1:
--*             port 18: MAC SA 00:11:22:33:22:11, vid0 = 5, vid1 = 10
--*             port 36: MAC SA 00:11:22:33:22:11, vid0 = 5, vid1 = 11
--*      3) Send two packet to port 54 to check the learning
--*             MAC DA 00:11:22:33:22:11, vid0 = 5, vid1 = 10
--*             MAC DA 00:11:22:33:22:11, vid0 = 5, vid1 = 11
--*      4) Ensure the packets was egressed only to ports 18, 36 appropriately
--*      5) Restore configuration
--*
--*      case B)
--*      all the same as case A except using control learning instead of
--*      auto-learning
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- local declarations
local devNum = devEnv.dev
local port1  = devEnv.port[1]
local port2  = devEnv.port[2]
local port3  = devEnv.port[3]
local port4  = devEnv.port[4]
-- this test is relevant for SIP_5_10 devices (BOBCAT 2 B0 and above)
SUPPORTED_SIP_DECLARE(devNum,"SIP_5_10")

local configDir = "dxCh/examples/configurations/"
local configFileAuto     = configDir .."fdb_fid_vid1_lu_learning_auto.txt"
local configFileControl  = configDir .."fdb_fid_vid1_lu_learning_ctrl.txt"
local deconfigFile       = configDir .."fdb_fid_vid1_lu_learning_deconfig.txt"


local macToLearn   = "001122332211" -- should be learned by FDB
local macBroadcast = "ffffffffffff"
local macSaDummy   = "000000000011" -- can be any
local vid0         = "81000005"
local vid1_1       = "8100000a" -- vid1 = 10
local vid1_2       = "8100000b" -- vid1 = 11
local payload      = ""
   .. "000102030405060708090a0b0c0d0e0f"
   .. "101112131415161718191a1b1c1d1e1f"
   .. "202122232425262728292a2b2c2d2e2f"
   .. "303132333435363738393a3b3c3d3e3f"

local pktForLearning1 = macBroadcast..macToLearn..vid0..vid1_1..payload
local pktForLearning2 = macBroadcast..macToLearn..vid0..vid1_2..payload

local pktToCheck1 = macToLearn..macSaDummy..vid0..vid1_1..payload
local pktToCheck2 = macToLearn..macSaDummy..vid0..vid1_2..payload


 -- all packets will be send in order of declaration in the table scenario
local scenario = {
   { -- send a packet to learn MAC SA, VID, VID1_1
      transmitInfo = {devNum = devNum, portNum = port2,
                      pktInfo = {fullPacket = pktForLearning1}}
   },

   { -- send a packet to learn MAC SA, VID, VID1_2
      transmitInfo = {devNum = devNum, portNum = port3,
                       pktInfo = {fullPacket = pktForLearning2}}
   },
   { -- check a MAC SA, VID, VID1_1 was learned (port2 counter should be incremented)
      transmitInfo = {devNum = devNum, portNum = port4,
                    pktInfo = {fullPacket = pktToCheck1}},
      egressInfo = {
         {portNum = port1, packetCount = 0},
         {portNum = port2, packetCount = 1},
         {portNum = port3, packetCount = 0}
      },
   },
   { -- check a MAC SA, VID, VID1_1 was learned (port3 counter should be incremented)
      transmitInfo = {devNum = devNum, portNum = port4,
                    pktInfo = {fullPacket = pktToCheck2}},
      egressInfo = {
         {portNum = port1, packetCount = 0},
         {portNum = port2, packetCount = 0},
         {portNum = port3, packetCount = 1}
      },
   },
}

local function sendAndCheckTraffic(trafficList)
   local rc = 0
   for _, traffic in ipairs(trafficList) do
      rc = luaTgfTransmitPacketsWithExpectedEgressInfo(traffic.transmitInfo,
                                                       traffic.egressInfo)
      if rc~=0 then
         setFailState()
         print("packet transmitting error: ".. returnCodes[rc])
         return false
      end
   end
   return (rc == 0)
end

------------------------------ MAIN PART ---------------------------------------
local isOk
print("-------------------------------------------------")
if is_multi_fdb_instance(devNum) then
    print("automatic learning is not supported ... skip this part .")
    isOk = true
else
    print("FDB double tag lookup key test (with automatic learning)")

    isOk = luaTgfSimpleTest(configFileAuto, deconfigFile,
                            sendAndCheckTraffic, scenario);
end
if isOk then
   print("-------------------------------------------------")
   print("FDB double tag lookup key test (with controlled learning)")
   isOk = luaTgfSimpleTest(configFileControl, deconfigFile,
                           sendAndCheckTraffic, scenario);
end

return isOk
