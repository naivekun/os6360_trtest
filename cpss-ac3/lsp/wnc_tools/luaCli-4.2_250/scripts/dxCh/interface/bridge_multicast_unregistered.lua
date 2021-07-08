--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* bridge_multicast_unregistered.lua
--*
--* DESCRIPTION:
--*       configuring of the forwarding state of unregistered multicast addresses
--*
--* FILE REVISION NUMBER:
--*       $Revision: 4 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  bridge_multicast_unregistered_func
--        @description  configures of the forwarding state of unregistered 
--                      multicast addresses
--
--        @param params         - params["forward"]: unregistered multicast
--                                addresses forwarding property
--                                params["drop"]: unregistered multicast
--                                addresses droping property
--                                params["flagNo"]: no-flag property
--
--        @return       true on success, otherwise false and error message
--
local function bridge_multicast_unregistered_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration
    local filtering_enabling

    -- Common variables initialization
    command_data:initInterfaceRangeIterator()
    
    -- Command specific variables initialization.
    filtering_enabling = getTrueIfNotNil(params["drop"])
   
    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
                        
            -- Unregistered multicast filtering enabling.
            command_data:clearLocalStatus()
            
            if true == command_data["local_status"] then
                result, values = 
                    cpssPerPortParamSet("cpssDxChBrgPortEgrFltUregMcastEnable",
                                        devNum, portNum, 
                                        filtering_enabling, "enable", "GT_BOOL")   
                if     0x10 == result then
                    command_data:setFailPortStatus() 
                    command_data:addWarning("It is not allowed to %s egress " ..
                                            "Filtering for bridged Unregistered " ..
                                            "Multicast packets on device %d " ..
                                            "port %d.", 
                                            boolEnableLowerStrGet(filtering_enabling), 
                                            devNum, portNum)                                                   
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at %d of egress Filtering " ..
                                          "for bridged Unregistered Multicast " ..
                                          "packets on device %d port %d: %s.", 
                                          boolEnablingLowerStrGet(filtering_enabling), 
                                          portNum, devNum,
                                          returnCodes[result])
                end 
            end    
        
            command_data:updateStatus() 
      
            command_data:updatePorts()            
        end     

        command_data:addWarningIfNoSuccessPorts(
            "Can not configure the forwarding state of unregistered " ..
            "multicast of all processed ports.")
    end
    
    command_data:analyzeCommandExecution()
     
    command_data:printCommandExecutionResults()
           
    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: bridge multicast unregistered
--------------------------------------------------------------------------------
CLI_addHelp("interface", "bridge", 
                                    "Bridge configuration subcommand")
CLI_addHelp("interface", "bridge multicast", 
                                    "Multicast Bridging configuration commands")
CLI_addCommand("interface", "bridge multicast unregistered", {
  func   = bridge_multicast_unregistered_func,
  help   = "Enable filtering of unregistered multicast for a given egress port",
  params = {
      { type = "named",
          "#forward_drop",
        mandatory = { "forward_drop" }
    }
  }
})

--------------------------------------------------------------------------------
-- command registration: no bridge multicast unregistered
--------------------------------------------------------------------------------
CLI_addHelp("interface", "no bridge", 
                                    "Bridge configuration subcommand")
CLI_addHelp("interface", "no bridge multicast", 
                                    "Multicast Bridging configuration commands")
CLI_addCommand("interface", "no bridge multicast unregistered", {
  func   = function(params)
               params.flagNo = true
               return bridge_multicast_unregistered_func(params)
           end,
  help   = "Disable filtering of unregistered multicast for a given egress port"
})
