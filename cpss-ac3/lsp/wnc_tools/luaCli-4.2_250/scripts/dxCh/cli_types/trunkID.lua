--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* trunkID.lua
--*
--* DESCRIPTION:
--*       trunkID type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 9 $
--*
--********************************************************************************

--includes
require("common/misc/wraplCpssCommon")

--constants


-- ************************************************************************
---
--  check_param_trunk
--        @description  ckecks trunkId value
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       true and trunkId on success, otherwise false and 
--                      error message
--
function check_param_trunk(param, name, desc)
    local index, result
    local maxTrunkId, trunkId
    local devList, devNum
    
    --Check if the input is valid
    result, trunkId = CLI_check_param_number(param, name, desc)
    if false == result then
        return false, string.format("Trunk %s is out of range", param)
    end
    
    trunkId = tonumber(trunkId)
    
    -- Checking of zero trunk.
    if 0 == trunkId then
        return false, "It is not allowed to handle trunk 0"
    elseif 0 > trunkId then
        return false, "Trunk id should be positive"
    end
    
    -- Checking od max trunk number
    devlist = wrLogWrapper("wrlDevList")
    for index, devNum in pairs(devlist) do
        maxTrunkId = system_capability_get_table_size(devNum, "TRUNK")
      
        if trunkId <= maxTrunkId   then
    return true, trunkId
end
    end
    
    return false, "Trunk " .. tostring(trunkId) .. "does not initialized"
end


-- ************************************************************************
---
--  complete_param_trunk
--        @description  complete trunkId value 
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       complete and help lists
--
function complete_param_trunk(param, name, desc)
    local values = 
        {{1,      "Default trunkId"}}
    local compl, help = CLI_complete_param_number(param, name, desc)
    local index
    
    for index = 1, #values do
        if prefix_match(param, tostring(values[index][1])) then
            table.insert(compl, tostring(values[index][1]))
            help[#compl] = values[index][2]
        end
    end
    
    return compl, help
end


-------------------------------------------------------
-- type registration: trunkID
-------------------------------------------------------
CLI_type_dict["trunkID"] = 
{
    checker  = check_param_trunk,
    complete = complete_param_trunk,
    min      = 1,
    max      = MAXIMUM_TRUNK_ID,
    help     = "Valid Port-Channel interface"
}


-------------------------------------------------------
-- type registration: trunk
-------------------------------------------------------
CLI_type_dict["trunk"] = 
{
    checker  = check_param_trunk,
    complete = complete_param_trunk,
    min      = 1,
    max      = MAXIMUM_TRUNK_ID,
    help     = "Valid Port-Channel interface"
}
-- function to reload the trunkId dictionary
function reloadTrunkIdDictionary()
    local devlist = wrLogWrapper("wrlDevList")
    if not devlist then 
        return -- no update at this time
    end 

    MAXIMUM_TRUNK_ID = 0
    for index, devNum in pairs(devlist) do
        maxTrunkId = system_capability_get_table_size(devNum, "TRUNK")
        
        --print("maxTrunkId" , to_string(maxTrunkId))

        if(maxTrunkId == nil) then
            -- the LUA ENV did not initialize this parameter
            break
        end
        
        if(MAXIMUM_TRUNK_ID == 0) then
            -- init for first device
            MAXIMUM_TRUNK_ID = maxTrunkId
        end
      
        if MAXIMUM_TRUNK_ID < maxTrunkId   then
            MAXIMUM_TRUNK_ID = maxTrunkId
        end
    end

    CLI_type_dict["trunkID"].max = MAXIMUM_TRUNK_ID
    CLI_type_dict["trunk"].max   = MAXIMUM_TRUNK_ID
    
    --print("MAXIMUM_TRUNK_ID" , to_string(MAXIMUM_TRUNK_ID))
    
end

table.insert(typeCallBacksList, reloadTrunkIdDictionary)

