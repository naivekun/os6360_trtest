--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ip_address.lua
--*
--* DESCRIPTION:
--*       setting of an IP address for the system
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  entry_didnt_checked_func
--        @description  check if the current dev and mac entry didn't already checked(call to wrapper)
--
--        @param params         - checked_entries   : list of the entries that already checked
--                                current_devNum    : current device number
--                                current_mac_entry : current mac entry
--
--        @return       true if current entry didn't checked, otherwise false
--
local function entry_didnt_checked_func(checked_entries, current_devNum, current_mac_entry)
    local key, val
    if(#checked_entries == 0) then
        return true
    end
    
    for key, val in pairs(checked_entries)  do
        if( current_devNum == val.devNum and
            to_string(table.sort(current_mac_entry)) == to_string(table.sort(val.mac_entry))) then
            return false
        end
    end
    return true
end


-- ************************************************************************
---
--  add_results_to_checked_entries_func
--        @description  save the result and values that has return from the wrapper
--
--        @param params         - checked_entries   : list of the entries that already checked
--                                devNum    : current device number
--                                mac_entry : current mac entry
--                                result    : dev and mac results
--                                values    : dev and mac values
--
--        @return      checked_entries
--
local function add_results_to_checked_entries_func(checked_entries, devNum, mac_entry, result, values)
    table.insert(checked_entries, {devNum = devNum, mac_entry = duplicate(mac_entry), result = result, values = values})
    return checked_entries
end


-- ************************************************************************
---
--  get_checked_results_func
--        @description  get the result and values for current dev and mac entry
--
--        @param params         - checked_entries   : list of the entries that already checked
--                                current_devNum    : current device number
--                                current_mac_entry : current mac entry
--
--        @return       result and values on success, otherwise false and error message
--
local function get_checked_results_func(checked_entries, current_devNum, current_mac_entry)
    local key, val
    for key, val in pairs(checked_entries)  do
        if( current_devNum == val.devNum and
            to_string(table.sort(current_mac_entry)) == to_string(table.sort(val.mac_entry))) then
            return val.result, val.values
        end
    end
    return false
end


-- ************************************************************************
---
--  ip_address_interface_func
--        @description  set's an IP address for the system on Ethernet
--                      interface
--
--        @param params         - params["mac-address"]: mac-address
--                                params["flagNo"]: no command property
--
--        @usage __global       - __global["ifRange"]: iterface range
--
--        @return       true on success, otherwise false and error message
--
local function ip_address_interface_func(params)
    -- Common variables declaration.
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command specific variables declaration.
    local system_ip, system_ip_prefix_length, default_gateway_ip
    local vlan_existence, vlanId, vlanInfo, foundVlanInfo
    local system_mac_address
    local configured_mac_entry_existend, mac_entry

    -- Common variables initialization
    command_data:initInterfaceDevPortRange()
    command_data:initInterfaceDeviceRange()

    -- Command specific variables initialization
    system_ip               = params["ipv4"]
    system_ip_prefix_length = params["default_ipv4-mask"]
    default_gateway_ip      = params["default_gateway"]
    vlanInfo                =
        { ["ipv4UcastRouteEn"]     = true,
          ["floodVidx"]            = 0xFFF }

    system_mac_address      = getGlobal("myMac")

    -- Checking of system mac-address.
    if true == command_data["status"] then
        command_data:addConditionalError(getTrueIfNil(system_mac_address),
                                         "System mac address is not defined")
        command_data:setFailStatusOnCondition(getTrueIfNil(system_mac_address))
    end

    -- Setting of system IP address, its prefix length and default gateway.
    if true == command_data["status"] then
        setGlobal("myIp",               system_ip)
        setGlobal("myIpPrefixLength",   system_ip_prefix_length)
        storeDeviceIp(system_ip)
        setGlobal("defaultGateway",     default_gateway_ip)
    end

    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()

            command_data:clearLocalStatus()

            -- enable PCL lookup1 on port (done only when Policy Based Routing used)
            pbr_port_enable(
                devNum, portNum, true --[[enable--]], 0 --[[pcl_id--]]);

            -- Vlan info updating.
            if true == command_data["local_status"]     then
                vlanInfo["portsMembers"]    = { portNum }
            end

            mac_entry = {
                isStatic = true,
                daRoute  = true,
                daCommand = "CPSS_MAC_TABLE_FRWRD_E",
                key = {
                    entryType = "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E",
                    key = {
                      macVlan = {
                            macAddr   = system_mac_address,
                      }
                    },
                },
                dstInterface = {
                    ["type"] = "CPSS_INTERFACE_PORT_E",
                    devPort  = {
                        devNum  = device_to_hardware_format_convert(devNum),
                        portNum = 63 
                    } 
                },
            }

            -- Searching of port router vlan.
            if true == command_data["local_status"]     then
                result, values, vlanId, foundVlanInfo =
                    vlan_id_found(devNum, vlanInfo)
                if        0 == result                       then
                    vlan_existence  = values
                elseif 0x10 == result                       then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("It is not allowed to get %d port " ..
                                          "router vlan on device %d.", portNum,
                                          devNum)
                elseif    0 ~= result                       then
                    if values == nil then
                        values = to_string(vlanInfo)
                    end
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at searching of %d port " ..
                                          "router vlan on device %d: %s.",
                                          portNum, devNum, values)
                end
            end

            -- Getting of unusable vlan.
            if (true == command_data["local_status"])       and
               (false == vlan_existence)                    then
                result, values =
                    get_highest_unused_vlan(devNum)
                if     (0 == result) and (nil ~= values)    then
                    vlanId  = values
                elseif (0 == result) and (nil == values)    then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addWarning("There is no unusable vlans " ..
                                            "on device %d.", devNum)
                elseif  0 ~= result                         then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at unusable vlan id " ..
                                          "getting on device %d: %s.",
                                          devNum, values)
                end
            end

            -- Activating of vlan.
            if (true == command_data["local_status"])       and
               (false == vlan_existence)                    then
                vlanInfo.fidValue = vlanId
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

            -- Default vlan id setting.
            if true == command_data["local_status"]     then
                --[[ result, values =
                    cpssPerPortParamSet("cpssDxChBrgVlanPortVidSet",
                                        devNum, portNum, vlanId, "vlanId",
                                        "GT_U16")   ]]--
                result, values = cpssAPIcall("cpssDxChBrgVlanPortVidSet", {
                    devNum = devNum, portNum = portNum,
                    vlanId = vlanId,
                    direction="CPSS_DIRECTION_INGRESS_E"})
                if     0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("It is not allowed to set " ..
                                            "default vlan id %d " ..
                                            "on device %d port %d.",
                                            vlanId, devNum, portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at default vlan id %d " ..
                                          "setting on device %d port %d: %s.",
                                          vlanId, devNum, portNum,
                                          returnCodes[result])
                end
            end

            -- Mac-entry updating.
            if true == command_data["local_status"]     then
                mergeTablesRightJoin(
                    mac_entry,
                    {["key"] = {
                       ["key"] ={
                         ["macVlan"]= {
                           ["vlanId"]= vlanId}
                         }
                       }
                    }                    )
            end

            -- Existend checking of configured mac-entry with system
            -- mac-address.
            if true == command_data["local_status"]     then
                result, values =
                    does_mac_entry_exist(devNum, false, mac_entry)
                if        0 == result   then
                    configured_mac_entry_existend   = values
                elseif 0x10 == result   then
                    command_data:setFailVlanStatus()
                    command_data:addWarning("It is not allowed to get " ..
                                            "mac-entry with system " ..
                                            "mac-address %s on device %d.",
                                            system_mac_address, devNum)
                elseif    0 ~= result   then
                    command_data:setFailVlanAndLocalStatus()
                    command_data:addError("Error at mac-entry getting with " ..
                                          "system mac-address %s " ..
                                          "on device %d: %s.",
                                          system_mac_address, devNum, values)
                end
            end

            -- Adding of new mac-entry with system mac-address.
            if (true == command_data["local_status"])   and
               (false == configured_mac_entry_existend) then
                result, values = mac_entry_set(devNum, mac_entry)
                if     0x10 == result   then
                    command_data:setFailVlanStatus()
                    command_data:addWarning("It is not allowed to write " ..
                                            "mac-entry with system " ..
                                            "mac-address %s on device %d.",
                                            system_mac_address, devNum)
                elseif    0 ~= result   then
                    command_data:setFailVlanAndLocalStatus()
                    command_data:addError("Error at mac-entry writing with " ..
                                          "system mac-address %s " ..
                                          "on device %d: %s.",
                                          system_mac_address, devNum, values)
                end
            end

            command_data:updateStatus()

            command_data:updatePorts()
        end

        command_data:addWarningIfNoSuccessPorts(
            "Can not configure system mac entry on all processed devices.")
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end

