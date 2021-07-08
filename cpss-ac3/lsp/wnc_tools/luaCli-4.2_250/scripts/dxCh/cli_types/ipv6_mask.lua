--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ipv6_mask.lua
--*
--* DESCRIPTION:
--*       ip v6 mask type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants

-- ************************************************************************
---
--  check_param_ipv6_mask
--        @description  checks ip v6 mask parameter
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return    true   and prefix length,  if no errors occur
--                   false  and error message,  if error occures
--
local function check_param_ipv6_mask(param, name, desc)
	local ret
	local s, e, i, ip
    local result, units_count

    s, e, ret = string.find(param, "/(%d+)")
      
    if (1 ~= s) or (string.len(param) ~= e)     then
        return false, "Wrong mask length form"    
    end 
        
    ret = tonumber(ret)
        
    if (0 == ret) or (128 < ret)                 then
        return false, "Mask length is out of bounds"
    end
    
	return true, ret
end


-- ************************************************************************
---
--  complete_param_ipv6_mask
--        @description  complete ip v6 mask 
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       complete and help lists
--
local function complete_param_ipv6_mask(param, name, desc)
    local values = 
        {{"/128",   "128-bit mask"}}
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


-- ************************************************************************
---
--  check_param_default_ipv6_mask
--        @description  check default ip v6 address mask
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return    true   and prefix length,  if no errors occur
--                   false  and error message,  if error occures
--
local function check_param_default_ipv6_mask(param, name, desc)
	local stat, ret

	stat, ret = check_param_ipv6_mask(param, data)
    
    if true == stat     then
        if (8 <= ret) and (ret <= 32)    then
            return true, ret
        else
            return false, "Mask prefix length is out of range"
        end
    else
        return stat, ret
    end
end


-- ************************************************************************
---
--  complete_param_default_ipv6_mask
--        @description  complete default ip v6 mask 
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       complete and help lists
--
local function complete_param_default_ipv6_mask(param, name, desc)
    local values = 
        {{"/30",   "30-bit mask"}}
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


-------------------------------------------------------
-- type registration: ipv6-mask
-------------------------------------------------------
CLI_type_dict["ipv6-mask"] = 
{
    checker     = check_param_ipv6_mask,
    complete    = complete_param_ipv6_mask,
    help        = "The ipv6 mask"
}


-------------------------------------------------------
-- type registration: default_ipv6-mask
-------------------------------------------------------
CLI_type_dict["default_ipv6-mask"] = 
{
    checker     = check_param_default_ipv6_mask,
    complete    = complete_param_default_ipv6_mask,
    help        = "The default ipv6 mask"
}


-------------------------------------------------------
-- type registration: ipv4_ipv6-mask
-------------------------------------------------------
CLI_type_dict["ipv4_ipv6-mask"] = 
{
    checker  = CLI_check_param_compound,
    complete = CLI_complete_param_compound,
    compound = { "ipv4-mask", "ipv6-mask" }  
}

-------------------------------------------------------
-- type registration: ipv4_ipv6_gw-mask
-------------------------------------------------------
CLI_type_dict["default_ipv4_ipv6-mask"] = 
{
    checker  = CLI_check_param_compound,
    complete = CLI_complete_param_compound,
    compound = { "default_ipv4-mask", "default_ipv6-mask" }  
}
