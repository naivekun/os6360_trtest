--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_vlan.lua
--*
--* DESCRIPTION:
--*       showing of vlan information
--*
--* FILE REVISION NUMBER:
--*       $Revision: 10 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  show_vlan_func
--        @description  shows vlan information
--
--        @param params         - params["all_device"]: all or given
--                                devices iterating property, could be
--                                irrelevant
--                                params["vlan"]: vlan Id, could be
--                                irrelevant;
--
--        @return       true on success, otherwise false and error message
--
local function show_vlan_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local second_vlan_id_string
    local vlan_ports, vlan_ports_string
    local tagged_ports, tagged_ports_presence, tagged_ports_string
    local untagged_ports, untagged_ports_presence, untagged_ports_string
    local first_vlan_tagged_string, second_vlan_tagged_string
    local vlan_mac_learning, vlan_mac_learning_string
    local header_string, footer_string
    local fdb_mode, fdb_mode_str;

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllDeviceRange(params)
    command_data:initDeviceVlanRange(params)

    -- Command specific variables initialization
    header_string       =
        "\n" ..
        "VLAN                           Ports                            Tag      MAC-Learning   FDB-mode\n" ..
        "-----  -----------------------------------------------------  --------  -------------- ---------\n"
    footer_string       = "\n"

    -- System specific data initialization.
    command_data:enablePausedPrinting()
    command_data:setEmergencyPrintingHeaderAndFooter(header_string,
                                                     footer_string)

    local vlanInfo
    local tagging_table
    local tcmd
    
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

            -- Vlan tagged, untagged and general ports list getting.
            if true == command_data["local_status"]                 then
                vlan_ports     = vlanInfo["portsMembers"]
                tagged_ports   = vlanInfo["portsTagging"]
                untagged_ports = excludeList(vlanInfo["portsMembers"],
                                             vlanInfo["portsTagging"])
            end
            tagging_table = {}

            if isNotEmptyTable(tagged_ports) and vlanInfo["portsTaggingCmd"] ~= nil then
                local tagging_commands = {
                    { enum = "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E", str = "tagged" },
                    { enum = "CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E", str = "tagged1" },
                    { enum = "CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E", str = "outer_tag0_inner_tag1" },
                    { enum = "CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E", str = "outer_tag1_inner_tag0" },
                    { enum = "CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E", str = "push_tag0" },
                    { enum = "CPSS_DXCH_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E", str = "pop_outer_tag" },
                    { enum = "CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E", str = "do_not_modify" }
                }

                local i, tag, j, port
                for i, tag in pairs(tagging_commands) do
                    tcmd = {}
                    for j,port in pairs(tagged_ports) do
                        if vlanInfo["portsTaggingCmd"][port] == tag.enum then
                            table.insert(tcmd,port)
                        end
                    end
                    if #tcmd > 0 then
                        table.insert(tagging_table, {
                            tagging = tag.str,
                            ports = devRangeStrGet(makeNestedTable({devNum}, tcmd))
                        })
                    end
                end
            elseif isNotEmptyTable(tagged_ports) then
                tcmd = tagged_ports;
                table.insert(tagging_table, {
                    tagging = "tagged",
                    ports = devRangeStrGet(makeNestedTable({devNum}, tcmd))
                })
            end

            if isNotEmptyTable(untagged_ports) then
                table.insert(tagging_table, {
                    tagging = "untagged",
                    ports = devRangeStrGet(makeNestedTable({devNum}, untagged_ports))
                })
            end
            if #tagging_table == 0 then
                table.insert(tagging_table, {
                    tagging = "",
                    ports = ""
                })
            end


            -- Vlan mac learning getting forming.
            if true == command_data["local_status"]                 then
                vlan_mac_learning   = vlanInfo["autoLearnDisable"]
            end

            --vlan_mac_learning_string    =
                --boolNegativeEnableLowerStrGet(vlan_mac_learning)

            if true == vlan_mac_learning  then
                vlan_mac_learning_string = "Control"
            else
                vlan_mac_learning_string = "Automatic"
            end

            fdb_mode = vlanInfo["fdbLookupKeyMode"];
            fdb_mode_str = "FID"; -- default
            if fdb_mode then
                if fdb_mode == 1
                or fdb_mode == "CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_VID1_E"
                then
                    fdb_mode_str = "FID_VID1";
                end
            end


            command_data:updateStatus()

            -- First resulting string formatting and adding.

            command_data["result"] =
                string.format("%-7s %-54s %-9s%-16s%-9s", tostring(vlanId),
                              tagging_table[1].ports,
                              tagging_table[1].tagging,
                              alignLeftToCenterStr(vlan_mac_learning_string,14),
                              alignLeftToCenterStr(fdb_mode_str,8))

            -- command_data:addResultToResultArrayOnCondition(
                -- tagging_table[1].ports ~= "")
            command_data:addResultToResultArray()


            local i
            for i = 2, #tagging_table do
                command_data["result"] =
                    string.format("%-7s %-54s %-9s%-16s", "",
                                  tagging_table[i].ports,
                                  tagging_table[i].tagging,
                                  "")
                command_data:addResultToResultArray()
            end

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
-- command registration: show vlan
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show vlan", {
    func   = show_vlan_func,
    help   = "vlan",
    params = {
        { type = "named",
            "#all_device",
            "#tag_vlan",
          mandatory = { "all_device" }
        }
    }
})

