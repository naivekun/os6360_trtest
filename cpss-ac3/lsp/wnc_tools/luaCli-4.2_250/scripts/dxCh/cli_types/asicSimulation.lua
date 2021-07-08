--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* asicSimulation.lua
--*
--* DESCRIPTION:
--*       ASIC Simulation types definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

--
-- Types defined:
--
-- slan_name    - SLAN name
--
do


-- ************************************************************************
---
--  check_param_slan_name
--        @description  Check that SLAN name length is less or equal to 8 
--                      symbols
--
--        @param param          - Parameter string
--        @param data           - Not used
--
--        @return    boolean check status
--        @return    true:param or false:error_string if failed
--
local function check_param_slan_name(param, data)

	if (string.len(param) > 8) then
		return false, "Name too long"
	end

	return true, param
end


-- ************************************************************************
---
--  complete_param_slan_name
--        @description  complete SLAN name  
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       complete and help lists
--
local function complete_param_slan_name(param, name, desc)
    local values = 
        {{"slan00",   "Default SLAN name for port 0"}}
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

CLI_type_dict["slan_name"] = {
    checker     = check_param_slan_name,
    complete    = complete_param_slan_name,
    help        = "SLAN name, up to 8 symbols string e.g. SLAN_236"
}

end
