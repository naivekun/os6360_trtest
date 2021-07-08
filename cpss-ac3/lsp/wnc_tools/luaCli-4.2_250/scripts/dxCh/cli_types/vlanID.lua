--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vlanID.lua
--*
--* DESCRIPTION:
--*       vlanID type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 10 $
--*
--********************************************************************************

--includes
local MAXIMUM_AUTOCOMPLETED_VLANS_NUMDER = 7

--constants


require("common/misc/wraplCpssCommon")
cmdLuaCLI_registerCfunction("wrlCpssDxChVlanList")

do

function max_vlanID(devID)
    if devID == "all" then
        local all_earch = true
        local devlist = wrLogWrapper("wrlDevList")
        for i=1,#devlist do
            if is_device_eArch_enbled(devlist[i]) == false then
                all_earch = false
            end
        end
        if all_earch == true then
            return 8191
        end
    else
        if is_device_eArch_enbled(devID) == true then
            return 8191
        end
    end
    return 4095
end

function vlan_complete_param_number_max(param, name, desc, varray, params)
    desc.max = max_vlanID(params["all-device"])
    return CLI_complete_param_number(param, name, desc)
end

-- ************************************************************************
---
--  check_param_vlan
--        @description  vlan Id checker
--
--        @param param              - checked parameter value
--        @param name               - checked parameter name
--        @param desc               - checked parameter range
--
--        @return       true and param on success, otherwise false and 
--                      error message
-- 
local function check_param_vlan(param, name, desc)
    local stat
    local vlan
    local devlist
    local i

    -- Entry parameter conversion.
    param   = tonumber(param)
    
    -- Entry parameter checking.
    if     param < 0    then
        return false, "vlanId could not be negative"
    elseif param == 0   then
        return false, "vlanId could not be equal to zero"
    end
    
    --Check if the input is valid
    stat, vlan = CLI_check_param_number(param, name, desc)
    if stat == false then
        return false, vlan
    end
    
    devlist = wrLogWrapper("wrlDevList")
    --Check if the vlan is valid
    for i=1,#devlist do
        if does_vlan_exists(devlist[i], vlan) then
            --it is return true
            return true, vlan
        end
    end
    
    --It isn't return an error string
    return false, string.format("VLAN %d not configured", vlan)
end


-- ************************************************************************
---
--  check_param_unchecked_vlan
--        @description  check's that vlan presences in vlan range but does
--                      not ckeck it's existence
--
--        @param param              - checked parameter value
--        @param name               - checked parameter name
--        @param desc               - checked parameter range
--        @param varray             - unused
--        @param params             - parameter for access other parameters in command line
--
--        @return       true and param on success, otherwise false and 
--                      error message
-- 
function check_param_unchecked_vlan(param, name, desc, varray, params)
    local result, values
    local vlanId
    local devlist
    local i

    --Check if the input is valid
    result, values = CLI_check_param_number(param, name, desc)
    if  true == result then
        vlanId = values
    else
        return false, param .. " is wrong vlan id"
    end
    
    if 0 == vlanId then
        return false, "It is not allowed to create Vlan 0"
    end

    if vlanId > max_vlanID(params["all_device"]) then
        return false, "It is not allowed to create Vlan "..vlanId
    end
    
    return true, vlanId
end