-- ************************************************************************
---
--  ip_address_vlan_configuration_func
--        @description  set's an IP address for the system on vlan
--                      interface
--
--        @param params         - params["mac-address"]: mac-address
--                                params["flagNo"]: no command property
--
--        @usage __global       - __global[ifRange]: interface vlan/port
--                                range;
--                                __global[ifType]: interface type
--
--        @return       true on success, otherwise false and error message
--
local function ip_address_vlan_configuration_func(params)
    -- Common variables declaration.
    local result, values
    local devNum, vlanId, vlan_info, portNum
    local command_data = Command_Data()
    -- Command specific variables declaration.
    local system_ip, system_ip_prefix_length, default_gateway_ip
    local system_mac_address
    local vlan_modification, vlan_ports_presence
    local configured_mac_entry_existend, mac_entry
    local checked_entries = {}
    
    -- Common variables initialization
    command_data:initInterfaceDevVlanRange()
    command_data:initInterfaceDeviceRange("dev_vlan_range")
    command_data:initDevVlanInfoIterator()

    -- Command specific variables initialization
    system_ip               = params["ipv4"]
    system_ip_prefix_length = params["default_ipv4-mask"]
    default_gateway_ip      = params["default_gateway"]
    system_mac_address      = getGlobal("myMac")


    mac_entry               =
        { ["key"]                   =
            { ["entryType"]             = "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E",
              ["key"] = {
                ["macVlan"] = { 
                      ["macAddr"]               = system_mac_address,
                }
              }
            },
          ["isStatic"]              = true,
          ["daCommand"]             = "CPSS_MAC_TABLE_FRWRD_E",
          ["daRoute"]               = true                                   
        }

    -- Checking of system mac-address.
    if true == command_data["status"] then
        command_data:addConditionalError(getTrueIfNil(system_mac_address),
                                         "System mac address is not defined")
        command_data:setFailStatusOnCondition(getTrueIfNil(system_mac_address))
    end

    -- Setting of system IP address, its prefix length and default gateway.
    if true == command_data["status"] then
        setGlobal("myIp",               system_ip)
        setGlobal("myIpPrefixLength",   system_ip_prefix_length)
        storeDeviceIp(system_ip)
        setGlobal("defaultGateway",     default_gateway_ip)
    end
  
    -- Main vlan handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, vlanId, vlan_info in
                                    command_data:getDeviceVlanInfoIterator() do
            command_data:clearVlanStatus()

            command_data:clearLocalStatus()
         
            -- Vlan info modification.
            if true == command_data["local_status"]     then
                vlan_modification =
                    logicalNot(
                        compareWithEtalonTable(
                            { ["portsMembers"]         = { },
                              ["ipv4UcastRouteEn"]     = true,
                              ["floodVidx"]            = 0xFFF },
                            vlan_info))
            end

            -- Vlan info modification.
            if (true == command_data["local_status"])   and
               (true == vlan_modification)              then
                vlan_info =
                    mergeTablesRightJoin(vlan_info,
                                         { ["portsMembers"]         = { },
                                           ["ipv4UcastRouteEn"]     = true,
                                           ["floodVidx"]            = 0xFFF })
            end

            -- Vlan modification.
            if (true == command_data["local_status"])   and
               (true == vlan_modification)              then
                result, values = vlan_info_set(devNum, vlanId, vlan_info)
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

            -- Vlan ports precence checking.
            if true == command_data["local_status"]     then
                vlan_ports_presence = isEmptyTable(vlan_info["portsMembers"])
                command_data:addConditionalWarning(
                    vlan_ports_presence, "Vlan %d does not contain ports",
                    vlanId)
                command_data:setFailVlanStatusOnCondition(vlan_ports_presence)
            end

            command_data:updateStatus()

            command_data:updateVlans()
        end

        command_data:addWarningIfNoSuccessVlans(
            "Can not configure all processed vlans.")
    end

    -- Main port handling cycle
    if true == command_data["status"]               then
        local iterator
        for iterator, devNum, vlanId, portNum in
                                    command_data:getDeviceVlanPortIterator() do
            command_data:clearVlanStatus()

            command_data:clearLocalStatus()
         
            -- Default vlan id setting.
            if true == command_data["local_status"]     then
                result, values =
                    cpssPerPortParamSet("cpssDxChBrgVlanPortVidSet",
                                        devNum, portNum, vlanId, "vlanId",
                                        "GT_U16")
                if     0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("It is not allowed to set " ..
                                            "default vlan id %d " ..
                                            "on device %d port %d.",
                                            vlanId, devNum, portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at default vlan id %d " ..
                                          "setting on device %d port %d: %s.",
                                          vlanId, devNum, portNum,
                                          returnCodes[result])
                end
            end

            -- Mac-entry updating.
            if true == command_data["local_status"]     then
                mergeTablesRightJoin(
                    mac_entry,
                    {
                      ["key"]= {
                        ["key"] = {
                          ["macVlan"] = {
                            { ["vlanId"]    = vlanId                           }
                          }
                        }
                      }
                    }
                )
            end
            -- Existend checking of configured mac-entry with system
            -- mac-address. 
            if true == command_data["local_status"]     then
                if(entry_didnt_checked_func(checked_entries, devNum, mac_entry)) then
                    result, values  =
                        does_mac_entry_exist(devNum, false, mac_entry)
                    checked_entries = add_results_to_checked_entries_func(checked_entries, devNum, mac_entry, result, values)
                else
                    result, values  =
                        get_checked_results_func(checked_entries, devNum, mac_entry)
                end
                 
                if        0 == result   then
                    configured_mac_entry_existend   = values
                elseif 0x10 == result   then
                    command_data:setFailVlanStatus()
                    command_data:addWarning("It is not allowed to get " ..
                                            "mac-entry with system " ..
                                            "mac-address %s on device %d.",
                                            system_mac_address, devNum)
                elseif    0 ~= result   then
                    command_data:setFailVlanAndLocalStatus()
                    command_data:addError("Error at mac-entry getting with " ..
                                          "system mac-address %s " ..
                                          "on device %d: %s.",
                                          system_mac_address, devNum, values)
                end
     
            end

            -- Adding of new mac-entry with system mac-address.
            if (true == command_data["local_status"])   and
               (false == configured_mac_entry_existend) then
                result, values = mac_entry_set(devNum, mac_entry)
                if     0x10 == result   then
                    command_data:setFailVlanStatus()
                    command_data:addWarning("It is not allowed to write " ..
                                            "mac-entry with system " ..
                                            "mac-address %s on device %d.",
                                            system_mac_address, devNum)
                elseif    0 ~= result   then
                    command_data:setFailVlanAndLocalStatus()
                    command_data:addError("Error at mac-entry writing with " ..
                                          "system mac-address %s " ..
                                          "on device %d: %s.",
                                          system_mac_address, devNum, values)
                                           
                end
            end

            command_data:updateStatus()

            command_data:updateVlans()

        end

        command_data:addWarningIfNoSuccessVlans(
            "Can not configure system mac entries with all processed vlans.")
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


