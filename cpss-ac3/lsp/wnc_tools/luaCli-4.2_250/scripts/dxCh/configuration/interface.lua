--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* interface.lua
--*
--* DESCRIPTION:
--*       entering Interface Configuration (Ethernet, Port Channel, ePort) mode
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
require("dxCh/configuration/interface_range_ethernet")

--constants


--------------------------------------------------------
-- command registration: interface
--------------------------------------------------------
CLI_addCommand("config", "interface", {
  func   = interface_range_ethernet_func,
  help   = "Enter Interface Configuration mode",
  params = {{ type = "named", "#interface_port_channel", 
              mandatory = { "interface_port_channel" }}}
})