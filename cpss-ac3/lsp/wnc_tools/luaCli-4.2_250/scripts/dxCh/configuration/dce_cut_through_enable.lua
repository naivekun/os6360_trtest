--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* dce_qcn_cnm_priority.lua
--*
--* DESCRIPTION:
--*       enabling/disabling of cut-through
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  dce_cut_through_enable_func_config
--        @description  enables/disables of cut-through
--
--        @param params         - params["flagNo"]: no command property
--
--        @return       true on success, otherwise false and error message
--
local function dce_cut_through_enable_func_config(params)
    -- Common variables declaration.
    local result, values
    local devFamily, devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration.
    local cut_through_enabling
    
    -- Common variables initialization.
    command_data:initAllDeviceRange(params)
    
    -- Command specific variables initialization.
    cut_through_enabling    = command_data:getTrueIfFlagNoIsNil(params)

    -- Main port handling cycle.
    if true == command_data["status"]               then
        local iterator
        for iterator, devNum, portNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()  
                    
            command_data:clearLocalStatus() 
            devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)
            
            if ("CPSS_PP_FAMILY_DXCH_LION_E"  == devFamily)     or
               ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily)     then
                command_data:setFailDeviceStatus()
                command_data:addWarning("Command will be implementer later")   
            else
                command_data:setFailDeviceStatus()
                command_data:addWarning("Family of device %d does not " ..
                                        "supported.", devNum)
            end
                
            command_data:updateStatus()
                        
            command_data:updateDevices()                
        end
        
        command_data:addWarningIfNoSuccessDevices(
            "Can not %s cut-through on all processed devices.", 
            boolEnableLowerStrGet(cut_through_enabling))
    end       
    
    command_data:analyzeCommandExecution()
     
    command_data:printCommandExecutionResults()
           
    return command_data:getCommandExecutionResults()                
end


--------------------------------------------------------------------------------
-- command registration: dce cut-through enable
--------------------------------------------------------------------------------
CLI_addHelp("config",       "dce cut-through",      "Cut-through subcommands")
CLI_addCommand("config",    "dce cut-through enable", {
  func   = dce_cut_through_enable_func_config,
  help   = "Enabling of cut-through",
  params = {
    { type = "named",
          "#all_device",
        mandatory = { "all_device" }
    }
  }
})

--------------------------------------------------------------------------------
-- command registration: no dce cut-through enable
--------------------------------------------------------------------------------
CLI_addHelp("config",       "no dce cut-through",   "Cut-through subcommands")
CLI_addCommand("config",    "no dce cut-through enable", {
  func   = function(params) 
               params.flagNo = true
               dce_cut_through_enable_func_config(params)
           end,
  help   = "Disabling of cut-through",
  params = {
    { type = "named",
          "#all_device",
        mandatory = { "all_device" }
    }
  }
})
