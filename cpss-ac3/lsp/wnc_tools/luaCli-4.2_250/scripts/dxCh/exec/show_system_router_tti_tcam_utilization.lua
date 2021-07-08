--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_system_router_tti_tcam_utilization.lua
--*
--* DESCRIPTION:
--*       Displaing of the router Ternary Content Addressable Memory (TCAM) 
--*       utilization (this TCAM is shared between the router and the TTI 
--*       engine)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 6 $
--*
--********************************************************************************

--includes
if not cli_C_functions_registered("wrlCpssDxChTcamUtilizationGet") then
    function wrlCpssDxChTcamUtilizationGet()
        return 0x11 -- GT_NOT_IMPLEMENTED
    end
end

--constants


-- ************************************************************************
---
--  show_system_router_tti_tcam_utilization_func_given_device
--        @description  shows the router Ternary Content Addressable Memory
--                      (TCAM) utilization for given device (this TCAM is 
--                      shared between the router and the TTI engine) 
--
--        @param command_data   - command execution data object
--        @param devNum         - checked device number
--        @param header_string  - printed header string
--
--        @return       true on success, otherwise false and error message
--
local function show_system_router_tti_tcam_utilization_func_given_device(command_data, devNum, header_string)
    -- Command specific variables initialization. 
    if nil == header_string then
        header_string                  = ""
    end
    local footer_string = "\n"
    local router_tti_tcam_utilization_string_prefix = 
                                "TTI TCAM utilization:\t"
    
    -- Getting of device router ternary content addressable memory utilization.
    command_data:clearLocalStatus()
    
    local ret, tcamGroup, entriesTotal, entriesUsed
    local tcam_tti_ = "CPSS_DXCH_TCAM_TTI_E"
    ret, tcamGroup, entriesTotal, entriesUsed = 
        wrLogWrapper("wrlCpssDxChTcamUtilizationGet","(devNum, tcam_tti_)",devNum, tcam_tti_)
    local s
    if ret == 0 then
        s = to_string(entriesUsed).."/"..to_string(entriesTotal)
        if entriesTotal ~= 0 then
            s = s .. "  ("..to_string(math.floor(entriesUsed*100/entriesTotal)).."%)"
        end
    else
        s = "n/a"
    end
    
    command_data:updateStatus()
    
    -- Device router ternary content addressable memory utilization 
    -- string formatting and adding.   
    command_data:setResultStr(router_tti_tcam_utilization_string_prefix, s)
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
--  show_system_router_tti_tcam_utilization_func_all_devices
--        @description  shows the router Ternary Content Addressable Memory 
--                      (TCAM) utilization for all avaible devices (this 
--                      TCAM is shared between the router and the TTI 
--                      engine)
--
--        @param command_data   - command execution data object
--
--        @return       true on success, otherwise false and error message
--
local function show_system_router_tti_tcam_utilization_func_all_devices(command_data)
    -- Common variables declaration
    local result, values
    local devNum
    
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in command_data:getAllAvailableDevicesIterator() do             
            result, values = show_system_router_tti_tcam_utilization_func_given_device(
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
--  show_system_router_tti_tcam_utilization_func
--        @description  shows the router Ternary Content Addressable Memory 
--                      (TCAM) utilization (this TCAM is shared between the 
--                      router and the TTI engine) 
--
--        @param params         - params["all_device"]: all or given devices
--                                devices iterating property
--
--        @return       true on success, otherwise false and error message 
--
local function show_system_router_tti_tcam_utilization_func(params)
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
            show_system_router_tti_tcam_utilization_func_given_device(command_data, 
                                                                  params["devID"])
    else
        result, values = 
            show_system_router_tti_tcam_utilization_func_all_devices(command_data)        
    end
    
    return result, values
end


--------------------------------------------------------------------------------
-- command registration: show system router-TTI-tcam utilization
--------------------------------------------------------------------------------
CLI_addHelp("exec", "show system", "Show system information")
CLI_addHelp("exec", "show system tti-tcam", "tti-tcam")
CLI_addCommand("exec", "show system tti-tcam utilization", {
  func = show_system_router_tti_tcam_utilization_func,
  help    = "tti-tcam utilization",
  params = {
      { type = "named",
          "#all_device",
        mandatory = { "all_device" }          
      }
  }
})
