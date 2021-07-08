--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* negotiation.lua
--*
--* DESCRIPTION:
--*       enabling/disabling of auto negotiation operation for the port speed  
--*       and duplex.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 8 $
--*
--********************************************************************************

--includes


--constants
local default_port_speed_autonegotiation  = true
local default_port_duplex_autonegotiation = true


-- ************************************************************************
---
--  negotiation_func
--        @description  enables auto negotiation operation for the port 
--                      speed  and duplex parameters; disables auto 
--                      negotiation operation for the port speed and duplex 
--                      parameters and sets it 
--
--        @param params         - params["port_speed"]: speed/duplex_letter 
--                                string 10h/10f/100h/100f/1000f/10000f 
--                                (optional)
--
--        @usage __global       - __global["ifRange"]: iterface range 
--
--        @return       true on success, otherwise false and error message
--
local function negotiation_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local port_speed_autonegotiation, port_speed_changing, port_speed
    local port_interface_mode_changing, port_interface_mode
    local port_duplex_autonegotiation, port_duplex_changing, port_duplex
    
    -- Command specific variables initialization
    if (nil == params.flagNo) and (nil == params["port_speed"]) then
        port_speed_autonegotiation  = default_port_speed_autonegotiation   
        port_duplex_autonegotiation = default_port_duplex_autonegotiation
    else
        port_speed_autonegotiation  = false   
        port_duplex_autonegotiation = false
    end    
    
    if true == command_data["status"] then
        if (nil == params.flagNo) and (false == port_speed_autonegotiation) then
            local port_speed_string = string.sub(params["port_speed"], 1, 
                                                 string.len(params["port_speed"]) - 1)
                                                     
            port_speed_changing = true
            if     "10"    == port_speed_string then
                port_speed = "CPSS_PORT_SPEED_10_E"
            elseif "100"   == port_speed_string then
                port_speed = "CPSS_PORT_SPEED_100_E"
            elseif "1000"  == port_speed_string then
                port_speed = "CPSS_PORT_SPEED_1000_E"
            elseif "10000" == port_speed_string then
                port_speed = "CPSS_PORT_SPEED_10000_E"
            else
                command_data:setFailStatus()
                command_data:addError("Can not recognize port speed")
            end
        else
            port_speed_changing = false
        end
    end
    
    if true == command_data["status"] then
        if (nil == params.flagNo) and (false == port_speed_autonegotiation) and
           (LION_DEVFAMILY ~= DeviceFamily) then
            port_interface_mode_changing = false
        else
            port_interface_mode_changing = true
        end
    end
    
    if true == command_data["status"] then
        if (nil == params.flagNo) and (false == port_duplex_autonegotiation) then
            local port_duplex_symbol = string.sub(params["port_speed"], 
                                                  string.len(params["port_speed"]), 
                                                  string.len(params["port_speed"]))
                                                  
            port_duplex_changing = true
            if     "h" == port_duplex_symbol then
                port_duplex = "CPSS_PORT_HALF_DUPLEX_E"
            elseif "f" == port_duplex_symbol then
                port_duplex = "CPSS_PORT_FULL_DUPLEX_E"
            else
                command_data:setFailStatus()
                command_data:addError("Can not recognize duplex mode")
            end
        else
            port_duplex_changing = false
        end
    end
    
    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
            command_data:clearPortStatus()               
                                                    
            -- Port speed autonegatiation mode setting.
            command_data:clearLocalStatus()  
                    
            result, values = cpssPerPortParamSet("cpssDxChPortSpeedAutoNegEnableSet",
                                                 devNum, portNum, 
                                                 port_speed_autonegotiation, 
                                                 "state", "GT_BOOL")
            if     0x10 == result then
                command_data:setFailPortStatus() 
                command_data:addWarning("It is not allowed to %s speed " ..
                                        "auto-negatiation of device %d port %d.", 
                                        boolPredicatStrGet(port_speed_autonegotiation), 
                                        devNum, portNum)
            elseif 0 ~= result then 
                command_data:setFailPortAndLocalStatus()
                command_data:addError("Error at speed auto-negatiation setting " ..
                                      "of device %d port %d: %s", devNum, portNum, 
                                      returnCodes[result])
            end              
               
            -- Port speed setting.      
            if LION_DEVFAMILY ~= DeviceFamily then            
                if (true == command_data["local_status"]) and
                   (true == port_speed_changing) then
                    result, values = cpssPerPortParamSet("cpssDxChPortSpeedSet",
                                                         devNum, portNum, 
                                                         port_speed, "speed", 
                                                         "CPSS_PORT_SPEED_ENT") 
                    if  0x10 == result then
                        command_data:setFailPortStatus() 
                        command_data:addWarning("It is not allowed to set speed " ..
                                                "%s of device %d port %d.", 
                                                speedStrGet(port_speed), devNum, 
                                                portNum)
                    elseif 0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error at speed setting of device " ..
                                              "%d port %d: %s", devNum, portNum, 
                                              returnCodes[result])
                    end          
                end
                
                command_data:updateStatus()
            else
                -- Port interface mode getting.
                if (true == command_data["local_status"]) and 
                   (true == port_speed_changing)          and
                   (true == port_interface_mode_changing) then
                    result, values = cpssPerPortParamGet("cpssDxChPortInterfaceModeGet",
                                                         devNum, portNum, "mode", 
                                                         "CPSS_PORT_INTERFACE_MODE_ENT")
                    if 0 == result then 
                        port_interface_mode = values["mode"]
                    else
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error at interface mode getting " ..
                                              "of device %d port %d: %s", devNum, 
                                              portNum, returnCodes[result])
                    end
                end   

                -- Port speed and interface mode setting.               
                if (true == command_data["local_status"]) and 
                   (true == port_speed_changing)          and
                   (true == port_interface_mode_changing) then
                    result, values = 
                        myGenWrapper("gtAppDemoLionPortModeSpeedSet", {
                                        { "IN", "GT_U8",  "dev",   devNum}, 
                                        { "IN", "GT_U8",  "port",  portNum},
                                        { "IN", "CPSS_PORT_INTERFACE_MODE_ENT", 
                                                   "mode",  port_interface_mode},
                                        { "IN", "CPSS_PORT_SPEED_ENT", 
                                                            "speed", port_speed}
                                     })               
                        if  0x10 == result then
                            command_data:setFailPortStatus() 
                            command_data:addWarning("It is not allowed to set " ..
                                                    "speed %s at interface mode " ..
                                                    "%s for device %d port %d.", 
                                                    speedStrGet(port_speed),
                                                    port_interface_mode, devNum, 
                                                    portNum)                                                   
                        elseif 0 ~= result then
                            command_data:setFailPortAndLocalStatus()
                            command_data:addError("Error at speed setting of " ..
                                                  "device %d port %d: %s", 
                                                  devNum, portNum, 
                                                  returnCodes[result])
                        end
                end
                
                command_data:updateStatus()                
            end            
                
            -- Port duplex autonegatiation mode setting.
            command_data:clearLocalStatus()
                 
            result, values = cpssPerPortParamSet("cpssDxChPortDuplexAutoNegEnableSet",
                                                 devNum, portNum, 
                                                 port_duplex_autonegotiation, 
                                                 "state", "GT_BOOL")     
            if     0x10 == result then
                command_data:setFailPortStatus() 
                command_data:addWarning("It is not allowed to %s duplex " ..
                                        "auto-negatiation of device %d port %d.", 
                                        boolPredicatStrGet(port_speed_autonegotiation), 
                                        devNum, portNum)                                                 
            elseif 0 ~= result then 
                command_data:setFailPortAndLocalStatus()
                command_data:addError("Error at duplex auto-negatiation setting " ..
                                      "of device %d port %d: %s", devNum, portNum, 
                                      returnCodes[result])
            end      

            -- Port duplex setting.
            if (true == command_data["local_status"]) and
               (true == port_duplex_changing) then
                result, values = cpssPerPortParamSet("cpssDxChPortDuplexModeSet",
                                                     devNum, portNum, 
                                                     port_duplex, "dMode", 
                                                     "CPSS_PORT_DUPLEX_ENT")                                                   
                if  0x10 == result then
                    command_data:setFailPortStatus()   
                    command_data:addWarning("It is not allowed to set %s duplex " ..
                                            "of device %d port %d.", 
                                            ENUM["CPSS_PORT_DUPLEX_ENT"][port_duplex], 
                                            devNum, portNum)  
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at duplex setting of device " ..
                                          "%d port %d: ", devNum, portNum, 
                                          returnCodes[result])
                end
            end

            command_data:updateStatus()
               
            command_data:updatePorts()                   
        end   
        
        command_data:addWarningIfNoSuccessPorts("Can not set speed and/or duplex " ..
                                                "for all processed ports.")
    end    
    
    -- Command data postprocessing
    command_data:analyzeCommandExecution()
     
    command_data:printCommandExecutionResults()
           
    return command_data:getCommandExecutionResults()  
end


--------------------------------------------
-- command registration: negotiation
--------------------------------------------
CLI_addCommand("interface", "negotiation", {
  func=negotiation_func,
  help="Enable auto negotiation",
  params={
    { type="named",
      { format="10h",      name="port_speed", help="Advertise  10 Mbps Half duplex"},
      { format="10f",      name="port_speed", help="Advertise  10 Mbps Full duplex"},
      { format="100h",     name="port_speed", help="Advertise 100 Mbps Half duplex"},
      { format="100f",     name="port_speed", help="Advertise 100 Mbps Full duplex"},
      { format="1000f",    name="port_speed", help="Advertise   1 Gbps Full duplex"},
      { format="10000f",   name="port_speed", help="Advertise  10 Gbps Full duplex"}
    }
  }
})
CLI_addCommand("interface", "no negotiation", {
  func=function(params)
      params.flagNo=true
      return negotiation_func(params)
  end,
  help="Disable auto negotiation"
})
