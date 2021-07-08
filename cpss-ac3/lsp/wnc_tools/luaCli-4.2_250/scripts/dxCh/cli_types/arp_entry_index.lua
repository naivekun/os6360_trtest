--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* arp_entry_index.lua
--*
--* DESCRIPTION:
--*       arp entry index type
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssDxChMaximumRouterArpIndex")

--constants


-- ************************************************************************
---
--  check_arp_entry_index
--        @description  checking of arp entry index
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter rang
--
--        @return       true and arp entry index on success, otherwise 
--                      false and error message
--
local function check_arp_entry_index(param, name, desc)
    local result, values 
    local index, devNum, devlist

    param = tonumber(param)
    
    if param == nil then
        return false, name .. " not a number"
    end

    devlist = wrLogWrapper("wrlDevList")

    for index, devNum in pairs(devlist) do
        result, values = wrLogWrapper("wrlCpssDxChMaximumRouterArpIndex","(devNum)",devNum)
        if    (0 == result) and (0 <= param) and (param <= values) then
            return true, param
        elseif 0 ~= result then
            return false, values
        end
    end    

    return false, "Entry " .. param .. " does not exist."
end


-- ************************************************************************
---
--  complete_arp_entry_index
--        @description  autocompleting of arp entry index
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter rang
--
--        @return       autocompleting and help array 
--
local function complete_arp_entry_index(param, name, desc)
    local values = 
        { {0,      "Zero arp entry index"} }
    local compl = {}
    local help  = {}
    local k
    
    for k = 1, #values do
        if prefix_match(param, tostring(values[k][1])) then
            table.insert(compl, tostring(values[k][1]))
            help[#compl] = values[k][2]
        end
    end
    return compl, help
end


--------------------------------------------
-- type registration: arp_entry_index
--------------------------------------------
CLI_type_dict["arp_entry_index"] = {
    checker = check_arp_entry_index,
    complete = complete_arp_entry_index,
    help = "Arp entry index"
} 