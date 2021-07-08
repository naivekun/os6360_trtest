--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vlan_mapping_all.lua
--*
--* DESCRIPTION:
--*       The test ofa  VLAN Translation feature and of appropriate CLI commands.
--*       An 'egress use-sid' translation is tested.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]
-- this test is relevant for SIP_5_10 devices (BOBCAT 2 B0 and above)
SUPPORTED_SIP_DECLARE(devNum,"SIP_5_10")

require ("dxCh/examples/common/common") -- vlanMappingTest function declaration

-- a packet should be captured on port 18 (vid0=8), on port 36(vid0=8, vid1=9)
return vlanMappingTest(
  "dxCh/examples/configurations/vlan_mapping_sid.txt",
  "dxCh/examples/configurations/vlan_mapping_sid_deconfig.txt",
  "dxCh/examples/packets/vlan_mapping.lua",
  {
     {portNum = port2, packetFields = {ieee802_1q_Tag="81000008"}},
     {portNum = port3, packetFields = {ieee802_1q_Tag=false,
                                    ieee802_1q_OuterTag = "81000008",
                                    ieee802_1q_InnerTag = "81000009"}}
  }
)
