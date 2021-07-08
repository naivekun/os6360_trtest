--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_ip_route.lua
--*
--* DESCRIPTION:
--*       setting the aging mode of the mac-address table
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlDxChMacAddressAgingModeConditionChecking")

--constants


-- ************************************************************************
---
--  mac_address_table_aging_mode_func
--        @description  set's the aging mode of the mac-address table
--
--        @param params             - params["mac_address_table_aging_mode
--                                    "]: property to delete or keep The 
--                                    dynamic addresses after they 
--                                    aged-out;
--                                    params["all_device"]: all or given 
--                                    devices iterating property, could be 
--                                    irrelevant
--
--        @return       true on success, otherwise false and error message
local function mac_address_table_aging_mode_func(params)
    -- Common variables declaration
    local result, values
    local devNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local aging_mode_setting_condition, aging_mode_setting_warning_string
    local aging_mode, aging_mode_string
    
    -- Common variables initialization
    command_data:initAllDeviceRange(params)    
    
    -- Command specific variables initialization 
    aging_mode_string = 
        command_data:getValueIfFlagNoIsNil(
            params, params["mac_address_table_aging_mode"], "no-removal")
    aging_mode  = fdbActionModeGet(aging_mode_string)

    -- Main device handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()
            
            command_data:clearLocalStatus()
            
            -- Mac-address aging mode setting condition determunation.
            if true == command_data["local_status"]         then            
                result, values, aging_mode_setting_warning_string = wrLogWrapper("wrlDxChMacAddressAgingModeConditionChecking", 
                                                                                    "(devNum, aging_mode)", devNum, aging_mode)
                if        0 == result                   then
                    aging_mode_setting_condition    = values
                elseif 0x10 == result                   then
                    command_data:setFailDeviceAndLocalStatus() 
                    command_data:addWarning("It is not allowed to check " ..
                                            "setting of the mac-address " .. 
                                            "table aging mode \"%s\" " ..
                                            "on device %d.", 
                                            actionModeStrGet(aging_mode), 
                                            devNum)                  
                elseif    0 ~= result                   then
                    command_data:setFailDeviceAndLocalStatus()    
                    command_data:addError("Error at checking of setting of " ..
                                          "the mac-address table aging mode " ..
                                          "\"%s\" on device %d: %s.", 
                                          actionModeStrGet(aging_mode), devNum, 
                                          values)                        
                end
            end            
            
            -- Mac-address aging mode setting condition checking.
            if (true  == command_data["local_status"])      and
               (false == aging_mode_setting_condition)      then  
                command_data:setFailDeviceStatus()
                command_data:addWarning("It is not allowed to set aging " ..
                                        "mode \"%s\" of the mac-address " ..
                                        "table on device %d: %s.", 
                                        actionModeStrGet(aging_mode), devNum, 
                                        aging_mode_setting_warning_string)                 
            end
            
            --  Configuring of the appDemo to support/not support the AA  
            --  messages to CPU.
            if (true == command_data["local_status"])       and
               (true == aging_mode_setting_condition)       then   
                result, values = 
                    myGenWrapper("appDemoSupportAaMessageSet",
                                 {{ "IN", "GT_BOOL", "supportAaMessage", false }})   
                if     0x10 == result then
                    command_data:setFailDeviceStatus() 
                    command_data:addWarning("Configuring of the appDemo to " ..
                                            "support/not support the AA " ..
                                            "messages to CPU is not allowed " ..
                                            "in device %d.", devNum) 
                elseif    0 ~= result then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at configuring of the " ..
                                          "appDemo to support/not support " ..
                                          "the AA messages to CPU on device " ..
                                          "%d: %s", devNum, returnCodes[result])
                end                                 
            end        
        
            -- Setting of the mac-address table aging mode.
            if (true == command_data["local_status"])       and
               (true == aging_mode_setting_condition)       then 
                result, values = 
                    myGenWrapper("cpssDxChBrgFdbActionModeSet",
                                 {{ "IN", "GT_U8",  "devNum",   devNum     },
                                  { "IN", "CPSS_FDB_ACTION_MODE_ENT",   
                                                    "mode",     aging_mode }})                                                             
                if     0x10 == result then
                    command_data:setFailDeviceStatus() 
                    command_data:addWarning("Setting of \"%s\" aging mode " ..
                                            "of the mac-address table is " ..
                                            "not allowed in device %d.", 
                                            actionModeStrGet(aging_mode), 
                                            devNum) 
                elseif    0 ~= result then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at setting of \"%s\" aging " ..
                                          "mode of the mac-address table " ..
                                          "on device %d: %s", 
                                          actionModeStrGet(aging_mode), devNum, 
                                          returnCodes[result])
                end                                 
            end                       
            
            -- Setting of Mac address table Automatic action mode.
            if (true == command_data["local_status"])       and
               (true == aging_mode_setting_condition)       then 
                result, values = 
                    cpssPerDeviceParamSet("cpssDxChBrgFdbMacTriggerModeSet",
                                          devNum, "CPSS_ACT_AUTO_E", "mode", 
                                          "CPSS_MAC_ACTION_MODE_ENT")
                if     0x10 == result then
                    command_data:setFailDeviceStatus() 
                    command_data:addWarning("Setting of Mac address table " ..
                                            "Automatic action mode is not " ..
                                            "allowed on device %d.", devNum) 
                elseif    0 ~= result then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at setting of Mac address " ..
                                          "table Automatic action mode on " ..
                                          "device %d: %s", devNum, 
                                          returnCodes[result])
                end                                 
            end

            command_data:updateStatus()             
            
            command_data:updateDevices()
        end
    end              
    
    command_data:analyzeCommandExecution()
       
    command_data:printCommandExecutionResults()
    
	return command_data:getCommandExecutionResults()     
end

CLI_type_dict["mac_address_table_aging_mode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "aging mode of the mac-address table",
    enum = {
        ["auto-removal"] = { value="auto_removal",
                help = "The dynamic addresses are deleted after they are aged-out" },
        ["no-removal"] = { value="no_removal",
                help = "The dynamic addresses are not deleted automatically" }
    }
}

--------------------------------------------------------------------------------
-- command registration: mac address-table aging-mode
--------------------------------------------------------------------------------
CLI_addCommand("config", "mac address-table aging-mode", {
  func   = mac_address_table_aging_mode_func,
  help   = "Setting the aging mode of the mac-address table",
  params = {
    { type = "values",
          "%mac_address_table_aging_mode"
    },
    { type = "named",  
          "#all_device",
      mandatory = { "all_device" }
    }
  }
})


--------------------------------------------------------------------------------
-- command registration: no mac address-table aging-mode
--------------------------------------------------------------------------------
CLI_addCommand("config", "no mac address-table aging-mode", {
  func   = function(params)
               params.flagNo = true
               return mac_address_table_aging_mode_func(params)
           end,
  help   = "Setting the aging mode of the mac-address table to default",
  params = {
      { type = "named",  
          "#all_device",
      mandatory = { "all_device" }
    }
  }
})
