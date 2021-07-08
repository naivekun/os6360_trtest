--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* mll_pointer_name.lua
--*
--* DESCRIPTION:
--*       mll pointer type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  CLI_check_param_mll_pointer_name
--        @description  This function validates that the string s is a 
--                      valid mll pointer name, it accepts lower/upper case 
--                      letters, underscore (_) it should contain less than
--                      31 characters 
--
--        @param s              - The string to validate
--
--        @return       true and entry name string if successful
--                      false and an error description if failed 
--
local function CLI_check_param_mll_pointer_name(s)
    local status
	local i, c
    
    status = true
    
    if true == status then
        if (0 >= string.len(s)) or (31 < string.len(s)) then
            status = false
        end
    end
    
    if true == status then
        for i = 1, string.len(s) do
            c = string.sub(s, i, i)
            if not ((c >= 'a' and c <= 'z') or	
                    (c >= 'A' and c <= 'Z') or (c=="_")) then
                status = false
                break
            end
        end    
    end
    
    if true == status then
        return true, s
    else
        return false, "The mll pointer name is invalid"
    end
end

-- ************************************************************************
---
--  CLI_complete_param_mll_pointer_name
--        @description  complete mll pointer name
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       complete mll pointer name and help lists
--
local function CLI_complete_param_mll_pointer_name(param, name, desc)
    local values = 
        {{"entry_00",      "Default entry name"}}
    local compl = {}
    local help  = {}
    local index
    
    for index = 1, #values do
        if prefix_match(param, tostring(values[index][1])) then
            table.insert(compl, tostring(values[index][1]))
            help[#compl] = values[index][2]
        end
    end
    
    return compl, help
end

CLI_type_dict["mll-pointer-name"] = {
    checker  = CLI_check_param_mll_pointer_name,
    complete = CLI_complete_param_mll_pointer_name,
    help     = "Mll pointer name"
} 