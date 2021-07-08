--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* mac_address_table_static.lua
--*
--* DESCRIPTION:
--*       Adding/deleting of MAC-layer station source address to/from
--*       the bridge table
--*
--* FILE REVISION NUMBER:
--*       $Revision: 8 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssDxChBrgFdbInvalidEntryIndexGet")


--constants


-- ************************************************************************
---
--  mac_address_table_func
--        @description  add destination MAC address to the bridge table
--
--        @return       true on success, otherwise false and error message
--
local function mac_address_table_func(isStatic, params)
    -- Common variables declaration
    local result, values
    local devNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local mac_address_adding_condition, mac_address_adding_warning

    -- Common variables initialization
    command_data:initAllDeviceRange(params)
    command_data:initDeviceVlanRange(params)
    command_data:initMacEntry(params)

    -- Command specific variables initialization
    command_data["mac_entry"]["isStatic"] = isStatic

    -- Main device/vlan handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, vlanId in command_data:getDeviceVlanIterator() do
            if params["vid1"] and not is_sip_5(devNum) then
                command_data["mac_entry"]["key"]["entryType"] = "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E"
            end

            command_data:clearVlanStatus()

            command_data:clearLocalStatus()

            -- Setting of mac-entry vlan.
            if true == command_data["local_status"] then
                local tmp = {["key"]={["key"]={["macVlan"]={["vlanId"]  = vlanId}}}}
                -- command_data["mac_entry"]["key"]["key"]["macVlan"]["vlanId"]  = vlanId
                mergeTablesRightJoin(command_data["mac_entry"], tmp)
            end

            -- Mac address adding condition getting.
            if true == command_data["local_status"] then
                result, values, mac_address_adding_warning =
                    mac_address_adding_condition_check(devNum,
                                                       command_data["mac_entry"])
                if        0 == result then
                    mac_address_adding_condition    = values
                elseif 0x10 == result then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addWarning("Mac aaddress adding condition " ..
                                            "checking is not allowed " ..
                                            "on device %d.", devNum)
                elseif 0 ~= result then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at mac aaddress adding " ..
                                          "condition checking on device %d: %s.",
                                          devNum, values)
                end
            end

            -- Mac address adding condition checking.
            if true == command_data["local_status"]     then
                command_data:setFailDeviceStatusOnCondition(
                    false == mac_address_adding_condition)
                command_data:addConditionalWarning(
                    false == mac_address_adding_condition,
                    "Adding of given mac-entry is not alloved " ..
                    "on device %d: %s.", devNum, mac_address_adding_warning)
            end

            -- Mac address entry setting.
            if (true == command_data["local_status"])   and
               (true == mac_address_adding_condition)   then
                result, values = mac_entry_set(devNum, command_data["mac_entry"])
                if     0x10 == result then
                    command_data:setFailDeviceStatus()
                    command_data:addWarning("Mac address entry setting is not " ..
                                            "allowed on device %d.", devNum)
                elseif 0 ~= result then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at setting of mac address " ..
                                          "entry on device %d: %s.", devNum,
                                          returnCodes[result])
                end
            end

            command_data:updateStatus()

            command_data:updateVlans()
        end

        command_data:addWarningIfNoSuccessVlans("No mac-entries was added.")
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

	return command_data:getCommandExecutionResults()
end


-- ************************************************************************
---
--  no_mac_address_table_func
--        @description  deletes destination MAC address from the bridge table
--
--        @return       true on success, otherwise false and error message
--
local function no_mac_address_table_func(params)
    -- Common variables declaration
    local result, values
    local devNum, entry_index
    local command_data = Command_Data()

    -- Common variables initialization
    command_data:initAllDeviceMacEntryIterator(params)

    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, entry_index in
                                    command_data:getValidMacEntryIterator() do
            command_data:clearEntryStatus()

            -- Mac address entry deleting.
            command_data:clearLocalStatus()
            if true == command_data["local_status"] then
                result, values = mac_entry_delete(devNum,
                      command_data["mac_entry"]["key"])
                if     0x10 == result then
                    command_data:setFailDeviceStatus()
                    command_data:addWarning("Mac address entry deleting is not " ..
                                            "allowed in device %d entry index %d.",
                                            devNum, entry_index)
                elseif 0 ~= result then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at deleting of %d mac address " ..
                                          "entry in device %d: %s.",
                                          entry_index, devNum,
                                          returnCodes[result])
                end
            end

            command_data:updateStatus()

            command_data:updateEntries()
        end
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

	return command_data:getCommandExecutionResults()
end

-------------------------------------------------
-- command registration: mac address-table static
-------------------------------------------------
CLI_addHelp("config", "mac", "MAC configuration")
CLI_addHelp("config", "mac address-table", "Bridge MAC address table configuration")

