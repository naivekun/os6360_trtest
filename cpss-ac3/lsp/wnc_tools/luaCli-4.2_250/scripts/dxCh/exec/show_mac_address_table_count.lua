--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_mac_address_table_count.lua
--*
--* DESCRIPTION:
--*       Showing of the number of addresses present in the Forwarding Database
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssDxChBrgFdbMacAddressTableCountGet")

--constants 


-- ************************************************************************
---
--  show_show_mac_address_table_count_func
--        @description  show's of the number of addresses present in the 
--                      Forwarding Database
--
--        @param params         - params["all_device"]: all or given 
--                                devices iterating property, could be 
--                                irrelevant
--                                params["mac-address"]: mac-address, could 
--                                be irrelevant;
--                                params["ethernet"]: interface range, 
--                                could be irrelevant;
--                                params["port-channel"]: trunk id,could be 
--                                irrelevant;
--                                params["vlan"]: vlan Id, could be 
--                                irrelevant
--
--        @return        true on success, otherwise false and error message
--
local function show_show_mac_address_table_count_func(params)
    -- Common variables declaration
    local result, values
    local devNum, entry_index
    local command_data = Command_Data()
    -- Command specific variables declaration
    local statistic_table
    local mac_address_tables_total_entries_count_string_prefix
    local mac_address_tables_total_entries_count_string_suffix 
    local mac_address_tables_total_entries_count
    local mac_address_tables_total_entries_count_string
    local mac_address_tables_free_entries_count_string_prefix
    local mac_address_tables_free_entries_count_string_suffix  
    local mac_address_tables_free_entries_count
    local mac_address_tables_free_entries_count_string 
    local mac_address_tables_used_entries_count_string_prefix
    local mac_address_tables_used_entries_count_string_suffix 
    local mac_address_tables_used_entries_count
    local mac_address_tables_used_entries_count_string
    local mac_address_tables_static_entries_count_string_prefix
    local mac_address_tables_static_entries_count_string_suffix  
    local mac_address_tables_static_entries_count
    local mac_address_tables_static_entries_count_string 
    local mac_address_tables_dynamic_entries_count_string_prefix
    local mac_address_tables_dynamic_entries_count_string_suffix 
    local mac_address_tables_dynamic_entries_count
    local mac_address_tables_dynamic_entries_count_string
    local mac_address_tables_skipped_entries_count_string_prefix
    local mac_address_tables_skipped_entries_count_string_suffix 
    local mac_address_tables_skipped_entries_count 
    local mac_address_tables_skipped_entries_count_string    
    local header_string, footer_string  
   
    -- Common variables initialization
    command_data:initAllDeviceRange(params)    
    command_data:initAllDeviceMacEntryIterator(params)
    command_data:clearResultArray()
    
    -- Command specific variables initialization 
    statistic_table                                         = {["total"]    = 0, 
                                                               ["free"]     = 0, 
                                                               ["used"]     = 0, 
                                                               ["static"]   = 0, 
                                                               ["dynamic"]  = 0, 
                                                               ["skipped"]  = 0}    
    header_string                                           = ""
    mac_address_tables_total_entries_count_string_prefix    = "Capacity:\t\t"
    mac_address_tables_total_entries_count_string_suffix    = ""
    mac_address_tables_total_entries_count                  = 0
    mac_address_tables_total_entries_count_string           = "n/a"
    mac_address_tables_free_entries_count_string_prefix     = "Free:\t\t\t"
    mac_address_tables_free_entries_count_string_suffix     = ""
    mac_address_tables_free_entries_count                   = 0
    mac_address_tables_free_entries_count_string            = "n/a"
    mac_address_tables_used_entries_count_string_prefix     = "Used:\t\t\t"
    mac_address_tables_used_entries_count_string_suffix     = "\n"
    mac_address_tables_used_entries_count                   = 0
    mac_address_tables_used_entries_count_string            = "n/a"
    mac_address_tables_static_entries_count_string_prefix   = 
                                                        "Static addresses:\t"
    mac_address_tables_static_entries_count_string_suffix   = ""
    mac_address_tables_static_entries_count                 = 0
    mac_address_tables_static_entries_count_string          = "n/a"
    mac_address_tables_dynamic_entries_count_string_prefix  = 
                                                        "Dynamic addresses:\t"
    mac_address_tables_dynamic_entries_count_string_suffix  = ""
    mac_address_tables_dynamic_entries_count                = 0
    mac_address_tables_dynamic_entries_count_string         = "n/a"
    mac_address_tables_skipped_entries_count_string_prefix  = 
                                                        "Skipped addresses:\t"
    mac_address_tables_skipped_entries_count_string_suffix  = ""
    mac_address_tables_skipped_entries_count                = 0
    mac_address_tables_skipped_entries_count_string         = "n/a"
    footer_string                                           = "\n"    
    
    -- Main device handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()
            
            command_data:clearLocalStatus()
            
            if true == command_data["local_status"] then            
                local mac_aged_ = command_data["mac_aged"]
                local mac_filter_ = command_data["mac_filter"]
                
                result, values = wrLogWrapper("wrlCpssDxChBrgFdbMacAddressTableCountGet", 
                                              "(devNum, 0, statistic_table, mac_aged_, mac_filter_)",
                                              devNum, 0, statistic_table, mac_aged_, mac_filter_)
                if        0 == result then
                    statistic_table = values
                elseif 0x10 == result then
                    command_data:setFailDeviceStatus() 
                    command_data:addWarning("Mac address-table count statistic " ..
                                            "getting is not allowed in device %d.", 
                                            devNum)                     
                elseif    0 ~= result then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at mac address-table count " ..
                                          "statistic getting is not allowed in " ..
                                          "device %d: %s", devNum, 
                                          returnCodes[result])
                end                        
            end
            
            command_data:updateStatus()             
            
            command_data:updateDevices()
        end
    end    
   
    -- Mac address-tables total entries count string forming.
    if true  == command_data["status"] then  
        mac_address_tables_total_entries_count          = 
                                                        statistic_table["total"]
        mac_address_tables_total_entries_count_string   = 
                                tostring(mac_address_tables_total_entries_count)
    end    
    
    -- Mac address-tables total entries count string formatting and adding.
    command_data:setResultStr(mac_address_tables_total_entries_count_string_prefix, 
                              mac_address_tables_total_entries_count_string,
                              mac_address_tables_total_entries_count_string_suffix)
    command_data:addResultToResultArray()
    
    -- Mac address-tables free entries count string forming.
    if true  == command_data["status"] then
        mac_address_tables_free_entries_count           = 
                                                        statistic_table["free"]
        mac_address_tables_free_entries_count_string    = 
                                tostring(mac_address_tables_free_entries_count)
    end
    
    -- Mac address-tables free entries count string formatting and adding.
    command_data:setResultStr(mac_address_tables_free_entries_count_string_prefix, 
                              mac_address_tables_free_entries_count_string,
                              mac_address_tables_free_entries_count_string_suffix)
    command_data:addResultToResultArray()
    
    -- Mac address-tables used entries count string forming.
    if true  == command_data["status"] then
        mac_address_tables_used_entries_count           = 
                                                        statistic_table["used"]
        mac_address_tables_used_entries_count_string    = 
                                tostring(mac_address_tables_used_entries_count)
    end
    
    -- Mac address-tables used entries count string formatting and adding.
    command_data:setResultStr(mac_address_tables_used_entries_count_string_prefix, 
                              mac_address_tables_used_entries_count_string,
                              mac_address_tables_used_entries_count_string_suffix)
    command_data:addResultToResultArray()    
    
    -- Mac address-tables static entries count string forming.
    if true  == command_data["status"] then
        mac_address_tables_static_entries_count         = 
                                                    statistic_table["static"]
        mac_address_tables_static_entries_count_string  = 
                            tostring(mac_address_tables_static_entries_count)
    end
    
    -- Mac address-tables static entries count string formatting and adding.
    command_data:setResultStr(mac_address_tables_static_entries_count_string_prefix, 
                              mac_address_tables_static_entries_count_string,
                              mac_address_tables_static_entries_count_string_suffix)
    command_data:addResultToResultArray()
        
    -- Mac address-tables dynamic entries count string forming.
    if true  == command_data["status"] then
        mac_address_tables_dynamic_entries_count        = 
                                                    statistic_table["dynamic"]
        mac_address_tables_dynamic_entries_count_string = 
                            tostring(mac_address_tables_dynamic_entries_count)
    end
    
    -- Mac address-tables dynamic entries count string formatting and adding.
    command_data:setResultStr(mac_address_tables_dynamic_entries_count_string_prefix, 
                              mac_address_tables_dynamic_entries_count_string,
                              mac_address_tables_dynamic_entries_count_string_suffix)
    command_data:addResultToResultArray()
    
    -- Mac address-tables skipped entries count string forming.
    if true  == command_data["status"] then
        mac_address_tables_skipped_entries_count        = 
                                                    statistic_table["skipped"]
        mac_address_tables_skipped_entries_count_string = 
                            tostring(mac_address_tables_skipped_entries_count)
    end
    
    -- Mac address-tables skipped entries count string formatting and adding.
    command_data:setResultStr(mac_address_tables_skipped_entries_count_string_prefix, 
                              mac_address_tables_skipped_entries_count_string,
                              mac_address_tables_skipped_entries_count_string_suffix)
    command_data:addResultToResultArray()    
    
    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr()
    command_data:setResultStrOnDevicesCount(header_string, command_data["result"], 
                                            footer_string,
                                            "There is no mac address information to show.\n")
        
    command_data:analyzeCommandExecution()
       
    command_data:printCommandExecutionResults()
    
	return command_data:getCommandExecutionResults()            
end


------------------------------------------------------
-- command registration: show mac address-table count
------------------------------------------------------
CLI_addCommand("exec", "show mac address-table count", {
  func   = show_show_mac_address_table_count_func,
  help   = "MAC addresses count",
  params = {
      { type = "named",
          "#all_device",
          "#vlan",
          "#interface_port_channel", 
          "#static_dynamic",
        mandatory = { "all_device" }
    }
  }
})