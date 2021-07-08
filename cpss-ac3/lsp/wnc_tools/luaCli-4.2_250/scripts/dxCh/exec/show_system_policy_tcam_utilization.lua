--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_system_policy_tcam_utilization.lua
--*
--* DESCRIPTION:
--*       Displaing of the policy (PCL) Ternary Content Addressable Memory 
--*       (TCAM) utilization
--*
--* FILE REVISION NUMBER:
--*       $Revision: 5 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  show_system_policy_tcam_utilization_func_given_device
--        @description  shows the policy (PCL) Ternary Content Addressable 
--                      Memory (TCAM) utilization for given device 
--
--        @param command_data   - command execution data object
--        @param devNum         - device number
--        @param header_string  - printed header string
--
--        @return       true on success, otherwise false and error message
--
local function show_system_policy_tcam_utilization_func_given_device(command_data, devNum, header_string)
    -- Common variables declaration
    local result, values
    -- Command  specific variables declaration
    local pcl_tcam_utilization, pcl_tcam_utilization_string
    local pcl_tcam_utilization_string_prefix, pcl_tcam_utilization_string_suffix
    local header_string, footer_string 
    
    -- Command specific variables initialization. 
    if nil == header_string then
        header_string                  = ""
    end
    pcl_tcam_utilization_string_prefix = 
                                    "Policy TCAM utilization (valid entries):\t"
    pcl_tcam_utilization_string_suffix = "%"
    footer_string                      = "\n"
    
    -- Getting of device policy ternary content addressable memory utilization.
    command_data:clearLocalStatus()
    
    if true  == command_data["local_status"] then 
        pcl_tcam_utilization = 0
        pcl_tcam_utilization_string = "n/a" 
        pcl_tcam_utilization_string_suffix = nil
    end
    
    command_data:updateStatus()
    
    -- Device exisitg ports bitmap 1 string formatting and adding.   
    command_data:setResultStr(pcl_tcam_utilization_string_prefix, 
                              pcl_tcam_utilization_string,
                              pcl_tcam_utilization_string_suffix)
    command_data:addResultToResultArray()
    
    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr()    
    command_data:setResultStr(header_string, command_data["result"], 
                              footer_string)
        
    command_data:analyzeCommandExecution()
       
    command_data:printCommandExecutionResults()
    
	return command_data:getCommandExecutionResults()    
end    


-- ************************************************************************
---
--  show_system_policy_tcam_utilization_func_all_devices
--        @description  shows the policy (PCL) Ternary Content Addressable 
--                      Memory (TCAM) utilization for all avaible devices
--
--        @param command_data   - command execution data object
--
--        @return       true on success, otherwise false and error message
--
local function show_system_policy_tcam_utilization_func_all_devices(command_data)
    -- Common variables declaration
    local result, values
    local devNum
    
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in command_data:getAllAvailableDevicesIterator() do             
            result, values = show_system_policy_tcam_utilization_func_given_device(
                                command_data, devNum, 
                                "Device " .. tostring(devNum) .. ":")
            if 0 ~= result then
                break        
            end
        end   
    end
    
    return result, values
end


-- ************************************************************************
---
--  show_system_policy_tcam_utilization_func
--        @description  shows the policy (PCL) Ternary Content Addressable 
--                      Memory (TCAM) utilization 
--
--        @param params         - params["all_device"]: all or given devices
--                                devices iterating property
--
--        @return       true on success, otherwise false and error message
--
local function show_system_policy_tcam_utilization_func(params)
    -- Common variables declaration
    local result, values
    local command_data = Command_Data()
    -- Command  specific variables declaration
    local all_devices_flag
    
    -- Common variables initialization.
    command_data:clearResultArray()    
    
    -- Command specific variables initialization. 
    if nil ~= params["devID"] then
        all_devices_flag = false
    else
        all_devices_flag = true
    end    

    if false == all_devices_flag then
        result, values = 
            show_system_policy_tcam_utilization_func_given_device(command_data, 
                                                                  params["devID"])
    else
        result, values = 
            show_system_policy_tcam_utilization_func_all_devices(command_data)        
    end
    
    return result, values
end


--------------------------------------------------------------------------------
-- command registration: show system policy-tcam utilization
--------------------------------------------------------------------------------
CLI_addHelp("exec", "show system policy-tcam", "policy-tcam")
CLI_addCommand("exec", "show system policy-tcam utilization", {
  func = show_system_policy_tcam_utilization_func,
  hep    = "policy-tcam utilization",
  params = {
      { type = "named",
          "#all_device",
        mandatory = { "all_device" }          
      }
  }
})