-- ************************************************************************
---
--  no_ip_address_func
--        @description  removes an IP address definition
--
--        @param params         - command params (not used)
--        @param command_data   - command execution data object
--
--        @usage __global       - __global["ifRange"]: iterface range (not
--                                used)
--                                __global[ifRange]: interface vlan/port
--                                range;
--                                __global[ifType]: interface type
--
--        @return       true on success, otherwise false and error message
--
local function no_ip_address_func(params, command_data)
    -- Common variables declaration.
    local result, values
    local devNum, portNum, devPorts
    -- Command specific variables declaration.
    local system_mac_address, system_mac_address_absence
    local vlanId, vlanInfo, vlan_ports, vlan_ports_absence

    -- Command specific variables initialization.
    devPorts                            =
        command_data:getEmptyDeviceNestedTable()
    system_mac_address                  = getGlobal("myMac")
    system_mac_address_absence          = isNil(system_mac_address)

    -- Setting to default of system IP address, its prefix length and default
    -- gateway.
    if true == command_data["status"] then
        setGlobal("myIp",               nil)
        setGlobal("myIpPrefixLength",   nil)
        setGlobal("defaultGateway",     nil)
    end

    -- Checking of system mac-address.
    if true == command_data["status"]       then
        command_data:addConditionalError(system_mac_address_absence,
                                         "System mac address is not defined")
        command_data:setFailStatusOnCondition(system_mac_address_absence)
    end

    -- Main mac-entries handling cycle
    if true == command_data["status"]       then
        local iterator
        for iterator, devNum, entry_index in
                                    command_data:getValidMacEntryIterator() do
            command_data:clearEntryStatus()

            command_data:clearLocalStatus()

            -- Getting of system mac-entry vlan.
            if true == command_data["local_status"] then
                vlanId  = command_data["mac_entry"]["key"]["key"]["macVlan"]["vlanId"]
            end

            -- Mac-enty deleting.
            if true == command_data["local_status"] then
                result, values = mac_entry_delete(devNum,
                    command_data["mac_entry"]["key"])
                if     0x10 == result then
                    command_data:setFailDeviceStatus()
                    command_data:addWarning("Mac address entry deleting is " ..
                                            "not allowed in device %d entry " ..
                                            "index %d.", devNum, entry_index)
                elseif 0 ~= result then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at deleting of %d mac " ..
                                          "address entry in device %d: %s.",
                                          entry_index, devNum,
                                          returnCodes[result])
                end
            end

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

            -- Collection of system mac entry ports.
            if true == command_data["local_status"]                 then
                vlan_ports          = vlanInfo["portsMembers"]
                devPorts[devNum]    = mergeLists(devPorts[devNum], vlan_ports)
            end

            -- Invalidating of existing vlan.
            if true == command_data["local_status"]                 then
                result, values =
                    cpssPerDeviceParamSet("cpssDxChBrgVlanEntryInvalidate",
                                          devNum, vlanId)
                if     0x10 == result then
                    command_data:setFailDeviceStatus()
                    command_data:addWarning("Invalidating of vlan %d is not " ..
                                            "allowed on device %d.", vlanId,
                                            devNum)
                elseif    0 ~= result then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at invalidating of vlan %d " ..
                                          "on device %d: %s", vlanId, devNum,
                                          returnCodes[result])
                end
            end

            command_data:updateStatus()

            command_data:updateEntries()
        end
    end

    -- Common variables initialization
    command_data:initDevPortRange(devPorts)

    -- Command specific variables initialization.
    vlan_ports_absence = command_data:isNoPortsInDevPortRange()

    -- Main port handling cycle
    if (true  == command_data["status"])    and
       (false == vlan_ports_absence)        then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()

            command_data:clearLocalStatus()

            -- disable PCL lookup1 on port (done only when Policy Based Routing used)
            pbr_port_enable(
                devNum, portNum, false --[[enable--]], 0 --[[pcl_id--]]);

            -- Default vlan id setting.
            if true == command_data["local_status"]     then
                result, values =
                    cpssPerPortParamSet("cpssDxChBrgVlanPortVidSet",
                                        devNum, portNum, 1, "vlanId",
                                        "GT_U16")
                if     0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("It is not allowed to set default " ..
                                            "vlan id 1 in device %d port %d.",
                                            devNum, portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at default vlan id 1 " ..
                                          "setting of device %d port %d: %s.",
                                          devNum, portNum, returnCodes[result])
                end
            end

            command_data:updateStatus()

            command_data:updatePorts()
        end

        command_data:addWarningIfNoSuccessPorts(
            "Can not set default vlan id to default value on all processed " ..
            "ports.",
            boolEnableLowerStrGet(enable_routing))
    end

    -- Checking of vlan ports presence.
    if true == command_data["status"]               then
        command_data:addConditionalWarning(
            vlan_ports_absence,
            "System mac-address vlan is not associated any ports.")
        command_data:setFailStatusOnCondition(vlan_ports_absence)
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


