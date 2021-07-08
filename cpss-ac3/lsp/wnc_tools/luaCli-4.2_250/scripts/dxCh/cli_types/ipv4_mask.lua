--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ipv4_mask.lua
--*
--* DESCRIPTION:
--*       ip v4 mask type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants

-- ************************************************************************
---
--  check_param_ipv4_mask
--        @description  checks ip v4 mask parameter
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return    true   and prefix length,  if no errors occur
--                   false  and error message,  if error occures
--
local function check_param_ipv4_mask(param, name, desc)
	local ret
	local s, e, i, ip
    local result, units_count
    local ip_form

    local was_last_mask_unit = false    
    function check_mask_number(number)
        local units_count = 0
        
        if (true == was_last_mask_unit) and (0 < number) then

            return false
        end
        
        while 0 ~= number do
            if 0 ~= bit_and(number, 0x80)   then
                units_count     = units_count + 1
                number          = bit_shl(number, 1)
                number          = bit_and(number, 0xFF) 
            else
               break
            end
        end

        if 0 == number                          then
            if 8 ~= units_count                     then
                was_last_mask_unit = true
            end
        
            return true, units_count
        else 
            return false
        end
    end
       
	ip = {}

	s, e, ip[1], ip[2], ip[3], ip[4] = 
        string.find(param, "(%d?%d?%d?)%.(%d?%d?%d?)%.(%d?%d?%d?)%.(%d?%d?%d?)")

	if (1 ~= s) or (string.len(param) ~= e)     then
		ip_form = false
    else
        ip_form = true    
	end

    if false == ip_form                         then
        s, e, ret = string.find(param, "/(%d+)")
      
        if (1 ~= s) or (string.len(param) ~= e)     then
            return false, "Wrong mask length form"    
        end 
        
        ret = tonumber(ret)
        
        if (0 == ret) or (32 < ret)                 then
            return false, "Mask length is out of bounds"
        end
    else
        ret = 0
        ip[1] = tonumber(ip[1])
        ip[2] = tonumber(ip[2])
        ip[3] = tonumber(ip[3])
        ip[4] = tonumber(ip[4])
        
        for i = 1, 4 do
            if 255 < ip[i] then
                return false, "Octet in IP address cannot be above 255"
            end
            
            result, units_count = check_mask_number(ip[i])
            if true == result   then
                ret = ret + units_count
            else
                return false, "Octet " .. i .. " breaks sequence of units"
            end            
        end       
    end
    
	return true, ret
end


-- ************************************************************************
---
--  complete_param_ipv4_mask
--        @description  complete ip v4 mask 
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       complete and help lists
--
local function complete_param_ipv4_mask(param, name, desc)
    local values = 
        {{"255.255.255.255",   "32-bit mask"}}
    return complete_param_with_predefined_values(param,name,desc,values)
end


-- ************************************************************************
---
--  check_param_default_ipv4_mask
--        @description  check default ip v4 address mask
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return    true   and prefix length,  if no errors occur
--                   false  and error message,  if error occures
--
local function check_param_default_ipv4_mask(param, name, desc)
	local stat, ret

	stat, ret = check_param_ipv4_mask(param, data)
    
    if true == stat     then
        if (8 <= ret) and (ret <= 32)    then
            return true, ret
        else
            return false, "Mask has wrong number of bits"
        end
    else
        return stat, ret
    end
end


-------------------------------------------------------
-- type registration: ipv4-mask
-------------------------------------------------------
CLI_type_dict["ipv4-mask"] = 
{
    checker     = check_param_ipv4_mask,
    complete    = complete_param_ipv4_mask,
    help        = "The ipv4 mask"
}

-------------------------------------------------------
-- type registration: ipv4-mask
-------------------------------------------------------
CLI_type_dict["default_ipv4-mask"] = 
{
    checker     = check_param_default_ipv4_mask,
    complete    = complete_param_ipv4_mask,
    help        = "The default ipv4 mask"
}
