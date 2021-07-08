--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* linkLocalMCFiltering.lua
--*
--* DESCRIPTION:
--*       The test for testing excluding link local multicast packets from
--*       eVLAN unregistered multicast command
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local sendData
local counters1, counters2
local ret, status

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]

-- this test is not relevant for ch1, ch2 devices
NOT_SUPPORTED_DEV_DECLARE(devNum, "CPSS_PP_FAMILY_CHEETAH_E", "CPSS_PP_FAMILY_CHEETAH2_E")

--generate broadcast packet
do
  ret, sendData = pcall(dofile, "dxCh/examples/packets/linkLocalMCFiltering.lua")

  if not ret then
    printLog ('Error in packet')
    setFailState()
    return
  end
end

local debug_on = true
local function _debug(debugString)
    if debug_on == true then
        print (debugString)
    end
end

transmitInfo = {
    devNum  = devNum,
    portNum = port1,
    pktInfo = {fullPacket = sendData}
}

--reset ports counters
resetPortCounters(devEnv.dev, devEnv.port[1])
resetPortCounters(devEnv.dev, devEnv.port[2])
resetPortCounters(devEnv.dev, devEnv.port[3])

executeLocalConfig("dxCh/examples/configurations/linkLocalMCFiltering.txt")

local function callWithErrorHandling(cpssApi, params)
   _debug("Try to call "..cpssApi)
   local rc, value = myGenWrapper(cpssApi, params,1)
   if rc ~= 0 then
       setFailState()
       _debug("ERROR of calling "..cpssApi..": "..returnCodes[rc])
   else
       _debug("OK of calling "..cpssApi)

   end
   return rc, value
end

-- Set "DROP_HARD" command for "IPv4 unregistered MC" packets in eVLAN 5
callWithErrorHandling(
    "cpssDxChBrgVlanUnkUnregFilterSet",{
    { "IN", "GT_U8",    "devNum",  devNum  },
    { "IN", "GT_U16",   "vlanId",  5  },
    { "IN", "CPSS_DXCH_BRG_VLAN_PACKET_TYPE_ENT",   "packetType",  "CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_MCAST_E"  },
    { "IN", "CPSS_PACKET_CMD_ENT",   "cmd",  "CPSS_PACKET_CMD_DROP_HARD_E"  }
})

-- Enable IP control traffic trapping/mirroring to CPU (IPv4 Control Protocols Running Over Link-Local Multicast)
callWithErrorHandling(
    "cpssDxChBrgVlanIpCntlToCpuSet",{
    { "IN", "GT_U8",    "devNum",  devNum  },
    { "IN", "GT_U16",   "vlanId",  5  },
    { "IN", "CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT",   "ipCntrlType",  "CPSS_DXCH_BRG_IP_CTRL_IPV4_E"  }
})

-- enable mirroring to CPU for IP Link Local Control protocol 13
callWithErrorHandling(
    "cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable",{
    { "IN", "GT_U8",   "devNum",  devNum  },
    { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",   "protocolStack",  "CPSS_IP_PROTOCOL_IPV4_E"  },
    { "IN", "GT_U8",   "protocol",  13  },
    { "IN", "GT_BOOL", "enable",  true }
})

-- Enable mirroring to CPU IPv4 packets with link-local Multicast addresses in range 224.0.0.0/24
callWithErrorHandling(
    "cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable",{
    { "IN", "GT_U8",   "devNum",  devNum  },
    { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",   "protocolStack",  "CPSS_IP_PROTOCOL_IPV4_E"  },
    { "IN", "GT_BOOL", "enable",  true }
})

-- Exclude unregistered "link local IPv4 MC" packets in command for "IPv4 unregistered MC" in eVLAN 5
callWithErrorHandling(
    "cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet",{
    { "IN", "GT_U8",   "devNum",  devNum  },
    { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",   "protocolStack",  "CPSS_IP_PROTOCOL_IPV4_E"  },
    { "IN", "GT_BOOL", "enable",  true }
})

-- packet is forwarded
egressMirrorInfoTable = {
    {portNum = port2  , packetCount = 0 },
    {portNum = port3  , packetCount = 1 }
}

-- transmit packet and check that egress as expected
ret = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressMirrorInfoTable)
if ret ~= 0 then
    printLog ("Mirror packet test failed \n")
    setFailState()
end

--Clear FDB Table
wraplCpssDxChBrgFdbFlush(devNum,1);

-- Include unregistered "link local IPv4 MC" packets from command for "IPv4 unregistered MC" in eVLAN 5
callWithErrorHandling(
    "cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet",{
    { "IN", "GT_U8",   "devNum",  devNum  },
    { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",   "protocolStack",  "CPSS_IP_PROTOCOL_IPV4_E"  },
    { "IN", "GT_BOOL", "enable",  false }
})

-- packet is dropped
egressMirrorInfoTable = {
    {portNum = port2  , packetCount = 0 },
    {portNum = port3  , packetCount = 1 }
}

-- transmit packet and check that egress as expected
ret = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressMirrorInfoTable)
if ret ~= 0 then
    printLog ("Mirror packet test failed \n")
    setFailState()
end

-- enable mirroring to CPU for IP Link Local Control protocol 13
callWithErrorHandling(
    "cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable",{
    { "IN", "GT_U8",   "devNum",  devNum  },
    { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",   "protocolStack",  "CPSS_IP_PROTOCOL_IPV4_E"  },
    { "IN", "GT_U8",   "protocol",  13  },
    { "IN", "GT_BOOL", "enable",  false }
})

-- packet is dropped
egressMirrorInfoTable = {
    {portNum = port2  , packetCount = 0 },
    {portNum = port3  , packetCount = 0 }
}

-- transmit packet and check that egress as expected
ret = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressMirrorInfoTable)
if ret ~= 0 then
    printLog ("Mirror packet test failed \n")
    setFailState()
end

printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/linkLocalMCFiltering_deconfig.txt")
