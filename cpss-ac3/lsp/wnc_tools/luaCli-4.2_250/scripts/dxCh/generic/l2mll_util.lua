--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* l2mll_utils.lua
--*
--* DESCRIPTION:
--*       l2mll utils
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- saved from redefinition print function
local debug_print = print;

---------------------------------------------------------------------------------------
-- L2MLL

-- param can contain replaced members of CPSS_DXCH_L2_MLL_PAIR_STC
-- firstMllNode, secondMllNode, nextPointer, entrySelector
-- command.move_first_to_second ={} used for shift_nodes
-- command.mark_second_last ={} used for marking the end of the chain
-- command.mark_second_not_last ={} used for marking the continuation of the chain
local function l2mll_pair_update(command_data, devNum, block, param, command)
local is_failed, result, OUT_values, apiName;
    local mll_pair, key, last;

    param = table_deep_copy(param);
    if not param then
        param = {};
    end
    if param.firstMllNode then
        param.firstMllNode.last = nil;
    end
    if param.secondMllNode then
        param.secondMllNode.last = nil;
    end

    if not command then
        command = {};
    end

    apiName = "cpssDxChL2MllPairRead";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",     "devNum",   devNum },
        { "IN", "GT_U32",   "mllPairEntryIndex",    block},
        { "IN", "CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT",   "mllPairReadForm",
            "CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E"},
        { "OUT", "CPSS_DXCH_L2_MLL_PAIR_STC",   "mllPairEntryPtr"}
    });

    if is_failed == true then
        return nil;
    end

    mll_pair = OUT_values.mllPairEntryPtr;
    -- override all given data
    key = next(param, nil);
    while key do
        mll_pair[key] = param[key];
        key = next(param, key);
    end
    if command.move_first_to_second then
        -- save last-bit and restore it after copy
        last = mll_pair.secondMllNode.last;
        mll_pair.secondMllNode = mll_pair.firstMllNode;
        mll_pair.secondMllNode.last = last;
        mll_pair.firstMllNode  = nil;
    end
    if command.mark_second_last then
        mll_pair.secondMllNode.last = true;
    end
    if command.mark_second_not_last then
        mll_pair.secondMllNode.last = nil;
    end

    apiName = "cpssDxChL2MllPairWrite";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",     "devNum",   devNum },
        { "IN", "GT_U32",   "mllPairEntryIndex",    block},
        { "IN", "CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT",   "mllPairWriteForm",
            "CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E"},
        { "IN", "CPSS_DXCH_L2_MLL_PAIR_STC",   "mllPairEntryPtr",    mll_pair}
    });

    if is_failed == true then
        return nil;
    end
    return true;
end

local function l2mll_ltt_entry_write(command_data, devNum, lttEntryIndex, ltt_entry)
    local is_failed, result, OUT_values, apiName;

    apiName = "cpssDxChL2MllLttEntrySet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",     "devNum",   devNum },
        { "IN", "GT_U32",   "index",    lttEntryIndex},
        { "IN", "CPSS_DXCH_L2_MLL_LTT_ENTRY_STC",   "lttEntryPtr", ltt_entry}
    });

    if is_failed == true then
        return nil;
    end
    return true;
end

