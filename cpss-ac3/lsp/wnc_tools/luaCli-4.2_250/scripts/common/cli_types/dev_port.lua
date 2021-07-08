--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* dev_port.lua
--*
--* DESCRIPTION:
--*       dev/port pair type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 8 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssDevicePortNumberGet")
cmdLuaCLI_registerCfunction("luaCLI_getPortGroups")

require("common/cli_types/port_range")

--constants


-- ************************************************************************
---
--  check_param_dev_port
--        @description  ckecks dev/port pair
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       true and {devID, portNum} on success, otherwise
--                      false and error message
--
function check_param_dev_port(param, name, desc)
    local result, values
    local devNum, devPorts, devMaxPortNum
    local portIndex, portNum
    local port_count = 0
    local returned_value
    local cpuPort

    result, values = check_param_port_range(param)

    if true == result then
        for devNum, devPorts in pairs(values) do
            for portIndex, portNum in pairs(devPorts) do
                port_count = port_count + 1

                returned_value = {["devId"]   = tonumber(devNum),
                                  ["portNum"] = tonumber(portNum)}

                if 1 < port_count then
                    return false, "Should be defined only one port"
                end
            end
        end
    else
        returned_value = values
    end

    if true == result then
        if "all" == trim(param) then
            result = 3
            returned_value = "Should be specified only one port"
        end
    end

    if true == result then
        if dxCh_family and is_sip_5(returned_value["devId"]) then
            devMaxPortNum = 8192
            result = 0
        else
            local inDevId = returned_value["devId"]
            result, values = wrLogWrapper("wrlCpssDevicePortNumberGet", "(inDevId)", inDevId)
            if 0 == result then
                devMaxPortNum = values
            else
                return false, "Error at maximum port number getting of device " ..
                              returned_value["devId"]
            end
        end
    end

    if (devEnv ~= nil) then
      cpuPort = devEnv.portCPU
    end

    if 0 == result then
        if (0 > returned_value["portNum"]) or
           (returned_value["portNum"] >= devMaxPortNum and returned_value["portNum"] ~= cpuPort) then
            return false, "Port is out of range"
        end
    end

    if 0 == result and returned_value["portNum"] ~= cpuPort and desc.micro_init ~= true then
        if false == does_port_exist(returned_value["devId"],
                                    returned_value["portNum"]) then
            return false, "Given port does not exists on device " ..
                          returned_value["devId"]
        end
    end

    return result, returned_value
end


-- ************************************************************************
---
--  check_param_remote_dev_port
--        @description  checks remote dev/port pair
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       true and {devID, portNum} on success, otherwise
--                      false and error message
--
local function check_param_remote_dev_port(param, name, desc)
    local result, values
    local devNum, devMaxNum, devPorts, devMaxPortNum
    local portIndex, portNum
    local port_count = 0
    local returned_value

    result, values = check_param_remote_port_range(param)

    if true == result then
        for devNum, devPorts in pairs(values) do
            for portIndex, portNum in pairs(devPorts) do
                port_count = port_count + 1

                returned_value = {["devId"]   = tonumber(devNum),
                                  ["portNum"] = tonumber(portNum)}

                if 1 < port_count then
                    return false, "Should be defined only one port"
                end
            end
        end
    else
        returned_value = values
    end

    if true == result then
        if "all" == trim(param) then
            result = 3
            returned_value = "Should be specified only one port"
        end
    end

    if true == result then
        devMaxPortNum = 8192
        devMaxNum = 1024

        if (0 > returned_value["devId"]) or
           (returned_value["devId"] >= devMaxNum) then
            return false, "Device Id is out of range"
        end

        if (0 > returned_value["portNum"]) or
           (returned_value["portNum"] >= devMaxPortNum) then
            return false, "Port is out of range"
        end
    end

    return result, returned_value
end


