--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* back_pressure.lua
--*
--* DESCRIPTION:
--*       enabling of the back pressure
--*
--* FILE REVISION NUMBER:
--*       $Revision: 6 $
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
---
--  back_pressure_func
--        @description  enabling of the back pressure of a given interface
--                      running at half-duplex mode
--
--        @param params         - params["vlan-range"]
--
--        @usage __global       - __global["ifRange"]: iterface range 
--
--        @return       true on success, otherwise false and error message
--
local function back_pressure_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration
    local port_duplex 
    local back_pressure_changing, back_pressure
    
    -- Command specific variables initialization    
    if nil == params.flagNo then
        back_pressure_changing  = true
        back_pressure           = true
    else
        back_pressure_changing  = true
        back_pressure           = false
    end    
    
    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
            command_data:clearPortStatus()
        
            -- Port duplex cheking.
            command_data:clearLocalStatus()
            
            result, values = cpssPerPortParamGet("cpssDxChPortDuplexModeGet",
                                                 devNum, portNum, "dMode", 
                                                 "CPSS_PORT_DUPLEX_ENT")
            if 0 == result then 
                port_duplex = values["dMode"]
            else
                command_data:setFailPortAndLocalStatus()
                command_data:addError("Error at duplex mode getting of device " ..
                                      "%d port %d: %s", devNum, portNum, 
                                      returnCodes[result])
            end  

            -- Port back pressure setting.
            if (true == command_data["local_status"]) and 
               (true == back_pressure_changing) then
                if "CPSS_PORT_HALF_DUPLEX_E" == port_duplex then
                    result, values = 
                        cpssPerPortParamSet("cpssDxChPortBackPressureEnableSet",
                                            devNum, portNum, back_pressure, 
                                            "state", "GT_BOOL")
                    if     0x10 == result then
                        command_data:setFailPortStatus() 
                        command_data:addWarning("It is not allowed to %s back " ..
                                                "pressure for device %d port %d.", 
                                                boolPredicatStrGet(back_pressure), 
                                                devNum, portNum)                                                   
                        elseif 0 ~= result then
                            command_data:setFailPortAndLocalStatus()
                            command_data:addError("Error at back pressure setting " ..
                                                  "of device %d port %d: %s", 
                                                  devNum, portNum, 
                                                  returnCodes[result])
                        end                                                         
                else
                    command_data:setFailPortStatus() 
                    command_data:addWarning("It is impossible to set a back " ..
                                            "pressure of device %d port %d, " ..
                                            "because port has full duplex mode.",
                                            devNum, portNum)                 
                end
            end    
        
            command_data:updateStatus()
               
            command_data:updatePorts() 
        end     

        command_data:addWarningIfNoSuccessPorts("Can not set back pressure for " ..
                                                "all processed ports.")
    end
    
    command_data:analyzeCommandExecution()
     
    command_data:printCommandExecutionResults()
           
    return command_data:getCommandExecutionResults()       
end


--------------------------------------------
-- command registration: back-pressure
--------------------------------------------
CLI_addCommand("interface", "back-pressure", {
  func=back_pressure_func,
  help="Enable back-pressure"
})
CLI_addCommand("interface", "no back-pressure", {
  func=function(params)
      params.flagNo=true
      return back_pressure_func(params)
  end,
  help="Disable back-pressure"
})
