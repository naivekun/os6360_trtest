--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vlan_mapping.lua
--*
--* DESCRIPTION:
--*       The test of a VLAN Translation feature and of appropriate CLI commands.
--*       There are three cases that test different aspect of CLI command:
--*       - test an 'ingress' and 'egress' translation.
--*       - test an 'all'vlan mapping
--*       - test egress vlan mapping with 'use-sid' keyword
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5") 

require ("dxCh/examples/common/common") -- vlanMappingTest function declaration

------------------------------ MAIN PART ---------------------------------------
print("-------------------------------------------------")
print("vlan mapping, test 'ingress' and 'egress' mapping")
return vlanMappingTest(
   "dxCh/examples/configurations/vlan_mapping.txt",
   "dxCh/examples/configurations/vlan_mapping_deconfig.txt",
   "dxCh/examples/packets/vlan_mapping.lua",
   {
      {devNum = devEnv.dev, portNum = devEnv.port[2],
       packetFields={ieee802_1q_Tag="81000007"}}
   }
)

-- print("-------------------------------------------------")
-- print("vlan mapping, test 'ingress' and 'egress' mapping")
-- local rc =vlanMappingTest(
--    "dxCh/examples/configurations/vlan_mapping.txt",
--    "dxCh/examples/configurations/vlan_mapping_deconfig.txt",
--    "dxCh/examples/packets/vlan_mapping.lua",
--    {
--       {portNum = 18, packetFields={ieee802_1q_Tag="81000007"}}
--    }
-- )
-- if rc ~= 0 then error("CASE 1") end


-- print("-------------------------------------------------")
-- print("vlan mapping, test 'all' mapping")

-- local rc = vlanMappingTest(
--    "dxCh/examples/configurations/vlan_mapping_all.txt",
--    "dxCh/examples/configurations/vlan_mapping_all_deconfig.txt",
--    "dxCh/examples/packets/vlan_mapping.lua",
--    {
--       {portNum = 18, packetFields = {ieee802_1q_Tag="81000005"}}
--    }
-- )
-- if rc ~= 0 then error("CASE 2") end


-- print("-------------------------------------------------")
-- print("vlan mapping, test mapping with 'use-sid' feature")
-- local rc = vlanMappingTest(
--    "dxCh/examples/configurations/vlan_mapping_sid.txt",
--    "dxCh/examples/configurations/vlan_mapping_sid_deconfig.txt",
--    "dxCh/examples/packets/vlan_mapping.lua",
--    {
--       {portNum = 18, packetFields = {ieee802_1q_Tag="81000008"}},
--       {portNum = 36, packetFields = {ieee802_1q_Tag=false,
--                                      ieee802_1q_OuterTag = "81000008",
--                                      ieee802_1q_InnerTag = "81000009"}}
--    }
-- )
-- if rc ~= 0 then error("CASE 3") end
