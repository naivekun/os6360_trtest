--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* channel_group.lua
--*
--* DESCRIPTION:
--*       channel-group type definition
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
-- channel-group  - channel group number, valid range: 0-...
--
do
-- ************************************************************************
---
--  check_param_channel_group
--        @description  Check channel group parameter
--
--        @param param          - Parameter string
--
--        @return       (true, number) on success, or (false, error_string) 
--                      if failed
--
local function check_param_channel_group(param, name, desc)
    local ret
    local i, c
    local numberOfTrunksInDevice
    
    ret = 0
    for i = 1,string.len(param) do
        c = string.sub(param, i, i)
        if type(tonumber(c)) ~= "nil" then
            --valid digit
            ret = (ret * 10) + tonumber(c)
        else 
            --Something is wrong with the format
            return false, "bad format" 
        end
    end
    
    -- check that "ret" is valid
    if (type(ret) == "nil") then
        return false, "This is not a number"
    end
    
    -- get system capability parameters 
    values = system_capability_managment(params)
    
    if NumberOfEntriesIn_TRUNK_table ~= nil then
       numberOfTrunksInDevice = NumberOfEntriesIn_TRUNK_table
    else
       numberOfTrunksInDevice = 0
    end
    
    -- check the range of "ret"
    if ((ret < 0) or (ret > numberOfTrunksInDevice)) then
        return false, "The channel group number is out of range"
    end 
    
    return true, ret
end

CLI_type_dict["channel-group"] = {
    checker = check_param_channel_group,
    help = "Enter channel group number, valid range: 0-" .. tostring(NumberOfEntriesIn_TRUNK_table)
}
end
