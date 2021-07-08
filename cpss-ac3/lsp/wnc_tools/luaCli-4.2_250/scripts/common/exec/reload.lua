--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* reload.lua
--*
--* DESCRIPTION:
--*       reloading of the device
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
--        @description  reload's of the device 
--
--        @return       true on success, otherwise false and error message
--
local function reload_func(params)
    -- Common variables declaration
    local result, values
    local devNum
    local command_data = Command_Data()
    
    -- Common variables initialization  
    command_data:initAllAvailableDevicesRange()    
    
    -- Main device handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()      
    
             -- Device soft reset. 
            command_data:clearLocalStatus()
                    
            if true  == command_data["local_status"] then
                if isPortExtender(devNum) == true then
                    result, values = myGenWrapper("cpssPxHwPpSoftResetTrigger", 
                            {{ "IN",  "GT_SW_DEV_NUM", "devNum", devNum }})
                else
                    result, values = myGenWrapper("cpssDxChHwPpSoftResetTrigger", 
                            {{ "IN",  "GT_U8", "devNum", devNum }})
                end
                if     0x10 == result then
                    command_data:setFailPortStatus() 
                    command_data:addWarning("It is not allowed to do soft " ..
                                            "reset of device %d.", devNum)                                                   
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at soft reset of device %d: %s.", 
                                          devNum, returnCodes[result])
                end         
            end
            
            command_data:updateStatus()                  
            
            command_data:updateDevices()            
        end
        
        command_data:addWarningIfNoSuccessDevices(
            "Can not reset all processed devices.")        
    end 

    command_data:analyzeCommandExecution()
    
    command_data:printCommandExecutionResults()
    
    return command_data:getCommandExecutionResults()    
end


--------------------------------------------------------
-- command registration: reload
--------------------------------------------------------
CLI_addCommand("exec", "reload", {
  func   = reload_func,
  help   = "Reload all stack units or specific unit"
})
