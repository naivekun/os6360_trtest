--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* mac_address_table_aging_time.lua
--*
--* DESCRIPTION:
--*       Setting of the aging time of the address table
--*
--* FILE REVISION NUMBER:
--*       $Revision: 6 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlDxChBrgFdbAgingTimeoutCheck")

--constants 


-- ************************************************************************
---
--  mac_address_table_aging_time_aging_time_func
--        @description  set's of the aging time of the address table
--
--        @param params         - params["port_duplex"]: duplex mode 
--                                string: half/full
--
--        @usage __global       - __global["ifRange"]: iterface range 
--
--        @return       true on success, otherwise false and error message
--
local function mac_address_table_aging_time_aging_time_func(params)
    -- Common variables declaration
    local result, values
    local devNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local action_mode, action_mode_correctness
    local agind_time_correctness, seconds
    
    -- Common variables initialization
    command_data:initAllDeviceRange(params)    
    
    -- Command specific variables initialization 
    if nil == params.flagNo then    
        seconds = params["second"]
    else
        seconds = 300
    end
        
    -- Main device handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()

            command_data:clearLocalStatus() 
            
            -- Getting of action_mode_correctness action mode.
            if true == command_data["local_status"] then   
                result, values =                                                    
                    cpssPerDeviceParamGet("cpssDxChBrgFdbMacTriggerModeGet",
                                          devNum, "mode", 
                                          "CPSS_MAC_ACTION_MODE_ENT")
                if        0 == result then
                    action_mode = values["mode"]
                elseif 0x10 == result then
                    command_data:setFailDeviceAndLocalStatus() 
                    command_data:addWarning("Getting of Mac address table " ..
                                            "action mode is not allowed " ..
                                            "on device %d.", devNum) 
                elseif    0 ~= result then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at getting of Mac address " ..
                                          "table action mode on device %d: %s", 
                                          devNum, returnCodes[result])
                end            
            end            
            
            -- Detecting of Mac address table action mode correctness.
            if true == command_data["local_status"] then 
                action_mode_correctness = "CPSS_ACT_AUTO_E" == action_mode
            end        

            -- Checking of Mac address table action mode correctness.
            if true == command_data["local_status"]         then 
                command_data:setFailDeviceAndLocalStatusIfFalseCondition(
                    action_mode_correctness)
                command_data:addWarningIfFalseCondition(
                    action_mode_correctness, "Mac address-table is not in " .. 
                    "automatical aging mode on device %d.", devNum)
            end
            
            -- Mac address-table aging timeout check.            
            if true == command_data["local_status"] then   
                result, values = wrLogWrapper("wrlDxChBrgFdbAgingTimeoutCheck", 
                                              "(devNum, seconds)", devNum, seconds)
                if        0 == result then                
                    agind_time_correctness = values
                elseif 0x10 == result then
                    command_data:setFailDeviceAndLocalStatus() 
                    command_data:addWarning("Mac address-table aging timeout " ..
                                            "%d checking is not allowed in " ..
                                            "device %d.", seconds, devNum)                     
                elseif    0 ~= result then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at mac address-table aging " ..
                                          "timeout %d checking on device %d: %s", 
                                          seconds, devNum, returnCodes[result])
                end
            end              

            -- Checking of Mac address-table aging timeout correctness.
            if true == command_data["local_status"]         then           
                command_data:setFailDeviceAndLocalStatusIfFalseCondition(
                    agind_time_correctness)
                command_data:addWarningIfFalseCondition(
                    agind_time_correctness, "Mac address-table aging timeout " ..
                    "%d setting is not allowed on device %d.", seconds, devNum)
            end
            
            -- Setting of Mac address-table aging timeout.           
            if true == command_data["local_status"]         then   
                result, values = 
                    cpssPerDeviceParamSet("cpssDxChBrgFdbAgingTimeoutSet", 
                                          devNum, seconds, "timeout", "GT_U32")
                if     0x10 == result then
                    command_data:setFailDeviceStatus() 
                    command_data:addWarning("Mac address-table aging timeout " ..
                                            "%d setting is not allowed in " ..
                                            "device %d.", seconds, devNum)                     
                elseif 0 ~= result then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at mac address-table aging " ..
                                          "timeout %d setting on device %d: %s", 
                                          seconds, devNum, returnCodes[result])
                end
            end    

            command_data:updateStatus()             
            
            command_data:updateDevices()
        end
        
        command_data:addWarningIfNoSuccessDevices(
            "Can not set the mac address-table aging timeout for all " ..
            "processed devices.")        
    end             
                      
    command_data:analyzeCommandExecution()
       
    command_data:printCommandExecutionResults()
    
    return command_data:getCommandExecutionResults()            
end


---------------------------------------------------------
-- command registration: mac address-table aging-time
--------------------------------------------------------- 
CLI_addCommand("config", "mac address-table aging-time", {
  func   = mac_address_table_aging_time_aging_time_func,
  help   = "Set the aging time of the address table",
  params = {
      { type = "values",
          "%second"
      },
      { type="named",
          "#all_device",
        mandatory = { "all_device" }
      }
  }
})


---------------------------------------------------------
-- command registration: no mac address-table aging-time
--------------------------------------------------------- 
CLI_addCommand("config", "no mac address-table aging-time", {
  func   = function(params)
               params.flagNo=true
               return mac_address_table_aging_time_aging_time_func(params)
           end,
  mac_address_table_aging_time_aging_time_func,
  help   = "Set the aging time of the address table",
  params = {
      { type="named",
          "#all_device",
        mandatory = { "all_device" }
      }
  }
})
