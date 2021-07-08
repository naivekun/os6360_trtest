--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* portNum.lua
--*
--* DESCRIPTION:
--*       portNum type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 7 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssDevicePortNumberGet")

--constants


-- ************************************************************************
---
--  check_param_port
--        @description  ckecks portNum value
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       true and portNum on success, otherwise false and
--                      error message
--
local function check_param_port(param, name, desc)
    local result, values, index
    local devId
    local dev_range = wrLogWrapper("wrlDevList")

    param = tonumber(param)

    if nil == param then
        return false, name .. " not a number"
    end

    for index, devId in pairs(dev_range) do
        result, values = wrLogWrapper("wrlCpssDevicePortNumberGet","(devId)",devId)
        if 0 ~= result then
            return false, "Port number reading error at device " ..
                          tostring(devId)
        end

        if param < values then
            if does_port_exist(devId, param) then
            return true, param
            else
                return false, "Given port does not exists on device " .. devId
            end
        end
    end

    return false, "There is no avaible devices, contains such port number"
end


-- ************************************************************************
---
--  complete_param_port
--        @description  complete portNum value
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       complete and help lists
--
local function complete_param_port(param, name, desc)
    local values =
        {{0,      "Default portNum"}}
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
-- type registration: portNum
-------------------------------------------------------
CLI_type_dict["portNum"] =
{
    checker  = check_param_port,
    complete = complete_param_port,
    min      = 0,
    help     = "Enter port number"
}

local function check_param_port_and_mask(param, name, desc)
    local numParam = tonumber(param)
    if numParam ~= nil then
        return check_param_port(param, name, desc)
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
    local res, val = check_param_port(key, name, desc)
    if (res == false) then
        return res,val
    end
    return true, param
end

-------------------------------------------------------
-- type registration: portNum
-------------------------------------------------------
CLI_type_dict["portNumWithMask"] =
{
    checker  = check_param_port_and_mask,
    help     = "Enter port number, mask is optional"
}


-------------------------------------------------------
-- type registration: port
-------------------------------------------------------
CLI_type_dict["port"] =
{
    checker  = check_param_port,
    complete = complete_param_port,
    min      = 0,
    help     = "Enter port number"
}

-- ************************************************************************
---
--  check_param_ePort
--        @description  ckecks eport
--
--        @param param          - checked parameter value
--
--        @return       true and ePortNum on success, otherwise
--                      false and error message
--
local function check_param_ePort(portNum)
    local min_ePortNum  = 256
    local max_ePortNum  = 8192

    if portNum == nil then
        return false, "Illegal ePort parameters"
    end
    if (tonumber(portNum) < min_ePortNum) or (tonumber(portNum) >= max_ePortNum) then
        return false, "ePortNum is out of range, minVal=256 maxVal = 8192"
    end
    return true, portNum
end

-------------------------------------------------------
-- type registration: dev_ePort
-------------------------------------------------------
CLI_type_dict["ePort"] =
{
    checker  = check_param_ePort,
    help     = "ePort, where ePort in range 256 to (8K-1)"
}

-- ************************************************************************
---
--  check_param_ePortNat
--        @description  checks eport
--
--        @param param          - checked parameter value
--
--        @return       true and ePortNum on success, otherwise
--                      false and error message
--
local function check_param_ePortNat(portNum)
    local min_ePortNum  = 128
    local max_ePortNum  = 4096 + 128

    if isSipVersion(devNum , "SIP_5_20") then
        min_ePortNum  = 512
        max_ePortNum  = 4096 + 512
    elseif isSipVersion(devNum , "SIP_5") then
        min_ePortNum  = 256
        max_ePortNum  = 4096 + 256
    end

    if portNum == nil then
        return false, "Illegal ePort parameters"
    end
    if (tonumber(portNum) < min_ePortNum) or (tonumber(portNum) >= max_ePortNum) then
        return false, "ePortNum is out of range, minVal = "..min_ePortNum.." maxVal = "..max_ePortNum
    end
    return true, portNum
end

local function help_msg_ePortNat()
    local min_ePortNum  = 128
    local max_ePortNum  = 4096 + 128

    if isSipVersion(devNum , "SIP_5_20") then
        min_ePortNum  = 512
        max_ePortNum  = 4096 + 512
    elseif isSipVersion(devNum , "SIP_5") then
        min_ePortNum  = 256
        max_ePortNum  = 4096 + 256
    end

    return "ePort, where ePort in range "..min_ePortNum.." to "..max_ePortNum
end

-------------------------------------------------------
-- type registration: dev_ePortNat
-------------------------------------------------------
CLI_type_dict["ePortNat"] =
{
    checker  = check_param_ePortNat,
    help     = help_msg_ePortNat()
}
