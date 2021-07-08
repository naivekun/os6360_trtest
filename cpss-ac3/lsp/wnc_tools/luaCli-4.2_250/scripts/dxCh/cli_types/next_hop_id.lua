--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* next_hop_id.lua
--*
--* DESCRIPTION:
--*       next hop id type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssDxChMaximumRouterNextHopEntryIndex")

--constants

-- ************************************************************************
---
--  check_next_hop_id
--        @description  checking of next hop id
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter rang
--
--        @return       true and next hop id on success, otherwise
--                      false and error message
--
local function check_next_hop_id(param, name, desc)
    local devlist
    local index, devId
    local result, maximum_entry_index

    param = tonumber(param)
    
    if param == nil then
        return false, name .. " not a number"
    end
    
    -- Next hop id range checking.
    devlist = wrLogWrapper("wrlDevList")
    for index, devId in pairs(devlist) do
        result, maximum_entry_index = 
            wrLogWrapper("wrlCpssDxChMaximumRouterNextHopEntryIndex", "(devId)", devId)
        if 0 ~= result  then
            return false, "Error at Maximum Base Route Entry index getting"
        end
        
        if param < maximum_entry_index then
            return true, param
        end
    end
    
    if 1 == #devlist    then
        return false, "Device " .. devlist[1] .. " does not support " ..
                      "next-hop entries"
    else
        return false, "Devices " .. tbltostr(devlist) .. " do not support " ..
                      "next-hop entries"    
    end
end


-- ************************************************************************
---
--  complete_next_hop_id
--        @description  autocompleting of next hop id
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter rang
--
--        @return       autocompleting and help array 
--
local function complete_next_hop_id(param, name, desc)
    local compl, help = CLI_complete_param_number(param, name, desc)

    return compl, help
end


--------------------------------------------
-- type registration: next_hop_id
--------------------------------------------
CLI_type_dict["next-hop-id"] = {
    checker     = check_next_hop_id,
    complete    = complete_next_hop_id,
    min         = 0,   
    help        = "Next Hop id"
}

