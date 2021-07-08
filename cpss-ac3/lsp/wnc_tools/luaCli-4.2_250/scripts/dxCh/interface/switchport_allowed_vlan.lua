--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* switchport_allowed_vlan.lua
--*
--* DESCRIPTION:
--*       adding/deleting of VLANs to/from a interface port range
--*
--* FILE REVISION NUMBER:
--*       $Revision: 5 $
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
---
--  switchport_allowed_vlan_func
--        @description  add's/deletes VLANs to/from a interface port range
--
--        @param params             - params["vlan-range"]: added vlan-
--                                    range;
--                                    params["tagged"]: property of tagged
--                                    packets seeting;
--                                    params["untagged"]: property of
--                                    untagged packets seeting;
--                                    params["remove"]: property of remove
--                                    command
--
--        @usage common_global      - common_global[ifRange]: interface
--                                    vlan/port range;
--                                    common_global[ifType]: interface type
--
--        @return       true on success, otherwise false and error message
--
local function switchport_allowed_vlan_func(params)
    -- Common variables declaration
    local result, values
    local devNum, vlanId
    local command_data = Command_Data()
    -- Command specific variables declaration
    local dev_vlan_ports = {}

    -- Common variables initialization
    command_data:initInterfaceDevPortRange()
    command_data:initInterfaceDeviceRange()
    command_data:initDeviceVlanRange(params)

    -- Command specific variables initialization
    dev_vlan_ports["added"]             =
        command_data:getDevicePortRangeIfConditionNil(params["remove"])
    dev_vlan_ports["removed"]           =
        command_data:getDevicePortRangeIfConditionNotNil(params["remove"])
 --print("params.port_tagging_cmd="..to_string(params.port_tagging_cmd))
    if params.port_tagging_cmd == "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E" then
        params.untagged = true
    elseif params.port_tagging_cmd ~= nil then
        params.tagged = true
    end
    dev_vlan_ports["tagged"]            =
        command_data:getDevicePortRangeIfConditionNotNil(params["tagged"])
    dev_vlan_ports["untagged"]          =
        command_data:getDevicePortRangeIfConditionNotNil(params["untagged"])
    dev_vlan_ports["tagging_command"]   = nil
    if params.tagged and params.port_tagging_cmd ~= nil then
        dev_vlan_ports["tagging_command"] = {}
        local dev,ports,i
        for dev, ports in pairs(command_data:getDevicePortRange()) do
            dev_vlan_ports["tagging_command"][dev] = {}
            for i = 1, #ports do
                dev_vlan_ports["tagging_command"][dev][ports[i]] = params.port_tagging_cmd
            end
        end
    end

    -- Device vlans configuring.
    if true == command_data["status"] then
        command_data:configureDeviceVlanRange(dev_vlan_ports)

        command_data:addWarningIfNoSuccessVlans(
            "Can not configure ports of all processed vlans.")

        -- bpe_802_1_br_port_extender support
        local devNum, vlanId, iterator;
        for iterator, devNum, vlanId in command_data:getInterfaceVlanIterator() do
            bpe_802_1_br_port_extender_flood_etag_cfg(devNum, vlanId, false);
        end

    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: switchport allowed vlan add
--------------------------------------------------------------------------------
CLI_type_dict["port_tagging_cmd"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Port tagging command",
    enum = {
        ["untagged"] = { value="CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E", help="if Tag0 and/or Tag1 were classified in the incoming packet" },
        ["tagged"] = { value="CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E", help="packet egress with Tag0" },
        ["tagged1"] = { value="CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E", help="packet egress with Tag1" },
        ["outer_tag0_inner_tag1"] = { value="CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E", help="Outer Tag0, Inner Tag1" },
        ["outer_tag1_inner_tag0"] = { value="CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E", help="Outer Tag1, Inner Tag0" },
        ["push_tag0"] = { value="CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E", help="TAG0 is added to the packet" },
        ["pop_outer_tag"] = { value="CPSS_DXCH_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E", help="the incoming packet outer tag is removed" },
        ["do_not_modify"] = { value="CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E", help="do not modify any tag" }
    }
}
CLI_addHelp("interface", "switchport",
                                    "Configure switch port definition in vlan")
CLI_addHelp("interface", "switchport allowed",
                                    "Configure switchport allowed parameters")
CLI_addHelp("interface", "switchport allowed vlan",
                                    "Configure switchport VLAN properties")
CLI_addCommand("interface", "switchport allowed vlan add", {
  func   = switchport_allowed_vlan_func,
  help   = "Configure to add VLANs to a general port",
  params = {
      { type = "values",
          "%vlan-range",
          "%port_tagging_cmd"
      }
  }
})


--------------------------------------------------------------------------------
-- command registration: switchport allowed vlan remove
--------------------------------------------------------------------------------
CLI_addCommand("interface", "switchport allowed vlan remove", {
  func   = function(params)
               params["remove"] = true
               switchport_allowed_vlan_func(params)
           end,
  help   = "Configure to removes VLANs from a general port",
  params = {
      { type = "values",
          "%vlan-range"
      }
  }
})
