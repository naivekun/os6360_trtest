--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_bridge_multicast_mode.lua
--*
--* DESCRIPTION:
--*       showing of multicast bridging mode
--*
--* FILE REVISION NUMBER:
--*       $Revision: 7 $
--*
--[[
---+++ CLI EXECUTION TRACE START +++---
show bridge multicast mode device ${dev}
---+++ CLI EXECUTION TRACE END +++---
]]--
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  show_bridge_multicast_mode_func
--        @description  show's multicast bridging mode
--
--        @param params         - params["vlan"]: vlan Id, could be
--                                irrelevant;
--                                params["all_device"]: all or given
--                                devices iterating property, could be
--                                irrelevant
--
--        @return       true on success, otherwise false and error message
--
local function show_bridge_multicast_mode_func(params)
    -- Common variables declaration
    local result, values
    local devNum, vlanId
    local command_data = Command_Data()
    -- Command specific variables declaration
    local signle_device_range, vlanInfo
    local ipv4_bridging_mode_enabling
    local ipv4_bridging_mode, ipv4_bridging_mode_string
    local ipv6_bridging_mode
    local ipv6_bridging_mode_enabling, ipv6_bridging_mode_string
    local header_string, footer_string

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllDeviceRange(params)
    command_data:initDeviceVlanRange(params)

    -- Command specific variables initialization
    signle_device_range = command_data:isOneOrNoDevicesInDevRange()
    header_string       =
        valueIfConditionTrue(
            signle_device_range,
            "\n" ..
            "VLAN   IPv4 Multicast mode  IPv6 Multicast mode\n" ..
            "-----  -------------------  -------------------\n",
            "\n" ..
            "DEVICE  VLAN   IPv4 Multicast mode  IPv6 Multicast mode\n" ..
            "------  -----  -------------------  -------------------\n")
    footer_string       = "\n"

    -- System specific data initialization.
    command_data:enablePausedPrinting()
    command_data:setEmergencyPrintingHeaderAndFooter(header_string,
                                                     footer_string)

    -- Main vlan handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, vlanId in
                                    command_data:getInterfaceVlanIterator() do
            command_data:clearVlanStatus()

            command_data:clearLocalStatus()

            -- Vlan information getting.
            if true == command_data["local_status"]                 then
                result, values = vlan_info_get(devNum, vlanId)
                if        0 == result then
                    vlanInfo = values
                elseif 0x10 == result then
                    command_data:setFailVlanAndLocalStatus()
                    command_data:addWarning("Information getting of vlan %d " ..
                                            "is not allowed on device %d.",
                                            vlanId, devNum)
                elseif    0 ~= result then
                    command_data:setFailVlanAndLocalStatus()
                    command_data:addError("Error at information getting of " ..
                                          "vlan %d on device %d: %s.", vlanId,
                                          devNum, values)
                end
            end

            -- Bridging mode getting.
            if true == command_data["local_status"]                 then
                ipv4_bridging_mode_enabling = vlanInfo["ipv4IpmBrgEn"]
                ipv4_bridging_mode          = vlanInfo["ipv4IpmBrgMode"]
                ipv6_bridging_mode_enabling = vlanInfo["ipv6IpmBrgEn"]
                ipv6_bridging_mode          = vlanInfo["ipv6IpmBrgMode"]
            end

            -- Bridging mode string forming.
            if true == command_data["local_status"]                 then
                ipv4_bridging_mode_string =
                    bridgingIpv4ModeStrGet(ipv4_bridging_mode_enabling,
                                                   ipv4_bridging_mode)
                ipv6_bridging_mode_string =
                    bridgingIpv6ModeStrGet(ipv6_bridging_mode_enabling,
                                                   ipv6_bridging_mode)
            end

            command_data:updateStatus()

            -- Resulting string formatting and adding.
            command_data["result"] =
                valueIfConditionTrue(
                    signle_device_range,
                    string.format("%-7s%-22s%-22s",
                                  tostring(vlanId),
                                  alignLeftToCenterStr(ipv4_bridging_mode_string,
                                                       20),
                                  alignLeftToCenterStr(ipv6_bridging_mode_string,
                                                       20)),
                    string.format("%-8s%-22s%-22s",
                                  tostring(devNum), tostring(vlanId),
                                  alignLeftToCenterStr(ipv4_bridging_mode_string,
                                                       20),
                                  alignLeftToCenterStr(ipv6_bridging_mode_string,
                                                       20)))
            command_data:addResultToResultArray()

            command_data:updateVlans()
        end
    end

    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr()
    command_data:setResultStrOnVlansCount(header_string,
                                          command_data["result"],
                                          footer_string,
                                          "There is no vlan information to show.\n")

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

	return command_data:getCommandExecutionResults()
end

--------------------------------------------------------------------------------
-- command registration: show mac address-table
--------------------------------------------------------------------------------
CLI_addHelp("exec", "show bridge multicast",
                                    "Showing of bridge multicast data")
CLI_addCommand("exec", "show bridge multicast mode", {
  func   = show_bridge_multicast_mode_func,
  help   = "Display Multicast VLAN bridging mode information",
  params = {
      { type = "named",
          "#all_device",
          "#vlan",
        mandatory = { "all_device" }
      }
  }
})