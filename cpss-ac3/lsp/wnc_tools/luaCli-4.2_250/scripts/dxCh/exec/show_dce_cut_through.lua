--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_dce_cut_through.lua
--*
--* DESCRIPTION:
--*       showing of the information on cut-through
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants

local function boolAddOrSub(value)
    return ((value and "Subtract") or "Add");
end

-- ************************************************************************
---
--  show_dce_cut_through_func
--        @description  show's the information on cut-through
--
--        @param params         - params["all"]: all devices port or all
--                                ports of given device cheking switch,
--                                could be irrelevant;
--                                params["devID"]: checked device number,
--                                relevant if params["all"] is not nil;
--                                params["ethernet"]: checked interface
--                                name, relevant if params["all"] is not
--                                nil;
--                                params["port-channel"]: checked interface
--                                name, relevant if params["all"] is not
--                                nil
--
--        @return       true on success, otherwise false and error message
--
local function show_dce_cut_through_func(params)
    -- Common variables declaration
    local result, values
    local devFamily, devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration
    local cut_through_enabling_string_prefix, cut_through_enabling_string_suffix
    local cut_through_enabling, cut_through_enabling_string
    local actually_default_packet_length_string_prefix
    local actually_default_packet_length_string_suffix
    local actually_default_packet_length, actually_default_packet_length_string
    local rebooting_default_packet_length_string_prefix
    local rebooting_default_packet_length_string_suffix
    local rebooting_default_packet_length
    local rebooting_default_packet_length_string
    local enabling_packet_length_text_block_string
    local enabling_packet_length_text_block_header_string
    local enabling_packet_length_text_block_footer_string
    local priority_string_prefix_prefix, priority_string_prefix_suffix
    local priority_string_prefix, priority_string_suffix
    local priority, priority_string
    local priority_text_block_string
    local priority_text_block_header_string, priority_text_block_footer_string
    local enabling_packet_length_priority_block_string
    local enabling_packet_length_priority_block_header_string
    local enabling_packet_length_priority_block_footer_string
    local admin_enabling, admin_enabling_string
    local operation_enabling, operation_enabling_string
    local untagged_enabling, untagged_enabling_string
    local interface_table_header_string, interface_table_footer_string
    local interface_table_string

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllInterfacesPortIterator(params)
    command_data:initInterfaceDeviceRange()
    command_data:initPriorityData()

    -- Command specific variables initialization.
    cut_through_enabling_string_prefix                  = "Cut Through is "
    cut_through_enabling_string_suffix                  = ""
    actually_default_packet_length_string_prefix        =
        "Default packet length: "
    actually_default_packet_length_string_suffix        = " "
    rebooting_default_packet_length_string_prefix       = "(Would be "
    rebooting_default_packet_length_string_suffix       = " after reboot)"
    enabling_packet_length_text_block_header_string     = ""
    enabling_packet_length_text_block_footer_string     = "\n"
    priority_string_prefix_prefix                       = "Priority "
    priority_string_prefix_suffix                       = ": "
    priority_string_suffix                              = ""
    priority_text_block_header_string                   = ""
    priority_text_block_footer_string                   = "\n"
    enabling_packet_length_priority_block_header_string = ""
    enabling_packet_length_priority_block_string        = ""
    enabling_packet_length_priority_block_footer_string = ""
    interface_table_header_string                       = ""
    interface_table_string                              = ""
    interface_table_footer_string                       = "\n"

    -- Main device handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in  command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()

            command_data:clearLocalStatus()

            devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)

            if ("CPSS_PP_FAMILY_DXCH_LION_E"  == devFamily)     or
               ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily)     or
                (is_sip_5_20(devNum))     then
                -- Getting of cut-through enabling.
                command_data:clearLocalStatus()

                if true  == command_data["local_status"]    then
                    cut_through_enabling                        = "n/a"
                    cut_through_enabling_string                 =
                        tostring(cut_through_enabling)
                end

                command_data:updateStatus()

                -- Cut-through enabling string formatting and adding.
                command_data:setResultStr(cut_through_enabling_string_prefix,
                                          cut_through_enabling_string,
                                          cut_through_enabling_string_suffix)
                command_data:addResultToResultArray()

                -- Getting of actually default packet.
                command_data:clearLocalStatus()

                if true  == command_data["local_status"] then
                    actually_default_packet_length              = "n/a"
                    actually_default_packet_length_string       =
                        tostring(actually_default_packet_length)
                end

                command_data:updateStatus()

                -- Actually default packet string formatting and adding.
                command_data:setResultStr(
                    actually_default_packet_length_string_prefix,
                    actually_default_packet_length_string,
                    actually_default_packet_length_string_suffix)

                -- Getting of rebooting default packet.
                command_data:clearStatus()

                if true  == command_data["status"] then
                    rebooting_default_packet_length             = "n/a"
                    rebooting_default_packet_length_string      =
                        tostring(rebooting_default_packet_length)
                end

                command_data:updateStatus()

                -- Rebooting default packet string formatting and adding.
                command_data:addToResultStr(
                    rebooting_default_packet_length_string_prefix,
                    rebooting_default_packet_length_string,
                    rebooting_default_packet_length_string_suffix)
                command_data:addResultToResultArray()

                -- Pri0rity text block string formatting and adding.
                enabling_packet_length_text_block_string  =
                    enabling_packet_length_text_block_header_string ..
                    command_data:getResultArrayStr() ..
                    enabling_packet_length_text_block_footer_string

                -- Priorities cycle initialization
                command_data:clearStatus()
                command_data:clearPrioritiesCount()
                command_data:clearResultArray()

                -- Priorities handling cycle
                if true == command_data["local_status"] then
                    local priority_iterator
                    for priority_iterator, priority in
                                        command_data:getPriorityIterator() do
                        command_data:clearPriorutyStatus()

                        command_data:clearLocalStatus()

                        -- Priority string prefix.
                        priority_string_prefix  =
                            priority_string_prefix_prefix ..
                            tostring(priority) .. priority_string_prefix_suffix

                        -- Priority getting.
                        if true  == command_data["local_status"]    then
                            result, values =
                                myGenWrapper("cpssDxChCutThroughUpEnableGet",
                                             {{"IN",    "GT_U8",    "devNum",
                                                                    devNum   },
                                              {"IN",    "GT_U8",    "up",
                                                                    priority },
                                              {"OUT",   "GT_BOOL",  "enable"
                                                                             }})
                            if       0 == result    then
                                priority    = values["enable"]
                            elseif 0x10 == result   then
                                command_data:setFailDeviceStatus()
                                command_data:addWarning("It is not allowed " ..
                                                        "to get enabling of " ..
                                                        "cut-through for a " ..
                                                        "priority %d on " ..
                                                        "device %d.", priority,
                                                        devNum)
                            elseif    0 ~= result   then
                                command_data:setFailDeviceAndLocalStatus()
                                command_data:addError("Error at getting of " ..
                                                      "cut-through enabling " ..
                                                      "for a priority %d on " ..
                                                      "device %d: %s.",
                                                      priority, devNum,
                                                      returnCodes[result])
                            end

                            if 0 == result then
                                priority_string     =
                                    boolEnabledUpperStrGet(priority)
                            else
                                priority_string     = "n/a"
                            end
                        end

                        command_data:updateStatus()

                        -- Priority string formatting and adding.
                        command_data:setResultStr(priority_string_prefix,
                                                  priority_string,
                                                  priority_string_suffix)
                        command_data:addResultToResultArray()

                        command_data:updatePriorities()
                    end
                end

                -- Priority text block string formatting and adding.
                priority_text_block_string  = command_data:getResultArrayStr()
                priority_text_block_string  =
                    command_data:getStrOnPrioritiesCount(
                        priority_text_block_header_string,
                        priority_text_block_string,
                        priority_text_block_footer_string,
                        "There is no global priority information to show.\n")

                -- Cut-through enabling, packet length, priority block string
                -- formatting.
                enabling_packet_length_priority_block_string =
                    enabling_packet_length_priority_block_string ..
                    enabling_packet_length_text_block_string ..
                    priority_text_block_string

                command_data:updateDevices()

            else
                command_data:addWarning(
                    "enabling of cut-through for a priority - \n" ..
                    "family of device %d is not supported.", devNum)
            end
        end
    end

    -- Cut-through enabling, packet length, priority block string adding.
    enabling_packet_length_priority_block_string    =
        command_data:getStrOnDevicesCount(
            enabling_packet_length_priority_block_header_string,
            enabling_packet_length_priority_block_string,
            enabling_packet_length_priority_block_footer_string,
            "There is no cut-through enabling, packet length and global " ..
            "priority information to show.\n")

    -- Common variables initialization
    command_data:clearResultArray()

    -- Resulting table string formatting
    command_data["result"]                          =
        enabling_packet_length_priority_block_string
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    command_data["warning_string"] = ""
    command_data["warning_array"] = {}

    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()

            devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)

            if ("CPSS_PP_FAMILY_DXCH_LION_E"    == devFamily)     or
               ("CPSS_PP_FAMILY_DXCH_LION2_E"   == devFamily)     or
               (is_sip_5_20(devNum))     then
                    -- Enabling of cut-through admin, operation and untagged
                    -- enabling.
                    if true == command_data["local_status"]     then
                        result, values =
                            myGenWrapper("cpssDxChCutThroughPortEnableGet",
                                         {{"IN",  "GT_U8",   "devNum",  devNum },
                                          {"IN",  "GT_PHYSICAL_PORT_NUM",
                                                            "portNum", portNum },
                                          {"OUT", "GT_BOOL", "enable"          },
                                          {"OUT", "GT_BOOL", "untaggedEnable"  }})
                        if        0 == result   then
                            admin_enabling      = values["enable"]
                            operation_enabling  = values["enable"]
                            untagged_enabling   = values["untaggedEnable"]
                        elseif 0x10 == result   then
                            command_data:setFailPortStatus()
                            command_data:addWarning("It is not allowed to " ..
                                                    "get cut-through " ..
                                                    "enabling on device %d "..
                                                    "port %d.", devNum, portNum)
                        elseif    0 ~= result   then
                            command_data:setFailPortAndLocalStatus()
                            command_data:addError("Error at getting of " ..
                                                  "cut-through on device %d " ..
                                                  "port %d: %s.", devNum,
                                                  portNum, returnCodes[result])
                        end

                        if 0 == result then
                            admin_enabling_string       =
                                boolEnabledUpperStrGet(admin_enabling)
                            operation_enabling_string   =
                                boolEnabledUpperStrGet(operation_enabling)
                            untagged_enabling_string    =
                                boolEnabledUpperStrGet(untagged_enabling)
                        else
                            admin_enabling_string       = "n/a"
                            operation_enabling_string   = "n/a"
                            untagged_enabling_string    = "n/a"
                        end
                    end

                    command_data:updateStatus()

                    -- Resulting string formatting and adding.
                    command_data["result"] =
                        string.format("%-13s %-11s %-11s %-11s",
                                      alignLeftToCenterStr(tostring(devNum) ..
                                                           "/" ..
                                                           tostring(portNum),
                                                           11),
                                      admin_enabling_string,
                                      operation_enabling_string,
                                      untagged_enabling_string)
                    command_data:addResultToResultArray()

                    command_data:updatePorts()
            end
        end
    end

    -- Resulting table string formatting.
    interface_table_header_string                       =
        " Interface     Admin        Oper      Untagged \n" ..
        "-----------  ----------  ----------  ----------\n"
    interface_table_footer_string                       = "\n"

    interface_table_string                          =
        command_data:getResultArrayStr()
    interface_table_string                          =
        command_data:getStrOnPortsCount(
            interface_table_header_string, interface_table_string,
            interface_table_footer_string,
            "Port enable staus: There is no interface priority information to show.\n")

    -- Resulting table string formatting
    command_data["result"]                          =
        interface_table_string

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()

    -- Common variables initialization
    command_data:clearResultArray()
    command_data["warning_string"] = ""
    command_data["warning_array"] = {}


    -- user defined ethernet configuraton
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in  command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()

            command_data:clearLocalStatus()

            if (is_sip_6(devNum))     then
                command_data:clearLocalStatus()

                command_data:updateStatus()
                command_data:clearStatus()
                command_data:clearResultArray()

                print("\n")
                -- loop over 4 UDEs
                   local udeIndex
                   local udeCutThroughEnable
                   local udeByteCount
                   local udeString
                if true == command_data["local_status"] then
                    for udeIndex = 0,3,1 do
                        command_data:clearLocalStatus()

                        -- UDE configuration getting.
                        if true  == command_data["local_status"]    then
                            result, values =
                                myGenWrapper("cpssDxChCutThroughUdeCfgGet",
                             {{"IN",    "GT_U8",    "devNum", devNum   },
                              {"IN",    "GT_U32",    "udeIndex", udeIndex },
                              {"OUT",   "GT_BOOL",   "udeCutThroughEnable"},
                              {"OUT",   "GT_U32",    "udeByteCount"}
                                 })

                            if       0 == result    then
                                udeCutThroughEnable = values["udeCutThroughEnable"]
                                udeByteCount        = values["udeByteCount"]
                            elseif 0x10 == result   then
                                command_data:setFailDeviceStatus()
                                command_data:addWarning("It is not allowed " ..
                                   "to get cut-through configuration of UDE %d." ..
                                   " on device %d.", udeIndex, devNum)
                            elseif    0 ~= result   then
                                command_data:setFailDeviceAndLocalStatus()
                                command_data:addError("Error at getting " ..
                                   "cut-through configuration of UDE %d." ..
                                   " on device %d. %s",
                                   udeIndex, devNum, returnCodes[result])
                            end

                            if 0 == result then
                                udeString     =
                                    "UDE " .. tostring(udeIndex) ..
                                    " udeCutThroughEnable " ..
                                     boolEnabledUpperStrGet(udeCutThroughEnable) ..
                                    " udeByteCount " .. tostring(udeByteCount)

                            else
                                udeString     =
                                    "UDE " .. tostring(udeIndex) .. " configuration n/a"
                            end
                        end

                        command_data:updateStatus()

                        -- Priority string formatting and adding.
                        command_data:setResultStr("",  udeString, "")
                        command_data:addResultToResultArray()

                    end
                end

                command_data["result"]                          =
                    command_data:getResultArrayStr()
                command_data:clearResultArray()
                command_data:analyzeCommandExecution()
                command_data:printCommandExecutionResults()

                command_data:updateDevices()

            else
                command_data:addWarning(
                    "user defined ethernet configuration - \n" ..
                    "family of device %d is not supported.", devNum)
            end
        end
    end

    command_data:clearDeviceStatus()
    command_data:clearLocalStatus()
    command_data:clearStatus()
    command_data:clearResultArray()
    command_data["warning_string"] = ""
    command_data["warning_array"] = {}

    -- Passenger Parsing Mode
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in  command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()

            command_data:clearLocalStatus()

            if (is_sip_6(devNum))     then
                command_data:clearLocalStatus()

                command_data:updateStatus()
                command_data:clearStatus()
                command_data:clearResultArray()
                print("\n")

                local parsingMode
                local parsingModeString
                if true == command_data["local_status"] then
                    command_data:clearLocalStatus()

                    -- passenger parsing mode getting.
                    result, values =
                        myGenWrapper("cpssDxChCutThroughPassengerParsingModeGet",
                     {{"IN",    "GT_U8",    "devNum", devNum   },
                      {"OUT",   "CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT",
                          "parsingMode"}
                         })

                    if       0 == result    then
                        parsingMode = values["parsingMode"]
                    elseif 0x10 == result   then
                        command_data:setFailDeviceStatus()
                        command_data:addWarning("It is not allowed " ..
                           "to get passenger parsing mode " ..
                           " on device %d.", devNum)
                    elseif    0 ~= result   then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error at getting " ..
                           "passenger parsing mode" ..
                           " on device %d. %s",
                           devNum, returnCodes[result])
                    end

                    if 0 == result then
                        parsingModeString     =
                            "Passenger parsing mode " .. tostring(parsingMode)

                    else
                        parsingModeString     =
                            "Passenger parsing mode  n/a"
                    end

                    command_data["result"] = parsingModeString
                    command_data:updateStatus()

                end

                command_data:analyzeCommandExecution()
                command_data:printCommandExecutionResults()

                command_data:updateDevices()

            else
                command_data:addWarning("Family of device %d does not " ..
                                        "supported.", devNum)
            end
        end
    end

    command_data:clearDeviceStatus()
    command_data:clearLocalStatus()
    command_data:clearStatus()
    command_data:clearResultArray()
    command_data["warning_string"] = ""
    command_data["warning_array"] = {}

    -- Packet Header Integrity check configuration
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in  command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()

            command_data:clearLocalStatus()

            if (is_sip_6(devNum))     then
                command_data:clearLocalStatus()
                command_data:updateStatus()
                command_data:clearStatus()
                command_data:clearResultArray()
                print("\n")

                local integrityCfg
                local integrityCfgString
                if true == command_data["local_status"] then
                    command_data:clearLocalStatus()

                    -- Packet Header Integrity check configuration getting.
                    result, values =
                        myGenWrapper("cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet",
                     {{"IN",    "GT_U8",    "devNum", devNum   },
                      {"OUT",   "CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC",
                          "integrityCfg"}
                         })

                    if       0 == result    then
                        integrityCfg = values["integrityCfg"]
                    elseif 0x10 == result   then
                        command_data:setFailDeviceStatus()
                        command_data:addWarning("It is not allowed " ..
                           "to get passenger parsing mode " ..
                           " on device %d.", devNum)
                    elseif    0 ~= result   then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error at getting " ..
                           "passenger parsing mode" ..
                           " on device %d. %s",
                           devNum, returnCodes[result])
                    end

                    if 0 == result then
                        integrityCfgString     =
                            "Packet Header Integrity check configuration \n"
                            .. "minByteCountBoundary " .. tostring(integrityCfg.minByteCountBoundary) .. "\n"
                            .. "maxByteCountBoundary " .. tostring(integrityCfg.maxByteCountBoundary) .. "\n"
                            .. "enableByteCountBoundariesCheck " .. tostring(integrityCfg.enableByteCountBoundariesCheck) .. "\n"
                            .. "enableIpv4HdrCheckByChecksum " .. tostring(integrityCfg.enableIpv4HdrCheckByChecksum) .. "\n"
                            .. "hdrIntergrityExceptionPktCmd " .. tostring(integrityCfg.hdrIntergrityExceptionPktCmd) .. "\n"
                            .. "hdrIntergrityExceptionCpuCode " .. tostring(integrityCfg.hdrIntergrityExceptionCpuCode) .. "\n"

                    else
                        integrityCfgString     =
                            "Packet Header Integrity check configuration  n/a"
                    end

                    command_data:updateStatus()

                    command_data["result"] = integrityCfgString
                end
                command_data:analyzeCommandExecution()
                command_data:printCommandExecutionResults()

                command_data:updateDevices()

            end
        end
    end

    command_data:clearDeviceStatus()
    command_data:clearLocalStatus()
    command_data:clearStatus()
    command_data:clearResultArray()
    command_data["warning_string"] = ""
    command_data["warning_array"] = {}
    print("\n")

    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()

            command_data:clearLocalStatus()

            if (is_sip_6(devNum))     then
                    -- Enabling of cut-through admin, operation and untagged
                    -- enabling.
                    local toSubtractOrToAdd;
                    local subtractedOrAddedValue;
                    local toSubtractOrToAddString;
                    local subtractedOrAddedValueString;

                    if true == command_data["local_status"]     then
                        result, values =
                            myGenWrapper("cpssDxChCutThroughPortByteCountUpdateGet",
                                         {{"IN",  "GT_U8",   "devNum",  devNum },
                                          {"IN",  "GT_PHYSICAL_PORT_NUM",
                                                            "portNum", portNum },
                                          {"OUT", "GT_BOOL", "toSubtractOrToAdd"          },
                                          {"OUT", "GT_U32", "subtractedOrAddedValue"  }})

                        if        0 == result   then
                            toSubtractOrToAdd       = values["toSubtractOrToAdd"];
                            subtractedOrAddedValue  = values["subtractedOrAddedValue"];
                            untagged_enabling   = values["untaggedEnable"]
                        elseif 0x10 == result   then
                            command_data:setFailPortStatus()
                            command_data:addWarning("It is not allowed to " ..
                                                    "get cut-through " ..
                                                    "enabling on device %d "..
                                                    "port %d.", devNum, portNum)
                        elseif    0 ~= result   then
                            command_data:setFailPortAndLocalStatus()
                            command_data:addError("Error at getting of " ..
                                                  "cut-through on device %d " ..
                                                  "port %d: %s.", devNum,
                                                  portNum, returnCodes[result])
                        end

                        if 0 == result then
                            toSubtractOrToAddString =
                                boolAddOrSub(toSubtractOrToAdd);
                            subtractedOrAddedValueString =
                                tostring(subtractedOrAddedValue);
                        else
                            toSubtractOrToAddString        = "n/a"
                            subtractedOrAddedValueString   = "n/a"
                        end
                    end

                    command_data:updateStatus()

                    -- Resulting string formatting and adding.
                    command_data["result"] =
                        string.format("%-13s %-11s %-11s",
                                      alignLeftToCenterStr(tostring(devNum) ..
                                                           "/" ..
                                                           tostring(portNum),
                                                           11),
                                      toSubtractOrToAddString,
                                      subtractedOrAddedValueString);
                    command_data:addResultToResultArray()

                    command_data:updatePorts()
            end
        end
    end

    -- Resulting table string formatting.
    interface_table_header_string =
        " Interface   sub-or-add    value     \n" ..
        "-----------  ----------  ----------  \n"
    interface_table_footer_string = "\n"
    interface_table_string                          =
        command_data:getResultArrayStr()
    interface_table_string                          =
        command_data:getStrOnPortsCount(
            interface_table_header_string, interface_table_string,
            interface_table_footer_string,
            "Port Byte Count Update - there is no interface priority information to show.\n")

    -- Resulting table string formatting
    command_data["result"]                          =
        interface_table_string

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    command_data["warning_string"] = ""
    command_data["warning_array"] = {}

    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: show dce cut-through
