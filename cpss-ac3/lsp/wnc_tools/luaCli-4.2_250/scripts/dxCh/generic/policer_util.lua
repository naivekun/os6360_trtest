--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* policer_util.lua
--*
--* DESCRIPTION:
--*       policer utils
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

CLI_type_dict["policer_stage"] = {
	checker = CLI_check_param_enum,
	complete = CLI_complete_param_enum,
	help = "Policer stages",
	enum = {
		["ingress0"] = { value="CPSS_DXCH_POLICER_STAGE_INGRESS_0_E", help="Ingress0" },
		["ingress1"] = { value="CPSS_DXCH_POLICER_STAGE_INGRESS_1_E", help="Ingress1" },
		["egress"] =  { value="CPSS_DXCH_POLICER_STAGE_EGRESS_E", help="Egress" }
   }
}

-- saved from redefinition print function
local debug_print = print;

---------------------------------------------------------------------------------------
--
--
local maxRateTableDB = {}

local function stageToName(stage)
    if type(stage) == "number" then
        if stage == 0 then
            return "ingress0";
        elseif stage == 1 then
            return "ingress1";
        elseif stage == 2 then
            return "egress";
        end
    end
    if type(stage) == "string" then
        local s = string.lower(stage);
        if string.match(s, "egress") then
            return "egress";
        else
            if string.match(s, "1") then
                return "ingress1";
            end
        end
    end
    return "ingress0";
end

local function policerDbMaxRateTableBuildKey(rate0, rate1)
    return string.format("r0=%d, r1=%d", rate0, rate1);
end

function policerDbMaxRateTableInit(devNum, stage, reset)
    local s = stageToName(stage);
    if not maxRateTableDB[devNum] then
        maxRateTableDB[devNum] = {};
    end
    if maxRateTableDB[devNum][s] and (not reset) then
        return;
    end
    maxRateTableDB[devNum][s] =
    {
        entries = {{base = 1, size = 127}}, -- current free entries ranges
        key_to_index = {},  -- <rate0,rate1> => index
        index_to_key = {},  -- index => <rate0,rate1>
        user_to_index = {}, -- user => index
        users_list = {}     -- index => {[user_key] = {}, ...}
    };
end

-- returns index in the rate table or nil
local function prvPolicerDbAddMaxRateTableEntry(devNum, stage, rate0, rate1, user_key)
    local s = stageToName(stage);
    if nil == maxRateTableDB[devNum] or
        nil == maxRateTableDB[devNum][s] then
        policerDbMaxRateTableInit(devNum, stage, false);
    end
    local entry_index = maxRateTableDB[devNum][s].user_to_index[user_key];
    if entry_index then
        -- found for the entry
        return entry_index;
    end
    local rate_key = policerDbMaxRateTableBuildKey(rate0, rate1);
    entry_index = maxRateTableDB[devNum][s].key_to_index[rate_key];
    if nil == entry_index then
        entry_index = DbMemoAlloc(
            maxRateTableDB[devNum][s].entries, 1 --[[size--]]);
        if nil == entry_index then
            return nil;
        end
        maxRateTableDB[devNum][s].key_to_index[rate_key]    = entry_index;
        maxRateTableDB[devNum][s].index_to_key[entry_index] =
            {rate0 = rate0, rate1 = rate1};
        maxRateTableDB[devNum][s].users_list[entry_index] = {};
    end
    maxRateTableDB[devNum][s].user_to_index[user_key] = entry_index;
    maxRateTableDB[devNum][s].users_list[entry_index][user_key] = {};
    return entry_index;
end

