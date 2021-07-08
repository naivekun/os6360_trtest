--*******************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
--*******************************************************************************
--* px_types_dce.lua
--*
--* DESCRIPTION:
--*       The file defines types needed for the DCE commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--*******************************************************************************

--*******************************************************************************
--  checkRange
--
--  @description Return true, if minVal >= number >= maxVal. Otherwise false
--
--*******************************************************************************
local function checkRange(number, minVal, maxVal)
    if ((tonumber(number) >= minVal) and (tonumber(number) <= maxVal)) then
        return true
    else
        return false
    end
end


--*******************************************************************************
--  addNumberToSortedListIfNotExist
--
--  @description Add new value to sorted list if current list does not contain
--               this value
--
--*******************************************************************************
local function addNumberToSortedListIfNotExist(list, number)
    local i, j, listSize, insertPosition
    local isNumberAlreadyExist = false

    listSize = #list
    insertPosition = listSize + 1

    for i = 1, listSize do
        if (list[i] == number) then
            isNumberAlreadyExist = true
            break
        elseif (list[i] > number) then
            insertPosition = i
            break
        end
    end

    if (not isNumberAlreadyExist) then
        for j = listSize, insertPosition, (-1) do
            list[j + 1] = list[j]
        end

        list[insertPosition] = number
    end

    return list
end