-- supported result table that can contain elements below, "_" is some number
-- update_chain_pointer = {pointer = {block = _, node = _}}
-- added_node = {block = _, node = _};
-- update_block_next_pointer = {block = _, next = {block = _, node = _}}
-- cleared_block = _;
-- shift_nodes = {block = _, low = _, high = _)};
-- node_found = {block = _, node = _};
-- new_block = true
local function util_l2mll_do_node_db_result(
    command_data, db_result, devNum, nodeData, lttEntryIndex, ltt_param)
    local is_failed, result, OUT_values, apiName;
    local sum_result;
    local node_block, node_index, node_in_pair;
    local pointer_block, data, command;

    sum_result = true;

    if db_result.node_found or db_result.added_node then
        node_block = nil;
        node_index = nil;
        if db_result.node_found then
            node_block = db_result.node_found.block;
            node_index = db_result.node_found.node;
        end
        if db_result.added_node then
            node_block = db_result.added_node.block;
            node_index = db_result.added_node.node;
        end
        if node_block == nil or node_index == nil then
            return command_data:addErrorAndPrint(
                "util_l2mll_add_node: write node - no node position");
        end
        if node_index == 0 then
            node_in_pair = "firstMllNode";
        else
            node_in_pair = "secondMllNode";
        end
        command = {};
        if db_result.new_block then
            command = {mark_second_last = {}};
        end
        sum_result = sum_result and l2mll_pair_update(
            command_data, devNum, node_block,
            {[node_in_pair] = nodeData} --[[-param-]],
            command);
    end

    if db_result.update_block_next_pointer then
        pointer_block = db_result.update_block_next_pointer.block;
        if not pointer_block then
            return command_data:addErrorAndPrint(
                "util_l2mll_add_node: update block pointer - no block index");
        end

        if db_result.update_block_next_pointer.next then
            node_block = db_result.update_block_next_pointer.next.block;
            node_index = db_result.update_block_next_pointer.next.node;
            if node_block == nil or node_index == nil then
                return command_data:addErrorAndPrint(
                    "util_l2mll_add_node: update block pointer - wrong next");
            end
            if node_index == 0 then
                node_in_pair = "CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E";
            else
                node_in_pair = "CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E";
            end
            data = {nextPointer = node_block, entrySelector = node_in_pair};
            command = {mark_second_not_last = {}};
         else
             data = {};
             command = {mark_second_last = {}};
         end

         sum_result = sum_result and l2mll_pair_update(
            command_data, devNum, pointer_block,
            data --[[-param-]], command);
    end

    if db_result.update_chain_pointer then
        node_block = 0;
        node_index = 0;
        node_in_pair = "CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E";
        if db_result.update_chain_pointer.pointer then
            node_block = db_result.update_chain_pointer.pointer.block;
            node_index = db_result.update_chain_pointer.pointer.node;
            if node_block == nil or node_index == nil then
                return command_data:addErrorAndPrint(
                    "util_l2mll_add_node: write ltt entry - no node position");
            end
            if node_index == 0 then
                node_in_pair = "CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E";
            else
                node_in_pair = "CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E";
            end
        end

        data = table_deep_copy(ltt_param);
        if not data then
            data = {};
        end
        data.mllPointer    = node_block;
        data.entrySelector = node_in_pair;

        sum_result = sum_result and l2mll_ltt_entry_write(
            command_data, devNum, lttEntryIndex, data)
    end

    if db_result.shift_nodes then
        if db_result.shift_nodes.low ~= 0 or
            db_result.shift_nodes.high ~= 0 then
            return command_data:addErrorAndPrint(
                "util_l2mll_add_node: shift nodes - wrong range");
        end
        node_block = db_result.shift_nodes.block;
        if node_block == nil then
            return command_data:addErrorAndPrint(
                "util_l2mll_add_node: shift nodes - no block index");
        end
        sum_result = sum_result and l2mll_pair_update(
            command_data, devNum, node_block,
            nil --[[-param-]],
            {move_first_to_second = {}} --[[command--]]);
    end

    return sum_result;
end

-- add node to L2Mll chain
-- devNum, chainName, nodeName - device number and names od chain and node in DB
-- nodeData - contents of CPSS_DXCH_L2_MLL_ENTRY_STC (not PAIR) for added node
-- lttEntryIndex - index in lookup table to update chain pointer
-- ltt_param - optional parametrs. ltt.mllMaskProfileEnable and ltt.mllMaskProfile
function util_l2mll_add_node(
    command_data, devNum, chainName, nodeName, nodeData, lttEntryIndex, ltt_param)
    local db_result, sum_result;
    db_result = sharedResourceNamedL2MllBlockedNodesAddNode(chainName, nodeName);
    if db_result == nil then
        return command_data:addErrorAndPrint(
            "util_l2mll_add_node: sharedResourceNamedL2MllBlockedNodesAddNode failed"
            .. " chainName " .. chainName .. " nodeName " .. nodeName);
    end

    return util_l2mll_do_node_db_result(
        command_data, db_result, devNum, nodeData, lttEntryIndex, ltt_param);
end

