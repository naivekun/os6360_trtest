--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* duplex.lua
--*
--* DESCRIPTION:
--*       setting of port duplex mode  
--*
--* FILE REVISION NUMBER:
--*       $Revision: 6 $
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
---
--  duplex_func
--        @description  sets port duplex mode
--
--        @param params         - params["port_duplex"]: duplex mode 
--                                string: half/full
--
--        @usage __global       - __global["ifRange"]: iterface range 
--
--        @return       true on success, otherwise false and error message
--
local function duplex_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local port_duplex_autonegotiation, port_duplex_changing, port_duplex    
    
    -- Command specific variables initialization
    if nil == params.flagNo then
        port_duplex_changing = true
        if     "half" == params["port_duplex"] then
            port_duplex = "CPSS_PORT_HALF_DUPLEX_E"
        elseif "full" == params["port_duplex"] then
            port_duplex = "CPSS_PORT_FULL_DUPLEX_E"
        end
    else
        port_duplex_changing = false
    end
    
    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
            command_data:clearPortStatus()   

            -- Port duplex autonegatiation mode check.
            command_data:clearLocalStatus() 
            result, values = cpssPerPortParamGet("cpssDxChPortDuplexAutoNegEnableGet",
                                                 devNum, portNum, "state", 
                                                 "GT_BOOL")
            if 0 == result then 
                port_duplex_autonegotiation = values["state"]                                                 
            else
                command_data:setFailPortAndLocalStatus()
                command_data:addError("Error at duplex auto-negatiation setting " ..
                                      "of device %d port %d: %s", devNum, 
                                      portNum, returnCodes[result])
            end    

            -- Port duplex setting.            
            if (true == command_data["local_status"]) and
               (true == port_duplex_changing) then
                if false == port_duplex_autonegotiation then
                    result, values = cpssPerPortParamSet("cpssDxChPortDuplexModeSet",
                                                         devNum, portNum, 
                                                         port_duplex, "dMode", 
                                                         "CPSS_PORT_DUPLEX_ENT")
                    if     0x10 == result then
                        command_data:setFailPortStatus()   
                        command_data:addWarning("It is not allowed to set %s " ..
                                                "duplex for device %d port %d.", 
                                                ENUM["CPSS_PORT_DUPLEX_ENT"][port_duplex], 
                                                devNum, portNum)                        
                    elseif 0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error at duplex setting of device " ..
                                              "%d port %d: ", devNum, portNum, 
                                              returnCodes[result])
                    end
                else
                    command_data:setFailPortStatus() 
                    command_data:addWarning("It is impossible to set a duplex " ..
                                            "mode of device %d port %d, " ..
                                            "because port has duplex " ..
                                            "auto-negatiation mode.", devNum, 
                                            portNum) 
                end
            end
            
            command_data:updateStatus()
               
            command_data:updatePorts() 
        end     

        command_data:addWarningIfNoSuccessPorts("Can not set duplex for all " ..
                                                "processed ports.")
    end        

    -- Command data postprocessing    
    command_data:analyzeCommandExecution()
     
    command_data:printCommandExecutionResults()
           
    return command_data:getCommandExecutionResults()   
end


--------------------------------------------
-- command registration: duplex
--------------------------------------------
CLI_addCommand("interface", "duplex", {
  func=duplex_func,
  help="Configure duplex operation",
  params={
    { type="named",
      { format="half", name="port_duplex", help="Force half-duplex operation"},
      { format="full", name="port_duplex", help="Force full duplex operation"},
      mandatory = { "port_duplex" }
    }
  }
})
CLI_addCommand("interface", "no duplex", {
  func=function(params)
      params.flagNo=true
      return duplex_func(params)
  end,
  help="Configure duplex operation"
})
