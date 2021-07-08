--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vlan_mtu.lua
--*
--* DESCRIPTION:
--*       setting of the maximum packet size on a VLAN (or range of Vlans)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 4 $
--*
--********************************************************************************

--includes

--constants 
local default_maximum_vlan_packet_size = 1522


-- ************************************************************************
---
--  vlan_mtu_func
--        @description  set's the port PVID
--
--        @param params             - params["flagNo"]: no command property
--
--        @return       true on success, otherwise false and error message
-- 
local function vlan_mtu_func(params)
    -- Common variables declaration
    local command_data = Command_Data()
    -- Command specific variables declaration
    local mru_value  
        
    -- Common variables initialization    
    command_data:initInterfaceDevVlanRange()
    command_data:initInterfaceDeviceRange("dev_vlan_range")

    -- Command specific variables initialization
    mru_value = command_data:getValueIfFlagNoIsNil(
                    params, params["max_vlan_packet_size"], 
                              default_maximum_vlan_packet_size)
    
    if true == command_data["status"] then
        command_data:configure_vlan_mru(mru_value)
    end
    
    command_data:analyzeCommandExecution()
    
    command_data:printCommandExecutionResults()
    
    return command_data:getCommandExecutionResults()
end

--------------------------------------------------------
-- command registration: vlan-mtu
--------------------------------------------------------
CLI_addCommand("vlan_configuration", "vlan-mtu", {
  func   = vlan_mtu_func,
  help   = "Setting of the maximum packet size on a VLAN (or range of Vlans)",
  params = {
      { type = "named",
          { format = "%max_vlan_packet_size",   name = "max_vlan_packet_size",
                                            help = "Maximum vlan packed size"
          }
      }
  }  
})
CLI_addCommand("vlan_configuration", "no vlan-mtu", {
  func   = function(params) 
               params.flagNo=true
               vlan_mtu_func(params)
           end,
  help   = "Setting of the maximum packet size on a VLAN (or range of Vlans) to default"
})
