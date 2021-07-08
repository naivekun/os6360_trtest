--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* clear_mac_address_table_dynamic.lua 
--*
--* DESCRIPTION:
--*       Removing of dynamic entries from the forwarding database
--*
--* FILE REVISION NUMBER:
--*       $Revision: 5 $
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
---
--  clear_mac_address_table_dynamic_func
--        @description  removes dynamic entries from the forwarding 
--                      database
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
--                                irrelevant;
--                                params["skipped"]: skipping property, 
--                                could be irrelevant;                      
--
--        @return        true on success, otherwise false and error message
--
local function clear_mac_address_table_dynamic_func(params)   
    -- Common variables declaration
    local result, values
    local devNum, entry_index
    local command_data = Command_Data()
    if (not params["static_dynamic_all"]) then
        params["static_dynamic_all"]="dynamic"
    end
    -- Common variables initialization
    command_data:initAllDeviceMacEntryIterator(params)

    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, entry_index in 
                                    command_data:getValidMacEntryIterator() do
            command_data:clearEntryStatus()     

            -- Mac address entry deleting.
            command_data:clearLocalStatus()    

            if true == command_data["local_status"] then   
                result, values = mac_entry_delete(devNum,
                    command_data["mac_entry"]["key"])
                if     0x10 == result then
                    command_data:setFailPortStatus() 
                    command_data:addWarning("Mac address entry deleting is not " ..
                                            "allowed in device %d entry index %d.", 
                                            devNum, entry_index)                     
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at deleting of %d mac address " ..
                                          "entry in device %d: %s", 
                                          entry_index, devNum, 
                                          returnCodes[result])
                end 
            end            
            
            command_data:updateStatus() 
            
            -- Mac address entry index invalidate.         
            command_data:clearLocalStatus()
            
            if true == command_data["local_status"] then   
                result, values = mac_entry_invalidate(devNum, entry_index)
                if     0x10 == result then
                    command_data:setFailDeviceStatus() 
                    command_data:addWarning("Mac address entry index " ..
                                            "%d invalidatind is not allowed " ..
                                            "on device %d.", entry_index, 
                                            devNum)                     
                elseif 0 ~= result then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at mac address entry index " ..
                                          "%d invalidating is not allowed " ..
                                          "on device %d: %s", entry_index, 
                                          devNum, returnCodes[result])
                end
            end              
            
            command_data:updateStatus() 
            
            command_data:updateEntries()
        end
    end            
        
    command_data:analyzeCommandExecution()
       
    command_data:printCommandExecutionResults()
    
	return command_data:getCommandExecutionResults()            
end


--------------------------------------------------------------------------------
-- command registration: clear mac address-table dynamic
--------------------------------------------------------------------------------
CLI_addHelp("exec", "clear mac", "MAC configuration")
CLI_addHelp("exec", "clear mac address-table", "Bridge MAC address table configuration")
CLI_addCommand("exec", "clear mac address-table", {
  func   = clear_mac_address_table_dynamic_func,
  help   = "dynamic addresses",
  params = {
      { type = "named",
          "#static_dynamic_all",
          "#all_device",
          "#mac-address",
          "#interface_port_channel",
          "#vlan",
          "#skipped",
        alt       = { address_interface_port_channel_vlan = 
                        { "mac-address", "ethernet", "port-channel", "vlan" }},
        mandatory = { "all_device", "address_interface_port_channel_vlan" }
    }
  }
})