-- ************************************************************************
---
--  no_ip_address_interface_func
--        @description  removes an IP address definition
--
--        @param params         - command params (not used)
--
--        @usage __global       - __global["ifRange"]: iterface range (not
--                                used)
--                                __global[ifType]: interface type
--
--        @return       true on success, otherwise false and error message
--
local function no_ip_address_interface_func(params)
    -- Common variables declaration.
    local status, error_string
    local command_data = Command_Data()
    -- Common variables initialization
    command_data:initInterfaceDevPortRange()
    command_data:initInterfaceDeviceRange()
    command_data:initAllDeviceMacEntryIterator(
        params,
        {
            daRoute  = true,
            daCommand = "CPSS_MAC_TABLE_FRWRD_E",
            key = {
                entryType = "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E",
                key = {
                  macVlan = {
                        macAddr=getGlobal("myMac"),
                  },
                },
            },
        }
    )


    status, error_string    = no_ip_address_func(params, command_data)

    return status, error_string
end


-- ************************************************************************
---
--  no_ip_address_vlan_configuration_func
--        @description  removes an IP address definition
--
--        @param params         - command params (not used)
--
--        @usage __global       - __global["ifRange"]: dev/vlan range
--                                __global[ifType]: interface type
--
--        @return       true on success, otherwise false and error message
--
local function no_ip_address_vlan_configuration_func(params)
    -- Common variables declaration.
    local status, error_string
    local command_data = Command_Data()

    -- Common variables initialization
    command_data:initInterfaceDevVlanRange()
    command_data:initInterfaceDeviceRange("dev_vlan_range")
    command_data:initAllDeviceMacEntryIterator(
        params,
        { ["key"] = { 
            ["entryType"]             = "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E",
            ["key"] = {
                ["macVlan"] = {
                  ["macAddr"] = system_mac_address,
                  ["vlanId"]  =  command_data:getAllVlansFromDevVlansRange()
                }
            },
          },
          ["daCommand"]             = "CPSS_MAC_TABLE_FRWRD_E",
          ["daRoute"]               = true                                   })

    status, error_string    = no_ip_address_func(params, command_data)

    return status, error_string
