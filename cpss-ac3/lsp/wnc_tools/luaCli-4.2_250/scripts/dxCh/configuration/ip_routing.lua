--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ip_routing.lua
--*
--* DESCRIPTION:
--*       enabling/disabling of routing globally
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  config_ip_routing_func
--        @description  enables/disables of routing globally
--
--        @param params         - params["flagNo"]: no-flag
--
--        @return       true on success, otherwise false and error message
--
local function config_ip_routing_func(params)
    -- Common variables declaration
    local result, values
    local devNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local enable_routing
    
    -- Common variables initialization
    command_data:initAllDeviceRange(params)    

    -- Command specific variables initialization 
    enable_routing  = getFalseIfNotNil(params["flagNo"])
    
    -- Main device handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in  command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()  
                    
            command_data:clearLocalStatus()                    

            -- Routing enabling setting.
            if true  == command_data["local_status"] then                        
                result, values = 
                    cpssPerDeviceParamSet("cpssDxChIpRoutingEnable",
                                          devNum, enable_routing, 
                                          "enableRouting", "GT_BOOL")
                                 
                if     0x10 == result then
                    command_data:setFailDeviceStatus() 
                    command_data:addWarning("Routing %s is not allowed on " ..
                                            "device %d.", 
                                            boolEnablingLowerStrGet(enable_routing), 
                                            devNum)                     
                elseif 0x1E == result then
                    command_data:setFailDeviceStatus() 
                    command_data:addWarning("Routing %s is not supported on " ..
                                            "device %d.", 
                                            boolEnablingLowerStrGet(enable_routing), 
                                            devNum)                                             
                elseif    0 ~= result then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at routing %s on device %d: %s.", 
                                          boolEnablingLowerStrGet(enable_routing),
                                          devNum, returnCodes[result])
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


--------------------------------------------------------------------------------
-- command registration: ip routing
--------------------------------------------------------------------------------
CLI_addHelp("config", "ip", "Global IP configuration commands")
CLI_addCommand("config", "ip routing", {
  func   = config_ip_routing_func,
  help   = "Enabling of routing globally",
  params = {
      { type = "named",
          "#all_device",
        mandatory = { "all_device" }
      }
  }
})

--------------------------------------------------------------------------------
-- command registration: no ip routing
--------------------------------------------------------------------------------
CLI_addHelp("config", "no ip", "Global IP configuration commands")
CLI_addCommand("config", "no ip routing", {
  func   = function(params)
               params.flagNo = true
               return config_ip_routing_func(params)
           end,
  help   = "Disabling of routing globally",
  params = {
      { type = "named",
          "#all_device",
        mandatory = { "all_device" }
      }
  }
})
