--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* bridge_multicast_ipv6_mode.lua
--*
--* DESCRIPTION:
--*       configuring of the multicast bridging mode for ipv6 multicast packets
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes
require("dxCh/interface/bridge_multicast_ipv4_mode")

--constants


-------------------------------------------------
-- command registration: mac address-table static
-------------------------------------------------
CLI_addCommand("vlan_configuration", "bridge multicast ipv6 mode", {
  func   = function(params)
               params.ipv6   = true
               return bridge_multicast_ipv_mode_func(params)
           end,
  help   = "Configure IPv6 multicast bridging mode",
  params = {
      { type="named",
          { format="mac-group", 
                        help = "Multicast bridging is based on the packet's " ..
                               "VLAN and MAC address"                         },
          { format="ip-group",  
                        help = "Multicast bridging is based on the packet's " ..
                               "VLAN and the IPv6 destination address for " ..
                               "IPv6 packets"                                 },
          { format="ip-src-group", 
                        help = "Multicast bridging is based on the packet's " ..
                               "VLAN, IPv6 destination address and IPv6 " ..
                               "source address for IPv6 packets"              },
        alt       = { multicast_group = 
                        { "mac-group", "ip-group", "ip-src-group" }           }, 
        mandatory = { "multicast_group" }
    }
  }
})
CLI_addCommand("vlan_configuration", "no bridge multicast ipv6 mode", {
  func   = function(params)
               params.flagNo = true
               params.ipv6   = true
               return bridge_multicast_ipv_mode_func(params)
           end,
  help   = "Setting to default of the multicast bridging mode for ipv6 " ..
           "multicast packets"
})
