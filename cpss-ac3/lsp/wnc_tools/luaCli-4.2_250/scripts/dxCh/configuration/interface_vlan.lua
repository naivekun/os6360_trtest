--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* interface_vlan.lua
--*
--* DESCRIPTION:
--*       entering of Vlan Interface Configuration mode
--*       deleting of Vlans
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes
require("dxCh/configuration/interface_range_vlan")

--constants 


-------------------------------------------------
-- command registration: interface vlan
-------------------------------------------------
CLI_addCommand("config", "interface vlan", {
  func   = interface_range_vlan_func,
  help   = "Configure an IEEE 802.1 VLAN",
  params = {
      { type="named",
          "#all_device",
          "#configurable_vlan",
        mandatory = { "all_device", "configurable_vlan" }
      }
  }
})


-------------------------------------------------
-- command registration: no interface vlan
-------------------------------------------------
CLI_addCommand("config", "no interface vlan", {
  func   = no_interface_range_vlan_func,
  help   = "Configure an IEEE 802.1 VLAN",
  params = {
      { type="named",
          "#all_device",
          "#deleted_vlan",
        mandatory = { "all_device", "deleted_vlan" }
      }
  }
})

CLI_addCommand("interface", "no interface vlan", {
  func   = no_interface_range_vlan_func,
  help   = "Configure an IEEE 802.1 VLAN",
  params = {
      { type="named",
          "#all_device",
          "#deleted_vlan",
        mandatory = { "all_device", "deleted_vlan" }
      }
  }
})
