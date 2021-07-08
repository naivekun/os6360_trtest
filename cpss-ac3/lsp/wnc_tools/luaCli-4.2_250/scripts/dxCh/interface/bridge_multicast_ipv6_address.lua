--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* bridge_multicast_ipv6_address.lua
--*
--* DESCRIPTION:
--*       registering/deregistering of IP-layer multicast address to the bridge
--*       table and assign interfaces to this multicast address
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes
require("dxCh/interface/bridge_multicast_mac_address")

--constants


----------------------------------------------------------------
-- command registration: bridge multicast ipv6 address group
----------------------------------------------------------------
CLI_addHelp("vlan_configuration", "bridge multicast ipv6",
                                    "Register IPv6 multicast address to the bridge table")
CLI_addHelp("vlan_configuration", "bridge multicast ipv6 address",
                                    "Register IPv6 multicast address to the bridge table")
CLI_addCommand("vlan_configuration", "bridge multicast ipv6 address group", {
  func   = function(params)
               params.dip = true
               return bridge_multicast_address_group_func(params)
           end,
  help   = "Forward a particular Ipv6 multicast address",
  params = {
      { type = "values",
          "%ipv6_mc"
      },
      { type = "named",
          "#interface",
          {format = "vid1 %unchecked-vlan"},
          requirements = {["vid1"] = {"interface"}},
          mandatory = { "interface" }
      }
  }
})

----------------------------------------------------------------
-- command registration: no bridge multicast ipv6 address group
----------------------------------------------------------------
CLI_addHelp("vlan_configuration", "no bridge multicast ipv6",
                                    "Register IPv6 multicast address to the bridge table")
CLI_addHelp("vlan_configuration", "bridge multicast ipv6 address",
                                    "Register IPv6 multicast address to the bridge table")
CLI_addCommand("vlan_configuration", "no bridge multicast ipv6 address group", {
  func   = function(params)
               params.dip = true
               return no_bridge_multicast_address_group_func(params)
           end,
  help   = "Forward a particular Ipv6 multicast address",
  params = {
      { type = "values",
          "%ipv6_mc"
      },
      { type = "named",
          {format = "vid1 %unchecked-vlan"},
      }
  }
})
