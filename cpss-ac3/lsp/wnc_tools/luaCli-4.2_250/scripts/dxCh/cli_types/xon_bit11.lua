--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* xon_bit11.lua
--*
--* DESCRIPTION:
--*       xon threshold type definition (max - BIT_11)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1$
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  check_param_xon_bit11
--        @description  ckecks xon threshold type definition (max - BIT_11)
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       true and xon on success, otherwise false and 
--                      error message
--
local function check_param_xon_bit11(param, name, desc)  
    param = tonumber(param)
    
    if nil == param then
        return false, name .. " not a number"
    end
  
    if (0 > param) or (BIT_11 - 1 < param) then
        return false, name .. " out of range"    
    end
    
    return true, param
end


-------------------------------------------------------
-- type registration: xon_bit11
-------------------------------------------------------
CLI_type_dict["xon_bit11"] = 
{
    checker  = check_param_xon_bit11,
    min      = 0,
    max      = BIT_11 - 1,
    complete = CLI_complete_param_number,
    help     = "set xon threshold"
} 