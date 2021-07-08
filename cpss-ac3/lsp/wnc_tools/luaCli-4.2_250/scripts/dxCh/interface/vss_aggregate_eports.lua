--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vss_aggregate_eports.lua
--*
--* DESCRIPTION:
--*     commands for aggregate-eports management in the 'vss of bc2-ac3' system
--*     according to doc:
--*     http://docil.marvell.com/webtop/drl/objectId/0900dd88801a06b3
--*     comands:
--*     add member:    vss aggregate-eports eport <ePort> [neighbor-vss-name <neighbor_vss_name>]
--*     remove member: no vss aggregate-eports eport <ePort> [neighbor-vss-name <neighbor_vss_name>]
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- reserved as default value
-- if needed another value, cpssDxChBrgL2EcmpIndexBaseEportSet should be called
local L2ECMP_LTT_Base = 0;

local function error_return(command_data, error_msg)
    command_data:setFailStatus()
    command_data:setFailLocalStatus()
    command_data:addError(error_msg)
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

local function vss_aggregate_eports(params)
    local command_data = Command_Data();
    local global_eport, num_of_ports;
    local iterator, devNum, loopDevNum, loopPortNum;
    local i, isError, ret, val;
    local start_global_eport_val = vss_internal_db_global.global_eport_range.start_eport;
    local end_global_eport_val   = vss_internal_db_global.global_eport_range.end_eport;
    local hwDevId, eport, neighbor_vss_name;
    local eportInfo, lc_device_name, lc_portNum;
    local connection, connection_port_num;
    local unset;
    local global_eportInfo, L2ECMP_eport_index;
    local L2ECMP_entries, L2ECMP_base, L2ECMP_base_allocated;
    local L2ECMP_allocated_size, L2ECMP_entries_num;
    local ecmpIdx, ecmpEPort, ecmphwDevId, ecmpLastVssName, apiName;
    local updated_entries_num, lttEcmpEnable;


    if params["command"] == "add" then
        unset = false;
    else
        unset = true;
    end

    -- Common variables initialization.
    command_data:initInterfaceRangeIterator();
    command_data:initInterfaceDeviceRange();

    if vss_internal_db_global.my_role ~= vss_role_cc_full_name then
        return error_return(command_data,
            "VSS Error: command valid only for initialized control card");
    end

    -- retrieve global_eport

    num_of_ports = 0;
    if true == command_data["status"] then
        local iterator
        for iterator, loopDevNum, loopPortNum in command_data:getPortIterator() do
            command_data:clearPortStatus();
            command_data:clearLocalStatus();
            if true == command_data["local_status"] then
                global_eport = loopPortNum;
                devNum = loopDevNum;
                num_of_ports = num_of_ports + 1;
            end
            command_data:updateStatus();
            command_data:updatePorts();
        end
    end

    if num_of_ports ~= 1 then
        return error_return(command_data,
            "VSS Error: command not valid - context must contain exactly one global eport");
    end

    if (global_eport < start_global_eport_val)
        or (global_eport > end_global_eport_val) then
        return error_return(command_data,
            "VSS Error: global eport out of range "
            .. tostring(start_global_eport_val)
            .. ".." .. tostring(end_global_eport_val));
    end

    -- retrieve eport and hwDevId

    eport = tonumber(params["eport"]);
    neighbor_vss_name = params["neighbor_vss_name"];
    if neighbor_vss_name == nil then
        hwDevId = vss_internal_db_global.my_hw_devNum;
    else
        if vss_internal_db_global.other_vss[neighbor_vss_name] == nil then
            return error_return(command_data,
                "VSS Error: neighbor not found in DB");
        end
        hwDevId = vss_internal_db_global.other_vss[neighbor_vss_name]["vss_id"];
    end

    -- retrieve connection port

    if neighbor_vss_name == nil then
        eportInfo = vss_internal_db_global.on_cc_info.eports[eport];
        if eportInfo == nil then
            return error_return(command_data,
                "VSS Error: eport for LC local port not found in DB");
        end
        lc_device_name  = eportInfo.lc_name;
        lc_portNum      = eportInfo.lc_portNum;
        if (lc_portNum == nil) or (lc_device_name == nil) then
            return error_return(command_data,
                "VSS Error: LC local port info not found in DB");
        end
        connection = vss_internal_db_global.neighbors[lc_device_name].on_cc_info.connection;
        if connection == nil then
            return error_return(command_data,
                "VSS Error: LC connection info not found in DB");
        end
        connection_port_num = connection.portNum;
        if connection_port_num == nil then
            return error_return(command_data,
                "VSS Error: LC connection port not found in DB");
        end

        if unset == false then -- add member
            if eportInfo.aggregate_eport ~= nil then
                if eportInfo.aggregate_eport == global_eport then
                    return error_return(command_data,
                        "VSS Warning: eport already member of given aggregate_eport");
                else
                    return error_return(command_data,
                        "VSS Error: eport already member of another aggregate_eport");
                end
            end
        else -- remove member
            if eportInfo.aggregate_eport ~= global_eport then
                return error_return(command_data,
                    "VSS Error: eport not a member of given aggregate_eport");
            end
        end
    else
        -- connection_port_num for remote eport not needed
    end

    -- retrieve L2ECMP values from DB

    L2ECMP_entries = nil;
    L2ECMP_base = nil;
    L2ECMP_allocated_size = 32; -- default
    L2ECMP_entries_num = 0;
    L2ECMP_eport_index = nil;
    global_eportInfo = vss_internal_db_global.on_cc_info.eports[global_eport];
    if global_eportInfo ~= nil then
        L2ECMP_base = global_eportInfo.L2ECMP_base;
        L2ECMP_allocated_size = global_eportInfo.L2ECMP_allocated_size;
        L2ECMP_entries = global_eportInfo.L2ECMP_entries;
        if L2ECMP_entries ~= nil then
            for i = 0,L2ECMP_allocated_size do
                if (L2ECMP_entries[i] == nil) then
                    L2ECMP_entries_num = i;
                    break;
                end
            end
            if L2ECMP_entries_num > 0 then
                for i = 0,(L2ECMP_entries_num - 1) do
                    if (L2ECMP_entries[i].eport == eport)
                        and (L2ECMP_entries[i].neighbor_vss_name == neighbor_vss_name) then
                        L2ECMP_eport_index = i;
                        break;
                    end
                end
            end
        end
        
        
        if unset == true and L2ECMP_entries_num <= 1 then
            -- indication that we going to remove the last member of the ecmp.
            -- so we can also remove global config from the global-eport

            -- set common eport config
            vss_global_func_on_cc_set_common_config_eport(command_data, devNum,global_eport,unset)
        end
        
    else
        -- this is first time that we configures this EPort.
        if (unset == false) then
            -- set common eport config
            vss_global_func_on_cc_set_common_config_eport(command_data, devNum,global_eport,unset)
        end
    
        
    end

    if (unset == false) and (L2ECMP_entries_num >= L2ECMP_allocated_size) then
        return error_return(command_data,
            "VSS Error: reached maximal amount of members of given aggregate_eport (L2ECMP DB)");
    end

    if (unset == false) and (L2ECMP_eport_index ~= nil) then
        return error_return(command_data,
            "VSS Error: eport already a member of given aggregate_eport (L2ECMP DB)");
    end

    if (unset == true) and (L2ECMP_eport_index == nil) then
        return error_return(command_data,
            "VSS Error: eport is not a member of given aggregate_eport (L2ECMP DB");
    end

    -- the first DB manipulation can fail, so it done before HW Configuration
    L2ECMP_base_allocated = false;
    if (unset == false) and (L2ECMP_base == nil) then
        L2ECMP_base = sharedResourceL2EcmpMemoAlloc(L2ECMP_allocated_size);
        if L2ECMP_base == nil then
            return error_return(command_data,
                "VSS Error: sharedResourceL2EcmpMemoAlloc failed (L2ECMP DB");
        end
        L2ECMP_base_allocated = true;
    end

    -- update TTI rules for source eport reassignment
    if neighbor_vss_name == nil then
        if unset == false then
            vss_on_cc_tti_rule_for_reassign_srcEPort(
                command_data, devNum, connection_port_num, global_eport, eport,false --[[-unset-]]);
            vss_on_cc_tti_rule_for_reassign_srcEPort(
                command_data, devNum, connection_port_num, eport, eport, true --[[-unset-]]);
        else
            vss_on_cc_tti_rule_for_reassign_srcEPort(
                command_data, devNum, connection_port_num, eport, eport, false --[[-unset-]]);
            vss_on_cc_tti_rule_for_reassign_srcEPort(
                command_data, devNum, connection_port_num, global_eport, eport, true --[[-unset-]]);
        end
    else
        -- no TTI rules needed both for standalone remote eport
        -- both for remote eport as member of global eport
    end

    -- L2ECMP Table Configuration
    if (unset == true) and (L2ECMP_entries_num <= (L2ECMP_eport_index + 1)) then
        -- when removind last L2ECMP entry not needed any HW update
    else
        if unset == false then
            -- append new entry at the end
            ecmpIdx     = L2ECMP_base + L2ECMP_entries_num;
            ecmpEPort   = eport;
            ecmphwDevId = hwDevId;
        else
            -- override entry being deleted by the last
            ecmpIdx     = L2ECMP_base + L2ECMP_eport_index;
            ecmpEPort   = L2ECMP_entries[L2ECMP_entries_num - 1].eport;
            ecmpLastVssName = L2ECMP_entries[L2ECMP_entries_num - 1].neighbor_vss_name;
            if ecmpLastVssName == nil then
                ecmphwDevId = vss_internal_db_global.my_hw_devNum;
            else
                if vss_internal_db_global.other_vss[ecmpLastVssName] == nil then
                    if L2ECMP_base_allocated == true then
                        sharedResourceL2EcmpMemoFree(L2ECMP_base, L2ECMP_allocated_size);
                    end
                    return error_return(command_data,
                        "VSS Error: neighbor not found in DB (Moving Last L2ECMP entry)");
                end
                ecmphwDevId = vss_internal_db_global.other_vss[ecmpLastVssName]["vss_id"];
            end
        end
        apiName = "cpssDxChBrgL2EcmpTableSet"
        isError, ret, val = genericCpssApiWithErrorHandler(command_data,
            apiName ,{
            { "IN",     "GT_U8",                           "devNum",     devNum },
            { "IN",     "GT_U32",                          "index",      ecmpIdx },
            { "IN",     "CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC", "ecmpEntryPtr",
                {targetEport = ecmpEPort, targetHwDevice = ecmphwDevId}}
        })
        if isError == true then
            if L2ECMP_base_allocated == true then
                sharedResourceL2EcmpMemoFree(L2ECMP_base, L2ECMP_allocated_size);
            end
            return error_return(command_data,
                "VSS Error: API execution error");
        end
    end

    -- L2ECMP LTT Entry Update (needed any case)
    if unset == false then
        updated_entries_num = L2ECMP_entries_num + 1;
    else
        updated_entries_num = L2ECMP_entries_num - 1;
    end
    if updated_entries_num > 0 then
        lttEcmpEnable = true;
    else
        lttEcmpEnable = false;
        updated_entries_num = 1; -- value 0 not suppotred by both API and HW
    end
    apiName = "cpssDxChBrgL2EcmpLttTableSet"
    isError, ret, val = genericCpssApiWithErrorHandler(command_data,
        apiName ,{
        { "IN",     "GT_U8", "devNum",     devNum },
        { "IN",     "GT_U32", "index",      (global_eport - L2ECMP_LTT_Base) },
        { "IN",     "CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC", "ecmpLttInfoPtr",
            {ecmpStartIndex = L2ECMP_base,
             ecmpNumOfPaths = updated_entries_num,
             ecmpEnable = lttEcmpEnable}}
    })
    if isError == true then
        if L2ECMP_base_allocated == true then
            sharedResourceL2EcmpMemoFree(L2ECMP_base, L2ECMP_allocated_size);
        end
        return error_return(command_data,
            "VSS Error: API execution error");
    end

    -- save in DB
    if neighbor_vss_name == nil then
        if unset == false then
            eportInfo.aggregate_eport = global_eport;
        else
            eportInfo.aggregate_eport = nil;
        end
    end

    if unset == false then
        if global_eportInfo == nil then
            vss_internal_db_global.on_cc_info.eports[global_eport] =
            {
                L2ECMP_entries =
                    {[0] = {eport = eport, neighbor_vss_name = neighbor_vss_name}},
                L2ECMP_base = L2ECMP_base, L2ECMP_allocated_size = L2ECMP_allocated_size
            };
        else
            global_eportInfo.L2ECMP_entries[L2ECMP_entries_num] =
                {eport = eport, neighbor_vss_name = neighbor_vss_name};
        end
    else
        if L2ECMP_entries_num <= 1 then
            vss_internal_db_global.on_cc_info.eports[global_eport] = nil;
            sharedResourceL2EcmpMemoFree(L2ECMP_base, L2ECMP_allocated_size);
        elseif (L2ECMP_entries_num - 1) == L2ECMP_eport_index then
            global_eportInfo.L2ECMP_entries[L2ECMP_eport_index] = nil;
        else
            global_eportInfo.L2ECMP_entries[L2ECMP_eport_index] =
                global_eportInfo.L2ECMP_entries[L2ECMP_entries_num - 1];
            global_eportInfo.L2ECMP_entries[L2ECMP_entries_num - 1] = nil;
        end
    end

    -------------------
    -- handle all the errors that may come from calling the function
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