--------------------------------------------------------------------------------
CLI_addHelp("exec", "show dce", "Show dce data center environment")
CLI_addCommand("exec", "show dce cut-through", {
    func   = show_dce_cut_through_func,
    help   = "Showing of the information on cut-through",
    params = {
        { type = "named",
            "#all_interfaces",
          mandatory = { "all_interfaces" }
        }
    }
})


-- ************************************************************************
---
--  show_dce_cut_through_bc_extract_fails_count_func
--        @description  show's cut through counter of fails extracting
--        CT Packet Byte Count by packet header.
--
--        @param params         - params["all"]: all devices port or all
--                                ports of given device cheking switch,
--                                could be irrelevant;
--                                params["devID"]: checked device number,
--                                relevant if params["all"] is not nil;
--                                params["ethernet"]: checked interface
--                                name, relevant if params["all"] is not
--                                nil;
--                                params["port-channel"]: checked interface
--                                name, relevant if params["all"] is not
--                                nil
--
--        @return       true on success, otherwise false and error message
--
local function show_dce_cut_through_bc_extract_fails_count_func(params)
    -- Common variables declaration
    local result, values
    local devNum
    local command_data = Command_Data()
    -- Command  specific variables declaration
    local failsCounter
    local failsCounterString = ""

    -- Common variables initialization
    command_data:clearResultArray()
    -- Common variables initialization.
    command_data:initAllDeviceRange(params)

    -- user defined ethernet configuraton
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in  command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()

            command_data:clearLocalStatus()

            if (is_sip_6(devNum))     then
                command_data:clearLocalStatus()

                command_data:updateStatus()
                command_data:clearStatus()
                command_data:clearResultArray()

                if true == command_data["local_status"] then
                    command_data:clearLocalStatus()

                    if true  == command_data["local_status"]    then
                        result, values =
                            myGenWrapper("cpssDxChCutThroughByteCountExtractFailsCounterGet",
                         {{"IN",    "GT_U8",    "devNum", devNum   },
                          {"OUT",   "GT_U32",    "countPtr"}
                             })

                        if       0 == result    then
                            failsCounter = values["countPtr"]
                        elseif 0x10 == result   then
                            command_data:setFailDeviceStatus()
                            command_data:addWarning("It is not allowed " ..
                               "to get cut-through counter of fails extracting " ..
                               "CT Packet Byte Count by packet header" ..
                               " on device %d.", devNum)
                        elseif    0 ~= result   then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("It is not allowed " ..
                               "to get cut-through counter of fails extracting " ..
                               "CT Packet Byte Count by packet header" ..
                               " on device %d. %s", devNum, returnCodes[result])
                        end

                        if 0 == result then
                            failsCounterString     =
                                "counter of fails extracting Byte Count" ..
                                " on device " .. tostring(devNum) ..
                                " is ".. tostring(failsCounter)

                        else
                            udeString     =
                                "counter of fails extracting Byte Count n/a"
                        end
                    end

                    command_data:updateStatus()

                    -- Priority string formatting and adding.
                    command_data:setResultStr("",  failsCounterString, "")
                    command_data:addResultToResultArray()

                end

                command_data["result"]                          =
                    command_data:getResultArrayStr()
                command_data:clearResultArray()
                command_data:analyzeCommandExecution()
                command_data:printCommandExecutionResults()

                command_data:updateDevices()

            else
                command_data:addWarning(
                    "user defined ethernet configuration - \n" ..
                    "family of device %d is not supported.", devNum)
            end
        end
    end

    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: show dce cut-through bc-extract-fails-count
--------------------------------------------------------------------------------
CLI_addHelp("exec", "show dce cut-through", "Show dce cut-through counters")
CLI_addCommand("exec", "show dce cut-through bc-extract-fails-count", {
    func   = show_dce_cut_through_bc_extract_fails_count_func,
    help   = "show cut through counter of fails extracting Byte Count by packet header",
    params = {
        { type = "named",
            "#all_device",
            mandatory = { "all_device" }
        }
    }
})