end


--------------------------------------------------------------------------------
-- command registration: ip address
--------------------------------------------------------------------------------
CLI_addCommand("interface", "ip address", {
  func   = ip_address_interface_func,
  help   = "Setting of an IP address for the system",
  params = {
      { type = "values",
          "%ipv4",
          "%default_ipv4-mask"
      },
      { type = "named",
          "#default_gateway"
      }
  }
})

CLI_addHelp("vlan_configuration", "ip", "IP configuration commands")
CLI_addCommand("vlan_configuration", "ip address", {
  func   = ip_address_vlan_configuration_func,
  help   = "Setting of an IP address for the system",
  params = {
      { type = "values",
          "%ipv4",
          "%default_ipv4-mask"
      },
      { type = "named",
          "#default_gateway"
      }
  }
})


--------------------------------------------------------------------------------
-- command registration: no ip address
--------------------------------------------------------------------------------
CLI_addCommand("interface", "no ip address", {
  func   = no_ip_address_interface_func,
  help   = "Removing of an IP address definition"
})

CLI_addHelp("vlan_configuration", "no ip", "IP configuration commands")
CLI_addCommand("vlan_configuration", "no ip address", {
  func   = no_ip_address_vlan_configuration_func,
  help   = "Removing of an IP address definition"
})
