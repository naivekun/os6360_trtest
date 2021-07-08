--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_mac_address_table_aging.lua
--*
--* DESCRIPTION:
--*       showing of the mode and aging time of addresses present in the  
--*       Forwarding Database
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  Command_Data:show_mac_address_table_aging_func
--        @description  show's of the mode and aging time of addresses  
--                      present in the Forwarding Database
--
--        @param params         - params["all_device"]: all or given devices
--                                devices iterating property
--
--        @return       true on success, otherwise false and error message
--
local function show_mac_address_table_aging_func(params)
    -- Common variables declaration
    local result, values
    local devNum
    local command_data = Command_Data()
  
    -- Common variables initialization
    command_data:initAllDeviceRange(params)
    command_data:clearResultArray()

    local table_info = {
        { head="Device",len=6,path="devNum" },
        { head="Aging Mode",len=18,align="c",path="mode"},
        { head="Aging Time",len=12,align="c",path="time"}
    }

    command_data:initTable(table_info)
    
    -- Main device handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()     

            command_data:clearLocalStatus()

            local entry = {devNum=devNum,mode="n/a",time="n/a"}
            -- Aging mode getting.
            if true == command_data["local_status"] then   
                result, values = cpssPerDeviceParamGet(
                                    "cpssDxChBrgFdbActionModeGet", devNum, 
                                    "mode", "CPSS_FDB_ACTION_MODE_ENT")
                command_data:handleCpssErrorDevice(result,
                        "get Mac-address table aging mode", devId)
                if        0 == result then 
                    entry.mode = actionModeStrGet(values["mode"])
                end                  
            end               
            
            -- Aging time getting.
            if true == command_data["local_status"] then   
                result, values = cpssPerDeviceParamGet(
                                    "cpssDxChBrgFdbAgingTimeoutGet", devNum, 
                                    "timeout", "GT_U32")
                command_data:handleCpssErrorDevice(result,
                        "get Mac-address table aging timeout", devId)
                if        0 == result then 
                    entry.time = values.timeout
                end
            end            
            
            command_data:updateStatus()             

            command_data:addTableRow(entry)
            
            command_data:updateEntries()
        end
    end

    -- Resulting table string formatting.
    command_data:showTblResult("There is no mac address aging-times to show.")
    
	return command_data:getCommandExecutionResults()            
end


-----------------------------------------------------------
-- command registration: show mac address-table aging
-----------------------------------------------------------
CLI_addCommand("exec", "show mac address-table aging", {
  func   = show_mac_address_table_aging_func,
  help   = "Showing of the mode and aging time of addresses present in the " ..
           "Forwarding Database.",
  params = {
      { type="named",
          "#all_device",
          mandatory = {"all_device"}
      }
  }
})