function util_l2mll_remove_node(
    command_data, devNum, chainName, nodeName, lttEntryIndex, ltt_param)
    local db_result, sum_result;
    db_result = sharedResourceNamedL2MllBlockedNodesRemoveNode(chainName, nodeName);
    if db_result == nil then
        return command_data:addErrorAndPrint(
            "util_l2mll_add_node: sharedResourceNamedL2MllBlockedNodesRemoveNode failed"
            .. " chainName " .. chainName .. " nodeName " .. nodeName);
    end

    return util_l2mll_do_node_db_result(
        command_data, db_result, devNum, nil --[[nodeData--]], lttEntryIndex, ltt_param);
end

function util_l2mll_remove_chain(
    command_data, devNum, chainName, lttEntryIndex, ltt_param)
    local nodes = sharedResourceNamedL2MllBlockedNodesGetNodesNames(chainName);
    local key, nodeName, ret;
    if not nodes then
        return nil;
    end
    ret = true;
    key = next(nodes, nil)
    while key do
        nodeName = nodes[key];
        ret = ret and util_l2mll_remove_node(
            command_data, devNum, chainName, nodeName, lttEntryIndex, ltt_param);
        key = next(nodes, key);
    end
    return ret;
end

--interactive test for debugging
function util_l2mll_add_remove_node_test()
    local devNum, nodeData;
    local command, args_line, args, w0, w1, ret;
    local command_data;
    local all_devices, devNum, hwDevNum;
    local is_failed, result, OUT_values, apiName;

    all_devices = wrlDevList();
    devNum = all_devices[1];
    command_data = Command_Data();
    apiName = "cpssDxChCfgHwDevNumGet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8", "devNum", devNum },
        { "OUT", "GT_HW_DEV_NUM", "hwDevNumPtr" }
    });
    if (is_failed == true) then
        command_data:analyzeCommandExecution();
        command_data:printCommandExecutionResults();
        return;
    end
    hwDevNum = OUT_values.hwDevNumPtr;

    while true do
        io.write("[TEST]> ");
        local line = io.read("*l");
        if line == nil then io.write('\n'); line = "exit" end

        if string.find(line, "[%w_]+") then
            command = string.sub(line, string.find(line, "[%w_]+"))
            args_line    = string.gsub(line,"[%w_]+%s*",'',1)            --strip command off line
            w1 = 1;
            args = {};
            for w0 in string.gmatch(args_line,"[%w_]+") do
                args[w1] = w0;
                w1 = w1 + 1;
            end
        else
            command = "";
        end

        if command == "help" then
            debug_print("commands:");
            debug_print("exit");
            debug_print("add chainName nodeName lttEntryIndex ePort");
            debug_print("remove chainName nodeName lttEntryIndex");
            debug_print("remove_chain chainName lttEntryIndex");
        elseif command == "exit" then
            break;
        elseif command == "add" then
            command_data = Command_Data();
            nodeData = {
                egressInterface = {
                    type = "CPSS_INTERFACE_PORT_E",
                    devPort = {
                        -- the CPSS inteface structure uses name "hwDevNum"
                        -- but LUA CLI uses name "devNum" (as in cpss 4.0)
                        hwDevNum = hwDevNum,
                        devNum = hwDevNum,
                        portNum = tonumber(args[4])--[[ePort--]]
                    }
                }
            };
             ret = util_l2mll_add_node(
                command_data, devNum, args[1]--[[chainName--]], args[2]--[[nodeName--]],
                nodeData, tonumber(args[3])--[[lttEntryIndex--]], {} --[[ltt_param--]]);
            debug_print("returned " .. to_string(ret));
            command_data:analyzeCommandExecution();
            command_data:printCommandExecutionResults();

        elseif command == "remove" then
            command_data = Command_Data();
            ret = util_l2mll_remove_node(
                command_data, devNum, args[1]--[[chainName--]], args[2]--[[nodeName--]],
                tonumber(args[3])--[[lttEntryIndex--]], {} --[[ltt_param--]]);
            debug_print("returned " .. to_string(ret));
            command_data:analyzeCommandExecution();
            command_data:printCommandExecutionResults();
        elseif command == "remove_chain" then
            command_data = Command_Data();
            ret = util_l2mll_remove_chain(
                command_data, devNum, args[1]--[[chainName--]],
                tonumber(args[2])--[[lttEntryIndex--]], {} --[[ltt_param--]]);
            debug_print("returned " .. to_string(ret));
            command_data:analyzeCommandExecution();
            command_data:printCommandExecutionResults();
        else
            debug_print("wrong command, use help");
        end
    end