CLI_addCommand("config", "mac address-table static", {
  func   = mac_address_table_func,
  help   = "Add static MAC destination address to the bridge table",
  constFuncArgs = {true},
  params = {
      { type = "values",
          "%mac-address"
      },
      { type = "named",
          "#all_device",
          "#vlan_range",
          "#interface_port_channel_remote",
          {format = "vid1 %unchecked-vlan"},
        requirements = {["vid1"] = {"all_device", "vlan_range", "interface_port_channel_remote"}},
        mandatory = { "all_device", "vlan_range", "interface_port_channel_remote" }
    }
  }
})

CLI_addCommand("config", "mac address-table dynamic", {
  func   = mac_address_table_func,
  help   = "Add dynamic MAC destination address to the bridge table",
  constFuncArgs = {false},
  params = {
      { type = "values",
          "%mac-address"
      },
      { type = "named",
          "#all_device",
          "#vlan_range",
          "#interface_port_channel",
          {format = "vid1 %unchecked-vlan"},
        requirements = {["vid1"] = {"all_device", "vlan_range"}},
        mandatory = { "all_device", "vlan_range", "interface_port_channel" }
    }
  }
})

CLI_addHelp("config", "no mac", "MAC configuration")
CLI_addHelp("config", "no mac address-table", "Bridge MAC address table configuration")

CLI_addCommand("config", "no mac address-table static", {
  func   = no_mac_address_table_func,
  help   = "Deletes destination MAC address from the bridge table",
  params = {
      { type = "values",
          "%mac-address"
      },
      { type = "named",
          "#all_device",
          "#vlan_range",
          "#interface_port_channel",
          {format = "vid1 %unchecked-vlan"},
        requirements = {["vid1"] = {"all_device", "vlan_range"}},
        mandatory = { "all_device", "vlan_range" }
      }
  }
})

CLI_addCommand("config", "no mac address-table dynamic", {
  func   = no_mac_address_table_func,
  help   = "Delete destination MAC address from the bridge table",
  params = {
      { type = "values",
          "%mac-address"
      },
      { type = "named",
          "#all_device",
          "#vlan_range",
          "#interface_port_channel",
          {format = "vid1 %unchecked-vlan"},
        requirements = {["vid1"] = {"all_device", "vlan_range"}},
        mandatory = { "all_device", "vlan_range" }
      }
  }
})

-- ************************************************************************
---
--  mac_address_table_func
--        @description  add destination MAC address to the bridge table
--
--        @return       true on success, otherwise false and error message
--
local function mac_address_table_remove_func(params)
    -- Common variables declaration
    local result, values
    local devNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local mac_address_adding_condition, mac_address_adding_warning

    -- Common variables initialization
    command_data:initAllDeviceRange(params)
    command_data:initDeviceVlanRange(params)
    -- to initilize Mac Entry Key with type = MAC and given mac-address
    command_data:initMacEntry(params)

    -- Command specific variables initialization
    command_data["mac_entry"]["isStatic"] = isStatic

    -- Main device/vlan handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, vlanId in command_data:getDeviceVlanIterator() do
            command_data:clearVlanStatus()
            command_data:clearLocalStatus()

            -- Setting of mac-entry vlan.
            if true == command_data["local_status"] then
                command_data["mac_entry"]["key"]["key"]["macVlan"]["vlanId"]  = vlanId
            end

            -- Mac address entry setting.
            if (true == command_data["local_status"]) then
                result, values = mac_entry_delete(devNum,
                                 command_data["mac_entry"]["key"])
                if     0x10 == result then
                    command_data:setFailDeviceStatus()
                    command_data:addWarning("Mac address entry removing is not " ..
                                            "allowed on device %d.", devNum)
                elseif 0 ~= result then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at removing of mac address " ..
                                          "entry on device %d: %s.", devNum,
                                          returnCodes[result])
                end
            end

            command_data:updateStatus()

            command_data:updateVlans()
        end

        command_data:addWarningIfNoSuccessVlans("No mac-entries was added.")
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

	return command_data:getCommandExecutionResults()
end

--[[ this command was added temporary because all other comammands deleting
     FDB record read all FDB and invalidate one or more (or less) records
     that cannot be done on GM.

CLI_addCommand("config", "mac address-table remove", {
  func   = mac_address_table_remove_func,
  help   = "Remove given MAC address from the bridge table",
  params = {
      { type = "values",
          "%mac-address"
      },
      { type = "named",
          "#all_device",
          "#vlan_range",
          {format = "vid1 %unchecked-vlan"},
        mandatory = { "all_device", "vlan_range" }
      }
  }
})

--]]

