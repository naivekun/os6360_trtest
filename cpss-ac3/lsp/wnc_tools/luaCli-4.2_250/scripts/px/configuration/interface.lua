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
--*       entering Ethernet Interface Configuration mode
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--constants

function px_interface_port_ethernet_func(params)
    -- Common variables declaration
    local result
    local command_data = Command_Data()
    -- Command specific variables declaration
    local port_channel

    -- Common variables initialization
    command_data:initDevPortRange(params)

    -- Interface mode changing
    if true == command_data["status"] then
      setGlobal("ifRange", command_data:getDevicePortRange())
      setGlobal("ifType", "ethernet")
      CLI_change_mode_push("interface")
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end

--------------------------------------------------------
-- command registration: interface
--------------------------------------------------------
CLI_addCommand("config", "interface", {
  func   = px_interface_port_ethernet_func,
  help   = "Enter Interface Configuration mode",
  params = {{ type = "named", "#interface_port_ethernet", 
              mandatory = { "interface_port_ethernet" }}}
})

--------------------------------------------------------
-- command registration: interface range ethernet
--------------------------------------------------------
CLI_addHelp("config", "interface range", "Select range of interfaces to configure")
CLI_addCommand("config", "interface range", {
  func   = px_interface_port_ethernet_func,
  help   = "Select range of interfaces to configure",
  params = {{ type = "named", "#ethernet_devId",
             mandatory = { "ethernet_devId" }}}
})