end

--[[
end
config
do debug
trace cpss-api lua-style
exit
lua
util_l2mll_add_remove_node_test();
add ch1 n1_1 5 11
add ch1 n1_2 5 12
add ch1 n1_3 5 13
add ch1 n1_1 5 0x3B
remove ch1 n1_1 5
add ch1 n1_1 5 11
remove ch1 n1_1 5
remove ch1 n1_2 5
remove ch1 n1_3 5

--]]

-----------------------------------------------------------------------------------------------
-- L3MLL

-- param can contain replaced members of CPSS_DXCH_IP_MLL_PAIR_STC
-- firstMllNode, secondMllNode, nextPointer
-- command.mark_second_last = true
-- command.mark_first_last = true
-- command.mark_both_not_last = true
local function l3mll_pair_update(command_data, devNum, block, param, command)
    local is_failed, result, OUT_values, apiName;
    local mll_pair, key, last1, last2;

    param = table_deep_copy(param);
    if not param then
        param = {};
    end
    if param.firstMllNode then
        param.firstMllNode.last = nil;
    end
    if param.secondMllNode then
        param.secondMllNode.last = nil;
    end

    if not command then
        command = {};
    end

    apiName = "cpssDxChIpMLLPairRead";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",     "devNum",   devNum },
        { "IN", "GT_U32",   "mllPairEntryIndex",    block},
        { "IN", "CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT",   "mllPairReadForm",
            "CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E"},
        { "OUT", "CPSS_DXCH_IP_MLL_PAIR_STC",   "mllPairEntryPtr"}
    });

-- skip GT_BAD_STATE error for SIP4 devices because of wrong HW default value of MLLRPFFailCMD0 in MLLEntry 
    if (is_device_eArch_enbled(devNum) or result ~= 0x07) then
       if is_failed == true then
           return nil;
       end
    end

    mll_pair = OUT_values.mllPairEntryPtr;
    -- save "last" bits
    last1 = mll_pair.firstMllNode.last;
    last2 = mll_pair.secondMllNode.last;

    -- override all given data
    key = next(param, nil);
    while key do
        mll_pair[key] = param[key];
        key = next(param, key);
    end

    -- restore "last" bits
    mll_pair.firstMllNode.last  = last1;
    mll_pair.secondMllNode.last = last2;

    -- implement commad if any
    if command.mark_first_last then
        mll_pair.firstMllNode.last  = true;
        mll_pair.secondMllNode.last = nil;
    end
    if command.mark_second_last then
        mll_pair.firstMllNode.last  = nil;
        mll_pair.secondMllNode.last = true;
    end
    if command.mark_both_not_last then
        mll_pair.firstMllNode.last  = nil;
        mll_pair.secondMllNode.last = nil;
    end

    -- fix needed for succsessfull conversion SW => HW by cpssDxChIpMcRouteEntriesWrite

    if not mll_pair.firstMllNode.mllRPFFailCommand
        or mll_pair.firstMllNode.mllRPFFailCommand == "CPSS_PACKET_CMD_FORWARD_E"
    then
        mll_pair.firstMllNode.mllRPFFailCommand = "CPSS_PACKET_CMD_BRIDGE_E";
    end
    if not mll_pair.secondMllNode.mllRPFFailCommand
        or mll_pair.secondMllNode.mllRPFFailCommand == "CPSS_PACKET_CMD_FORWARD_E"
    then
        mll_pair.secondMllNode.mllRPFFailCommand = "CPSS_PACKET_CMD_BRIDGE_E";
    end

    apiName = "cpssDxChIpMLLPairWrite";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",     "devNum",   devNum },
        { "IN", "GT_U32",   "mllPairEntryIndex",    block},
        { "IN", "CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT",   "mllPairWriteForm",
            "CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E"},
        { "IN", "CPSS_DXCH_IP_MLL_PAIR_STC",   "mllPairEntryPtr",    mll_pair}
    });

    if is_failed == true then
        return nil;
    end
    return true;
end