--*******************************************************************************
--  getList
--
--  @description Convert string like "0,2,3-5,6" to list of numbers
--
--  @param str      - string with list of numbers
--  @param minValue - min. applicable number
--  @param maxValue - max. applicable number
--
--  @return ret - convertation status: true on success; otherwise false
--          val - if (ret == true):  sorted list with numbers
--                if (ret == false): error message
--
--*******************************************************************************
local function getList(str, minValue, maxValue)
    local list = {}
    local i, j, c, strLen
    local isPrevCharWasDigit = false
    local isRangeStarted = false
    local rangeStart, rangeDirection
    local number, intNumber


    -- if nil/""/"all" value
    if ((str == nil) or (str == "") or (string.sub("all", 1, string.len(str)) == str)) then

        for i = minValue, maxValue do
            list[#list + 1] = i
        end

        return true, list

    -- if "none" value
    elseif (string.sub("none", 1, string.len(str)) == str) then

        return true, list

    end


    -- add breaker to the end of line
    str = str .. ","

    strLen = string.len(str)
    number = ""
    rangeStart = ""

    for i = 1, strLen do
        c = string.sub(str, i, i)

        -- if current char is digit
        if ((c >= '0') and (c <= '9')) then

            number = number .. c
            isPrevCharWasDigit = true

        -- if range delimiter
        elseif ((c == '-') and (isPrevCharWasDigit) and (not isRangeStarted)) then

            if (not checkRange(number, minValue, maxValue)) then
                return false, "Number is out of range " .. minValue ..
                              ".." .. maxValue .. ": " .. number
            end

            isPrevCharWasDigit = false
            isRangeStarted = true

            rangeStart = tonumber(number)
            number = ""

        -- if breaker
        elseif ((c == ',') and (isPrevCharWasDigit)) then

            if (not checkRange(number, minValue, maxValue)) then
                return false, "Number is out of range " .. minValue ..
                              ".." .. maxValue .. ": " .. number
            end

            isPrevCharWasDigit = false
            intNumber = tonumber(number)
            number = ""

            if (isRangeStarted) then
                isRangeStarted = false

                -- if second number less than first
                if (rangeStart > intNumber) then
                    rangeDirection = -1
                else
                    rangeDirection = 1
                end

                -- add all numbers on range
                for j = rangeStart, intNumber, rangeDirection do
                    list = addNumberToSortedListIfNotExist(list, j)
                end
            else
                list = addNumberToSortedListIfNotExist(list, intNumber)
            end

        -- unexpected value
        else

            return false, "Wrong input format: " .. str

        end
    end

    return true, list
end


--*******************************************************************************
--  CLI_check_dce_qcn_list
--
--  @description Checker for DCE QCN list parameters
--
--  @param param - current parameter value
--
--  @return ret - check status: true on success; otherwise false
--          val - if (ret == false): error message
--                if (ret == true):  list with values
--
--*******************************************************************************
local function CLI_check_dce_qcn_list(param, name, desc)
    return getList(param, desc.min, desc.max)
end


--*******************************************************************************
--  CLI_check_dce_qcn_bitmap
--
--  @description Checker for DCE QCN bitmap parameters
--
--  @param param - current parameter value
--
--  @return ret - check status: true on success; otherwise false
--          val - if (ret == false): error message
--                if (ret == true):  parameter value
--
--*******************************************************************************
local function CLI_check_dce_qcn_bitmap(param, name, desc)
    local value

    if (string.sub(param, 1, 2) == "0b") then       -- if binary
        value = tonumber(string.sub(param, 3), 2)
    elseif(string.sub(param, 1, 2) == "0x") then    -- if heximal
        value = tonumber(string.sub(param, 3), 16)
    else                                            -- hope that it will be decimal
        value = tonumber(param)
    end


    if (value == nil) then
        return false, "Wrong format of input parameter: \"" .. param .. "\""
    elseif ((desc.min ~= nil) and (value < desc.min)) then
        return false, "Parameter is less than " .. desc.min
    elseif ((desc.max ~= nil) and (value > desc.max)) then
        return false, "Parameter is greater that " .. desc.max
    else
        return true, value
    end
end


--*******************************************************************************
--  CLI_complete_dce_qcn_bitmap
--
--  @description Completer for DCE QCN bitmap parameters
--
--  @param param - current parameter value
--
--  @return ret - check status: true on success; otherwise false
--          val - if (ret == false): error message
--                if (ret == true):  parameter value
--
--*******************************************************************************
local function CLI_complete_dce_qcn_bitmap(param, name, desc)
    if ((desc.min == nil) and (desc.max == nil)) then
        return {}, {}
    end

    local help = "<"

    if ((nil ~= desc.min) and (nil ~= desc.max)) then
        help = help .. tostring(desc.min) .. "-" .. tostring(desc.max)
    elseif (nil ~= desc.min) then
        help = help .. tostring(desc.min) .. " and above"
    elseif (nil ~= desc.max) then
        help = help .. tostring(desc.max) .. " and below"
    end

    help = help .. ">"

    if (type(desc.help) == "string") then
        help = help .. "   " .. desc.help .. " (applicable input format: binary, hex, decimal)"
    end

    return {}, { def = help }
end


-------------------------------------------------------
-- type registration: dce_cut_through_packet_length
-------------------------------------------------------
CLI_type_dict["dce_cut_through_packet_length"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min = 1,
    max = 16257,
    help = "Minimal packet size in bytes for Cut-Through"
}

-------------------------------------------------------
-- type registration: dce_cut_through_buffer_limits
-------------------------------------------------------
CLI_type_dict["dce_cut_through_buffer_limits"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min = 0,
    max = 0xFFFF,
    help = "Max amount of buffers for Cut-Through traffic"
}

-------------------------------------------------------
-- type registration: dce_pfc_profile
-------------------------------------------------------
CLI_type_dict["dce_pfc_profile"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min = 0,
    max = 7,
    help = "PFC profile index"
}

-------------------------------------------------------
-- type registration: dce_pfc_threshold
-------------------------------------------------------
CLI_type_dict["dce_pfc_threshold"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min = 0,
    max = 0x1FFFFF,
    help = "PFC Threshold"
}

-------------------------------------------------------
-- type registration: dce_qcn_port_list
-------------------------------------------------------
CLI_type_dict["dce_qcn_port_list"] = {
    checker = CLI_check_dce_qcn_list,
    min = 0,
    max = 16,
    help = "List of ports.\n" ..
           "Examples:\n" ..
           "   0,2-4,6 - means ports 0,2,3,4,6\n" ..
           "   \"all\"   - for all ports (default)\n" ..
           "   \"none\"  - information related to ports will not been shown"
}

-------------------------------------------------------
-- type registration: dce_qcn_tc_list
-------------------------------------------------------
CLI_type_dict["dce_qcn_tc_list"] = {
    checker = CLI_check_dce_qcn_list,
    min = 0,
    max = 7,
    help = "List of Traffic Classes.\n" ..
           "Examples:\n" ..
           "   0,2-4,6 - means Traffic Classes 0,2,3,4,6\n" ..
           "   \"all\"   - for all Traffic Classess (default)\n" ..
           "   \"none\"  - information related to TC will not been shown"
}

-------------------------------------------------------
-- type registration: dce_qcn_profile_list
-------------------------------------------------------
CLI_type_dict["dce_qcn_profile_list"] = {
    checker = CLI_check_dce_qcn_list,
    min = 0,
    max = 15,
    help = "List of CN Profiles.\n" ..
           "Examples:\n" ..
           "   0,2-4,6 - means profiles 0,2,3,4,6\n" ..
           "   \"all\"   - for all profiles (default)\n" ..
           "   \"none\"  - information related to CN Profiles will not been shown"
}

-------------------------------------------------------
-- type registration: dce_qcn_profile_list
-------------------------------------------------------
CLI_type_dict["dce_qcn_feedback_lsb"] = {
    checker = CLI_check_dce_qcn_bitmap,
    complete = CLI_complete_dce_qcn_bitmap,
    min = 0,
    max = 31,
    help = "CN Feedback Lsb bits that used for qFb calculation"
}

-------------------------------------------------------
-- type registration: dce_qcn_feedback_min_max
-------------------------------------------------------
CLI_type_dict["dce_qcn_feedback_min_max"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min = 0,
    max = 0xFFFFFF,
    help = "Minimal or maximal value of CN Feedback"
}

-------------------------------------------------------
-- type registration: dce_qcn_feedback_exp_weight
-------------------------------------------------------
CLI_type_dict["dce_qcn_feedback_exp_weight"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min = -8,
    max = 7,
    help = "Exponent weight for CN Feedback calculation"
}

-------------------------------------------------------
-- type registration: dce_qcn_cp_sample_int_table_entry_index
-------------------------------------------------------
CLI_type_dict["dce_qcn_cp_sample_int_table_entry_index"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min = 0,
    max = 7,
    help = "CN Sample Interval Table entry index"
}

-------------------------------------------------------
-- type registration: dce_qcn_cp_sample_int_table_interval
-------------------------------------------------------
CLI_type_dict["dce_qcn_cp_sample_int_table_interval"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min = 0,
    max = 0xFFFF,
    help = "CN sampling interval in resolution of 16 bytes"
}

-------------------------------------------------------
-- type registration: dce_qcn_cp_sample_int_table_rand_bitmap
-------------------------------------------------------
CLI_type_dict["dce_qcn_cp_sample_int_table_rand_bitmap"] = {
    checker = CLI_check_dce_qcn_bitmap,
    complete = CLI_complete_dce_qcn_bitmap,
    min = 0,
    max = 0xFFFF,
    help = "Bitmap that indicates which sampling interval bits are randomize. " ..
           "0 - don't randomize; 1 - randomize"
}
