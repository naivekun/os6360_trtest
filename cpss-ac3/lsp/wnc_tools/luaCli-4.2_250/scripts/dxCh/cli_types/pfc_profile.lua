--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pfc_profile.lua
--*
--* DESCRIPTION:
--*       PFC profile type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1$
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  check_param_pfc_profile
--        @description  ckecks PFC profile type definition 
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       true and pfc profile on success, otherwise false 
--                      and error message
--
local function check_param_pfc_profile(param, name, desc)  
    param = tonumber(param)
    
    if nil == param then
        return false, name .. " not a number"
    end
  
    if (0 > param) or (7 < param) then
        return false, name .. " out of range"    
    end
    
    return true, param
end


-------------------------------------------------------
-- type registration: pfc_profile
-------------------------------------------------------
CLI_type_dict["pfc_profile"] = 
{
    checker  = check_param_pfc_profile,
    min      = 0,
    max      = 7,
    complete = CLI_complete_param_number,
    help     = "set PFC profile"
}