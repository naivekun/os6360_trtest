--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* set_point_bytes.lua
--*
--* DESCRIPTION:
--*       set point bytes definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1$
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  check_param_set_point_bytes
--        @description  ckecks set point bytes value 
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       true and second on success, otherwise false and 
--                      error message
--
local function check_param_set_point_bytes(param, name, desc)  
    param = tonumber(param)
    
    if nil == param then
        return false, name .. " not a number"
    end
    
    if (512 > param) or (4294967295 < param) then
        return false, name .. " out of range"    
    end
    
    return true, param
end


-------------------------------------------------------
-- type registration: second
-------------------------------------------------------
CLI_type_dict["set_point_bytes"] = 
{
    checker  = check_param_set_point_bytes,
    min      = 512,
    max      = 4294967295,
    complete = CLI_complete_param_number,
    help     = "set point bytes specification"
} 
