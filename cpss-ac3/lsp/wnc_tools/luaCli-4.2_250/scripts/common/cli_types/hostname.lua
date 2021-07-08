--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* hostname.lua
--*
--* DESCRIPTION:
--*       hostname type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants

--
-- Types defined:
--
-- hostname  - Host name
--


-- ************************************************************************
---
--  CLI_check_param_hostname
--        @description  This function validates that the string s is a 
--                      valid host name, it accepts lower/upper case 
--                      letters, numbers, underscore (_) it should contain 
--                      less than 64 characters 
--
--        @param s              - The string to validate
--
--        @return       false and an error description if failed validation
--                      true if successful
--
function CLI_check_param_hostname(s)
    local status
	local i, c
    
    status = true
    
    if true == status then
        if (0 >= string.len(s)) or (63 < string.len(s)) then
            status = false
        end
    end
    
    if true == status then
        c = string.sub(s, 1, 1)
        if not ((c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z') or 
                (c=="_")) then
            status = false  
        end
    end
    
    if true == status then
        for i = 2, string.len(s) do
            c = string.sub(s, i, i)
            if not ((c >= '0' and c <= '9') or (c >= 'a' and c <= 'z') or	
                    (c >= 'A' and c <= 'Z') or (c=="_")) then
                status = false
                break
            end
        end    
    end
    
    if true == status then
        return true, s
    else
        return false, "The host name is invalid"
    end
end

CLI_type_dict["hostname"] = {
    checker = CLI_check_param_hostname,
    help = "This system's network name"
}