-- ************************************************************************
---
--  complete_param_dev_port
--        @description  complete dev/port pairr
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       complete and help lists
--
function complete_param_dev_port(param, name, desc)
    local values = {{"0/0",   "Default dev/port"}}
    local compl  = {}
    local help   = {}
    local index

    for index = 1, #values do
        if prefix_match(param, tostring(values[index][1])) then
            table.insert(compl, tostring(values[index][1]))
            help[#compl] = values[index][2]
        end
    end

    return compl, help
end

-- ************************************************************************
---
--  check_param_dev_ePort
--        @description  ckecks dev/port pair
--
--        @param param          - checked parameter value
--
--        @return       true and {devID, portNum} on success, otherwise
--                      false and error message
--                      
function check_param_dev_ePort(param)
  local devNum, portNum
  local returned_value = {}
  local i
  local min_devNum    = 0
  local max_devNum    = 255
  local min_ePortNum  = 256
  local max_ePortNum  = 8192

  i = string.find(param, "/")    -- find delimiter
  if i == nil then 
    return false, "Illegal dev/ePort delimiter" 
  end

  devNum = tonumber(string.sub(param, 1, i-1))
  portNum = tonumber(string.sub(param, i+1, #param))

  if devNum == nil or portNum == nil then
    return false, "Illegal dev/ePort parameters" 
  end

  if (not is_device_eArch_enbled(devNum)) then
    return false, "device is not an eArch"
  end

  if (devNum < min_devNum) or (devNum > max_devNum) then
    return false, "devNum is out of range"
  end

  if (portNum < min_ePortNum) or (portNum >= max_ePortNum) then
    return false, "ePortNum is out of range"
  end

  returned_value = {["devId"] = devNum, ["portNum"] = portNum}
  return true, returned_value
end

-------------------------------------------------------
-- type registration: dev_ePort
-------------------------------------------------------
CLI_type_dict["dev_ePort"] =
{
    checker  = check_param_dev_ePort,
    help     = "dev/ePort, where ePort in range 256 to (8K-1)"
}

-------------------------------------------------------
-- type registration: portNum
-------------------------------------------------------
CLI_type_dict["dev_port"] =
{
    checker  = check_param_dev_port,
    complete = complete_param_dev_port,
    help     = "Ethernet port"
}

-------------------------------------------------------
-- type registration: portNum
-------------------------------------------------------
CLI_type_dict["dev_port_mic"] =
{
    checker  = check_param_dev_port,
    complete = complete_param_dev_port,
    micro_init = true,
    help     = "Ethernet port"
}

-------------------------------------------------------
-- type registration:  device (may be remote) / portNum
-------------------------------------------------------
CLI_type_dict["remote_dev_port"] =
{
    checker  = check_param_remote_dev_port,
    complete = complete_param_dev_port,
    help     = "Ethernet port"
}


-- ************************************************************************
---
--  check_param_dev_port
--        @description  ckecks for valid port-group
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       true, portGroup on success, otherwise
--                      false and error message
--

local function check_param_port_group(param, name, desc)
  local portGroups,paramNum,index,dev,i,v
  local all_devices = wrLogWrapper("wrlDevList")
  local res={}

  paramNum=tonumber(param)
  if (paramNum==nil) and (param~="all") then return false, "Port-group must be a number" end

  for index,dev in pairs(all_devices) do
    portGroups=luaCLI_getPortGroups(dev)
    if (#portGroups>#res) then res=portGroups end
  end

  if (param=="all") then return true,param end

  for i,v in pairs(res) do
    if (v==paramNum) then return true,v  end
  end

  return false,"Port-group does not exist on device"

end

-- ************************************************************************
---
--  complete_param_dev_port
--        @description  completes portgroup number
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       complete and help lists
--

local function complete_param_port_group(param, name, desc)
  local values = {{"all","Apply to all port groups"}}
  local res = {}
  local compl = {}
  local help  = {}
  local index,dev,portGroups
  local all_devices = wrLogWrapper("wrlDevList")


  for index,dev in pairs(all_devices) do
    portGroups=luaCLI_getPortGroups(dev)
    if (#portGroups>#res) then res=portGroups end
  end


  for index=1,#res do
    table.insert(values, {res[index],"Port-group"})
  end

  for index = 1, #values do
    if prefix_match(param, tostring(values[index][1])) then
      table.insert(compl, tostring(values[index][1]))
      help[#compl] = values[index][2]
    end
  end

  return compl, help

end
-------------------------------------------------------
-- type registration: portGroup
-------------------------------------------------------
CLI_type_dict["portGroup"] =
{
    checker  = check_param_port_group,
    complete = complete_param_port_group,
    help     = "Device portgroup"
}
