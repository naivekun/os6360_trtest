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
--*       An 'all' (i.e both ingress and egress) translation is tested.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5") 

require ("dxCh/examples/common/common") -- vlanMappingTest function declaration

-- a packet should be captured on port 18 with vlan 5
return vlanMappingTest(
   "dxCh/examples/configurations/vlan_mapping_all.txt",
   "dxCh/examples/configurations/vlan_mapping_all_deconfig.txt",
   "dxCh/examples/packets/vlan_mapping.lua",
   {
      {
         devNum = devEnv.dev,
         portNum = devEnv.port[2],
         packetFields = {ieee802_1q_Tag="81000005"}}
   }
)
