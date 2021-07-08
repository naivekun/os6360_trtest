--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* tpid_range.lua
--*
--* DESCRIPTION:
--*       port-range type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants

--
-- Types defined:
--
-- tpid_range   - The range
--
-- ************************************************************************
---
--  check_param_tpid_range
--        @description  Check parameter that it is in the correct form
--                      1,2-4,6
--
--        @param param          - Parameter string
--
--        @return       boolean check status
--        @return       number:paramList or string:error_string if failed
--        @return       The format of paramList { id, id, ... }
--
local function check_param_tpid_range(param)
    local str = param .. ","
    local s,i,start,stop
    local result = {}
    local alonErr = "Wrong interface format, required format is tpid.\nTo indicate tpid 1 use 1.\nTo indicate tpid 1-3,6 1-3,6"
    while str ~= "" do
        i = string.find(str,",",1,true)
        s = string.sub(str, 1, i-1)
        str = string.sub(str,i+1)
        -- s is "number" or "number-number"
        i = string.find(s,"-",1,true)
        if i == nil then
            start = tonumber(s)
            stop = start
        else
            start = tonumber(string.sub(s,1,i-1))
            stop = tonumber(string.sub(s,i+1))
        end
        if start == nil or stop == nil then
            return false, alonErr
        end
        if start < 0 or start > 7 or stop < 0 or stop > 7 then
            return false, alonErr
        end
        for i = start, stop do
            table.insert(result,i)
        end
    end
    return true,result
end


CLI_type_dict["tpid_range"] = {
    checker = check_param_tpid_range,
    help = "The range of ports. Example: 0/2-4,6 means device 0 ports 2,3,4,6 or \"all\" for all ports"
}

CLI_type_dict["tpid_index"] = {
    checker  = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min=0,
    max=7,
    help = "TPID table entry index"
}

CLI_type_dict["4_or_8_bytes_tag"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "4/8/6 bytes-tag",
    enum = {
        ["4-bytes-tag"] = { value="4-bytes-tag", help="4-bytes-tag" },
        ["8-bytes-tag"] = { value="8-bytes-tag", help="8-bytes-tag" },-- sip_5
        ["6-bytes-tag"] = { value="6-bytes-tag", help="6-bytes-tag" } -- sip_5_15
   }
};

CLI_type_dict["6_or_8_bytes_tag"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "8/6 bytes-tag",
    enum = {
        ["8-bytes-tag"] = { value="8-bytes-tag", help="8-bytes-tag" },-- sip_5
        ["6-bytes-tag"] = { value="6-bytes-tag", help="6-bytes-tag" } -- sip_5_15
   }
};