--------------------------------------------------------------------------------
-- command registration: add/remove eport to/from eport aggregation
--------------------------------------------------------------------------------
CLI_addCommand({"interface", "interface_eport"}, "vss aggregate-eports", {
    func = function(params)
        params.command = "add"
        return vss_aggregate_eports(params)
    end,
    help = "VSS add ePort to ePort Aggregation",
    params = {
        {
            type = "named",
            {format = "eport %s", name = "eport", help = "added eport" },
            {format = "neighbor-vss-name %s",
                name = "neighbor_vss_name",
                help = "The unique name of the neighbor vss (Optional parameter)" },
            requirements = {
                ["neighbor_vss_name"] = {"eport"}},
            mandatory = {"eport"}
        }
    }
})

CLI_addCommand({"interface", "interface_eport"}, "no vss aggregate-eports", {
    func = function(params)
        params.command = "remove"
        return vss_aggregate_eports(params)
    end,
    help = "VSS remove ePort from ePort Aggregation",
    params = {
        {
            type = "named",
            {format = "eport %s", name = "eport", help = "added eport" },
            {format = "neighbor-vss-name %s",
                name = "neighbor_vss_name",
                help = "The unique name of the neighbor vss (Optional parameter)" },
            requirements = {
                ["neighbor_vss_name"] = {"eport"}},
            mandatory = {"eport"}
        }
    }
})