-- ************************************************************************
---
--  complete_param_vlan
--        @description  autocompleting of new or configurable vlan id
--
--        @param param              - checked parameter value
--        @param name               - checked parameter name
--        @param desc               - checked parameter range
--
--        @return       completing and help array
-- 
local function complete_param_vlan(param, name, desc)
    local devlist = wrLogWrapper("wrlDevList")
    local k,v
    local vl={}
    local autocomp_count = 0
    --Check if the vlan is valid
    for k=1,#devlist do
        local devNum = devlist[k]
        local dev_vlan = wrLogWrapper("wrlCpssDxChVlanList", "(devNum)", devNum)
        for v=1,#dev_vlan do
            vl[dev_vlan[v]] = 1
        end
    end
    local compl, help = CLI_complete_param_number(param, name, desc)
    for k,v in pairs(vl) do
        autocomp_count = autocomp_count + 1
        if MAXIMUM_AUTOCOMPLETED_VLANS_NUMDER < autocomp_count then
            break
        end
    
        if prefix_match(param, k) then
            table.insert(compl,k)
            help[#compl] = "existing vlan"
        end
    end
    return compl,help
end


-------------------------------------------------------
-- type registration: vlan
-------------------------------------------------------
CLI_type_dict["vlan"] = {
    checker = check_param_vlan,
    min=1,
    max=4095,
    complete = vlan_complete_param_number_max,
    help = "Specify IEEE 802.1Q VLAN ID"
}


-------------------------------------------------------
-- type registration: vlanId
-------------------------------------------------------
CLI_type_dict["vlanId"] = {
    checker = check_param_vlan,
    min=1,
    max=4095,
    complete = vlan_complete_param_number_max,
    help = "Specify IEEE 802.1Q VLAN ID"
}

-------------------------------------------------------
-- type registration: vlanIdNoCheck
-------------------------------------------------------
CLI_type_dict["vlanIdNoCheck"] = {
    checker = CLI_check_param_number,
    min=1,
    max=4095,
    complete = vlan_complete_param_number_max,
    help = "Specify IEEE 802.1Q VLAN ID"
}



-- ************************************************************************
---
--  check_param_vlan_or_all
--        @description  check's Vlan id parameter or "all" that it is
--                      in the correct form
--
--        @param param              - parameter string
--        @param name               - parameter name string
--        @param desc               - parameter description
--
--        @return        device numer
--
local function check_param_vlan_or_all(param,name,desc)
    if param == "all" then
        return true, param
    end
    return check_param_vlan(param,name,desc)
end


-- ************************************************************************
---
--  complete_value_vlanId_all
--        @description  autocompletes vlan number parameter
--
--        @param param              - parameter string
--        @param name               - parameter name string
--        @param desc               - parameter description
--        @param varray             - unused
--        @param params             - parameter for access other parameters in command line
--
--        @return        device number autocomplete
--
local function complete_value_vlanId_all(param, name, desc, varray, params)
    local compl, help = vlan_complete_param_number_max(param, name, desc, varray, params)
    if prefix_match(param, "all") then
        table.insert(compl,"all")
        help[#compl] = "Apply to all vlans"
    end
    return compl, help
end

-------------------------------------------------------
-- type registration: vlanIdOrAll
-------------------------------------------------------
CLI_type_dict["vlanIdOrAll"] = {
    checker = check_param_vlan_or_all,
    min=1,
    max=4095,
    complete = complete_value_vlanId_all,
    help = "Specify IEEE 802.1Q VLAN ID"
}



-- ************************************************************************
---
--  check_param_vlan_or_any
--        @description  vlan Id of Any checker
--
--        @param param              - checked parameter value
--        @param name               - checked parameter name
--        @param desc               - checked parameter range
--
--        @return       true and param on success, otherwise false and 
--                      error message
-- 
local function check_param_vlan_or_any(param, name, desc)
    if param == "any" then
        return true, param
    end
    return check_param_vlan(param,name,desc)
end

-- ************************************************************************
---
--  complete_param_vlan_or_any
--        @description  autocompletes vlanId parameter
--
--        @return        vlanId autocomplete
--
local function complete_param_vlan_or_any(param, name, desc, varray, params)
    local compl, help = vlan_complete_param_number_max(param, name, desc, varray, params)
    if prefix_match(param, "any") then
        table.insert(compl,"any")
        help[#compl] = "Apply to any vlan"
    end
    return compl, help
end

-------------------------------------------------------
-- type registration: vlanIdAny
-------------------------------------------------------
CLI_type_dict["vlanIdOrAny"] = {
    checker = check_param_vlan_or_any,
    min=1,
    max=4095,
    complete = complete_param_vlan_or_any,
    help = "Specify IEEE 802.1Q VLAN ID"
}

local function check_param_vlan_and_mask(param, name, desc)
    local numParam = tonumber(param)
    if numParam ~= nil then
        return check_param_vlan(param, name, desc)
    end
    local sIndex = string.find(param, "/")
    if sIndex == nil then
        return false, "Invalid value"
    end
    local key = string.sub(param, 0, sIndex-1)
    local mask = string.sub(param, sIndex+1)

    if (tonumber(key) == nil or tonumber(mask) == nil) then
        return false, "Invalid value"
    end
    isMask = true
    local res, val = check_param_vlan(key, name, desc)
    if (res == false) then
        return res,val
    end
    return true, param
end

-------------------------------------------------------
-- type registration: vlanId with mask
-------------------------------------------------------
CLI_type_dict["vlanIdWithMask"] = {
    checker = check_param_vlan_and_mask,
    help = "Specify IEEE 802.1Q VLAN ID, mask is optional"
}


-------------------------------------------------------
-- type registration: unchecked-vlan
-------------------------------------------------------
CLI_type_dict["unchecked-vlan"] = {
    checker  = check_param_unchecked_vlan,
    complete = vlan_complete_param_number_max,
    min=1,
    max=4095,
    help = "Specify IEEE 802.1Q VLAN ID"
}


-------------------------------------------------------
-- type registration: configurable-vlan
-------------------------------------------------------
CLI_type_dict["configurable-vlan"] = {
    checker  = check_param_unchecked_vlan,
    complete = vlan_complete_param_number_max,
    min=1,
    max=4095,
    help = "Specify IEEE 802.1Q VLAN ID"
}

end

