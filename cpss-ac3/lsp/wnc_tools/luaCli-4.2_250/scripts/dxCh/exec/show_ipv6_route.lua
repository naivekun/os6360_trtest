--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_ipv6_route.lua
--*
--* DESCRIPTION:
--*       showing of the global status of IP routing and the configured IP v6 
--*       Routes
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes
require("dxCh/exec/show_ip_route")

--constants


--------------------------------------------------------------------------------
-- command registration: show ipv6 route
--------------------------------------------------------------------------------
CLI_addHelp("exec", "show ipv6", "IPv6 Information")
CLI_addCommand("exec", "show ipv6 route", {
  func   = function(params)
               params.ip_protocol = "CPSS_IP_PROTOCOL_IPV6_E"
               return show_ip_route_func(params)
           end,
  help   = "Current state of the ipv6 routing table",
  params = {
      { type = "named",
          "#all_device",
        mandatory = { "all_device" }
      }
  }
})