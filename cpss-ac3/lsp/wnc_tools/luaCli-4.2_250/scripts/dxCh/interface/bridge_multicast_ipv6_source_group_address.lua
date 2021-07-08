--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* bridge_multicast_ipv6_source_group_address.lua
--*
--* DESCRIPTION:
--*       registering/deregistering of source IP address - multicast IP address
--*       pair to the bridge table and assign interfaces to this source-group
--*       pair
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes
require("dxCh/interface/bridge_multicast_mac_address")

--constants


--------------------------------------------------------------------------------
-- command registration: bridge multicast ipv6 source-group address source
--------------------------------------------------------------------------------
CLI_addHelp("vlan_configuration", "bridge multicast ipv6 source-group",
                                    "Register source IP address")
CLI_addHelp("vlan_configuration", "bridge multicast ipv6 source-group address",
                                    "Register source IP address")
CLI_addCommand("vlan_configuration", "bridge multicast ipv6 source-group address source", {
  func   = function(params)
               params.sip = true
               return bridge_multicast_address_group_func(params)
           end,
  help   = "Register source IP address - multicast IP address pair to the " ..
           "bridge table",
  params={
      { type = "values",
          "%ipv6"
      },
      { type = "named",
          "#ipv6_dip",
          "#interface",
          {format = "vid1 %unchecked-vlan"},
          requirements = {["interface"] = {"ipv6_dip"}, ["vid1"] = {"interface"}},
          mandatory = {"ipv6_dip"}
      }
  }
})

--------------------------------------------------------------------------------
-- command registration: no bridge multicast ipv6 source-group address source
--------------------------------------------------------------------------------
CLI_addHelp("vlan_configuration", "no bridge multicast ipv6 source-group",
                                    "Register source IP address")
CLI_addHelp("vlan_configuration", "no bridge multicast ipv6 source-group address",
                                    "Register source IP address")
CLI_addCommand("vlan_configuration", "no bridge multicast ipv6 source-group " ..
                                     "address source", {
  func   = function(params)
               params.sip = true
               return no_bridge_multicast_address_group_func(params)
           end,
  help   = "Deregister source IP address - multicast IP address pair to the " ..
           "bridge table",
  params={
      { type = "values",
          "%ipv6"
      },
      { type = "named",
          "#ipv6_dip",
          {format = "vid1 %unchecked-vlan"},
          requirements = {["vid1"] = {"ipv6_dip"}},
          mandatory = {"ipv6_dip"}
      }
  }
})