local function prvPolicerDbRemoveMaxRateTableEntryByUser(devNum, stage, user_key)
    local s = stageToName(stage);
    if nil == maxRateTableDB[devNum] then
        return;
    end
    if nil == maxRateTableDB[devNum][s] then
        return;
    end
    local entry_index = maxRateTableDB[devNum][s].user_to_index[user_key];
    if nil == entry_index then
        -- not found for the user
        return;
    end
    maxRateTableDB[devNum][s].user_to_index[user_key] = nil;
    maxRateTableDB[devNum][s].users_list[entry_index][user_key] = nil;
    if (next(maxRateTableDB[devNum][s].users_list[entry_index])) then
        -- another users of this entry found
        return;
    end
    DbMemoFree(
        maxRateTableDB[devNum][s].entries, entry_index, 1 --[[size--]]);
    local rate_key_rates = maxRateTableDB[devNum][s].index_to_key[entry_index];
    local rate_key = policerDbMaxRateTableBuildKey(
        rate_key_rates.rate0, rate_key_rates.rate1);
    maxRateTableDB[devNum][s].key_to_index[rate_key]    = nil;
    maxRateTableDB[devNum][s].index_to_key[entry_index] = nil;
    maxRateTableDB[devNum][s].users_list[entry_index]   = nil;
end

-- returns found/allocated Max Rate Table Entry index
local function prvPolicerDbMaxRateTableEntryAlloc(
    devNum, stage, max_rate0, rate0, size0, max_rate1, rate1, size1, hw_entry_index)
    -- For effective sharing Max Rate Table Entry entries
    -- incoming bucket rate and size parameters should be converted
    -- to HW format as represented in Max Rate Table Entry.
    -- Temporary rates used as is.
    return prvPolicerDbAddMaxRateTableEntry(
        devNum, stage, max_rate0, max_rate1, hw_entry_index);
end

local function prvPolicerDbMaxRateTableEntryFree(devNum, stage, hw_entry_index)
    prvPolicerDbRemoveMaxRateTableEntryByUser(devNum, stage, hw_entry_index);
end

-- input:
-- hw_entry_index - base of envelope
-- coupling_flag0
-- entry_list - list of member bucket pair parameters, index below is
-- 0-based index of envelope member
-- {[index] = {
--       bucket0 = {max_rate = max_rate0, rate = rate0, size = size0},
--       bucket1 = {max_rate = max_rate1, rate = rate1, size = size1}},
--       ... }
-- returns nil on fail or
-- {[index] = {
--       max_rate_index = max_rate_index,
--       bucket0 = {max_rate = max_rate0, rate = rate0, size = size0},
--       bucket1 = {max_rate = max_rate1, rate = rate1, size = size1}}
--       ... }
function policerDbMaxRateTableEntryEnvelopeAlloc(
    devNum, stage, hw_entry_index, coupling_flag0, entry_list)
    local i,j;
    local envelope_size;
    local result = {};
    local max_rate_index;
    local bucket0, bucket1;

    --determinate envelope suze
    envelope_size = 0;
    while entry_list[envelope_size] do
        envelope_size = envelope_size + 1;
    end

    -- build result
    if not coupling_flag0 then
        for i = 0,(envelope_size - 1) do
            bucket0 = entry_list[envelope_size - 1 - i].bucket0;
            bucket1 = entry_list[envelope_size - 1 - i].bucket1;
            max_rate_index = prvPolicerDbMaxRateTableEntryAlloc(
                devNum, stage,
                bucket0.max_rate,
                bucket0.rate,
                bucket0.size,
                bucket1.max_rate,
                bucket1.rate,
                bucket1.size,
                (hw_entry_index + i));
            if not max_rate_index then
                -- rollback
                j = 0;
                while result[j] do
                    prvPolicerDbMaxRateTableEntryFree(
                        devNum, stage, (hw_entry_index + j));
                    j = j + 1;
                end
                return nil;
            end
            result[i] = {
                max_rate_index = max_rate_index,
                bucket0 = deepcopy(bucket0),
                bucket1 = deepcopy(bucket1)
            };
        end
    else --coupling_flag0==true
        local i0, b0, i1, b1;
        local abs_bucket_num;
        for i = 0,(envelope_size - 1) do
            -- to bucket0
            abs_bucket_num = (2 * i);
            if abs_bucket_num < envelope_size then
                i0 = (envelope_size - 1 - abs_bucket_num); b0 = "bucket0";
            else
                i0 = ((2 * envelope_size) - 1 - abs_bucket_num); b0 = "bucket1";
            end
            -- to bucket1
            abs_bucket_num = (2 * i) + 1;
            if abs_bucket_num < envelope_size then
                i1 = (envelope_size - 1 - abs_bucket_num); b1 = "bucket0";
            else
                i1 = ((2 * envelope_size) - 1 - abs_bucket_num); b1 = "bucket1";
            end
            bucket0 = entry_list[i0][b0];
            bucket1 = entry_list[i1][b1];
            max_rate_index = prvPolicerDbMaxRateTableEntryAlloc(
                devNum, stage,
                bucket0.max_rate,
                bucket0.rate,
                bucket0.size,
                bucket1.max_rate,
                bucket1.rate,
                bucket1.size,
                (hw_entry_index + i));
            if not max_rate_index then
                -- rollback
                j = 0;
                while result[j] do
                    prvPolicerDbMaxRateTableEntryFree(
                        devNum, stage, (hw_entry_index + j));
                    j = j + 1;
                end
                return nil;
            end
            result[i] = {
                max_rate_index = max_rate_index,
                bucket0 = deepcopy(bucket0),
                bucket1 = deepcopy(bucket1)
            };
        end
    end
    return result;
