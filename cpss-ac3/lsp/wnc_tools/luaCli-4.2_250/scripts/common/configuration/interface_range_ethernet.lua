--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* interface_range_ethernet.lua
--*
--* DESCRIPTION:
--*       specifing of a list of <interface mode, speed> pairs 
--*
--* FILE REVISION NUMBER:
--*       $Revision: 6 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  interface_range_ethernet_func
--        @description  enables/disables ingress filtering of n interface
--
--        @param params         - params["devId"]: device number, all ports
--                                of which should be added to dev/port range 
--                                of command execution data object;
--                                given dev/port range (relevant, if others
--                                entry cases are irrelevant);
--                                params["ethernet"]: entry ethernet 
--                                interface dev/port, could be irrelevant;
--                                params["port-channel"]: entry trunk, 
--                                could be irrelevant
--
--        @return       true on success, otherwise false and error message
-- 
function interface_range_ethernet_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local port_channel

    -- Common variables initialization
    command_data:initDevPortRange(params)
    
    -- Command specific variables initialization.
    port_channel    = params["port-channel"]

    -- Interface mode changing
    if true == command_data["status"] then
      setGlobal("ifRange", command_data:getDevicePortRange())
      if nil ~= params["eport"] then
        setGlobal("ifType", "eport")
        CLI_change_mode_push("interface_eport")
      else
        setGlobal("ifType", "ethernet")
        CLI_change_mode_push("interface")
      end
    end

    -- Port-channel data setting.
    if true == command_data["status"] then
        setGlobal("ifPortChannel", port_channel)
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------
-- command registration: interface range ethernet
--------------------------------------------------------
CLI_addCommand("config", "interface range", {
  func   = interface_range_ethernet_func,
  help   = "Select range of interfaces to configure",
  params = {{ type = "named", "#ethernet_devId", mandatory = { "ethernet_devId" }}}
})
