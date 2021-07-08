--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_mac_address_table_aging_time.lua
--*
--* DESCRIPTION:
--*       showing of the aging time of addresses present in the Forwarding 
--*       Database
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  Command_Data:show_mac_address_table_aging_time_func
--        @description  show's of the aging time of addresses present in 
--                      the Forwarding Database
--
--        @param params         - params["all_device"]: all or given devices
--                                devices iterating property
--
--        @return       true on success, otherwise false and error message
--
function show_mac_address_table_aging_time_func(params)
    -- Common variables declaration
    local result, values
    local devNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local aging_time, aging_time_string
    local header_string, footer_string
    
    -- Common variables initialization
    command_data:initAllDeviceRange(params)
    command_data:clearResultArray()

    -- Command specific variables initialization    
    header_string = 
        "\n" ..
        "Device   Aging Time \n" ..
        "------  ------------\n"
    footer_string = "\n"    
    
    -- Main device handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()     

            -- Mac address entry getting.
            command_data:clearLocalStatus()

            if true == command_data["local_status"] then   
                result, values = cpssPerDeviceParamGet(
                                    "cpssDxChBrgFdbAgingTimeoutGet", devNum, 
                                    "timeout", "GT_U32")
                if        0 == result then 
                    aging_time = values["timeout"]
                elseif 0x10 == result then
                    command_data:setFailDeviceStatus() 
                    command_data:addWarning("Mac address-table aging timeout " ..
                                            "getting is not allowed on " ..
                                            "device %d.", devNum)                     
                elseif    0 ~= result then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at mac address-table aging " ..
                                          "timeout getting on device %d.", 
                                          devNum, returnCodes[result])
                end
                
                if 0 == result then       
                    aging_time_string = tostring(aging_time)
                else
                    aging_time_string = "n/a"
                end                  
            end            
            
            command_data:updateStatus()             
            
            -- Resulting string formatting and adding.
            command_data["result"] = 
                string.format("%-8s %-15s", tostring(devNum), 
                              aging_time_string)  
            command_data:addResultToResultArray()
            
            command_data:updateDevices()
        end
    end

    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr()
    command_data:setResultStrOnDevicesCount(header_string, command_data["result"], 
                                            footer_string,
                                            "There is no mac address aging-times to show.\n")
        
    command_data:analyzeCommandExecution()
       
    command_data:printCommandExecutionResults()
    
	return command_data:getCommandExecutionResults()            
end


-----------------------------------------------------------
-- command registration: show mac address-table aging-time
-----------------------------------------------------------
CLI_addCommand("exec", "show mac address-table aging-time", {
  func   = show_mac_address_table_aging_time_func,
  help   = "Showing of the aging time of addresses present in the Forwarding Database.",
  params = {
      { type="named",
          "#all_device",
          mandatory = {"all_device"}
      }
  }
})
