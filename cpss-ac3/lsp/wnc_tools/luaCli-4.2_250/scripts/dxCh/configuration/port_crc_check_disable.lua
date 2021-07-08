--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* port_crc_check.lua
--*
--* DESCRIPTION:
--*       disabling of CRC check
--*
--* FILE REVISION NUMBER:
--*       $Revision: 4 $
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
---
--  port_crc_check_disable_func
--        @description  disables of CRC check
--
--        @param params         - params["flagNo"]: no-flag
--
--        @usage __global       - __global["ifRange"]: iterface range 
--
--        @return       true on success, otherwise false and error message
--
local function port_crc_check_disable_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration
    local crc_check_changing, crc_check
    
    -- Command specific variables initialization    
    if nil == params.flagNo then
        crc_check_changing  = true
        crc_check           = false
    else
        crc_check_changing  = true
        crc_check           = true
    end    
    
    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
            command_data:clearPortStatus()
        
            -- CRC check disabling.
            command_data:clearLocalStatus()
            
            if (true == command_data["local_status"]) and 
               (true == crc_check_changing) then
                result, values = cpssPerPortParamSet("cpssDxChPortCrcCheckEnableSet",
                                                     devNum, portNum, 
                                                     crc_check, "enable", 
                                                     "GT_BOOL")                                                 
                if     0x10 == result then
                    command_data:setFailPortStatus() 
                    command_data:addWarning("It is not allowed to %s crc check " ..
                                           "for device %d port %d.", 
                                           boolPredicatStrGet(crc_check), devNum, 
                                           portNum)                                                   
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at crc check setting of device " ..
                                          "%d port %d: %s", devNum, portNum, 
                                          returnCodes[result])
                end                                                         
            end    
        
            command_data:updateStatus()
               
            command_data:updatePorts() 
        end     

        command_data:addWarningIfNoSuccessPorts("Can not set crc check for all " ..
                                                "processed ports.")
    end
    
    command_data:analyzeCommandExecution()
     
    command_data:printCommandExecutionResults()
           
    return command_data:getCommandExecutionResults()       
end


--------------------------------------------
-- command registration: port crc check disable
--------------------------------------------
CLI_addCommand("interface", "port crc check disable", {
  func=port_crc_check_disable_func,
  help="Disabling of CRC check"
})
CLI_addCommand("interface", "no port crc check disable", {
  func=function(params)
      params.flagNo=true
      return port_crc_check_disable_func(params)
  end,
  help="Enabling of CRC check"
})
