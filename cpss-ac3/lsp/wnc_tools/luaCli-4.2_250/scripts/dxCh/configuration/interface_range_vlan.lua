--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* interface_vlan.lua
--*
--* DESCRIPTION:
--*       entering of Vlan range Interface Configuration mode
--*       deleting of Vlans range
--*
--* FILE REVISION NUMBER:
--*       $Revision: 9 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("cpssDxChBrgVlanEntryInvalidate")

--constants


-- ************************************************************************
---
--  interface_range_vlan_func
--        @description  enter's Vlan range Interface Configuration mode
--
--        @param params             - params["all_device"]: property of
--                                    applying of all avaible devices,
--                                    could be inrelevant
--                                    params["devID"]: applied device
--                                    number, irrelevant when
--                                    params["all_device"] is defined
--                                    params["vlan"]: vlan number
--
--        @return       true on success, otherwise false and error message
--
function interface_range_vlan_func(params)
    -- Common variables declaration
    local result, values
    local devNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local vlanId, vlanInfo, vlan_existing, vlans_count

    -- Common variables initialization
    command_data:initAllDeviceRange(params)
    command_data:initDeviceVlanRange(params)

    -- Command specific variables initialization
    vlanInfo    = { portsMembers = {}, floodVidx = 0xFFF , naMsgToCpuEn=true, unregIpmEVidx=0xFFF, autoLearnDisable = true}
    vlanInfo.portIsolationMode = "CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_CMD_E"
    vlans_count = getNestedTableSize(command_data["dev_vlan_range"])

    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, vlanId in
                                command_data:getInterfaceVlanUnsafeIterator() do
            command_data:clearVlanStatus()

            -- Checking of vlan exitsitng.
            command_data:clearLocalStatus()

            if true == command_data["local_status"]     then
                values          = does_vlan_exists(devNum, vlanId)
                vlan_existing   = values
            end

            vlanInfo.fidValue = vlanId

            -- Setting of non-existing vlan.
            if (true == command_data["local_status"])   and
               (false == vlan_existing)                 then
                result, values = vlan_info_set(devNum, vlanId, vlanInfo)
                if     0x10 == result then
                    command_data:setFailVlanAndLocalStatus()
                    command_data:addWarning("Information setting of vlan %d " ..
                                            "is not allowed on device %d.",
                                            vlanId, devNum)
                elseif    0 ~= result then
                    command_data:setFailVlanAndLocalStatus()
                    command_data:addError("Error at information setting of " ..
                                          "vlan %d on device %d: %s.", vlanId,
                                          devNum, values)
                end
            end

            command_data:updateStatus()

            command_data:updateVlans()
        end
    end

    if (true == command_data["status"]) and
       (0 < vlans_count)                then
        CLI_change_mode_push("vlan_configuration")
        setGlobal("ifType",     "vlan")
        setGlobal("ifRange",    command_data["dev_vlan_range"])
    end

    command_data:addWarningIfNoSuccessVlans(
        "There is no vlans to enter Vlan Interface Configuration mode.")

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


-- ************************************************************************
---
--  no_interface_range_vlan_func
--        @description  deletes Vlan range
--
--        @param params             - params["all_device"]: property of
--                                    applying of all avaible devices,
--                                    could be inrelevant
--                                    params["devID"]: applied device
--                                    number, irrelevant when
--                                    params["all_device"] is defined
--                                    params["vlan"]: vlan number
--
--        @return       true on success, otherwise false and error message
--
function no_interface_range_vlan_func(params)
    -- Common variables declaration
    local result, values
    local devNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local vlanId, vlan_existing

    -- Common variables initialization
    command_data:initAllDeviceRange(params)
    command_data:initDeviceVlanRange(params)

    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, vlanId in
                                command_data:getInterfaceVlanUnsafeIterator() do
            command_data:clearVlanStatus()

            -- Checking of vlan exitsitng.
            command_data:clearLocalStatus()

            if true == command_data["local_status"]     then
                values          = does_vlan_exists(devNum, vlanId)
                if true == values then
                    vlan_existing   = values
                else
                    command_data:setFailLocalStatus()
                    command_data:addWarning("Vlan %d is not exist on device %d." ..
                                            vlanId, devNum)
                end
            end

            -- Invalidating of existing vlan.
            if (true == command_data["local_status"])   and
               (true == vlan_existing)                  then
                result, values =
                    cpssPerDeviceParamSet("cpssDxChBrgVlanEntryInvalidate",
                                          devNum, vlanId)
                if     0x10 == result then
                    command_data:setFailDeviceStatus()
                    command_data:addWarning("Invalidating of vlan %d is not " ..
                                            "allowed in device %d.", vlanId,
                                            devNum)
                elseif    0 ~= result then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at invalidating of vlan %d " ..
                                          "in device %d: %s", vlanId, devNum,
                                          returnCodes[result])
                end
            end

            command_data:updateStatus()

            command_data:updateVlans()
        end

        command_data:addWarningIfNoSuccessVlans("There is no vlans to delete.")
    end

    if true == command_data["status"] then
        CLI_change_mode_push("config")
        setGlobal("ifType",  nil)
        setGlobal("ifRange", nil)
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


-------------------------------------------------
-- command registration: interface range vlan
-------------------------------------------------
CLI_addHelp("config", "interface range", "Select range of interfaces to configure")
CLI_addCommand("config", "interface range vlan", {
  func   = interface_range_vlan_func,
  help   = "IEEE 802.1Q Vlans",
  params = {
      { type="named",
          "#all_device",
          "#configurable_vlan_range_all",
        mandatory = { "all_device", "configurable_vlan_range_all" }
      }
  }
})


-------------------------------------------------
-- command registration: no interface range vlan
-------------------------------------------------
CLI_addHelp("config", "no interface range", "Select range of interfaces to configure")
CLI_addCommand("config", "no interface range vlan", {
  func   = no_interface_range_vlan_func,
  help   = "IEEE 802.1Q Vlans",
  params = {
      { type="named",
          "#all_device",
          "#vlan_range_all",
        mandatory = { "all_device", "vlan_range_all" }
      }
  }
})

CLI_addCommand("interface", "no interface range vlan", {
  func   = no_interface_range_vlan_func,
  help   = "IEEE 802.1Q Vlans",
  params = {
      { type="named",
          "#all_device",
          "#vlan_range_all",
        mandatory = { "all_device", "vlan_range_all" }
      }
  }
})