local function mc_next_hop_entry_write(command_data, devNum, mcNextHopEntryIndex, mcNextHopEntry)
    local is_failed, result, OUT_values, apiName;

    apiName = "cpssDxChIpMcRouteEntriesWrite";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",     "devNum",   devNum },
        { "IN", "GT_U32",   "routeEntryIndex",    mcNextHopEntryIndex},
        { "IN", "CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC",   "routeEntryPtr", mcNextHopEntry}
    });

    if is_failed == true then
        return nil;
    end
    return true;
end

-- support for results of L3Mll chain manipulation functions db_result:
-- returned table that can contain elements below, "_" is some number
-- chain_created = {first_block = _}
-- added_node = {block = _, node = _, [last_node = true]};
--      (when overriding node found by name last_node == nil)
--      (if last and node ~= 0 reset the "last" bit of previous node in HW)
-- update_block_next_pointer = {block = _, next = _} (and reset the "last" bit in HW)
-- cleared_blocks = {_, ...}
local function util_l3mll_do_node_db_result(
    command_data, db_result, devNum, nodeData, mcNextHopEntryIndex, mcNextHopEntry)
    local is_failed, result, OUT_values, apiName;
    local sum_result, result;
    local node_block, node_in_pair;
    local pointer_block, data, command;
    local k;

    sum_result = true;

    if db_result.added_node then
        if db_result.added_node.block == nil
        or db_result.added_node.node  == nil then
            return command_data:addErrorAndPrint(
                "util_l3mll_do_node_db_result: add node - no node position");
        end
        command = {};
        node_block = db_result.added_node.block;
        if db_result.added_node.node == 0 then
            node_in_pair = "firstMllNode";
            if db_result.added_node.last_node then
                command.mark_first_last = true;
            end
        else
            node_in_pair = "secondMllNode";
            if db_result.added_node.last_node then
                command.mark_second_last = true;
            end
        end
        data = {[node_in_pair] = table_deep_copy(nodeData)};
        result = l3mll_pair_update(
            command_data, devNum, node_block, data, command);
        sum_result = result and sum_result;
    end

    if db_result.update_block_next_pointer then
        if db_result.update_block_next_pointer.block == nil
        or db_result.update_block_next_pointer.next  == nil then
            return command_data:addErrorAndPrint(
                "util_l3mll_do_node_db_result: upate_pointer - no data");
        end
        node_block = db_result.update_block_next_pointer.block;
        pointer_block = db_result.update_block_next_pointer.next;
        command = {mark_both_not_last = true};
        data = {nextPointer = pointer_block};
        result = l3mll_pair_update(
            command_data, devNum, node_block, data, command);
        sum_result = result and sum_result;
    end

    if db_result.chain_created then
        if db_result.chain_created.first_block == nil then
            return command_data:addErrorAndPrint(
                "util_l3mll_do_node_db_result: chain_created - no data");
        end
        pointer_block = db_result.chain_created.first_block;
        data = table_deep_copy(mcNextHopEntry);
        data.internalMLLPointer = pointer_block;
        if not data.cmd or
            data.cmd == "CPSS_PACKET_CMD_FORWARD_E"
        then
            data.cmd = "CPSS_PACKET_CMD_ROUTE_E"
        end
        if not data.RPFFailCommand or
            data.RPFFailCommand == "CPSS_PACKET_CMD_FORWARD_E"
        then
            data.RPFFailCommand = "CPSS_PACKET_CMD_BRIDGE_E"
        end
        result = mc_next_hop_entry_write(
            command_data, devNum, mcNextHopEntryIndex, data);
        sum_result = result and sum_result;
    end

    if db_result.cleared_blocks then
        command = {mark_both_not_last = true};
        data = {firstMllNode = {}, secondMllNode = {}, nextPointer = 0};
        k = next(db_result.cleared_blocks, nil);
        while k do
            node_block = db_result.cleared_blocks[k];
            result = l3mll_pair_update(
                command_data, devNum, node_block, data, command);
            sum_result = result and sum_result;
            k = next(db_result.cleared_blocks, k);
        end
    end

    return sum_result;
end

