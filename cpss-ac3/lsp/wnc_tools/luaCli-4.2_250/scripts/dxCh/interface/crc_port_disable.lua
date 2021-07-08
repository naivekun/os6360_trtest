--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* crc_port_disable.lua
--*
--* DESCRIPTION:
--*       enabling/disabling of CRC check on a specific port (or a range of 
--*       ports)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
---
--  crc_port_disable_func
--        @description  enables/disables CRC check on a specific port (or a 
--                      range of ports) 
--
--        @param params         - params["flagNo"]: no-flag
--
--        @return       true on success, otherwise false and error message
--
local function crc_port_disable_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration
    local crc_enable

    -- Common variables initialization
    command_data:initInterfaceDevPortRange()
    
    -- Command specific variables initialization.
    crc_enable = command_data:getFalseIfFlagNoIsNil(params)
        
    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
                       
            -- The CRC checking enabling.
            command_data:clearLocalStatus()
            
            if true == command_data["local_status"] then
    			result, values = 
                    cpssPerPortParamSet("cpssDxChPortCrcCheckEnableSet",
                                        devNum, portNum, crc_enable, "enable", 
                                        "GT_BOOL")   
                if     0x10 == result then
                    command_data:setFailPortStatus() 
                    command_data:addWarning("It is not allowed to %s CRC " ..
                                            "checking on device %d port %d.", 
                                            boolEnableLowerStrGet(crc_enable), 
                                            portNum, devNum)                                                   
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at %s of CRC checking " ..
                                          "on device %d port %d: %s.", 
                                          boolEnablingLowerStrGet(crc_enable), 
                                          portNum, devNum, returnCodes[result])
                end 
            end    
        
            command_data:updateStatus() 
      
            command_data:updatePorts()            
        end     

        command_data:addWarningIfNoSuccessPorts(
            "Can not %s CRC checking of all processed ports.", 
            boolEnableLowerStrGet(crc_enable))
    end
    
    command_data:analyzeCommandExecution()
     
    command_data:printCommandExecutionResults()
           
    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------
-- command registration: crc port disable
--------------------------------------------------------
CLI_addHelp("interface", "crc", "CRC subcommads")
CLI_addHelp("interface", "crc port", "CRC port subcommads")
CLI_addCommand("interface", "crc port disable", {
  func   = crc_port_disable_func,
  help   = "Disabling of CRC check on a specific port (or a range of ports)"
})

--------------------------------------------------------
-- command registration: no crc port disable
--------------------------------------------------------
CLI_addHelp("interface", "no crc", "CRC no-subcommads")
CLI_addHelp("interface", "no crc port", "CRC port no-subcommads")
CLI_addCommand("interface", "no crc port disable", {
  func   = function(params) 
               params.flagNo = true
               crc_port_disable_func(params)
           end,
  help   = "Enabling of CRC check on a specific port (or a range of ports)"
})
