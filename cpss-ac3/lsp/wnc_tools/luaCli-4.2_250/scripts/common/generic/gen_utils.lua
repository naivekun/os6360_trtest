--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* gen_utils.lua
--*
--* DESCRIPTION:
--*       generic utils
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
--[[
the functions are :
    -- run and check cpssApi return code , if API return result != GT_OK --> set's warning/error message
    genericCpssApiWithErrorHandler(command_data,apiName, params)

    -- function to support iteration on all devices that we got from parameter "#all_device"
    generic_all_device_func(specificPerDeviceFunc,params)

    -- bind specific 'per interface' function to be called for all interfaces.
    generic_port_range_func(specificPerPortFunc,params,supportInfo)

    -- function to support 'show' that iteration on all devices that we got from parameter "#all_device"
    generic_all_device_show_func(specificPerDeviceFunc,params)

    -- function to support 'show' that iteration on all ports that we got from parameter "#all_interfaces"
    generic_all_ports_show_func(specificPerPortFunc,params,supportInfo)

the commands are:
    CLI_addCommand("exec", "cls",
    CLI_addCommand("exec", "shell-execute",
]]--
cmdLuaCLI_registerCfunction("luaShellExecute")
cmdLuaCLI_registerCfunction("luaShellExecute_inOtherThread")
cmdLuaCLI_registerCfunction("luaConsoleClearScreen")
cmdLuaCLI_registerCfunction("luaTaskCreate")
cmdLuaCLI_registerCfunction("luaOsSigSemCreate")
cmdLuaCLI_registerCfunction("luaOsSigSemWait")
cmdLuaCLI_registerCfunction("luaOsSigSemDelete")


--##################################
--##################################

-- saved from redefinition print function
local debug_print = print;

local debug_on = false
local function _debug(...)
    if debug_on == true then
        debug_print (...)
    end
end

function check_expected_value(name , expected_value , actualValue)
    local compare_special = {string_case_insensitive = true}

    local descrption = "Comparing expected value(s) of " .. name

    local diffValues = table_differences_get(expected_value, actualValue , nil, compare_special)

    if not diffValues then
        local pass_string = descrption .. " PASSED."
        printLog (pass_string)
            -- makes LOG too big when all is OK -->
            -- "With value: " .. to_string(expected_value))
        testAddPassString(pass_string)
    else
        -- not the expected value
        local error_string = "ERROR : " .. descrption .. " FAILED."
        printLog (error_string .. " due to: " , to_string(diffValues))
        testAddErrorString(error_string)
    end

    return
end

-----------------------------------------------------------------------------------
-- string_for_table_... functions are utilities for printing tables.
-- they return strings for printing.
--
-- separators is a lua-table that contains
-- {left = <left_side_separator>, middle = <regular separator>,
--   right = <right_side_separator>, horizontal = <horizontal_separator>}
-- default is {left = "", middle = " ", right = "", horizontal = "-"}
--
-- column_list is a lua-table indexed 1,2,... containing column descriptions
-- {<column1 description>, <column2 description>, ...}
--
-- entry is a lua-table, containing values for printing indexed by keys
--    specified in column descriptions as {id = 1, vid = 2, ...}
--
-- column description is
-- {<key_path>, [<title>], ["%C_FORMAT {<type>[, [<prefix>], [<postfix>]]}
-- example {"tti_rule_index", , {"X", "0x", }}
--
-----------------------------------------------------------------------------------

local function sft_parse_separators(separators)
    local sep = {left = "", middle = " ", right = "", horizontal = "-"};
    local sep_keys = {"left", "middle", "right", "horizontal"};
    local sk, key, val;

    if (separators == nil) then
        return sep;
    end

    if (type(separators) == "string") then
        sep[middle] = separators;
        return sep;
    end

    if (type(separators) ~= "table") then
        print("sft_parse_separators error");
        return sep;
    end

    sk = next(sep_keys, nil);
    while (sk ~= nil) do
        key = sep_keys[sk];
        val = separators[key];
        if (type(val) == "string") then
            sep[key] = val;
        elseif (val ~= nil) then
            print("sft_parse_separators error");
        end
        sk = next(sep_keys, sk);
    end

    return sep;
end

local function sft_parse_column_description(column_description)
    local col_desc = {};
    local length, index;
    local path, title, format, fmt, len, l1, s1;
    local flags, convert, fmt1;

    if (type(column_description) == "string") then
        col_desc["path"]   = {column_description};
        col_desc["title"]  = column_description;
        length             = string.len(column_description);
        col_desc["format"] =
        {type = ("%" .. string.format("%d", length) .. "d"),
         prefix = "", postfix = ""};
        return col_desc;
    end

    if (type(column_description) ~= "table") then
        print("sft_parse_column_description error");
        return nil;
    end

    index = 1;
    path = column_description["path"];
    if (path == nil) then
        path = column_description[index];
        index = index + 1;
    end
    if (path == nil) then
        print("sft_parse_column_description error");
        return nil;
    end

    title = column_description["title"];
    if (title == nil) then
        title = column_description[index];
        index = index + 1;
    end
    if (title == nil) then
        if (type(path) == "string") then
            title = path;
        end
    end

    format = column_description["format"];
    if (format == nil) then
        fmt = column_description[index];
        index = index + 1;
        if (type(fmt) == "string") then
                fmt = {fmt, "", ""};
        end
        if (type(fmt) ~= "table") then
            print("sft_parse_column_description error");
            return nil;
        end
        s1 = fmt[1];
        if (type(s1) ~= "string") then
            print("sft_parse_column_description error");
            return nil;
        end
        if (s1 == "hex") then
            format = {prefix = "0x", postfix = fmt[2],
                ["flags"] = fmt["flags"], ["convert"] = fmt["convert"]};
            s1 = "X";
        elseif (s1 == "string") then
            format = {prefix = fmt[2], postfix = fmt[3],
                flags = "-", ["convert"] = fmt["convert"]};
            s1 = "s";
        else
            format = {prefix = fmt[2], postfix = fmt[3],
                ["flags"] = fmt["flags"], ["convert"] = fmt["convert"]};
        end
        if (format["prefix"] == nil) then
            format["prefix"] = "";
        end
        if (format["postfix"] == nil) then
            format["postfix"] = "";
        end

        len = string.len(title);
        l1  = string.len(format["prefix"]) + string.len(format["postfix"]);
        if (l1 >= len) then
            print("sft_parse_column_description error");
            return nil;
        end

        flags = format["flags"];
        if (flags == nil) then
            flags = "";
        end;

        len = len - l1;
        if (format["prefix"] == "") then
            format["type"]  =
                "%" .. string.format("%s%d", flags, len) .. s1;
        else
            format["type"]  =
                "%" .. string.format("%s%d.%d", flags, len, len) .. s1;
        end
        convert = format["convert"];
        if (convert ~= nil) and (type(convert) ~= "table") then
            print("sft_parse_column_description error");
            return nil;
        end
        format["convert"] = convert;
    end

    if (type(path) == "table") then
        col_desc["path"] = path;
    elseif (type(path) == "string") then
        col_desc["path"] = {path};
    else
        print("sft_parse_column_description error");
        return nil;
    end
    col_desc["title"] = title;
    col_desc["format"] = format;
    return col_desc;
end

function string_for_table_title(separators, columns_list)
    local key, val, first;
    local sep = sft_parse_separators(separators);
    local col;
    local str = sep["left"];

    first = true;
    key = next(columns_list, nil);
    while (key ~= nil) do
        if (first == true) then
            first = false;
        else
            str = str .. sep["middle"];
        end
        val = columns_list[key];
        col = sft_parse_column_description(val);
        str = str .. col["title"];
        key = next(columns_list, key);
    end
    str = str .. sep["right"];
    return str;
end

function string_for_table_break(separators, columns_list)
    local key, val, first;
    local sep = sft_parse_separators(separators);
    local col, len, s1, rep, l1;
    local str = sep["left"];

    first = true;
    key = next(columns_list, nil);
    while (key ~= nil) do
        if (first == true) then
            first = false;
        else
            str = str .. sep["middle"];
        end
        val = columns_list[key];
        col = sft_parse_column_description(val);
        len = string.len(col["title"]);
        l1  = string.len(sep["horizontal"]);
        rep = 1 + ((len - (len % l1)) / l1);
        s1 = string.rep(sep["horizontal"], rep);
        s1 = string.sub(s1, 1, len);
        str = str .. s1;
        key = next(columns_list, key);
    end
    str = str .. sep["right"];
    return str;
end

function string_for_table_line(separators, columns_list, entry)
    local key, val, first, key1, path, format;
    local sep = sft_parse_separators(separators);
    local col, s1, convert, val1;
    local str = sep["left"];

    first = true;
    key = next(columns_list, nil);
    while (key ~= nil) do
        if (first == true) then
            first = false;
        else
            str = str .. sep["middle"];
        end
        val = columns_list[key];
        col = sft_parse_column_description(val);

        -- extract value from entry by path
        path = col["path"];
        val = entry;
        key1 = next(path, nil);
        while (key1 ~= nil) do
            val = val[path[key1]];
            if (val == nil) then
                print("string_for_table_line <path not found> error " .. path[key1]);
                return nil;
            end
            key1 = next(path, key1);
        end
        if (type(val) == "table") then
            print("string_for_table_line <path to table> error " .. path[key1]);
            return nil;
        end

        -- formating value
        format = col["format"];

        -- optional conversion value
        convert = format["convert"];
        if (convert ~= nil) then
            val1 = convert[val];
            if (val1 ~= nil) then
                val = val1;
            end
        end

        s1 = format["type"];
        str = str
            .. (format["prefix"] .. string.format(s1, val) .. format["postfix"]);

        -- next iteration
        key = next(columns_list, key);
    end
    str = str .. sep["right"];
    return str;
end
-----------------------------------------------------------------------------------------------------------
-- converts table with any indexes and entries to the new table
-- with the same entries and indexes 1..n
-- returns the result table
-- can be used for compresstion list-table after removing members
function convert_table_to_list(tab)
    local key, entry, num, list;
    list = {};
    num = 1;
    key = next(tab, nil);
    while (key ~= nil) do
        list[num] = tab[key];
        num = num + 1;
        key = next(tab, key);
    end
    return list;
end

-- inplace compressing of list
-- used after removing members
function list_table_inplace_compress(list)
    local key, entry, src_num, dst_num, max_num;
    max_num = 0;
    key = next(list, nil);
    while (key ~= nil) do
        if max_num < key then
            max_num = key;
        end
        key = next(list, key);
    end
    dst_num = 1;
    for src_num = 1,max_num do
        if list[src_num] ~= nil then
            if src_num ~= dst_num then
                list[dst_num] = list[src_num];
                list[src_num] = nil;
            end
            dst_num = dst_num + 1;
        end
    end
end

-- this function returns table that contains of pairs
-- key = {expected = v1, actual = v2} for each different expected_table.key == v1 and actual_table.key == v2
-- if tables contain sub tries the function calls itself recursive over sub tries
-- equal_vals :
--      is a list of lists of values that will not be treated as different
--      nil cannot be obtained as table member value, empty list {} should be specified instead
--      for example {{"Text", "text"}, {"1", "1.0", "+1"}}
--      if nil specified the default list used {{0, "", false, {} --[[instead of nil--]]}}
--      if all different values must be distinguished, specify {} but not nil
-- compare_special :
--      is list of 'compare' cases with next fields:
--          string_case_insensitive = true/false/nil
--              this allow to compare strings as 'case insensitive' (and actually using 'lower case' before the compare)
--      if nil specified then it is ignored.
--
function table_differences_get(expected_table, actual_table, equal_vals , compare_special)
local key, entry, k1, v1;
    local found1, found2;
    local diff = {};

    -- default equal values
    if equal_vals == nil then
        equal_vals = {{0, "", false, {} --[[instead of nil--]]}}
    end

    if compare_special == nil then
        compare_special = {}
    end

    -- both are not tables - comparing
    if type(expected_table) ~= "table" and type(actual_table) ~= "table" then
        local is_diff
        if type(expected_table) == "string" and type(actual_table) == "string" and
            compare_special.string_case_insensitive == true
        then
            local lower_expected_table = string.lower(expected_table)
            local lower_actual_table = string.lower(actual_table)

            is_diff = (lower_expected_table ~= lower_actual_table)
        else
            is_diff = (expected_table ~= actual_table)
        end

        if is_diff then
            if equal_vals ~= nil then
                -- searching in equal_vals - maybe to ignore difference
                key = next(equal_vals, nil);
                while (key ~= nil) do
                    entry = equal_vals[key];
                    found1 = false;
                    found2 = false;
                    k1 = next(entry, nil);
                    while (k1 ~= nil) do
                        v1 = entry[k1];
                        -- support using {} instead of nil
                        if type(v1) == "table" then
                            v1 = v1[1];
                        end
                        if expected_table == v1 then
                            found1 = true;
                        end
                        if actual_table == v1 then
                            found2 = true;
                        end
                        k1 = next(entry, k1);
                    end
                    if found1 == true and found2 == true then
                        return nil;
                    end
                    key = next(equal_vals, key);
                end
            end
            return {expected = expected_table, actual = actual_table};
        else
            return nil;
        end
    end

    -- one is not table - difference
    if type(expected_table) ~= "table" or type(actual_table) ~= "table" then
        return {expected = expected_table, actual = actual_table};
    end

    -- both are tables
    -- accomulating differences using by expected_table
    key = next(expected_table, nil);
    while (key ~= nil) do
        diff[key] = table_differences_get(expected_table[key], actual_table[key], equal_vals, compare_special);
        key = next(expected_table, key);
    end
    -- accomulating differences using by actual_table only for keys
    -- missed in expected_table (keys present in expected_table already accomulated)
    key = next(actual_table, nil);
    while (key ~= nil) do
        if expected_table[key] == nil then
            diff[key] = table_differences_get(expected_table[key], actual_table[key], equal_vals, compare_special);
        end
        key = next(actual_table, key);
    end

    -- if table empty return nil instead of it
    if next(diff, nil) == nil then
        return nil;
    end

    return diff;
end

-- the funtion returns the deep copy of given table
function table_deep_copy(tab)
    local key, entry;
    local copy = {};
    if type(tab) ~= "table" then
        return tab;
    end
    key = next(tab, nil);
    while (key ~= nil) do
        copy[key] = table_deep_copy(tab[key]);
        key = next(tab, key);
    end
    return copy;
end

-- the function returns deep copy of the table cleaning up null values
-- tab - source table
-- strip_tree - true means delete empty sub-trees {}
--              false - delete leafs only
-- null_vals - list of values treated as nil
---            default is {0, "", false} - used if nil specified
function table_deep_copy_stripped(tab, strip_tree, null_vals)
    local key, entry;
    local copy = {}
    if null_vals == nil then
        null_vals = {0, "", false};
    end
    -- table leafs case
    if type(tab) ~= "table" then
        key = next(null_vals, nil);
        while (key ~= nil) do
            if tab == null_vals[key] then
                return nil;
            end
            key = next(null_vals, key);
        end
        return tab;
    end
    -- sub-tree case
    key = next(tab, nil);
    while (key ~= nil) do
        copy[key] =
            table_deep_copy_stripped(
                tab[key], strip_tree, null_vals);
        key = next(tab, key);
    end
    -- empty table case
    if strip_tree == true then
        if next(copy, nil) == nil then
            return nil;
        end
    end
    return copy;
end

-- returns if "subtree" is equal to some subtree of "tree" true, else false
function table_is_equal_to_subtree(
    tree, subtree, recursion_limit, case_sensitive, print_error)
    if (type(tree) ~= "table") or (type(subtree) ~= "table") then
        if case_sensitive and (type(tree) == "string") and (type(subtree) == "string") then
            local u_tree = string.upper(tree);
            local u_subtree = string.upper(subtree);
            if (u_tree ~= u_subtree) then
                if print_error then
                    print("different: " .. u_tree .. " and " .. u_subtree .. "\n");
                    return false;
                end
            end
            return true;
        end
        if (tree ~= subtree) then
            if print_error then
                print("different: " .. to_string(tree) ..
                      " and " .. to_string(subtree) .. "\n");
            end
            return false;
        end
        return true;
    end
    if recursion_limit <= 0 then
        if print_error then
            print("recursion_limit reached 0\n");
        end
        return false;
    end
    for k,v in pairs(subtree) do
        if not table_is_equal_to_subtree(
            tree[k], subtree[k], (recursion_limit - 1), case_sensitive, print_error) then
            if print_error then
                print("different under key: " .. k .. "\n");
            end
            return false;
        end
    end
    return true;
end

-- port list has structure {[dev1] = {port1, port2, ...}, [dev2] = {port1, port2, ...}, ...}
-- function copyes to new list only suuch ports: low <= port <= high
function port_list_limited_copy(list, low, high)
    local dev, port, src, dst, new_list, k, i;
    new_list = {};
    dev = next(list, nil);
    while dev do
        src = list[dev];
        dst = {};
        i = 1;
        k = next(src, nil);
        while k do
            port = src[k];
            if low <= port and port <= high then
                dst[i] = port;
                i = i + 1;
            end
            k = next(src, k);
        end
        new_list[dev] = dst;
        dev = next(list, dev);
    end
    return new_list;
end

-----------------------------------------------------------------------------------------------------------
-- alloc/free generic functions
-- functions manage Database of free memory chunks
-- in such form: {{base = <chunk_base>, size = <chunk_size>}, ...}
-- Database should be initialized by caller and typically contain one chunk

-- function DbMemoAlloc(DB, size)
-- allocates chunk of <size> memory elements from <DB>
-- the function removes the allocated chunk from <DB>
-- returns the base of allocated chunk on success or nil on fail
function DbMemoAlloc(DB, size)
    local key, entry, base;
    key = next(DB, nil);
    while key ~= nil do
        entry = DB[key];
        if entry.size >= size then
            base = entry.base;
            if entry.size == size then
                DB[key] = nil;
            else
                entry.size = entry.size - size;
                entry.base = entry.base + size;
            end
            return base;
        end
        key = next(DB, key);
    end
    return nil;
end

-- function DbMemoFree(DB, base, size)
-- adds to <DB> chunk of <size> memory elements from <base>
-- the function used free the chunk previously allocated from <DB>
-- returns true (OK) or nil (Error)
function DbMemoFree(DB, base, size)
    local key, entry, free_key;
    local low_idx, high_idx, low_base, high_base;
    local low_initial = -1;
    local high_initial = 0xFFFFFFFF;

    low_base  = low_initial;
    high_base = high_initial;
    key = next(DB, nil);
    while key ~= nil do
        entry = DB[key];
        if entry.base <= base then
            if entry.base > low_base then
                low_base = entry.base;
                low_idx  = key;
            end
        end
        if entry.base > base then
            if entry.base < high_base then
                high_base = entry.base;
                high_idx  = key;
            end
        end
        key = next(DB, key);
    end

    if (low_base ~= low_initial) then
        if (low_base + DB[low_idx].size) > base then
            return nil; -- overlapping with being freed
        end
    end
    if (high_base ~= high_initial) then
        if (high_base < (base + size)) then
            return nil; -- overlapping with being freed
        end
    end

    -- joinig low, being_freed and high
    if (low_base ~= low_initial) and (high_base ~= high_initial) then
        if ((low_base + DB[low_idx].size + size) == high_base) then
            DB[low_idx].size =
                DB[low_idx].size + size + DB[high_idx].size;
            DB[high_idx] = nil;
            return true;
        end
    end

    -- joinig low and being_freed
    if (low_base ~= low_initial) then
        if (low_base + DB[low_idx].size) == base then
            DB[low_idx].size = DB[low_idx].size + size;
            return true;
        end
    end

    -- joinig high and being_freed
    if (high_base ~= high_initial) then
        if (high_base == (base + size)) then
            DB[high_idx].size =
                DB[high_idx].size + size;
            DB[high_idx].base = base;
            return true;
        end
    end

    -- adding new free chunk
    free_key = 1;
    while true do
        if DB[free_key] == nil then
            break;
        end
        free_key = free_key + 1;
    end
    DB[free_key] = {base = base, size = size};
    return true;
end

-- removes given memory chunk from memory DB entry specified by key
-- to mark the removed memory busy
-- returns the true on success or nil on fail
local function prvDbMemoRemoveFromEntry(DB, key, base, size)
    local entry = DB[key];
    local free_key;

    if (entry.base > base) or
        ((entry.base + entry.size) < (base + size)) then
        return nil;
    end
    if entry.base == base then
        if entry.size == size then
            -- remove all of entry
            DB[key] = nil;
        else
            -- remove origin of entry
            entry.base = entry.base + size;
            entry.size = entry.size - size;
        end
    elseif (entry.base + entry.size) == (base + size) then
        -- remove end part of the entry (never all of it)
        entry.size = entry.size - size;
    else
        -- adding new entry - looking for unused key value
        free_key = 1;
        while true do
            if DB[free_key] == nil then
                break;
            end
            free_key = free_key + 1;
        end
        --high part of not removed memory - new node
        DB[free_key] = {};
        DB[free_key].base = (base + size);
        DB[free_key].size = (entry.base + entry.size) - (base + size);
        --low part of not removed memory
        entry.size = (base - entry.base);
    end

    return true;
end

-- function DbMemoSetAllocated(DB, base, size)
-- checks that all of given chunk <base, size> of memory elements from <DB>
-- is free and removes it from free memory (i.e. sets it allocated)
-- returns the true on success or nil on fail
function DbMemoSetAllocated(DB, base, size)
    local key, entry, ret;
    key = next(DB, nil);
    while key ~= nil do
        entry = DB[key];
        ret = prvDbMemoRemoveFromEntry(DB, key, base, size);
        if ret ~= nil then
            return true;
        end
        key = next(DB, key);
    end
    return nil;
end

-- function DbMemoGenAlloc(DB, alignment, size, options)
-- allocates chunk of <size> memory elements from <DB>
-- with base aligned to <alignment>
-- options is "high", "low" or "any" (nil means "any")
-- requires the highest or the lowest of free mery chunks.
-- the function removes the allocated chunk from <DB>
-- returns the base of allocated chunk on success or nil on fail
function DbMemoGenAlloc(DB, alignment, size, options)
    local key, entry, base;
    local w, aligned_base, saved_base;
    local ret, saved_base, saved_key;
    if options == nil then
        options = "any";
    end
    if options == "any" then
    elseif options == "high" then
        saved_base = -1;
    elseif options == "low" then
        saved_base = 0xFFFFFFFF;
    else
        --wrong options
        debug_print("DbMemoGenAlloc: wrong options");
        return nil;
    end
    saved_key = nil;
    key = next(DB, nil);
    while key ~= nil do
        entry = DB[key];
        w = entry.base + alignment - 1;
        aligned_base = w - (w % alignment);
        if (aligned_base + size) <= (entry.base + entry.size) then
            if options == "any" then
                saved_key = key;
                break;
            elseif options == "high" then
                if saved_base < entry.base then
                    saved_base = entry.base;
                    saved_key = key;
                end
            elseif options == "low" then
                if saved_base > entry.base then
                    saved_base = entry.base;
                    saved_key = key;
                end
            end
        end
        key = next(DB, key);
    end

    if saved_key == nil then
        return nil;
    end

    entry = DB[saved_key];
    if options == "high" then
        w = (entry.base + entry.size) - size;
        base = w - (w % alignment);
    else
        -- both "low" and "any"
        w = entry.base + alignment - 1;
        base = w - (w % alignment);
    end

    ret = prvDbMemoRemoveFromEntry(DB, saved_key, base, size);
    if ret == nil then
        -- must never occur.
        debug_print("prvDbMemoRemoveFromEntry unexpected error");
        return nil;
    end
    return base;
end

-- find memory by name in namedMemoDB or allocate if not found
-- save in DB with key == name-parameter
-- freeMemoDB, alignment, size, options - passed to DbMemoGenAlloc
-- returns the base of allocated chunk on success or nil on fail
function DbNamedMemoryAlloc(namedMemoDB, freeMemoDB, name, alignment, size, options)
    local entry = namedMemoDB[name];
    if entry == nil then
        -- not found by name - allocate
        entry = {};
        entry.size = size;
        entry.base = DbMemoGenAlloc(freeMemoDB, alignment, size, options);
        if entry.base == nil then
            return nil;
        end
        -- save
        namedMemoDB[name] = entry;
        return entry.base;
    end
    if entry.size ~= size then
        debug_print("DbNamedMemoAlloc found memory with different size");
    end
    if (entry.base % alignment) ~= 0 then
        debug_print("DbNamedMemoAlloc found memory with wrong alignment");
    end
    return entry.base;
end

-- find memory by name in namedMemoDB or set it allocated if not found
-- save in DB with key == name-parameter
-- freeMemoDB, base, size - passed to DbMemoSetAllocated
-- returns the true on success or nil on fail
function DbNamedMemorySetAllocated(namedMemoDB, freeMemoDB, name, base, size)
    local entry = namedMemoDB[name];
    local ret;
    if entry == nil then
        ret = DbMemoSetAllocated(freeMemoDB, base, size);
        if ret == nil then
            return nil
        end
        entry = {};
        entry.size = size;
        entry.base = base;
        namedMemoDB[name] = entry;
    end
    if entry.base ~= base then
        debug_print("DbNamedMemorySetAllocated found memory with different base");
    end
    if entry.size ~= size then
        debug_print("DbNamedMemorySetAllocated found memory with different size");
    end
    return true;
end

-- find memory by name in namedMemoDB
-- free the memory to freeMemoDB
-- and erase from namedMemoDB
-- returns the true on success or nil on fail
function DbNamedMemoryFree(namedMemoDB, freeMemoDB, name)
    local entry = namedMemoDB[name];
    local ret;
    if entry == nil then
        return nil;
    end
    ret = DbMemoFree(freeMemoDB, entry.base, entry.size);
    if ret == nil then
        debug_print("DbNamedMemoryFree: DbMemoFree failed");
    end
    namedMemoDB[name] = nil;
    return ret;
end

--interactive test for debugging
function DbNamedMemoryTest()
    local freeMemo = {{base = 0, size = 16}};
    local namedMemo = {};
    local command, args_line, args, w0, w1, ret;
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
            debug_print("init low hig ");
            debug_print("dump");
            debug_print("alloc name alignment size options");
            debug_print("free name");
            debug_print("set name base size");
        elseif command == "exit" then
            break;
        elseif command == "init" then
            freeMemo = {{base = tonumber(args[1]), size = tonumber(args[2])}};
            namedMemo = {};
        elseif command == "dump" then
            debug_print("freeMemo " .. to_string(freeMemo));
            debug_print("namedMemo " .. to_string(namedMemo));
        elseif command == "alloc" then
            ret = DbNamedMemoryAlloc(
                namedMemo, freeMemo,
                args[1]--[[name--]],
                tonumber(args[2])--[[alignment--]],
                tonumber(args[3])--[[size--]],
                args[4]--[[options--]]);
            debug_print("returned " .. to_string(ret));
        elseif command == "free" then
            ret = DbNamedMemoryFree(
                namedMemo, freeMemo,
                args[1]--[[name--]])
                debug_print("returned " .. to_string(ret));
        elseif command == "set" then
            ret = DbNamedMemorySetAllocated(
                namedMemo, freeMemo,
                args[1]--[[name--]],
                tonumber(args[2])--[[base--]],
                tonumber(args[3])--[[size--]]);
            debug_print("returned " .. to_string(ret));
        else
            debug_print("wrong command, use help");
        end
    end
end

--[[
This sequence was used to test DbNamedMemory
lua
DbNamedMemoryTest();
init 16 16
dump
alloc m1 4 3 high
dump
alloc m1 4 3 high
alloc m2 4 3 high
dump
alloc m3 2 3 low
dump
set m4 21,2
dump
alloc m5 1 2 any
dump
free m4
dump
free m3
dump
free m5
dump
free m1
dump
alloc m6 1 3 high
dump
free m6
dump
exit
.
--]]

-- Chains of blocked_nodes management
-- Needed for L2Mll Entries management.
-- This model supposes that
-- 1. Each block contains the same amount of nodes (nodes_in_block parameter, 2 for L2Mll)
--    and pointer to chain continuation.
-- 2. The pointer to the blocked_nodes chain is a pair of next_block_index and node_index (in block)
--    pointer = {block = index_in_memory, node = index_in_block} and makes
--    all nodes in block = pointer.block from pointer.index up to nodes_in_block
--    members of the chain. The first nodes of the block are not chain members.
--    The block fullfilled from the end and pointer into it updated each time.
-- 3. The order of nodes in the chain is not imported and the new node added to the
--    first found free place in used blocks (if not found new block allocated)
-- 4. Functions AddNode and RemoveNode have named and unnamed versions
--    The named versions of functions call unnamed versions.
-- 5. Database for unnamed versions reflects the HW representation
--    It can contain several chains with common continuation (even from
--    different nodes of the same block), but there are no functions to
--    support such configuration.
--    blockedNodesDb = {
--    nodes_in_block = 2,
--    blocks = {
--        [block_index] = {next = {block = block_index, node = node_index}}
--    }}
--    for block last in the chain [block_index] = {}
--
-- returns result that explains all needed changes in HW or nil on fail
-- chainPtr for new chain must be nil
--
-- returned table that can contain elements below, "_" is some number
-- update_chain_pointer = {pointer = {block = _, node = _}}
-- added_node = {block = _, node = _};
-- update_block_next_pointer = {block = _, next = {block = _, node = _}}
-- cleared_block = _;
-- shift_nodes = {block = _, low = _, high = _)};
-- node_found = {block = _, node = _};
-- new_block = true

function DbBlockedNodesAddNode(freeMemoDb, blockedNodesDb, chainPtr)
    local prev_block = nil;
    local chain_length = 0;
    local is_new_block = nil;
    while true do
        if chainPtr == nil then
            -- allocating new block - will be appended to the chain
            chainPtr = {
                block = DbMemoAlloc(freeMemoDb, 1 --[[size--]]),
                -- the node index will be decreased below
                node = blockedNodesDb.nodes_in_block
            };
            if chainPtr.block == nil then
                return nil;
            end
            -- save new block in db
            blockedNodesDb.blocks[chainPtr.block] = {};
            is_new_block = true;
        else
            chainPtr = table_deep_copy(chainPtr);
        end
        if blockedNodesDb.blocks[chainPtr.block] == nil then
            debug_print("DbBlockedNodesAddNode - wrong block pointer");
            return nil; -- wrong parameter or error in DB
        end
        if chainPtr.node ~= 0 then
            chainPtr.node = chainPtr.node - 1;
            if prev_block == nil then
                return {update_chain_pointer = {pointer = chainPtr},
                    added_node = chainPtr,
                    new_block = is_new_block};
            else
                blockedNodesDb.blocks[prev_block] =
                    {next = table_deep_copy(chainPtr)};
                return {
                    update_block_next_pointer =
                    {
                        block = prev_block,
                        next = chainPtr
                    },
                    added_node = chainPtr,
                    new_block = is_new_block};
            end
        end
        prev_block = chainPtr.block;
        chainPtr = blockedNodesDb.blocks[chainPtr.block].next;
        chain_length = chain_length + 1;
        if chain_length > 1000 then
            debug_print("DbBlockedNodesAddNode - chain_length > 1000");
            return nil; -- wrong parameter or error in DB
        end
    end
end

-- returns result that explains all needed changes in HW or nil on fail
function DbBlockedNodesRemoveNode(freeMemoDb, blockedNodesDb, chainPtr, nodePtr)
    local prev_block = nil;
    local chain_length = 0;
    local saved_next, next_ptr, shift_nodes;
    while true do
        if chainPtr == nil then
            debug_print("DbBlockedNodesRemoveNode - node not found in chain");
            return nil; -- wrong parameter or error in DB
        end
        if blockedNodesDb.blocks[chainPtr.block] == nil then
            debug_print("DbBlockedNodesRemoveNode - wrong block pointer");
            return nil; -- wrong parameter or error in DB
        end
        if chainPtr.block == nodePtr.block then
            break;
        end
        prev_block = chainPtr.block;
        chainPtr = blockedNodesDb.blocks[chainPtr.block].next;
        chain_length = chain_length + 1;
        if chain_length > 1000 then
            debug_print("DbBlockedNodesRemoveNode - chain_length > 1000");
            return nil; -- wrong parameter or error in DB
        end
    end
    if chainPtr.node > nodePtr.node then
        debug_print("DbBlockedNodesRemoveNode - specified node is out of used part of block");
        return nil; -- wrong parameter or error in DB
    end
    if nodePtr.node >= blockedNodesDb.nodes_in_block then
        debug_print("DbBlockedNodesRemoveNode - specified node index is too big");
        return nil; -- wrong parameter or error in DB
    end
    if (chainPtr.node + 1) >= blockedNodesDb.nodes_in_block then
        -- removing block with last node
        saved_next = table_deep_copy(blockedNodesDb.blocks[chainPtr.block].next);
        blockedNodesDb.blocks[chainPtr.block] = nil;
        DbMemoFree(freeMemoDb, chainPtr.block --[[base--]], 1 --[[size--]]);
        if prev_block == nil then
            return {update_chain_pointer = {pointer = saved_next},
                cleared_block = chainPtr.block};
        else
            blockedNodesDb.blocks[prev_block].next = table_deep_copy(saved_next);
            return {
                update_block_next_pointer =
                {
                    block = prev_block,
                    next = saved_next;
                },
                cleared_block = chainPtr.block};
        end
    else
        -- increasing pointer to the same block
        next_ptr = {
            block = chainPtr.block,
            node = (chainPtr.node + 1)};
        shift_nodes = nil;
        if chainPtr.node < nodePtr.node then
            shift_nodes = {block = chainPtr.block,
                low = chainPtr.node, high = (nodePtr.node - 1)};
        end
        if prev_block == nil then
            return {
                update_chain_pointer = {pointer = next_ptr},
                shift_nodes = shift_nodes};
        else
            blockedNodesDb.blocks[prev_block].next =
                table_deep_copy(next_ptr);
            return {
                update_block_next_pointer =
                {
                    block = prev_block,
                    next = next_ptr;
                },
                shift_nodes = shift_nodes};
        end
    end
end

-- named versions of functions
-- structure of names database:
-- namesDb = {[chainName] = {
--     pointer = {block = block_global_index, node = node_in_block_index},
--     nodes = {[node_name] =
--          {block = block_global_index, node = node_in_block_index}}}
function DbNamedBlockedNodesAddNode(
    freeMemoDb, blockedNodesDb, namesDb, chainName, nodeName)
    local chain = namesDb[chainName];
    local chainPtr = nil;
    local result;
    local nodes = nil;
    local nodePtr = nil;
    if chain then
        if chain.pointer == nil or chain.nodes == nil then
            debug_print("DbNamedBlockedNodesAddNode - wrong chain node in DB");
            return nil; -- wrong parameter or error in DB
        end;
        chainPtr = chain.pointer;
        nodes = chain.nodes;
        nodePtr = nodes[nodeName];
        if nodePtr ~= nil then
            return {node_found = nodePtr};
        end
    end
    result = DbBlockedNodesAddNode(freeMemoDb, blockedNodesDb, chainPtr);
    if result == nil then
        return nil;
    end
    if namesDb[chainName] == nil then
        namesDb[chainName] = {
                pointer = result.update_chain_pointer.pointer,
                nodes = {[nodeName] = result.added_node};
            };
    else
        if result.update_chain_pointer then
            namesDb[chainName].pointer = result.update_chain_pointer.pointer;
        end
        if result.added_node then
            namesDb[chainName].nodes[nodeName] = result.added_node;
        end
    end
    return result;
end

function DbNamedBlockedNodesRemoveNode(
    freeMemoDb, blockedNodesDb, namesDb, chainName, nodeName)
    local chain = namesDb[chainName];
    local chainPtr = nil;
    local result;
    local nodes = nil;
    local nodePtr = nil;
    local key, low, high, block;

    if chain == nil then
        return nil;
    end;
    if chain.pointer == nil or chain.nodes == nil then
        debug_print("DbNamedBlockedNodesRemoveNode - wrong chain node in DB");
        return nil; -- wrong parameter or error in DB
    end;
    chainPtr = chain.pointer;
    nodes = chain.nodes;
    nodePtr = nodes[nodeName];
    if nodePtr == nil then
        return nil;
    end
    result = DbBlockedNodesRemoveNode(freeMemoDb, blockedNodesDb, chainPtr, nodePtr);
    if result == nil then
        return nil;
    end

    nodes[nodeName] = nil;
    if result.update_chain_pointer then
        if result.update_chain_pointer.pointer then
            namesDb[chainName].pointer =
                result.update_chain_pointer.pointer;
        else
            -- chain pointer updated to nil
            -- so remove chain
            namesDb[chainName] = nil;
        end
    end
    if result.shift_nodes then
        block  = result.shift_nodes.block;
        low    = result.shift_nodes.low;
        high   = result.shift_nodes.high;
        key = next(nodes, nil);
        while key do
            if nodes[key].block == block
                and nodes[key].node >= low
                and nodes[key].node <= high
                then
                nodes[key].node = nodes[key].node + 1;
            end
            key = next(nodes, key);
        end
    end
    return result;
end

--interactive test for debugging
function DbBlockedNodesTest()
    local freeMemo = {{base = 0, size = 16}};
    local blockedNodesDb = {
        nodes_in_block = 2,
        blocks = {}};
    local namesDb = {};
    local command, args_line, args, w0, w1, ret;
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
            debug_print("init nodes_per_block memo_low memo_high ");
            debug_print("dump");
            debug_print("add chainName nodeName");
            debug_print("remove chainName nodeName");
        elseif command == "exit" then
            break;
        elseif command == "init" then
            freeMemo = {{base = tonumber(args[2]), size = tonumber(args[3])}};
            blockedNodesDb = {
                nodes_in_block = tonumber(args[1]),
                blocks = {}};
            namesDb = {};
        elseif command == "dump" then
            debug_print("freeMemo " .. to_string(freeMemo));
            debug_print("blockedNodesDb " .. to_string(blockedNodesDb));
            debug_print("namesDb " .. to_string(namesDb));
        elseif command == "add" then
            ret = DbNamedBlockedNodesAddNode(
                freeMemo, blockedNodesDb, namesDb,
                args[1]--[[chainName--]],
                args[2]--[[nodeName--]]);
            debug_print("returned " .. to_string(ret));
        elseif command == "remove" then
            ret = DbNamedBlockedNodesRemoveNode(
                freeMemo, blockedNodesDb, namesDb,
                args[1]--[[chainName--]],
                args[2]--[[nodeName--]]);
            debug_print("returned " .. to_string(ret));
        else
            debug_print("wrong command, use help");
        end
    end
end

--[[
testing commands:
lua
DbBlockedNodesTest();
init 2 100 20
add ch1 n1_1
dump
add ch1 n1_2
dump
add ch1 n1_1
dump
add ch1 n1_3
dump
add ch2 n2_1
dump
add ch1 n1_4
dump
add ch1 n1_5
dump
remove ch1 n1_2
dump
remove ch1 n1_1
dump
remove ch1 n1_3
dump
remove ch1 n1_5
dump
remove ch1 n1_4
dump
remove ch2 n2_1
dump
--]]

-- Chains of L3 blocked_nodes management
-- Needed for L3Mll Entries management.
-- This model supposes that
-- 1. Each block contains the same amount of nodes (nodes_in_block parameter, 2 for L3Mll)
--    and pointer to chain next block.
-- 2. The pointer to the blocked_nodes chain is a first_block_index
--    All nodes in block from 0 up to block_free_node <= nodes_in_block
--    are members of the chain.
-- 3. The order of nodes in the chain is not imported and the new node appended to the
--    free place in the last block (if not new block allocated)
-- 4. Functions AppendNode and RemoveChain have named and unnamed versions
--    The named versions of functions call unnamed versions.
---   Removing single node from the chain not supported.
-- 5. Database for unnamed versions reflects the HW representation
--    It can contain several chains with common continuation,
--    but there are no functions to support such configuration.
--    blockedNodesDb = {
--    nodes_in_block = 2,
--    blocks = {
--        [block_index] = {next_block = next_block_index, first_free_node = node_index}}
--    }}
--    for last block next_block == nil
--    for full block first_free_node == nil
-- 6. Chain pointer is only first_block or nil. (not as in L2Mll because never updated)
--
-- returns result that explains all needed changes in HW or nil on fail
-- chainFirstBlock for new chain must be nil
--
-- returned table that can contain elements below, "_" is some number
-- chain_created = {first_block = _}
-- added_node = {block = _, node = _, [last_node = true]};
--      (when overriding node found by name last_node == nil)
--      (if last and node ~= 0 reset the "last" bit of previous node in HW)
-- update_block_next_pointer = {block = _, next = _} (and reset the "last" bit in HW)
-- cleared_blocks = {_, ...}

function DbBlockedL3NodesAppendNode(freeMemoDb, blockedL3NodesDb, chainFirstBlock)
    local block, blockPtr;
    local free_node;
    local prev_block = nil;
    if not chainFirstBlock then
        if blockedL3NodesDb.nodes_in_block > 1 then
            free_node = 1;
        else
            free_node = nil;
        end
        block = DbMemoAlloc(freeMemoDb, 1 --[[size--]]);
        if block == nil then
            return nil;
        end
        -- save new block in db
        blockedL3NodesDb.blocks[block] = {first_free_node = free_node};
        return {
            chain_created = {first_block = block},
            added_node = {block = block, node = 0, last_node = true}
        };
    end
    -- first block
    block = chainFirstBlock;
    blockPtr = blockedL3NodesDb.blocks[block];
    if not blockPtr then
        debug_print("DbBlockedL3NodesAppendNode - wrong chain first block pointer");
        return nil; -- wrong parameter or error in DB
    end
    -- find last block
    while blockPtr.next_block do
        block = blockPtr.next_block;
        blockPtr = blockedL3NodesDb.blocks[block];
        if not blockPtr then
            debug_print("DbBlockedL3NodesAppendNode - wrong next block pointer in DB");
            return nil; -- wrong parameter or error in DB
        end
    end
    -- last block not full case
    if blockPtr.first_free_node then
        free_node = blockPtr.first_free_node;
        if (free_node + 1) < blockedL3NodesDb.nodes_in_block then
            blockPtr.first_free_node = free_node + 1;
        else
            blockPtr.first_free_node = nil;
        end
        return {
            added_node = {block = block, node = free_node, last_node = true}
        };
    end
    -- last block full case
    prev_block = block;
    if blockedL3NodesDb.nodes_in_block > 1 then
        free_node = 1;
    else
        free_node = nil;
    end
    block = DbMemoAlloc(freeMemoDb, 1 --[[size--]]);
    if block == nil then
        return nil;
    end
    -- save new block in db
    blockedL3NodesDb.blocks[prev_block].next_block = block;
    blockedL3NodesDb.blocks[block] = {first_free_node = free_node};
    return {
        added_node = {block = block, node = 0, last_node = true},
        update_block_next_pointer = {block = prev_block, next = block}
    };
end

function DbBlockedL3NodesRemoveChain(freeMemoDb, blockedL3NodesDb, chainFirstBlock)
    local block, blockPtr;
    local block_list = {};
    local block_num;
    local block_cur;
    if not chainFirstBlock then
        debug_print("DbBlockedL3NodesRemoveChain - wrong chain first block pointer");
        return nil;
    end
    -- add all block indexes to block_list
    -- first block
    block = chainFirstBlock;
    blockPtr = blockedL3NodesDb.blocks[block];
    if not blockPtr then
        debug_print("DbBlockedL3NodesRemoveChain - wrong chain first block pointer");
        return nil; -- wrong parameter or error in DB
    end
    block_num = 1;
    block_list[block_num] = block;
    -- pass chain to the end
    while blockPtr.next_block do
        block = blockPtr.next_block;
        blockPtr = blockedL3NodesDb.blocks[block];
        if not blockPtr then
            debug_print("DbBlockedL3NodesAppendNode - wrong next block pointer in DB");
            return nil; -- wrong parameter or error in DB
        end
        block_num = block_num + 1;
        block_list[block_num] = block;
    end

    for block_cur = 1,block_num  do
        block = block_list[block_cur];
        blockedL3NodesDb.blocks[block] = nil;
        DbMemoFree(freeMemoDb,block --[[base--]], 1 --[[size--]]);
    end
    return {cleared_blocks = block_list};
end

-- named versions of functions
-- structure of names database:
-- namesDb = {[chainName] = {
--     first_block = block_global_index,
--     nodes = {[node_name] =
--          {block = block_global_index, node = node_in_block_index}}}
function DbNamedBlockedL3NodesAppendNode(
    freeMemoDb, blockedL3NodesDb, l3NamesDb, chainName, nodeName)
    local chain = l3NamesDb[chainName];
    local result;
    local nodes = nil;
    local nodePtr = nil;
    local first_block;
    if chain then
        if chain.first_block == nil or chain.nodes == nil then
            debug_print("DbNamedBlockedL3NodesAppendNode - wrong chain node in DB");
            return nil; -- wrong parameter or error in DB
        end;
        nodes = chain.nodes;
        nodePtr = nodes[nodeName];
        if nodePtr ~= nil then
            return {
                added_node = {
                    block = nodePtr.block,
                    node = nodePtr.node
                }};
        end
    end
    first_block = nil;
    if chain then
        first_block = chain.first_block;
    end
    result = DbBlockedL3NodesAppendNode(freeMemoDb, blockedL3NodesDb, first_block);
    if result == nil then
        return nil;
    end
    if (result.added_node == nil)
        or (result.added_node.block == nil)
        or (result.added_node.node == nil) then
        debug_print(
            "DbNamedBlockedL3NodesAppendNode - wrong internal result on unnamed level");
        return nil; -- wrong parameter or error in DB
    end;
    if l3NamesDb[chainName] == nil then
        l3NamesDb[chainName] = {
                first_block = result.chain_created.first_block,
                nodes = {[nodeName] = {
                    block = result.added_node.block,
                    node  = result.added_node.node
                }};
            };
    else
        if result.added_node then
            l3NamesDb[chainName].nodes[nodeName] = {
                block = result.added_node.block,
                node  = result.added_node.node
            };
        end
    end
    return result;
end


function DbNamedBlockedL3NodesRemoveChain(
    freeMemoDb, blockedL3NodesDb, l3NamesDb, chainName)
    local first_block;
    if l3NamesDb[chainName] == nil then
        return nil;
    end
    first_block = l3NamesDb[chainName].first_block;
    l3NamesDb[chainName] = nil;
    return DbBlockedL3NodesRemoveChain(
        freeMemoDb, blockedL3NodesDb, first_block);
end

--interactive test for debugging
function DbBlockedL3NodesTest()
    local freeMemo = {{base = 0, size = 16}};
    local blockedL3NodesDb = {
        nodes_in_block = 2,
        blocks = {}};
    local l3NamesDb = {};
    local command, args_line, args, w0, w1, ret;
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
            debug_print("init nodes_per_block memo_low memo_high ");
            debug_print("dump");
            debug_print("append chainName nodeName");
            debug_print("remove_chain chainName");
        elseif command == "exit" then
            break;
        elseif command == "init" then
            freeMemo = {{base = tonumber(args[2]), size = tonumber(args[3])}};
            blockedL3NodesDb = {
                nodes_in_block = tonumber(args[1]),
                blocks = {}};
            namesL3Db = {};
        elseif command == "dump" then
            debug_print("freeMemo " .. to_string(freeMemo));
            debug_print("blockedL3NodesDb " .. to_string(blockedL3NodesDb));
            debug_print("l3NamesDb " .. to_string(l3NamesDb));
        elseif command == "append" then
            ret = DbNamedBlockedL3NodesAppendNode(
                freeMemo, blockedL3NodesDb, l3NamesDb,
                args[1]--[[chainName--]],
                args[2]--[[nodeName--]]);
            debug_print("returned " .. to_string(ret));
        elseif command == "remove_chain" then
            ret = DbNamedBlockedL3NodesRemoveChain(
                freeMemo, blockedL3NodesDb, l3NamesDb,
                args[1]--[[chainName--]]);
            debug_print("returned " .. to_string(ret));
        else
            debug_print("wrong command, use help");
        end
    end
end

--[[
testing commands:
lua
DbBlockedL3NodesTest();
init 2 100 20
append ch1 n1_1
dump
append ch1 n1_2
dump
append ch1 n1_1
dump
append ch1 n1_3
dump
append ch2 n2_1
dump
append ch1 n1_4
dump
append ch1 n1_5
dump
remove_chain ch1
dump
remove_chain ch2
dump
--]]

--##################################
--##################################

-- function to build the list of 'OUT' parameters that need to be compared
-- NOTE:            currently no support for 'INOUT'
--        look for next format of 'OUT' parameter :
--                    [1]           [2]                                 [3]            [4-optional]
--                  { "OUT",     "CPSS_DXCH_PORT_PIP_PROTOCOL_ENT",    "protocol",     protocol    },
--                  --> when field [4] 'exists' it is 'compared' with the actual value returned from the CPSS call !!!
--        @return
--          1. true/false - operation failed
--          2. list of 'OUT' parameters
--
local function buildExpectedOutValues(apiName, params)
    local did_error = false
    local expectOutParams = {}

    for index,entry in pairs(params) do
        if entry[1] == "OUT" and entry[4] then -- entry[4] hold the value to compare with.
            -- add the entry to the 'expected OUT list'
            expectOutParams[#expectOutParams + 1] = entry
        end
    end

    return did_error , expectOutParams
end
-- function to check that the actual out parameters are as expected
--        look for next format of 'OUT' parameter :
--                    [1]           [2]                                 [3]            [4-optional]
--                  { "OUT",     "CPSS_DXCH_PORT_PIP_PROTOCOL_ENT",    "protocol",     protocol    },
--                  --> when field [4] 'exists' it is 'compared' with the actual value returned from the CPSS call !!!
--        @return
--          1. true/false - operation failed
--              true  - API returned values != GT_OK
--                      or when 'OUT' parameters not match expected in expectOutParams --> see comments above , about fields [4],[5]
--              false - API returned GT_OK
local function checkOutParameters(command_data,apiName, actualOutParams , expectOutParams)
    local did_error = false

    _debug("actualOutParams" , to_string(actualOutParams))
    --print("expectOutParams" , to_string(expectOutParams))

    if actualOutParams == nil then
        return true -- error
    end

    for index,entry in pairs(expectOutParams) do
        local param_name = entry[3]
        local param_expected_value = entry[4]

        if param_name == nil  or param_expected_value == nil then
            command_data:addError(
                "ERROR: '" .. param_name .. "' bad format from the 'expected' parameters " ..
                to_string(entry))
            -- bad format from the 'expected' parameters
            did_error = true
            break
        end

        --print("actualOutParams["..param_name.."]" , to_string(actualOutParams[param_name]))
        --print("param_expected_value" , to_string(param_expected_value))

        local compare_special = {string_case_insensitive = true}

        local diffValues = table_differences_get(param_expected_value, actualOutParams[param_name], nil, compare_special)
        if diffValues then
            command_data:addError(
                "ERROR: '" .. param_name .. "' is not 'as expected':" ..
                to_string(diffValues))
            -- not the expected value
            did_error = true
            break
        end

    end

    return did_error
end

-- ************************************************************************
---
--  genericCpssApiWithErrorHandler
--        @description  run and check cpssApi return code
--                      if API return result != GT_OK --> set's warning/error message
--                      for 'OUT' parameters it 'may' compare to expected result (and raise error if not match)
--
--        NOTE: command_data.dontStateErrorOnCpssFail to state "allow bypass error 'registration'"
--
--        @param apiName   - the API name
--        @param params    - The parameters of the command
--                      the format is for 'IN' parameter :
--                    [1]           [2]                                 [3]             [4]
--                  { "IN",     "CPSS_DXCH_PORT_PIP_PROTOCOL_ENT",    "protocol",     protocol },
--
--                      the format is for 'OUT' parameter :
--                    [1]           [2]                                 [3]            [4-optional]
--                  { "OUT",     "CPSS_DXCH_PORT_PIP_PROTOCOL_ENT",    "protocol",     protocol    },
--                  --> when field [4] 'exists' it is 'compared' with the actual value returned from the CPSS call !!!
--
--        @return
--          1. true/false - operation failed
--              true  - API returned values != GT_OK
--                      or when 'OUT' parameters not match expected in 'OUT' fields --> see comments above , about field [4]
--              false - API returned GT_OK
--          2. the value that returned by the cpssApi .
--          3. the combined 'OUT' parameters of the API
--
function genericCpssApiWithErrorHandler(command_data,apiName, params)
    _debug("before calling : " .. apiName .. " with parameters: " .. to_string(params))

    --printCallStack()

    local did_error

    local devNum     = params[1][4]
    local usePort
    if params[2] ~= nil then
        local param2type = params[2][2]

        if param2type == "GT_PHYSICAL_PORT_NUM" or param2type == "GT_PORT_NUM" then
            usePort = true
        else
            usePort = false
        end
    end

    -- call to run the API
    local result,OUT_values = myGenWrapper(apiName, params, 1)
    _debug(apiName .. " ended with result : " .. to_string(result))

    for_error_apiName = apiName .. " " .. ",params = " .. to_string(params) -- needed for the generic error by command_data

    if usePort == true then
        local portNum = params[2][4]
        command_data:handleCpssErrorDevPort(result, for_error_apiName , devNum , portNum)
    else
        command_data:handleCpssErrorDevice(result, for_error_apiName , devNum)
    end

    if result == 0 then
        --did_error = false  -- no error
        local expected_OUT_values
        did_error , expected_OUT_values = buildExpectedOutValues(apiName, params)

        if did_error == false then
            did_error = checkOutParameters(command_data,apiName,OUT_values , expected_OUT_values)
        end
    else
        did_error = true   -- did error
    end

    if did_error and
       not command_data.dontStateErrorOnCpssFail -- allow bypass error 'registration'
    then
        command_data:addError("ERROR: " .. apiName .. " FAILED")
        command_data:setFailStatus()
        command_data:setFailLocalStatus()
    end

    return did_error , result , OUT_values
end


--##################################
--##################################
-- function to support iteration on all devices that we got from parameter "#all_device"
-- prototype of : specificPerDeviceFunc(command_data,devNum,params)
function generic_all_device_func(specificPerDeviceFunc,params)
    local command_data
    if not params.command_data then
        command_data = Command_Data()
    else
        command_data = params.command_data
    end

    -- Common variables initialization
    command_data:initAllDeviceRange(params)

    -- now iterate
    command_data:iterateOverDevices(specificPerDeviceFunc,params)

    if not params.bypass_final_return then
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
        return command_data:getCommandExecutionResults()
    else
        -- the caller still have to do the 'final return'
        return true
    end

end

--##################################
--##################################

-- bind specific 'per interface' function to be called for all interfaces.
-- special parameters that looked at:
--          params.command_data                --> should be always used
--          params.bypass_final_return         --> allow to skip 'analysing,print,execution' of 'end of command'
-- allow to define which interfaces are supported by the function :
--  supportInfo = {eport = , ethernet =}
--  if supportInfo is nil then considered 'support ALL interfaces'
-- prototype of : specificPerPortFunc(command_data, devNum, portNum, params)
function generic_port_range_func(specificPerPortFunc,params,supportInfo)
    local didError = false
    local command_data
    if not params.command_data then
        command_data = Command_Data()
    else
        command_data = params.command_data
    end

    local supportedInterfaces = {}
    if supportInfo == nil then
        supportInfo = {eport = true, ethernet = true}
    else
        -- we trust the info from the caller ... to state exact supported interfaces
    end

    if supportInfo.ethernet then
        table.insert(supportedInterfaces,"ethernet")
    end
    if supportInfo.eport then
        table.insert(supportedInterfaces,"eport")
    end

    local isSupported

    if params.all_interfaces then
        command_data:initAllInterfacesPortIterator(params)

        if params.all_interfaces == "all" then
            isSupported = true
        elseif params.all_interfaces == "ethernet" and supportInfo.ethernet then
            isSupported = true
        elseif params.all_interfaces == "eport" and supportInfo.eport then
            isSupported = true
        elseif params.all_interfaces == "devID" then
            isSupported = true
        else
            command_data:setFailStatus()
            command_data:setFailLocalStatus()
            command_data:addError(params.all_interfaces .. " interface is not supported")
            isSupported = false
        end

    else
        command_data:initInterfaceDevPortRange()
        isSupported = command_data:checkIterfaceType(supportedInterfaces)
    end

    if isSupported == true then
        -- now iterate
        command_data:iterateOverPorts(specificPerPortFunc, params)
    end

    if not params.bypass_final_return then
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
        return command_data:getCommandExecutionResults()
    else
        -- the caller still have to do the 'final return'
        return true
    end

end

--##################################
--##################################


local function internal_showTblResult(params)
    local command_data
    if not params.command_data then
        command_data = Command_Data()
    else
        command_data = params.command_data
    end

    local header_string = params.header_string
    local footer_string = params.footer_string

    local resultFunc

    if params.use_port_info == true then
        resultFunc = command_data.setResultStrOnPortCount
    else
        resultFunc = command_data.setResultStrOnEntriesCount
    end

    --print("params",to_string(params))

    command_data:setResultArrayToResultStr()

    local function table_length(T)
      local count = 0
      for dummy in pairs(T) do count = count + 1 end
      return count
    end

    local message_on_no_entries = "There is no information to show.\n"

    if(0 == table_length(command_data["result"])) then
        print(message_on_no_entries)
    else
        resultFunc(command_data,
                header_string,
                command_data["result"],
                footer_string,
                message_on_no_entries)
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()
end


-- function to support 'show' that iteration on all devices that we got from parameter "#all_device"
-- prototype of : specificPerDeviceFunc(command_data,devNum,params)
-- special parameters that looked at:
--          params.command_data                --> should be always used
--          params.bypass_final_return         --> allow to skip 'getCommandExecutionResults' of 'end of command'
--          params.header_string               --> the 'header' of the table
--          params.footer_string               --> the 'footer' of the table
--          params.disablePausedPrinting       --> indication that we NOT want 'Type <CR> to continue, Q<CR> to stop:'
--
function generic_all_device_show_func(specificPerDeviceFunc,params)
    local from_caller_bypass_final_return = params.bypass_final_return

    local command_data
    if not params.command_data then
        command_data = Command_Data()
    else
        command_data = params.command_data
    end

    -- Common variables initialization
    command_data:clearResultArray()

    params.command_data = command_data
    params.bypass_final_return = true

    -- Command specific variables initialization
    local header_string = params.header_string
    local footer_string = params.footer_string

    -- System specific data initialization.
    if not params.disablePausedPrinting then
        command_data:enablePausedPrinting()
    end
    command_data:setEmergencyPrintingHeaderAndFooter(header_string,
                                                     footer_string)


    generic_all_device_func(specificPerDeviceFunc,params)

    -- print the table.
    internal_showTblResult(params)

    if not from_caller_bypass_final_return then
        return command_data:getCommandExecutionResults()
    else
        -- the caller still have to do the 'final return'
        return true
    end

end
--##################################
--##################################

-- function to support 'show' that iteration on all ports that we got from parameter "#all_interfaces"
-- prototype of : specificPerPortFunc(command_data, devNum, portNum, params)
-- special parameters that looked at:
--          params.command_data                --> should be always used
--          params.bypass_final_return         --> allow to skip 'getCommandExecutionResults' of 'end of command'
--          params.header_string               --> the 'header' of the table
--          params.footer_string               --> the 'footer' of the table
--          params.disablePausedPrinting       --> indication that we NOT want 'Type <CR> to continue, Q<CR> to stop:'
-- allow to define which interfaces are supported by the function :
--  supportInfo = {eport = , ethernet =}
--  if supportInfo is nil then considered 'support ALL interfaces'
function generic_all_ports_show_func(specificPerPortFunc,params,supportInfo)
    local from_caller_bypass_final_return = params.bypass_final_return
    local command_data
    if not params.command_data then
        command_data = Command_Data()
    else
        command_data = params.command_data
    end

    -- Common variables initialization
    command_data:clearResultArray()

    params.command_data = command_data
    params.bypass_final_return = true
    params.use_port_info = true

    -- Command specific variables initialization
    local header_string = params.header_string
    local footer_string = params.footer_string

    -- System specific data initialization.
    command_data:enablePausedPrinting()
    command_data:setEmergencyPrintingHeaderAndFooter(header_string,
                                                     footer_string)

    -- let the 'per port' iterator be called
    generic_port_range_func(specificPerPortFunc,params,supportInfo)

    -- print the table.
    internal_showTblResult(params)

    if not from_caller_bypass_final_return then
        return command_data:getCommandExecutionResults()
    else
        -- the caller still have to do the 'final return'
        return true
    end

end
--##################################
--##################################

-- function to support 'recursive show' that iteration on all ports that we got from parameter "#all_interfaces"
-- special parameters that looked at:
--          params.command_data                --> should be always used
--          params.bypass_final_return         --> allow to skip 'getCommandExecutionResults' of 'end of command'
--          params.header_string               --> the 'header' of the table
--          params.footer_string               --> the 'footer' of the table
--          params.disabledPrinting            --> indication that the table shouldnt be printed'
--          params.disablePausedPrinting       --> indication that we NOT want 'Type <CR> to continue, Q<CR> to stop:'
-- allow to define which interfaces are supported by the function :
--  supportInfo = {eport = , ethernet =}
--  if supportInfo is nil then considered 'support ALL interfaces'
function generic_all_ports_recursive_show_func (specificPerPortFunc,params,supportInfo)
    local from_caller_bypass_final_return = params.bypass_final_return
    local command_data
    command_data = params.command_data

    -- Common variables initialization
    command_data:clearResultArray()

    params.bypass_final_return = true
    params.use_port_info = true

    -- Command specific variables initialization
    local header_string = params.header_string
    local footer_string = params.footer_string

    command_data:setEmergencyPrintingHeaderAndFooter(header_string,
                                                     footer_string)

    -- let the 'per port' iterator be called
    generic_port_range_func(specificPerPortFunc,params,supportInfo)

    -- print the table.
    if  not params.disablePrinting then
        internal_showTblResult(params)
        -- re-enable paused printing
        command_data:enablePausedPrinting()
    end

    if not from_caller_bypass_final_return then
        return command_data:getCommandExecutionResults()
    else
        -- the caller still have to do the 'final return'
        return true
    end

end

--##################################
--##################################
CLI_addHelp("exec", "cls", "clear screen")
CLI_addCommand("exec", "cls",
    { func = function(params)
        luaConsoleClearScreen()
        end,
      help = "clear screen"
    });
--##################################
--##################################
threads = {}    -- list of all live threads
local function new_instance_shell_cmd_run(command)
    print("Start: new_instance_shell_cmd_run:" .. command)
    local taskId = luaTaskCreate("xTask", "common/generic/newInstanceTask.lua", {cmd = command})
    if type(taskId) ~= "number" then
        print("Task ID:" .. tostring(taskId))
        return false
    end
    print("Task ID:" .. tostring(taskId))
    -- insert it in the list
    local command_0 = splitline(command)[1]
    print("Command to insert into table:" .. command_0)
    table.insert(threads, {command = command_0, taskId = taskId})
    return true, taskId
end

function new_instance_shell_cmd_wait(taskId, wait)
    local rc, iteration

    for iteration = 0, wait*10 do
        -- check instance priority
        rc = cpssGenWrapper("osGetTaskPrior", {
            {"IN","GT_U32", "tid", taskId},
            {"OUT","GT_U32","prio"}
        })
        if rc ~= 0 then
            -- the task finished
            print("The task finished: " .. tostring(taskId) .. " running time: " .. tostring(iteration/10))
            return true
        end
        -- the task still running
        delay(100)
    end

    print("The task still running: " .. tostring(taskId))

    -- the task still running
    return false
end

function new_instance_shell_cmd_check(command)
    local rc
    print("Start: new_instance_shell_cmd_check: " .. command)
    instance = 1
    while instance <= #threads do
        if threads[instance].command == command then
            -- wait for task finish
            print("Wait for task finish: " .. to_string(threads[instance].taskId))
            rc = new_instance_shell_cmd_wait(threads[instance].taskId, 300)
            if rc == true then
                print("Task: " .. threads[instance].command .." finished successfully")
                table.remove(threads, instance)
                instance = instance - 1
            end
        end
        instance = instance + 1
    end
end

function new_instance_shell_cmd_kill(taskId)
    local rc
    print("Start: new_instance_shell_cmd_kill: " .. taskId .. " threads# " .. to_string(#threads))
    for instance = 1,#threads do
        print("Task: " .. to_string(threads[instance].taskId) .." command: " .. threads[instance].command)

        if threads[instance].taskId == taskId then
            -- Task found
            print("Start killing task", to_string(threads[instance].taskId))
            -- check task life status
            rc = new_instance_shell_cmd_wait(threads[instance].taskId, 0)
            if rc == false then
                -- the task is active - kill task
                rc = cpssGenWrapper("osTaskDelete", {
                    {"IN","GT_U32", "tid", threads[instance].taskId}
                })
                if rc == 0 then
                    print("Task: " .. to_string(threads[instance].taskId) .." killed")
                else
                    print("Error killing task: " .. to_string(threads[instance].taskId) .." rc = ".. to_string(rc))
                end
            else
                print("Task: " .. to_string(threads[instance].taskId) .." not active - nothing to do")
            end
            -- remove task from database
            table.remove(threads, instance)
            -- exit loop
            return true
        end
    end

    -- task not found
    return false
end



-- function aggregate the parameters for calling any 'C' function
local function func_shell_execute(params)
    -- strip optional "do " at the begin
    local cmdline = string.gsub(params["_cmdline"],"^%s*do%s+","",1)
    local full_string = string.gsub(cmdline,"^%s*shell%-execute%s+","",1)
    if full_string == cmdline then
        full_string = string.gsub(cmdline,"^%s*shell%-execute%-in%-parallel%s+","",1)
    end

    --print (to_string(params))
    -- call to execute the command + it's parameters
    if not params.runInParallel then
        local rc = luaShellExecute(full_string)
        if rc == LGT_OK then
            return true
        end
        if rc == LGT_FAIL then
            print("rc==GT_FAIL, possible reason: function doesn't exists")
            return false
        end
        if returnCodes[rc] ~= nil then
            print("rc="..returnCodes[rc])
        end
        return false
    else
        new_instance_shell_cmd_run(full_string)
    end
end

local help_shell_execute = "allow to execute any 'C' function with it's parameters"
CLI_addHelp("exec", "shell-execute", help_shell_execute)
CLI_addCommand("exec", "shell-execute",
    { func = func_shell_execute,
      help = help_shell_execute,
      first_optional = "param1", -- param1 and all other after it are 'optional'
      params = {
        { type="values",
           { format = "%s", name = "funcName", help = "function name" },
           { format = "%s", name = "param1", help = "parameter [1]" },
           { format = "%s", name = "param2", help = "parameter [2]" },
           { format = "%s", name = "param3", help = "parameter [3]" },
           { format = "%s", name = "param4", help = "parameter [4]" },
           { format = "%s", name = "param5", help = "parameter [5]" },
           { format = "%s", name = "param6", help = "parameter [6]" },
           { format = "%s", name = "param7", help = "parameter [7]" }
        },
      }
    });

--##################################
--##################################
local help_shell_execute_in_parallel = "(in parallel - non blocking) allow to execute any 'C' function with it's parameters"
CLI_addHelp("exec", "shell-execute-in-parallel", help_shell_execute_in_parallel)
CLI_addCommand("exec", "shell-execute-in-parallel",
    { func = function(params)
            params.runInParallel = true
            return func_shell_execute(params)
        end,
      help = help_shell_execute_in_parallel,
      first_optional = "param1", -- param1 and all other after it are 'optional'
      params = {
        { type="values",
           { format = "%s", name = "funcName", help = "function name" },
           { format = "%s", name = "param1", help = "parameter [1]" },
           { format = "%s", name = "param2", help = "parameter [2]" },
           { format = "%s", name = "param3", help = "parameter [3]" },
           { format = "%s", name = "param4", help = "parameter [4]" },
           { format = "%s", name = "param5", help = "parameter [5]" },
           { format = "%s", name = "param6", help = "parameter [6]" },
           { format = "%s", name = "param7", help = "parameter [7]" }
        },
      }
    });

-- does the same as string.gsub, but treating "magic" symbols as regular
-- and supports only plain string as replacement
-- patrameters:
-- str      - source string
-- pattern  - substring to be replaced
-- repl_str - string to replcace the pattern
-- max_reps - maximal amount of replacements, (optional, if nil replaced all found matches)
-- returns the result string and number of replacements
function string_replace_global(str, pattern, repl_str, max_reps)
    local s = str;
    local r = "";
    local n = 0;
    local pos0, pos1, s1, s2, len;
    if not max_reps then
        max_reps = 0x1000000; -- very big positive number
    end
    while true do
        pos0, pos1 = string.find(s, pattern, 1, true --[[plain--]]);
        if not pos0 then
            return (r .. s), n;
        end
        len = string.len(s);
        if pos0 == 1 then
            s1 = "";
        else
            s1 = string.sub(s, 1, (pos0 - 1));
        end
        if pos1 >= len then
            s2 = "";
        else
            s2 = string.sub(s, (pos1 + 1), len);
        end
        r = r .. s1 .. repl_str;
        s = s2;
        n = n + 1;
        if n >= max_reps then
            return (r .. s), n;
        end
    end
end

-- this function returns the copy of string str with replacement of all
-- variables in the text by given values
-- parameters:
-- str          - the source string containing variable (vars) names in frames of
--                prefix and postfix, for example if name = VAR1 it can be framed as
--                $(VAR1) or %VAR1% or &VAR1 and all the framed vaiable name should
--                be replaced by variable value
-- vars_values  - table {name1 = value1, ...} of variables names and values
-- vars_prefix  - variable name frame prefix as "$(" or "%" or "&"
-- vars_postfix - variable name frame postfix as ")" or "%" or ""
-- vars_prefix and vars_postfix are optional, default is ""
function string_replace_vars_list(str, vars_values, vars_prefix, vars_postfix)
    local k,v,s;
    if not vars_prefix then
       vars_prefix  = "";
    end
    if not vars_postfix then
       vars_postfix  = "";
    end
    s = str;
    for k,v in pairs(vars_values) do
        s = string_replace_global(
            s, (vars_prefix .. k .. vars_postfix), v);
    end
    return s;
end

--[[
lua
print(string_replace_global("w1, w2, w3, w2, w1","w1","rep1"));
print(string_replace_global("w1, w2, w3, w2, w1","w2","rep2"));
_s = "word1 <name1> word2 <name2> word3 <name3>" ..
    "\n" ..
    "word4 <name4> word5 <name5> word6 <name6>";
_v = {
    name1 = "val1",
    name2 = "val2",
    name3 = "val3",
    name4 = "val4",
    name5 = "val5",
    name6 = "val6",
}
_r = string_replace_vars_list(_s, _v, "<", ">");
print(_r);
_s = nil;
_d = nil;
_r = nil;
.
--]]
