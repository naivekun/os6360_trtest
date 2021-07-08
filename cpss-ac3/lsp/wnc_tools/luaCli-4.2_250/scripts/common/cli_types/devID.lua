--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* devID.lua
--*
--* DESCRIPTION:
--*       devID type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 5 $
--*
--********************************************************************************

--includes

--constants

--
-- Types defined:
--
-- devID  - device ID number
--
require("common/misc/wraplCpssCommon")

-- Device ID type
do

-- ************************************************************************
---
--  check_value_deviceid
--        @description  check's device number parameter that it is in
--                      the correct form
--
--        @param param              - parameter string
--        @param name               - parameter name string
--
--        @return        device numer
--
local function check_value_deviceid(param,name,desc)
    param = tonumber(param)
    if param == nil then
        return false, name .. " not a number"
    end
    if param < 0 or param > 255 then
        return false, "The device ID is out of range: 0..255"
    end
    if not wrLogWrapper("wrlIsDevIdValid", "(param)", param) then
        return false, "The device ID doesn't exist"
    end
    return true, param
end


-- ************************************************************************
---
--  complete_value_deviceid
--        @description  autocompletes device number parameter
--
--        @param param              - parameter string
--        @param name               - parameter name string
--
--        @return        device number autocomplete
--
local function complete_value_deviceid(param,name,desc)
    local all_devices = wrLogWrapper("wrlDevList")
    local compl, help = CLI_complete_param_number(param, name, desc)
    local i
    for i = 1, #all_devices do
        if prefix_match(param,tostring(all_devices[i])) then
            table.insert(compl, tostring(all_devices[i]))
            help[#compl] = "Apply to given device ID in system"
        end
    end
    return compl, help
end


-- ************************************************************************
---
--  check_value_deviceid_all
--        @description  check's device number parameter or "all" that it is
--                      in the correct form
--
--        @param param              - parameter string
--        @param name               - parameter name string
--
--        @return        device numer
--
local function check_value_deviceid_all(param, name, desc, varray, params)
  local status, val
  
  params.devFamily = nil
  
  if param == "all" then
    return true, param
  end
  
  status, val = check_value_deviceid(param,name,desc)
  if not status then
    return status, val
  end
  
  params.devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(val)", val)

  return status, val
end


-- ************************************************************************
---
--  complete_value_deviceid_all
--        @description  autocompletes device number parameter
--
--        @param param              - parameter string
--        @param name               - parameter name string
--
--        @return        device number autocomplete
--
local function complete_value_deviceid_all(param,name,desc)
    local compl, help = complete_value_deviceid(param,name,desc)
    if prefix_match(param, "all") then
        table.insert(compl,"all")
        help[#compl] = "Apply to all devices"
    end
    return compl, help
end

-- ************************************************************************
---
--  check_value_deviceSlvId
--        @description  check's slave device number parameter that it is in
--                      the correct form
--
--        @param param              - parameter string
--        @param name               - parameter name string
--
--        @return        device numer
--
local function check_value_deviceSlvId(param, name, desc)
  
  local data
  local m = string.match(param, "%x+", 3)

  if (not prefix_match("0x", param)) or (m == nil) then
    data = tonumber(param)
  else        
    data = tonumber(m, 16)
  end


  if data == nil then
    return false, name .. " not a number"
  end
  if data < 0 or data > 255 then
    return false, "The device ID is out of range: 0..255"
  end
  return true, data
end


-- ************************************************************************
---
--  complete_value_deviceSlvId
--        @description  autocompletes slave device number parameter
--
--        @param param              - parameter string
--        @param name               - parameter name string
--
--        @return
--
local function complete_value_deviceSlvId(param,name,desc)
    return {}, {def="Slave device number in range <0...255> or <0x00 ... 0xFF>"}
end

-------------------------------------------------------
-- type registration: devID
-------------------------------------------------------
CLI_type_dict["devID"] = {
    checker = check_value_deviceid,
    complete = complete_value_deviceid,
    min         = 0,
    max         = 127,
    help = "Enter device ID"
}

-------------------------------------------------------
-- type registration: devID_all
-------------------------------------------------------
CLI_type_dict["devID_all"] = {
    checker = check_value_deviceid_all,
    complete = complete_value_deviceid_all,
    help = "Enter device ID or 'all' for all devices"
}

-------------------------------------------------------
-- type registration: devSlvID
-------------------------------------------------------
CLI_type_dict["devSlvID"] = {
    checker = check_value_deviceSlvId,
    complete = complete_value_deviceSlvId,
    help = "Enter slave device ID"
}

end