end

-- get maximal rates for envelope entries
-- input:
-- hw_metring_entry_index - base of envelope
-- envelope_size
-- coupling_flag0
-- returns nil on fail or
-- {[index] = {
--       max_rate_index = max_rate_index,
--       rate0 = max_rate0, rate1 = max_rate1}
--       ... }
function policerDbMaxRateTableEntryEnvelopeGet(
    devNum, stage, hw_metring_entry_index, coupling_flag0, envelope_size)
    local i,j;
    local result = {};
    local max_rate_index;
    local s = stageToName(stage);
    local index_to_key_data;

    if nil == maxRateTableDB[devNum] then
        return nil;
    end
    if nil == maxRateTableDB[devNum][s] then
        return nil;
    end

    for i = 0, (envelope_size - 1) do
        j = hw_metring_entry_index + i;
        max_rate_index = maxRateTableDB[devNum][s].user_to_index[j];
        if nil == max_rate_index then
            -- not found for the user
            return nil;
        end
        index_to_key_data =
            maxRateTableDB[devNum][s].index_to_key[max_rate_index];
        if nil == index_to_key_data then
            -- not found for the user
            return nil;
        end
        result[i] = {
            max_rate_index = max_rate_index,
            rate0 = index_to_key_data.rate0,
            rate1 = index_to_key_data.rate1,
        };
    end

    if not coupling_flag0 then
        -- when no coupling_flag0 - rates already at place
        return result;
    end

    -- rebuild result when coupling_flag0
    local save_res = deepcopy(result);
    local i0, b0, i1, b1;
    local abs_bucket_num;
    for i = 0,(envelope_size - 1) do
        -- from rate0
        abs_bucket_num = (2 * i);
        if abs_bucket_num < envelope_size then
            i0 = abs_bucket_num; b0 = "rate0";
        else
            i0 = (abs_bucket_num - envelope_size); b0 = "rate1";
        end
        -- from rate1
        abs_bucket_num = (2 * i) + 1;
        if abs_bucket_num < envelope_size then
            i1 = abs_bucket_num; b1 = "rate0";
        else
            i1 = (abs_bucket_num - envelope_size); b1 = "rate1";
        end
        result[i0][b0] = save_res[i].rate0;
        result[i0][b0] = save_res[i].rate1;
    end
    return result;
end

function policerDbMaxRateTableEntryEnvelopeFree(
    devNum, stage, hw_entry_index, envelope_size)
    local i;
    for i = 0,(envelope_size - 1) do
        prvPolicerDbMaxRateTableEntryFree(
            devNum, stage, (hw_entry_index + i));
    end
end

