--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_bridge_multicast_unregistered.lua
--*
--* DESCRIPTION:
--*       showing of the unregistered multicast command configuration 
--*
--* FILE REVISION NUMBER:
--*       $Revision: 5 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  show_bridge_multicast_unregistered_func
--        @description  show's the unregistered multicast command configuration  
--
--        @param params         - params["all_device"]: all or given 
--                                devices iterating property, could be 
--                                irrelevant
--                                params["ethernet"]: interface range, 
--                                could be irrelevant;
--                                params["port-channel"]: trunk id,could be 
--                                irrelevant
--
--        @return       true on success, otherwise false and error message
--
local function show_bridge_multicast_unregistered_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum, devNumTemp, trunkId
    local command_data = Command_Data()
    -- Command specific variables declaration
    local ports_filtering_showing, port_channel_filtering_showing
    local filtering_enabling, filtering_enabling_string
    local header_string, footer_string    
    
    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllDeviceRange(params)       
    command_data:initDevPortRange(params)
    
    -- Command specific variables initialization 
    ports_filtering_showing         = getTrueIfNil(params["port-channel"])
    port_channel_filtering_showing  = getTrueIfNil(params["ethernet"])
    header_string       = 
        "\n" ..
        " Interface   Unregistered\n" ..
        "-----------  ------------\n"
    footer_string       = "\n"     
    
    -- System specific data initialization.
    command_data:enablePausedPrinting()
    command_data:setEmergencyPrintingHeaderAndFooter(header_string, 
                                                     footer_string)       
    
    -- Main port handling cycle
    if (true == command_data["status"])         and 
       (true == ports_filtering_showing)        then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
            
            command_data:clearLocalStatus()
            
            -- Get unregistered multicast filtering enable on port.
            if true == command_data["local_status"] then
                result, values = 
                    cpssPerPortParamGet("cpssDxChBrgPortEgrFltUregMcastEnableGet",
                                        devNum, portNum, "enable", "GT_BOOL")  
                if        0 == result then    
                    filtering_enabling = values["enable"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus() 
                    command_data:addWarning("It is not allowed to get egress " ..
                                            "Filtering enabling for bridged " ..
                                            "Unregistered Multicast packets " ..
                                            "on device %d port %d.", devNum, 
                                            portNum)                                                   
                elseif    0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at getting of egress Filtering " ..
                                          "enabling for bridged Unregistered " ..
                                          "Multicast packets on device %d " ..
                                          "port %d: %s.", devNum, portNum, 
                                          returnCodes[result])
                end 
                
                if 0 == result then 
                    filtering_enabling_string = 
                        boolForwardDropGet(filtering_enabling)
                else
                    filtering_enabling_string = "n/a"
                end
            end               

            command_data:updateStatus()            

            -- Resulting string formatting and adding.
            command_data["result"] = 
                string.format("%-13s%-15s", 
                              alignLeftToCenterStr(tostring(devNum) .. "/" .. 
                                                   tostring(portNum), 11), 
                              alignLeftToCenterStr(filtering_enabling_string, 
                                                   13))                                                       
            command_data:addResultToResultArray()
            
            command_data:updatePorts()
        end
    end
    
    -- Main port-channel handling cycle
    if (true == command_data["status"])         and 
       (true == port_channel_filtering_showing) then
        local iterator
                trunkId = params[params["interface_port_channel"]]

        for iterator, devNum in command_data:getDevicesIterator() do

            command_data:clearDeviceStatus()
            command_data:clearLocalStatus()

            -- Get a port-channel port.
            if true == command_data["local_status"]     then
                local port_found = false
                local trunk_dev_ports = {}
                result, values = get_trunk_device_port_list(trunkId)
                if 0 == result then
                    trunk_dev_ports = values
                    for devNumTemp, portNums in pairs(trunk_dev_ports) do
                        if devNumTemp == devNum then
                                portNum = portNums[1]
                                port_found = true
                                break
                        end
                    end
                elseif 0x10 == result then
                        command_data:setFailStatus()
                        command_data:addWarning("Getting of %d trunk information is not " ..
                                                                        "allowed: %s.", trunkId, values)
                elseif 0 ~= result then
                        command_data:setFailStatus()
                        command_data:addError("Error at getting of %d trunk information: %s.",
                                                                  trunkId, values)
                end

                if false == port_found then
                        command_data:setFailStatus()
                        command_data:addWarning("No local ports in %d trunk.", trunkId)
                end

                -- Get the drop value for one of trunk's ports
                result, values = 
                    cpssPerPortParamGet("cpssDxChBrgPortEgrFltUregMcastEnableGet",
                                        devNum, portNum, "enable", "GT_BOOL")  
                if        0 == result then    
                    filtering_enabling = values["enable"]
                elseif 0x10 == result then
                    command_data:setFailTrunkStatus() 
                    command_data:addWarning("It is not allowed to get egress " ..
                                            "Filtering enabling for bridged " ..
                                            "Unregistered Multicast packets " ..
                                            "on device %d port %d.", devNum, 
                                            portNum) 
                elseif    0 ~= result then
                    command_data:setFailTrunkAndLocalStatus()
                    command_data:addError("Error at getting of egress Filtering " ..
                                          "enabling for bridged Unregistered " ..
                                          "Multicast packets on device %d " ..
                                          "port %d: %s.", devNum, portNum, 
                                          returnCodes[result])
                end 
                
                if 0 == result then 
                    filtering_enabling_string = 
                        boolForwardDropGet(filtering_enabling)
                else
                    filtering_enabling_string = "n/a"
                end
            end                   
            
            command_data:updateStatus()            

            -- Resulting string formatting and adding.
            command_data["result"] = 
                string.format("%-13s%-15s", 
                              alignLeftToCenterStr("Ch " .. tostring(trunkId), 
                                                   11), 
                              alignLeftToCenterStr(filtering_enabling_string, 
                                                   13))
            command_data:addResultToResultArray()
            
            command_data:updateTrunks()
        end
    end           
    
    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr()
    command_data:setResultStrOnPortsAndTrunksCount(
        header_string, command_data["result"], footer_string,
        "There is no uregistered multicast information to show.\n")
        
    command_data:analyzeCommandExecution()
       
    command_data:printCommandExecutionResults()
    
        return command_data:getCommandExecutionResults() 
end


--------------------------------------------------------------------------------
-- command registration: show bridge multicast unregistered
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show bridge multicast unregistered", {
  func   = show_bridge_multicast_unregistered_func,
  help   = "Whether an egress port is to forward or filter unregistered " ..
           "multicast frames",
  params = {
      { type = "named",
          "#all_device",      
          "#interface_port_channel",          
        mandatory = { "all_device" }
      }
  }
})