--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ipv6_route.lua
--*
--* DESCRIPTION:
--*       creating/destroing of IP v6 Routes
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes
require("dxCh/configuration/ip_route")

--constants


--------------------------------------------------------------------------------
-- command registration: ipv6 route
--------------------------------------------------------------------------------
CLI_addHelp("config", "ipv6",       "IPV6 configuration")
CLI_addCommand("config", "ipv6 route", {
  func   = function(params)
               params.ip_protocol = "CPSS_IP_PROTOCOL_IPV6_E"
               return ip_route_func(params)
           end,
  help   = "Creating of IP v6 Routes",
  params = {
      { type = "values",
          "%ipv6",
          "%ipv6-mask"
      },
      { type="named",
          "#next_hop",
        mandatory = { "next_hop" }
      }
  }
})

--------------------------------------------------------------------------------
-- command registration: no ipv6 route
--------------------------------------------------------------------------------
CLI_addHelp("config", "no ipv6",    "IPV6 configuration")
CLI_addCommand("config", "no ipv6 route", {
  func   = function(params)
               params.ip_protocol = "CPSS_IP_PROTOCOL_IPV6_E"
               return no_ip_route_func(params)
           end,
  help   = "Destroing of IP v6 Routes",
  params = {
      { type = "values",
          "%ipv6",
          "%ipv6-mask"
      }
  }
})