function util_l3mll_append_node(
    command_data, devNum, chainName, nodeName, nodeData, mcNextHopEntryIndex, mcNextHopEntry)
    local db_result;
    db_result = sharedResourceNamedL3MllBlockedNodesAppendNode(chainName, nodeName);
    if db_result == nil then
        return command_data:addErrorAndPrint(
            "util_l3mll_append_node: sharedResourceNamedL3MllBlockedNodesAppendNode failed"
            .. " chainName " .. chainName .. " nodeName " .. nodeName);
    end

    return util_l3mll_do_node_db_result(
        command_data, db_result, devNum, nodeData,
        mcNextHopEntryIndex, mcNextHopEntry);
end

function util_l3mll_remove_chain(command_data, devNum, chainName)
    local db_result;
    db_result = sharedResourceNamedL3MllBlockedNodesRemoveChain(chainName);
    if db_result == nil then
        return command_data:addErrorAndPrint(
            "util_l3mll_remove_chain: "
            .. " chainName " .. chainName .. " not found");
    end
    return util_l3mll_do_node_db_result(
        command_data, db_result, devNum, nil --[[nodeData--]],
        nil --[[mcNextHopEntryIndex--]], nil --[[mcNextHopEntry--]]);
end

--interactive test for debugging
function util_l3mll_append_node_remove_chain_test()
    local devNum, nodeData;
    local command, args_line, args, w0, w1, ret;
    local command_data;
    local all_devices, devNum, hwDevNum;
    local is_failed, result, OUT_values, apiName;

    all_devices = wrlDevList();
    devNum = all_devices[1];
    command_data = Command_Data();
    apiName = "cpssDxChCfgHwDevNumGet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8", "devNum", devNum },
        { "OUT", "GT_HW_DEV_NUM", "hwDevNumPtr" }
    });
    if (is_failed == true) then
        command_data:analyzeCommandExecution();
        command_data:printCommandExecutionResults();
        return;
    end
    hwDevNum = OUT_values.hwDevNumPtr;

    while true do
        io.write("[TEST]> ");
        local line = io.read("*l");
        if line == nil then io.write('\n'); line = "exit" end

        if string.find(line, "[%w_]+") then
            command = string.sub(line, string.find(line, "[%w_]+"))
            args_line    = string.gsub(line,"[%w_]+%s*",'',1)            --strip command off line
            w1 = 1;
            args = {};
            for w0 in string.gmatch(args_line,"[%w_]+") do
                args[w1] = w0;
                w1 = w1 + 1;
            end
        else
            command = "";
        end

        if command == "help" then
            debug_print("commands:");
            debug_print("exit");
            debug_print("append chainName nodeName mcHextHopIndex ePort");
            debug_print("remove_chain chainName");
        elseif command == "exit" then
            break;
        elseif command == "append" then
            command_data = Command_Data();
            nodeData = {
                nextHopInterface = {
                    type = "CPSS_INTERFACE_PORT_E",
                    devPort = {
                        -- the CPSS inteface structure uses name "hwDevNum"
                        -- but LUA CLI uses name "devNum" (as in cpss 4.0)
                        hwDevNum = hwDevNum,
                        devNum = hwDevNum,
                        portNum = tonumber(args[4])--[[ePort--]]
                    }
                },
                nextHopVlanId = 1
            };
            ret = util_l3mll_append_node(
                command_data, devNum,
                args[1]--[[chainName--]], args[2]--[[nodeName--]], nodeData,
                tonumber(args[3])--[[mcNextHopEntryIndex--]], {} --[[mcNextHopEntry--]]);
            debug_print("returned " .. to_string(ret));
            command_data:analyzeCommandExecution();
            command_data:printCommandExecutionResults();
        elseif command == "remove_chain" then
            command_data = Command_Data();
            ret = util_l3mll_remove_chain(
                command_data, devNum, args[1]--[[chainName--]]);
            debug_print("returned " .. to_string(ret));
            command_data:analyzeCommandExecution();
            command_data:printCommandExecutionResults();
        else
            debug_print("wrong command, use help");
        end
    end
end

--[[
end
config
do debug
trace cpss-api lua-style
exit
lua
util_l3mll_append_node_remove_chain_test();
append ch1 n1_1 5 11
append ch1 n1_2 5 12
append ch2 n2_1 3 21
append ch1 n1_3 5 13
append ch1 n1_1 5 0x3B
remove_chain ch1
remove_chain ch2
exit
--]]