-- To configure MaxRateTable in HW
-- maxRateData is the data returned from policerDbMaxRateTableEntryEnvelopeAlloc
-- function
-- {[index] = {
--       max_rate_index = max_rate_index,
--       bucket0 = {max_rate = max_rate0, rate = rate0, size = size0},
--       bucket1 = {max_rate = max_rate1, rate = rate1, size = size1}}
--       ... }
function policerDbMaxRateTableEntryEnvelopeStoreToHw(
    devNum, stage, maxRateData)
    local ret, val, i, b;
    local bn = {"bucket0", "bucket1"};
    local max_rate_index, bucket;

    local s = stageToName(stage);
    local stage_val;
    if s == "egress" then
        stage_val = "CPSS_DXCH_POLICER_STAGE_EGRESS_E";
    elseif s == "ingress1" then
        stage_val = "CPSS_DXCH_POLICER_STAGE_INGRESS_1_E";
    else
        stage_val = "CPSS_DXCH_POLICER_STAGE_INGRESS_0_E";
    end

    i = 0;
    while maxRateData[i] do
        max_rate_index = maxRateData[i].max_rate_index;
        for b = 1,2 do
            bucket = maxRateData[i][bn[b]];
            ret,val = myGenWrapper("cpssDxChPolicerTokenBucketMaxRateSet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","CPSS_DXCH_POLICER_STAGE_TYPE_ENT","stage",stage_val},
                {"IN","GT_U32","entryIndex",max_rate_index},
                {"IN","GT_U32","bucketIndex",(b - 1)},
                {"IN","GT_U32","meterEntryRate",bucket.rate},
                {"IN","GT_U32","meterEntryBurstSize",bucket.size},
                {"IN","GT_U32","maxRate",bucket.max_rate},
            })
            if ret ~= 0 then
                print("Could not set PolicerTokenBucketMaxRate");
                return nil;
            end
        end
        i = i + 1;
    end
    return true;
end

--interactive test for debugging
function policerDbMaxRateTableTest()
    local command, args_line, args, w0, w1, ret;
    local envelope_size = 0;
    local envelope = {};
    local coupling_flag0 = false;
    local maxRateData = {};
    while true do
        io.write("[TEST]> ");
        local line = io.read("*l");
        if line == nil then io.write('\n'); line = "exit" end

        if string.find(line, "[%w_-]+") then
            command = string.sub(line, string.find(line, "[%w_-]+"))
            args_line    = string.gsub(line,"[%w_-]+%s*",'',1)            --strip command off line
            w1 = 1;
            args = {};
            for w0 in string.gmatch(args_line,"[%w_-]+") do
                args[w1] = w0;
                w1 = w1 + 1;
            end
        else
            command = "";
        end

        if command == "help" then
            debug_print("commands:");
            debug_print("exit");
            debug_print("init dev stage");
            debug_print("dump");
            debug_print("prv-alloc-entry dev stage index max-rate0 rate0 size0 max-rate1 rate1 size1");
            debug_print("prv-free-entry dev stage index ");
            debug_print("envelope-reset");
            debug_print("envelope-add max-rate0 rate0 size0 max-rate1 rate1 size1");
            debug_print("envelope-alloc dev stage index coupling_flag0");
            debug_print("envelope-max-rates-store dev stage");
            debug_print("envelope-free dev stage index");
        elseif command == "exit" then
            break;
        elseif command == "init" then
            policerDbMaxRateTableInit(tonumber(args[1]), args[2], args[3]);
        elseif command == "dump" then
            debug_print("maxRateTableDB " .. to_string(maxRateTableDB));
        elseif command == "prv-alloc-entry" then
            ret = prvPolicerDbMaxRateTableEntryAlloc(
                tonumber(args[1]), --devNum,
                args[2],           --stage,
                tonumber(args[4]), --max_rate0,
                tonumber(args[5]), --rate0,
                tonumber(args[6]), --size0,
                tonumber(args[7]), --max_rate1,
                tonumber(args[8]), --rate1,
                tonumber(args[9]), --size1,
                tonumber(args[3])) --hw_entry_index);
            debug_print("returned " .. to_string(ret));
        elseif command == "prv-free-entry" then
            ret = prvPolicerDbMaxRateTableEntryFree(
                tonumber(args[1]), --devNum,
                args[2],           --stage,
                tonumber(args[3])) --hw_entry_index)
            debug_print("returned " .. to_string(ret));
        elseif command == "envelope-reset" then
            envelope_size = 0;
            envelope = {};
        elseif command == "envelope-add" then
            envelope[envelope_size] = {
                bucket0 = {
                    max_rate = tonumber(args[1]),
                    rate = tonumber(args[2]),
                    size = tonumber(args[3])},
                bucket1 = {
                    max_rate = tonumber(args[4]),
                    rate = tonumber(args[5]),
                    size = tonumber(args[5])}
            };
            envelope_size = envelope_size + 1;
        elseif command == "envelope-alloc" then
            if tonumber(args[4]) == 0 then
                coupling_flag0 = false;
            else
                coupling_flag0 = true;
            end
            ret = policerDbMaxRateTableEntryEnvelopeAlloc(
                tonumber(args[1]), --devNum,
                args[2],           --stage,
                tonumber(args[3]), --hw_entry_index,
                coupling_flag0,
                envelope);
            if ret then
                maxRateData = deepcopy(ret);
            end
            debug_print("returned " .. to_string(ret));
        elseif command == "envelope-max-rates-store" then
            ret = policerDbMaxRateTableEntryEnvelopeStoreToHw(
                tonumber(args[1]), --devNum,
                args[2],           --stage,
                maxRateData);
            debug_print("returned " .. to_string(ret));
        elseif command == "envelope-free" then
            ret = policerDbMaxRateTableEntryEnvelopeFree(
                tonumber(args[1]), --devNum,
                args[2],           --stage,
                tonumber(args[3]), --hw_entry_index,
                envelope_size);
            debug_print("returned " .. to_string(ret));
        else
            debug_print("wrong command, use help");
        end
    end
