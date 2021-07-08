--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* num_range.lua
--*
--* DESCRIPTION:
--*       num-range type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

--returns pair of splitted number and the rest of the string
--supported decimal and hexadecimal numbers
local function split_number_from_string(str)
    local len = string.len(str);
    local i,start,stop;

    if (len > 2) and (string.sub(str,1,2) == "0x") then
        -- hexadecimal number - special case because
        -- "0" and "0x1" are numbers, but "0x" is not number
        start = 3;
        stop  = 1; -- anycase we have 0 at the beginning
    else
        start = 1;
        stop  = 0;
    end

    for i = start,len do
        if tonumber(string.sub(str,1,i)) then
            stop = i;
        else
            break;
        end
    end

    if stop == 0 then
        return nil, str;
    elseif stop >= len then
        return tonumber(str), nil;
    else
        return tonumber(string.sub(str,1,stop)), string.sub(str,(stop + 1),-1);
    end
end

--returns pair of splitted not-number prefix and the rest of the string
--supported decimal and hexadecimal numbers
local function split_not_number_from_string(str)
    local len = string.len(str);
    local i,stop;

    stop = 0;
    for i = 1,len do
        if tonumber(string.sub(str,i,i)) then
            stop = i;
            break;
        end
    end

    if stop == 0 then
        return str,nil;
    elseif stop == 1 then
        return nil,str;
    else
        return string.sub(str,1,(stop - 1)), string.sub(str,stop,-1);
    end
end

--convert string to list numbers and separating substrings
--supported decimal and hexadecimal numbers
local function split_string_to_separated_numbers(str)
    local res = {};
    local s = str;
    local p,n;

    n = 0;
    while true do
        p,s = split_number_from_string(s);
        n = n + 1;
        res[n] = p;
        if not s then
            break;
        end
        p,s = split_not_number_from_string(s);
        n = n + 1;
        res[n] = p;
        if not s then
            break;
        end
    end
    return res;
end

--
-- Types defined:
--
-- number_range   - The range of numbers
--
-- ************************************************************************
---
--  check_param_number_range
--        @description  Check parameter that it is in the correct form
--
--        @param param          - Parameter string
--
--        @return       boolean check status
--        @return       number:paramList or string:error_string if failed
--        @return       The format of return table:
--                          {{low1,high1}, val2, ...}
--                      sample: 1,3,4-6,10-13,15 => {1,3,{4,6},{10,13},15}
--
function check_param_number_range(param)
    local list = split_string_to_separated_numbers(param);
    local in_index, out_index, res;
    in_index = 1;
    out_index = 1;
    res = {};
    while list[in_index] do
        if type(list[in_index]) ~= "number" then
            return false, nil;
        end
        if list[in_index + 1] == "," or list[in_index + 1] == nil then
            res[out_index] = list[in_index];
            in_index  = in_index + 2;
            out_index = out_index + 1;
        elseif list[in_index + 1] == "-" then
            if type(list[in_index + 2]) ~= "number" then
                return false, nil;
            end
            if list[in_index] >= list[in_index + 2] then
                return false, nil;
            end
            if list[in_index + 3] ~= "," and list[in_index + 3] ~= nil then
                return false, nil;
            end
            res[out_index] = {list[in_index], list[in_index + 2]};
            in_index  = in_index + 4;
            out_index = out_index + 1;
        else
            -- error state
            return false, nil;
        end
    end
    --Return true and the list
    return true, res
end

CLI_type_dict["number_range"] = {
    checker = check_param_number_range,
    help = "The range of numbers. Example: 2-4,6 means 2,3,4,6"
}

-- returns ireartor for %number_range content
-- The format %number_range table:
-- {{low1,high1}, val2, ...}
-- sample: 1,3,4-6,10-13,15 => {1,3,{4,6},{10,13},15}
function number_range_iterator(number_range)
    local iter = 0;
    if type(number_range[1]) == "table" then
        iter = {1, -1};
    elseif type(number_range[1]) == "number" then
        iter = 0;
    else
        return nil;
    end
    local function iter_func(range, iterator)
        local iter, val;
        --just attempt to increment
        if type(iterator) == "number" then
            iter = iterator + 1;
        elseif type(iterator) == "table" then
            iter = {iterator[1], (iterator[2] + 1)};
            val = range[iter[1]][1] + iter[2];
            if val <= range[iter[1]][2] then
                --next in subrange {low, high}
                return iter, val;
            end
            --out of subrange {low, high}
            iter = iterator[1] + 1;
        else
            --wrong data
            return nil;
        end
        if type(iter) == "number" then
            if not range[iter] then
                -- end reached
                return nil;
            end
            if type(range[iter]) == "table" then
                -- begin table
                iter = {iter, 0};
                val = range[iter[1]][1];
            else
                val = range[iter];
            end
        end
        return iter, val;
    end
    return iter_func, number_range, iter;
end