end

--[[
This sequence was used to test policerDbMaxRateTable
lua
policerDbMaxRateTableTest();
init 0 i0
dump
prv-alloc-entry 0 i0 100 10000 2000 30000 10001 2001 30001
dump
prv-alloc-entry 0 i0 101 10010 2010 30010 10011 2011 30011
dump
prv-alloc-entry 0 i0 102 10020 2020 30020 10021 2021 30021
dump
prv-free-entry 0 i0 100
dump
prv-alloc-entry 0 i0 100 10000 2000 30000 10001 2001 30001
dump
prv-free-entry 0 i0 100
dump
prv-free-entry 0 i0 101
dump
prv-alloc-entry 0 i0 100 10020 2020 30020 10021 2021 30021
dump
prv-free-entry 0 i0 102
dump
prv-free-entry 0 i0 100
dump
envelope-reset
envelope-add 10000 2000 30000 10001 2001 30001
envelope-add 10010 2010 30010 10011 2011 30011
envelope-add 10020 2020 30020 10021 2021 30021
envelope-alloc 0 i0 100 0
dump
envelope-free 0 i0 100
dump
envelope-reset
envelope-add 10000 2000 30000 10001 2001 30001
envelope-add 10010 2010 30010 10011 2011 30011
envelope-add 10020 2020 30020 10021 2021 30021
envelope-alloc 0 i0 100 1
dump
envelope-max-rates-store 0 i0
envelope-free 0 i0 100
dump
exit
.
--]]

-- get policer mru
function generic_policer_mru_get(devNum, stage)
    local command_data = Command_Data();
    local apiName = "cpssDxCh3PolicerMruGet";
    local did_error , result , OUT_values =
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "CPSS_DXCH_POLICER_STAGE_TYPE_ENT", "stage", stage},
            { "OUT", "GT_U32", "mruSizePtr"},
        });
    return did_error, OUT_values.mruSizePtr;
end

function generic_policer_mru_set(devNum, stage, mru)
    local command_data = Command_Data();
    local apiName = "cpssDxCh3PolicerMruSet";
    local did_error , result , OUT_values =
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "CPSS_DXCH_POLICER_STAGE_TYPE_ENT", "stage", stage},
            { "IN", "GT_U32", "mruSize", mru},
        });
    return did_error;
end




